#define TRACE_NAME "DTC"
#include "trace.h"

#define TLVL_GetData 10
#define TLVL_GetJSONData 11
#define TLVL_ReadBuffer 12
#define TLVL_ReadNextDAQPacket 13
#define TLVL_ReadNextDCSPacket 14
#define TLVL_SendDCSRequestPacket 15
#define TLVL_SendReadoutRequestPacket 16
#define TLVL_VerifySimFileInDTC 17
#define TLVL_VerifySimFileInDTC2 18
#define TLVL_VerifySimFileInDTC3 19
#define TLVL_WriteSimFileToDTC 20
#define TLVL_WriteSimFileToDTC2 21
#define TLVL_WriteSimFileToDTC3 22
#define TLVL_WriteDetectorEmulatorData 23
#define TLVL_WriteDataPacket 24



#include "DTC.h"

#include <sstream> // Convert uint to hex string


#include <iostream>
#include <fstream>

#include <unistd.h>

DTCLib::DTC::DTC(DTC_SimMode mode, int dtc, unsigned rocMask, std::string expectedDesignVersion) : DTC_Registers(mode, dtc, rocMask, expectedDesignVersion),
daqbuffer_(), dcsbuffer_(), lastDAQBufferActive_(false), lastDCSBufferActive_(false),
bufferIndex_(0), first_read_(true), daqDMAByteCount_(0), dcsDMAByteCount_(0),
lastReadPtr_(nullptr), nextReadPtr_(nullptr), dcsReadPtr_(nullptr)
{
	//ELF, 05/18/2016: Rick reports that 3.125 Gbp
	//SetSERDESOscillatorClock(DTC_SerdesClockSpeed_25Gbps); // We're going to 2.5Gbps for now
	TLOG(TLVL_INFO) << "CONSTRUCTOR";
}

DTCLib::DTC::~DTC()
{
	TLOG(TLVL_INFO) << "DESTRUCTOR";
	ReleaseAllBuffers();
	daqbuffer_.clear();
	dcsbuffer_.clear();
	lastReadPtr_ = nullptr;
	nextReadPtr_ = nullptr;
	dcsReadPtr_ = nullptr;
}

//
// DMA Functions
//
std::vector<DTCLib::DTC_DataBlock> DTCLib::DTC::GetData(DTC_Timestamp when)
{
	TLOG(TLVL_GetData) << "GetData begin";
	std::vector<DTC_DataBlock> output;

	first_read_ = true;
	TLOG(TLVL_GetData) << "GetData: Releasing " << daqbuffer_.size() + (lastDAQBufferActive_ ? -1 : 0) << " buffers";
	device_.read_release(DTC_DMA_Engine_DAQ, static_cast<unsigned>(daqbuffer_.size() + (lastDAQBufferActive_ ? -1 : 0)));

	mu2e_databuff_t* last = daqbuffer_.back();
	daqbuffer_.clear();
	if (lastDAQBufferActive_)
	{
		daqbuffer_.push_back(last);
	}
	last = nullptr;
	lastDAQBufferActive_ = false;

	DTC_DataHeaderPacket* packet = nullptr;

	try
	{
		// Read the header packet
		auto tries = 0;
		size_t sz;
		while (packet == nullptr && tries < 3)
		{
			TLOG(TLVL_GetData) << "GetData before ReadNextDAQPacket, tries=" << tries;
			packet = ReadNextDAQPacket(first_read_ ? 100 : 1);
			if (packet != nullptr)
			{
				TLOG(TLVL_GetData) << "GetData after ReadDMADAQPacket, ts=0x" << std::hex << packet->GetTimestamp().GetTimestamp(true);
			}
			tries++;
			//if (packet == nullptr) usleep(5000);
		}
		if (packet == nullptr)
		{
			TLOG(TLVL_GetData) << "GetData: Timeout Occurred! (Lead packet is nullptr after retries)";
			return output;
		}

		if (packet->GetTimestamp() != when && when.GetTimestamp(true) != 0)
		{
			TLOG(TLVL_ERROR) << "GetData: Error: Lead packet has wrong timestamp! 0x" << std::hex << when.GetTimestamp(true) << "(expected) != 0x" << std::hex << packet->GetTimestamp().GetTimestamp(true);
			delete packet;
			lastDAQBufferActive_ = true;
			return output;
		}

		sz = packet->GetByteCount();
		when = packet->GetTimestamp();

		delete packet;
		packet = nullptr;

		TLOG(TLVL_GetData) << "GetData: Adding pointer " << (void*)lastReadPtr_ << " to the list (first)";
		output.push_back(DTC_DataBlock(reinterpret_cast<DTC_DataBlock::pointer_t*>(lastReadPtr_), sz));

		auto done = false;
		while (!done)
		{
			size_t sz2 = 0;
			TLOG(TLVL_GetData) << "GetData: Reading next DAQ Packet";
			packet = ReadNextDAQPacket();
			if (packet == nullptr) // End of Data
			{
				TLOG(TLVL_GetData) << "GetData: Next packet is nullptr; we're done";
				done = true;
				nextReadPtr_ = nullptr;
			}
			else if (packet->GetTimestamp() != when)
			{
				TLOG(TLVL_GetData) << "GetData: Next packet has ts=0x" << std::hex << packet->GetTimestamp().GetTimestamp(true) << ", not 0x" << std::hex << when.GetTimestamp(true) << "; we're done";
				done = true;
				nextReadPtr_ = lastReadPtr_;
				lastDAQBufferActive_ = true;
			}
			else
			{
				TLOG(TLVL_GetData) << "GetData: Next packet has same ts=0x" << std::hex << packet->GetTimestamp().GetTimestamp(true)
					<< ", continuing (bc=0x" << std::hex << packet->GetByteCount() << ")";
				sz2 = packet->GetByteCount();
			}
			delete packet;
			packet = nullptr;

			if (!done)
			{
				TLOG(TLVL_GetData) << "GetData: Adding pointer " << (void*)lastReadPtr_ << " to the list";
				output.push_back(DTC_DataBlock(reinterpret_cast<DTC_DataBlock::pointer_t*>(lastReadPtr_), sz2));
			}
		}
	}
	catch (DTC_WrongPacketTypeException& ex)
	{
		TLOG(TLVL_WARNING) << "GetData: Bad omen: Wrong packet type at the current read position";
		nextReadPtr_ = nullptr;
	}
	catch (DTC_IOErrorException& ex)
	{
		nextReadPtr_ = nullptr;
		TLOG(TLVL_WARNING) << "GetData: IO Exception Occurred!";
	}
	catch (DTC_DataCorruptionException& ex)
	{
		nextReadPtr_ = nullptr;
		TLOG(TLVL_WARNING) << "GetData: Data Corruption Exception Occurred!";
	}

	TLOG(TLVL_GetData) << "GetData RETURN";
	delete packet;
	return output;
} // GetData

std::string DTCLib::DTC::GetJSONData(DTC_Timestamp when)
{
	TLOG(TLVL_GetJSONData) << "GetJSONData BEGIN";
	std::stringstream ss;
	TLOG(TLVL_GetJSONData) << "GetJSONData before call to GetData";
	auto data = GetData(when);
	TLOG(TLVL_GetJSONData) << "GetJSONData after call to GetData, data size " << data.size();

	for (size_t i = 0; i < data.size(); ++i)
	{
		TLOG(TLVL_GetJSONData) << "GetJSONData constructing DataPacket:";
		auto test = DTC_DataPacket(data[i].blockPointer);
		TLOG(TLVL_GetJSONData) << test.toJSON();
		TLOG(TLVL_GetJSONData) << "GetJSONData constructing DataHeaderPacket";
		auto theHeader = DTC_DataHeaderPacket(test);
		ss << "DataBlock: {";
		ss << theHeader.toJSON() << ",";
		ss << "DataPackets: [";
		for (auto packet = 0; packet < theHeader.GetPacketCount(); ++packet)
		{
			auto packetPtr = static_cast<void*>(reinterpret_cast<char*>(data[i].blockPointer) + 16 * (1 + packet));
			ss << DTC_DataPacket(packetPtr).toJSON() << ",";
		}
		ss << "]";
		ss << "}";
		if (i + 1 < data.size())
		{
			ss << ",";
		}
	}

	TLOG(TLVL_GetJSONData) << "GetJSONData RETURN";
	return ss.str();
}

void DTCLib::DTC::WriteSimFileToDTC(std::string file, bool /*goForever*/, bool overwriteEnvironment, std::string outputFileName, bool skipVerify)
{
	bool success = false;
	int retryCount = 0;
	while (!success && retryCount < 5)
	{

		TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC BEGIN";
		auto writeOutput = outputFileName != "";
		std::ofstream outputStream;
		if (writeOutput)
		{
			outputStream.open(outputFileName, std::ios::out | std::ios::binary);
		}
		auto sim = getenv("DTCLIB_SIM_FILE");
		if (!overwriteEnvironment && sim != nullptr)
		{
			file = std::string(sim);
		}


		TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC file is " << file << ", Setting up DTC";
		DisableDetectorEmulator();
		DisableDetectorEmulatorMode();
		//ResetDDR();  // this can take about a second
		ResetDDRWriteAddress();
		ResetDDRReadAddress();
		SetDDRDataLocalStartAddress(0x0);
		SetDDRDataLocalEndAddress(0xFFFFFFFF);
		EnableDetectorEmulatorMode();
		SetDetectorEmulationDMACount(1);
		SetDetectorEmulationDMADelayCount(250); // 1 microseconds
		uint64_t totalSize = 0;
		auto n = 0;

		auto sizeCheck = true;
		TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC Opening file";
		std::ifstream is(file, std::ifstream::binary);
		TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC Reading file";
		while (is && is.good() && sizeCheck)
		{
			TLOG(TLVL_WriteSimFileToDTC2) << "WriteSimFileToDTC Reading a DMA from file..." << file;
			auto buf = reinterpret_cast<mu2e_databuff_t*>(new char[0x10000]);
			is.read(reinterpret_cast<char*>(buf), sizeof(uint64_t));
			if (is.eof())
			{
				TLOG(TLVL_WriteSimFileToDTC2) << "WriteSimFileToDTC End of file reached.";
				delete[] buf;
				break;
			}
			auto sz = *reinterpret_cast<uint64_t*>(buf);
			//TLOG(5) << "Size is " << << ", writing to device", (long long unsigned)sz);
			is.read(reinterpret_cast<char*>(buf) + 8, sz - sizeof(uint64_t));
			if (sz < 80 && sz > 0)
			{
				auto oldSize = sz;
				sz = 80;
				memcpy(buf, &sz, sizeof(uint64_t));
				uint64_t sixtyFour = 64;
				memcpy(reinterpret_cast<uint64_t*>(buf) + 1, &sixtyFour, sizeof(uint64_t));
				bzero(reinterpret_cast<uint64_t*>(buf) + 2, sz - oldSize);
			}
			//is.read((char*)buf + 8, sz - sizeof(uint64_t));
			if (sz > 0 && (sz + totalSize < 0xFFFFFFFF || simMode_ == DTC_SimMode_LargeFile))
			{
				TLOG(TLVL_WriteSimFileToDTC2) << "WriteSimFileToDTC Size is " << sz << ", writing to device";
				if (writeOutput)
				{
					TLOG(TLVL_WriteSimFileToDTC3) << "WriteSimFileToDTC: Stripping off DMA header words and writing to binary file";
					outputStream.write(reinterpret_cast<char*>(buf) + 16, sz - 16);
				}

				auto exclusiveByteCount = *(reinterpret_cast<uint64_t*>(buf) + 1);
				TLOG(TLVL_WriteSimFileToDTC3) << "WriteSimFileToDTC: Inclusive byte count: " << sz << ", Exclusive byte count: " << exclusiveByteCount;
				if (sz - 16 != exclusiveByteCount)
				{
					TLOG(TLVL_ERROR) << "WriteSimFileToDTC: ERROR: Inclusive Byte count " << sz << " is inconsistent with exclusive byte count " << exclusiveByteCount
						<< " for DMA at 0x" << std::hex << totalSize << " (" << sz - 16 << " != " << exclusiveByteCount << ")";
					sizeCheck = false;
				}

				totalSize += sz - 8;
				n++;
				TLOG(TLVL_WriteSimFileToDTC3) << "WriteSimFileToDTC: totalSize is now " << totalSize << ", n is now " << n;
				WriteDetectorEmulatorData(buf, static_cast<size_t>(sz));
			}
			else if (sz > 0)
			{
				TLOG(TLVL_WriteSimFileToDTC2) << "WriteSimFileToDTC DTC memory is now full. Closing file.";
				sizeCheck = false;
			}
			delete[] buf;
		}

		TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC Closing file. sizecheck=" << sizeCheck << ", eof=" << is.eof() << ", fail=" << is.fail() << ", bad=" << is.bad();
		is.close();
		if (writeOutput) outputStream.close();
		SetDDRDataLocalEndAddress(static_cast<uint32_t>(totalSize - 1));
		success = skipVerify || VerifySimFileInDTC(file, outputFileName);
		retryCount++;
	}

	if (retryCount == 5)
	{
		TLOG(TLVL_ERROR) << "WriteSimFileToDTC FAILED after 5 attempts! ABORTING!";
		exit(4);
	}
	else
	{
		TLOG(TLVL_INFO) << "WriteSimFileToDTC Took " << retryCount << " attempts to write file";
	}

	SetDetectorEmulatorInUse();
	TLOG(TLVL_WriteSimFileToDTC) << "WriteSimFileToDTC END";
}

bool DTCLib::DTC::VerifySimFileInDTC(std::string file, std::string rawOutputFilename)
{
	uint64_t totalSize = 0;
	auto n = 0;
	auto sizeCheck = true;

	auto writeOutput = rawOutputFilename != "";
	std::ofstream outputStream;
	if (writeOutput)
	{
		outputStream.open(rawOutputFilename + ".verify", std::ios::out | std::ios::binary);
	}

	auto sim = getenv("DTCLIB_SIM_FILE");
	if (file.size() == 0 && sim != nullptr)
	{
		file = std::string(sim);
	}

	ResetDDRReadAddress();
	TLOG(TLVL_VerifySimFileInDTC) << "VerifySimFileInDTC Opening file";
	std::ifstream is(file, std::ifstream::binary);
	if (!is || !is.good())
	{
		TLOG(TLVL_ERROR) << "VerifySimFileInDTC Failed to open file " << file << "!";
	}

	TLOG(TLVL_VerifySimFileInDTC) << "VerifySimFileInDTC Reading file";
	while (is && is.good() && sizeCheck)
	{
		TLOG(TLVL_VerifySimFileInDTC2) << "VerifySimFileInDTC Reading a DMA from file..." << file;
		auto buf = reinterpret_cast<mu2e_databuff_t*>(new char[0x10000]);
		is.read(reinterpret_cast<char*>(buf), sizeof(uint64_t));
		if (is.eof())
		{
			TLOG(TLVL_VerifySimFileInDTC2) << "VerifySimFileInDTC End of file reached.";
			delete[] buf;
			break;
		}
		auto sz = *reinterpret_cast<uint64_t*>(buf);
		//TLOG(5) << "Size is " << << ", writing to device", (long long unsigned)sz);
		is.read(reinterpret_cast<char*>(buf) + 8, sz - sizeof(uint64_t));
		if (sz < 80 && sz > 0)
		{
			auto oldSize = sz;
			sz = 80;
			memcpy(buf, &sz, sizeof(uint64_t));
			uint64_t sixtyFour = 64;
			memcpy(reinterpret_cast<uint64_t*>(buf) + 1, &sixtyFour, sizeof(uint64_t));
			bzero(reinterpret_cast<uint64_t*>(buf) + 2, sz - oldSize);
		}

		if (sz > 0 && (sz + totalSize < 0xFFFFFFFF || simMode_ == DTC_SimMode_LargeFile))
		{
			TLOG(TLVL_VerifySimFileInDTC2) << "VerifySimFileInDTC Expected Size is " << sz << ", reading from device";
			auto exclusiveByteCount = *(reinterpret_cast<uint64_t*>(buf) + 1);
			TLOG(TLVL_VerifySimFileInDTC3) << "VerifySimFileInDTC: Inclusive byte count: " << sz << ", Exclusive byte count: " << exclusiveByteCount;
			if (sz - 16 != exclusiveByteCount)
			{
				TLOG(TLVL_ERROR) << "VerifySimFileInDTC: ERROR: Inclusive Byte count " << sz << " is inconsistent with exclusive byte count " << exclusiveByteCount
					<< " for DMA at 0x" << std::hex << totalSize << " (" << sz - 16 << " != " << exclusiveByteCount << ")";
				sizeCheck = false;
			}

			totalSize += sz;
			n++;
			TLOG(TLVL_VerifySimFileInDTC3) << "VerifySimFileInDTC: totalSize is now " << totalSize << ", n is now " << n;
			//WriteDetectorEmulatorData(buf, static_cast<size_t>(sz));
			DisableDetectorEmulator();
			SetDetectorEmulationDMACount(1);
			EnableDetectorEmulator();

			mu2e_databuff_t* buffer;
			auto tmo_ms = 1500;
			TLOG(TLVL_VerifySimFileInDTC) << "VerifySimFileInDTC - before read for DAQ ";
			auto sts = device_.read_data(DTC_DMA_Engine_DAQ, reinterpret_cast<void**>(&buffer), tmo_ms);
			if (writeOutput && sts > 8)
			{
				TLOG(TLVL_VerifySimFileInDTC3) << "VerifySimFileInDTC: Writing to binary file";
				outputStream.write(reinterpret_cast<char*>(*buffer + 8), sts - 8);
			}
			size_t readSz = *(reinterpret_cast<uint64_t*>(buffer));
			TLOG(TLVL_VerifySimFileInDTC) << "VerifySimFileInDTC - after read, sz=" << sz << " sts=" << sts << " rdSz=" << readSz;

			// DMA engine strips off leading 64-bit word
			TLOG(6) << "VerifySimFileInDTC - Checking buffer size";
			if (static_cast<size_t>(sts) != sz - sizeof(uint64_t))
			{
				TLOG(TLVL_ERROR) << "VerifySimFileInDTC Buffer " << n << " has size 0x" << std::hex << sts
					<< " but the input file has size 0x" << std::hex << sz - sizeof(uint64_t) << " for that buffer!";

				device_.read_release(DTC_DMA_Engine_DAQ, 1);
				delete[] buf;
				is.close();
				if (writeOutput) outputStream.close();
				return false;
			}

			TLOG(TLVL_VerifySimFileInDTC2) << "VerifySimFileInDTC - Checking buffer contents";
			size_t cnt = sts % sizeof(uint64_t) == 0 ? sts / sizeof(uint64_t) : 1 + (sts / sizeof(uint64_t));

			for (size_t ii = 0; ii < cnt; ++ii)
			{
				auto l = *(reinterpret_cast<uint64_t*>(*buffer) + ii);
				auto r = *(reinterpret_cast<uint64_t*>(*buf) + ii + 1);
				if (l != r)
				{
					size_t address = totalSize - sz + ((ii + 1) * sizeof(uint64_t));
					TLOG(TLVL_ERROR) << "VerifySimFileInDTC Buffer " << n << " word " << ii << " (Address in file 0x" << std::hex << address << "):"
						<< " Expected 0x" << std::hex << r << ", but got 0x" << std::hex << l << ". Returning False!";
					delete[] buf;
					is.close();
					if (writeOutput) outputStream.close();
					device_.read_release(DTC_DMA_Engine_DAQ, 1);
					return false;
				}
			}
			device_.read_release(DTC_DMA_Engine_DAQ, 1);
		}
		else if (sz > 0)
		{
			TLOG(TLVL_VerifySimFileInDTC2) << "VerifySimFileInDTC DTC memory is now full. Closing file.";
			sizeCheck = false;
		}
		delete[] buf;
	}

	TLOG(TLVL_VerifySimFileInDTC) << "VerifySimFileInDTC Closing file. sizecheck=" << sizeCheck << ", eof=" << is.eof() << ", fail=" << is.fail() << ", bad=" << is.bad();
	TLOG(TLVL_INFO) << "VerifySimFileInDTC: The Detector Emulation file was written correctly";
	is.close();
	if (writeOutput) outputStream.close();
	return true;
}

// ROC Register Functions
uint16_t DTCLib::DTC::ReadROCRegister(const DTC_Link_ID& link, const uint8_t address)
{
	SendDCSRequestPacket(link, DTC_DCSOperationType_Read, address);
	bool done = false;
	uint16_t data = 0;
	while (!done) {
		auto reply = ReadNextDCSPacket();
		if (reply != nullptr)
		{
			TLOG(TLVL_TRACE) << "Got packet, link=" << static_cast<int>(reply->GetRingID()) << ", address=" << static_cast<int>(reply->GetAddress()) << ", data=" << static_cast<int>(reply->GetData());

			auto datatmp = reply->GetData();
			delete reply;
			reply = nullptr;
			if (reply->GetAddress() != address || reply->GetRingID() != link) continue;

			data = datatmp;
			done = true;

		}
	}
	return data;
}

void DTCLib::DTC::WriteROCRegister(const DTC_Link_ID& link, const uint8_t address, const uint16_t data)
{
	SendDCSRequestPacket(link, DTC_DCSOperationType_Write, address, data);
}

uint16_t DTCLib::DTC::ReadExtROCRegister(const DTC_Link_ID& link, const uint8_t block, const uint16_t address)
{
	uint16_t addressT = address & 0x7FFF;
	WriteROCRegister(link, 12, block);
	WriteROCRegister(link, 13, addressT);
	WriteROCRegister(link, 13, addressT | 0x8000);
	return ReadROCRegister(link, 22);
}

void DTCLib::DTC::WriteExtROCRegister(const DTC_Link_ID& link, const uint8_t block, const uint8_t address, const uint16_t data)
{
	uint16_t dataT = data & 0x7FFF;
	WriteROCRegister(link, 12, block + (address << 8));
	WriteROCRegister(link, 13, dataT);
	WriteROCRegister(link, 13, dataT | 0x8000);
}

std::string DTCLib::DTC::ROCRegDump(const DTC_Link_ID& link)
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);
	o << "{";
	o << "\"Forward Detector 0 Status\": " << ReadExtROCRegister(link, 8, 0) << ",\n";
	o << "\"Forward Detector 1 Status\": " << ReadExtROCRegister(link, 9, 0) << ",\n";
	o << "\"Command Handler Status\": " << ReadExtROCRegister(link, 10, 0) << ",\n";
	o << "\"Packet Sender 0 Status\": " << ReadExtROCRegister(link, 11, 0) << ",\n";
	o << "\"Packet Sender 1 Status\": " << ReadExtROCRegister(link, 12, 0) << ",\n";
	o << "\"Forward Detector 0 Errors\": " << ReadExtROCRegister(link, 8, 1) << ",\n";
	o << "\"Forward Detector 1 Errors\": " << ReadExtROCRegister(link, 9, 1) << ",\n";
	o << "\"Command Handler Errors\": " << ReadExtROCRegister(link, 10, 1) << ",\n";
	o << "\"Packet Sender 0 Errors\": " << ReadExtROCRegister(link, 11, 1) << ",\n";
	o << "\"Packet Sender 1 Errors\": " << ReadExtROCRegister(link, 12, 1) << "\n";
	o << "}";

	return o.str();
}

void DTCLib::DTC::SendReadoutRequestPacket(const DTC_Link_ID& link, const DTC_Timestamp& when, bool quiet)
{
	DTC_HeartbeatPacket req(link, when);
	TLOG(TLVL_SendReadoutRequestPacket) << "SendReadoutRequestPacket before WriteDMADAQPacket - DTC_HeartbeatPacket";
	if (!quiet) std::cout << req.toJSON() << std::endl;
	WriteDMAPacket(req);
	TLOG(TLVL_SendReadoutRequestPacket) << "SendReadoutRequestPacket after  WriteDMADAQPacket - DTC_HeartbeatPacket";
}

void DTCLib::DTC::SendDCSRequestPacket(const DTC_Link_ID& link, const DTC_DCSOperationType type, const uint8_t address, const uint16_t data, bool quiet)
{
	DTC_DCSRequestPacket req(link,  type, address, data);
	TLOG(TLVL_SendDCSRequestPacket) << "SendDCSRequestPacket before WriteDMADCSPacket - DTC_DCSRequestPacket";
	if (!quiet) std::cout << req.toJSON() << std::endl;

	if (!ReadDCSReception()) EnableDCSReception();

	WriteDMAPacket(req);
	TLOG(TLVL_SendDCSRequestPacket) << "SendDCSRequestPacket after  WriteDMADCSPacket - DTC_DCSRequestPacket";
}

DTCLib::DTC_DataHeaderPacket* DTCLib::DTC::ReadNextDAQPacket(int tmo_ms)
{
	TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket BEGIN";
	if (nextReadPtr_ != nullptr)
	{
		TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket BEFORE BUFFER CHECK nextReadPtr_=" << (void*)nextReadPtr_ << " *nextReadPtr_=0x" << std::hex << *(uint16_t*)nextReadPtr_;
	}
	else
	{
		TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket BEFORE BUFFER CHECK nextReadPtr_=nullptr";
	}
	auto newBuffer = false;
	// Check if the nextReadPtr has been initialized, and if its pointing to a valid location
	if (nextReadPtr_ == nullptr
		|| nextReadPtr_ >= reinterpret_cast<uint8_t*>(daqbuffer_.back()) + sizeof(mu2e_databuff_t)
		|| nextReadPtr_ >= reinterpret_cast<uint8_t*>(daqbuffer_.back()) + daqDMAByteCount_
		|| *static_cast<uint16_t*>(nextReadPtr_) == 0)
	{
		newBuffer = true;
		if (first_read_)
		{
			lastReadPtr_ = nullptr;
		}
		TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket Obtaining new DAQ Buffer";
		void* oldBufferPtr = &daqbuffer_.back()[0];
		auto sts = ReadBuffer(DTC_DMA_Engine_DAQ, tmo_ms); // does return code
		if (sts <= 0)
		{
			TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket: ReadBuffer returned " << sts << ", returning nullptr";
			return nullptr;
		}
		// MUST BE ABLE TO HANDLE daqbuffer_==nullptr OR retry forever?
		nextReadPtr_ = &daqbuffer_.back()[0];
		TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket nextReadPtr_=" << (void*)nextReadPtr_ << " *nextReadPtr_=0x" << std::hex << *(unsigned*)nextReadPtr_ << " lastReadPtr_=" << (void*)lastReadPtr_;
		void* bufferIndexPointer = static_cast<uint8_t*>(nextReadPtr_) + 2;
		if (nextReadPtr_ == oldBufferPtr && bufferIndex_ == *static_cast<uint32_t*>(bufferIndexPointer))
		{
			TLOG(TLVL_ReadNextDAQPacket) << "New buffer is the same as old. Releasing buffer and retrying";
			nextReadPtr_ = nullptr;
			//We didn't actually get a new buffer...this probably means there's no more data
			//Try and see if we're merely stuck...hopefully, all the data is out of the buffers...
			device_.read_release(DTC_DMA_Engine_DAQ, 1);
			return nullptr;
		}
		bufferIndex_++;
	}
	//Read the next packet
	TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket reading next packet from buffer: nextReadPtr_=" << (void*)nextReadPtr_;
	if (newBuffer)
	{
		daqDMAByteCount_ = static_cast<uint16_t>(*static_cast<uint16_t*>(nextReadPtr_));
		nextReadPtr_ = reinterpret_cast<uint8_t*>(nextReadPtr_) + 2;
		*static_cast<uint32_t*>(nextReadPtr_) = bufferIndex_;
		nextReadPtr_ = reinterpret_cast<uint8_t*>(nextReadPtr_) + 6;
	}
	auto blockByteCount = *reinterpret_cast<uint16_t*>(nextReadPtr_);
	TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket: blockByteCount=" << blockByteCount << ", daqDMAByteCount=" << daqDMAByteCount_
		<< ", nextReadPtr_=" << (void*)nextReadPtr_ << ", *nextReadPtr=" << (int)*((uint16_t*)nextReadPtr_);
	if (blockByteCount == 0 || blockByteCount == 0xcafe)
	{
		TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket: blockByteCount is 0, returning NULL!";
		return nullptr;
	}

	auto test = DTC_DataPacket(nextReadPtr_);
	TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket: current+blockByteCount=" << (void*)(reinterpret_cast<uint8_t*>(nextReadPtr_) + blockByteCount)
		<< ", end of dma buffer=" << (void*)(daqbuffer_.back()[0] + daqDMAByteCount_ + 8); // +8 because first 8 bytes are not included in byte count
	if (reinterpret_cast<uint8_t*>(nextReadPtr_) + blockByteCount > daqbuffer_.back()[0] + daqDMAByteCount_ + 8)
	{
		blockByteCount = static_cast<uint16_t>(daqbuffer_.back()[0] + daqDMAByteCount_ + 8 - reinterpret_cast<uint8_t*>(nextReadPtr_));// +8 because first 8 bytes are not included in byte count
		TLOG(TLVL_ReadNextDAQPacket) << "Adjusting blockByteCount to " << blockByteCount << " due to end-of-DMA condition";
		test.SetWord(0, blockByteCount & 0xFF);
		test.SetWord(1, (blockByteCount >> 8));
	}

	TLOG(TLVL_ReadNextDAQPacket) << test.toJSON();
	auto output = new DTC_DataHeaderPacket(test);
	TLOG(TLVL_ReadNextDAQPacket) << output->toJSON();
	if (static_cast<uint16_t>((1 + output->GetPacketCount()) * 16) != blockByteCount)
	{
		TLOG(TLVL_ERROR) << "Data Error Detected: PacketCount: " << output->GetPacketCount()
			<< ", ExpectedByteCount: " << (1u + output->GetPacketCount()) * 16u << ", BlockByteCount: " << blockByteCount;
		throw DTC_DataCorruptionException();
	}
	first_read_ = false;

	// Update the packet pointers

	// lastReadPtr_ is easy...
	lastReadPtr_ = nextReadPtr_;

	// Increment by the size of the data block
	nextReadPtr_ = reinterpret_cast<char*>(nextReadPtr_) + blockByteCount;

	TLOG(TLVL_ReadNextDAQPacket) << "ReadNextDAQPacket RETURN";
	return output;
}

DTCLib::DTC_DCSReplyPacket* DTCLib::DTC::ReadNextDCSPacket()
{
	TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket BEGIN";
	if (dcsReadPtr_ == nullptr || dcsReadPtr_ >= reinterpret_cast<uint8_t*>(dcsbuffer_.back()) + sizeof(mu2e_databuff_t) || *reinterpret_cast<uint16_t*>(dcsReadPtr_) == 0 || *reinterpret_cast<uint16_t*>(dcsReadPtr_) == 0xcafe )
	{
		TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket Obtaining new DCS Buffer";
		std::cout << "ReadNextDCSPacket Obtaining new DCS Buffer " << std::endl;
		auto retsts = ReadBuffer(DTC_DMA_Engine_DCS);
		if (retsts > 0)
		{
			dcsReadPtr_ = &dcsbuffer_.back()[0];
			TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket dcsReadPtr_=" << (void*)dcsReadPtr_ << " dcsBuffer_=" << (void*)dcsbuffer_.back();
			std::cout << "ReadNextDCSPacket dcsReadPtr_=" << (void*)dcsReadPtr_ << " dcsBuffer_=" << (void*)dcsbuffer_.back() << std::endl;

			  // Move past DMA byte count
			  dcsReadPtr_ = static_cast<char*>(dcsReadPtr_) + 8;
		}
		else
		{
			TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket ReadBuffer returned " << retsts;
			std::cout << "ReadNextDCSPacket ReadBuffer returned " << retsts << std::endl;
			return nullptr;
		}
	}


	//Read the next packet
	TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket Reading packet from buffer: dcsReadPtr_=" << (void*)dcsReadPtr_;
	auto dataPacket = DTC_DataPacket(dcsReadPtr_);
	TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket: DTC_DataPacket: " << dataPacket.toJSON();
	std::cout << "ReadNextDCSPacket: DTC_DataPacket: " << dataPacket.toJSON() << std::endl;
	auto output = new DTC_DCSReplyPacket(dataPacket);
	TLOG(TLVL_ReadNextDCSPacket) << output->toJSON();
	std::cout << "Converted to DCS: " << output->toJSON() << std::endl;

	// Update the packet pointer

	// Increment by the size of the data block
	dcsReadPtr_ = static_cast<char*>(dcsReadPtr_) + 16;

	TLOG(TLVL_ReadNextDCSPacket) << "ReadNextDCSPacket RETURN";
	return output;
}

void DTCLib::DTC::WriteDetectorEmulatorData(mu2e_databuff_t* buf, size_t sz)
{
	if (sz < dmaSize_)
	{
		sz = dmaSize_;
	}
	auto retry = 3;
	int errorCode;
	do
	{
		TLOG(TLVL_WriteDetectorEmulatorData) << "WriteDetectorEmulatorData: Writing buffer of size " << sz;
		errorCode = device_.write_data(DTC_DMA_Engine_DAQ, buf, sz);
		retry--;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw DTC_IOErrorException();
	}
}

//
// Private Functions.
//
int DTCLib::DTC::ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms)
{
	mu2e_databuff_t* buffer;
	auto retry = 2;
	int errorCode;
	do
	{
		TLOG(TLVL_ReadBuffer) << "ReadBuffer before device_.read_data";
		errorCode = device_.read_data(channel, reinterpret_cast<void**>(&buffer), tmo_ms);
		retry--;
		//if (errorCode == 0) usleep(1000);
	} while (retry > 0 && errorCode == 0);
	if (errorCode == 0)
	{
		TLOG(TLVL_ReadBuffer) << "ReadBuffer: Device timeout occurred! ec=" << errorCode << ", rt=" << retry;
	}
	else if (errorCode < 0) throw DTC_IOErrorException();
	else
	{
		TLOG(TLVL_ReadBuffer) << "ReadDataPacket buffer_=" << (void*)buffer << " errorCode=" << errorCode << " *buffer_=0x" << std::hex << *(unsigned*)buffer;
		if (channel == DTC_DMA_Engine_DAQ)
		{
			daqbuffer_.push_back(buffer);
		}
		else if (channel == DTC_DMA_Engine_DCS)
		{
			dcsbuffer_.push_back(buffer);
		}
	}
	return errorCode;
}

void DTCLib::DTC::WriteDataPacket(const DTC_DataPacket& packet)
{
			TLOG(TLVL_WriteDataPacket) << "WriteDataPacket: Writing packet: " << packet.toJSON();
  mu2e_databuff_t buf;
  uint64_t size = packet.GetSize() + sizeof(uint64_t);
  uint64_t packetSize = packet.GetSize();
  if(size < static_cast<uint64_t>(dmaSize_)) size = dmaSize_;

  memcpy(&buf[0], &packetSize, sizeof(uint64_t));
  memcpy(&buf[8], packet.GetData(), packet.GetSize() * sizeof(uint8_t));


		auto retry = 3;
		int errorCode;
		do
		{
		  errorCode = device_.write_data(DTC_DMA_Engine_DCS, &buf, size);
			retry--;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			throw DTC_IOErrorException();
		}
}

void DTCLib::DTC::WriteDMAPacket(const DTC_DMAPacket& packet)
{
	WriteDataPacket(packet.ConvertToDataPacket());
}

