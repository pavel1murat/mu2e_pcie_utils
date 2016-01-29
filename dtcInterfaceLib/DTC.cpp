#include "DTC.h"
#include <sstream> // Convert uint to hex string
#include <iostream>
#include <fstream>
#include <iomanip> // std::setw, std::setfill
#include <chrono>
#ifndef _WIN32
# include <unistd.h>
# include "trace.h"
#else
#endif
#define TRACE_NAME "MU2EDEV"

DTCLib::DTC::DTC(DTCLib::DTC_SimMode mode) : DTC_Registers(mode),
daqbuffer_(nullptr), buffers_used_(0), dcsbuffer_(nullptr),
bufferIndex_(0), first_read_(true), daqDMAByteCount_(0), dcsDMAByteCount_(0),
lastReadPtr_(nullptr), nextReadPtr_(nullptr), dcsReadPtr_(nullptr)
{
#ifdef _WIN32
#pragma warning(disable: 4996)
#endif
	char* sim = getenv("DTCLIB_SIM_FILE");
	if (sim != NULL)
	{
		std::ifstream is(sim, std::ifstream::binary);
		while (is && is.good())
		{
			TRACE(4, "Reading a DMA from file...");
			mu2e_databuff_t* buf = (mu2e_databuff_t*)new mu2e_databuff_t();
			is.read((char*)buf, sizeof(uint64_t));
			uint64_t sz = *((uint64_t*)*buf);
			TRACE(4, "Size is %llu, writing to device", (long long unsigned)sz);
			is.read((char*)buf + 8, sz - sizeof(uint64_t));
			if (sz > 0) {
				WriteDetectorEmulatorData(buf, sz);
			}
		}
		is.close();
		EnableDetectorEmulator();
	}
}

DTCLib::DTC::~DTC()
{
	device_.close();
	daqbuffer_ = nullptr;
	dcsbuffer_ = nullptr;
	lastReadPtr_ = nullptr;
	nextReadPtr_ = nullptr;
	dcsReadPtr_ = nullptr;
}

//
// DMA Functions
//
std::vector<void*> DTCLib::DTC::GetData(DTC_Timestamp when)
{
	TRACE(19, "DTC::GetData begin");
	std::vector<void*> output;
#if 0
	//This code is disabled in favor of a separate Software CFO Emulator
	for (auto ring : DTC_Rings)
	{
		if (!ringEnabledMode_[ring].TimingEnable)
		{
			if (ringEnabledMode_[ring].TransmitEnable)
			{
				TRACE(19, "DTC::GetData before SendReadoutRequestPacket");
				SendReadoutRequestPacket(ring, when, quiet);
				if (debug)
				{
					std::cout << "Sent ReadoutRequest. Press any key." << std::endl;
					std::string dummy;
					getline(std::cin, dummy);
				}
				int maxRoc;
				if ((maxRoc = ReadRingROCCount(ring)) != DTC_ROC_Unused)
				{
					for (uint8_t roc = 0; roc <= maxRoc; ++roc)
					{
						TRACE(19, "DTC::GetData before DTC_DataRequestPacket req");
						DTC_DataRequestPacket req(ring, (DTC_ROC_ID)roc, when, true, (uint16_t)debugCount);
						TRACE(19, "DTC::GetData before WriteDMADAQPacket - DTC_DataRequestPacket");
						if (!quiet) std::cout << req.toJSON() << std::endl;
						WriteDMADAQPacket(req);
						TRACE(19, "DTC::GetData after  WriteDMADAQPacket - DTC_DataRequestPacket");
					}
				}
			}
		}
	}
#endif
	first_read_ = true;
	TRACE(19, "DTC::GetData: Releasing %i buffers", buffers_used_);
	device_.read_release(DTC_DMA_Engine_DAQ, buffers_used_);
	buffers_used_ = 0;
	DTC_DataHeaderPacket* packet = nullptr;

	try
	{
		// Read the header packet
		int tries = 0;
		while (packet == nullptr && tries < 3)
		{
			TRACE(19, "DTC::GetData before ReadNextDAQPacket, tries=%i", tries);
			packet = ReadNextDAQPacket(first_read_ ? 10000 : 1);
			TRACE(19, "DTC::GetData after  ReadDMADAQPacket");
			tries++;
			//if (packet == nullptr) usleep(5000);
		}
		if (packet == nullptr)
		{
			TRACE(19, "DTC::GetData: Timeout Occurred! (Lead packet is nullptr after retries)");
			return output;
		}

		if (packet->GetTimestamp() != when && when.GetTimestamp(true) != 0)
		{
			TRACE(0, "DTC::GetData: Error: Lead packet has wrong timestamp! 0x%llX(expected) != 0x%llX"
				, (unsigned long long)when.GetTimestamp(true), (unsigned long long)packet->GetTimestamp().GetTimestamp(true));
			delete packet;
			return output;
		}
		else
		{
			when = packet->GetTimestamp();
		}
		delete packet;
		packet = nullptr;

		TRACE(19, "DTC::GetData: Adding pointer %p to the list (first)", (void*)lastReadPtr_);
		output.push_back(lastReadPtr_);

		bool done = false;
		while (!done)
		{
			packet = ReadNextDAQPacket();
			if (packet == nullptr)  // End of Data
			{
				done = true;
				nextReadPtr_ = nullptr;
			}
			else if (packet->GetTimestamp() != when)
			{
				done = true;
				nextReadPtr_ = lastReadPtr_;
			}
			delete packet;
			packet = nullptr;

			TRACE(19, "DTC::GetData: Adding pointer %p to the list", (void*)lastReadPtr_);
			if (!done) output.push_back(lastReadPtr_);
		}
	}
	catch (DTC_WrongPacketTypeException ex)
	{
		TRACE(19, "DTC::GetData: Bad omen: Wrong packet type at the current read position");
		nextReadPtr_ = nullptr;
	}
	catch (DTC_IOErrorException ex)
	{
		nextReadPtr_ = nullptr;
		TRACE(19, "DTC::GetData: IO Exception Occurred!");
	}
	catch (DTC_DataCorruptionException ex)
	{
		nextReadPtr_ = nullptr;
		TRACE(19, "DTC::GetData: Data Corruption Exception Occurred!");
	}

	TRACE(19, "DTC::GetData RETURN");
	delete packet;
	return output;
}   // GetData

std::string DTCLib::DTC::GetJSONData(DTC_Timestamp when)
{
	TRACE(19, "DTC::GetJSONData BEGIN");
	std::stringstream ss;
	TRACE(19, "DTC::GetJSONData before call to GetData");
	std::vector<void*> data = GetData(when);
	TRACE(19, "DTC::GetJSONData after call to GetData, data size %llu", (unsigned long long)data.size());

	for (size_t i = 0; i < data.size(); ++i)
	{
		TRACE(19, "DTC::GetJSONData constructing DataPacket:");
		DTC_DataPacket test = DTC_DataPacket(data[i]);
		TRACE(19, test.toJSON().c_str());
		TRACE(19, "DTC::GetJSONData constructing DataHeaderPacket");
		DTC_DataHeaderPacket theHeader = DTC_DataHeaderPacket(test);
		ss << "DataBlock: {";
		ss << theHeader.toJSON() << ",";
		ss << "DataPackets: [";
		for (int packet = 0; packet < theHeader.GetPacketCount(); ++packet)
		{
			void* packetPtr = (void*)(((char*)data[i]) + 16 * (1 + packet));
			ss << DTC_DataPacket(packetPtr).toJSON() << ",";
		}
		ss << "]";
		ss << "}";
		if (i + 1 < data.size()) { ss << ","; }
	}

	TRACE(19, "DTC::GetJSONData RETURN");
	return ss.str();
}

// ROC Register Functions
uint16_t DTCLib::DTC::ReadROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address)
{
	SendDCSRequestPacket(ring, roc, DTC_DCSOperationType_Read, address);
	auto reply = ReadNextDCSPacket();
	if (reply != nullptr) {
		return reply->GetData();
	}
	return 0;
}

void DTCLib::DTC::WriteROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address, const uint16_t data)
{
	SendDCSRequestPacket(ring, roc, DTC_DCSOperationType_Write, address, data);
}

uint16_t DTCLib::DTC::ReadExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint16_t address)
{
	uint16_t addressT = address & 0x7FFF;
	WriteROCRegister(ring, roc, 12, block);
	WriteROCRegister(ring, roc, 13, addressT);
	WriteROCRegister(ring, roc, 13, addressT | 0x8000);
	return ReadROCRegister(ring, roc, 22);
}

void DTCLib::DTC::WriteExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint8_t address, const uint16_t data)
{
	uint16_t dataT = data & 0x7FFF;
	WriteROCRegister(ring, roc, 12, block + (address << 8));
	WriteROCRegister(ring, roc, 13, dataT);
	WriteROCRegister(ring, roc, 13, dataT | 0x8000);
}

std::string DTCLib::DTC::ROCRegDump(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc)
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);
	o << "{";
	o << "\"Forward Detector 0 Status\": " << ReadExtROCRegister(ring, roc, 8, 0) << ",\n";
	o << "\"Forward Detector 1 Status\": " << ReadExtROCRegister(ring, roc, 9, 0) << ",\n";
	o << "\"Command Handler Status\": " << ReadExtROCRegister(ring, roc, 10, 0) << ",\n";
	o << "\"Packet Sender 0 Status\": " << ReadExtROCRegister(ring, roc, 11, 0) << ",\n";
	o << "\"Packet Sender 1 Status\": " << ReadExtROCRegister(ring, roc, 12, 0) << ",\n";
	o << "\"Forward Detector 0 Errors\": " << ReadExtROCRegister(ring, roc, 8, 1) << ",\n";
	o << "\"Forward Detector 1 Errors\": " << ReadExtROCRegister(ring, roc, 9, 1) << ",\n";
	o << "\"Command Handler Errors\": " << ReadExtROCRegister(ring, roc, 10, 1) << ",\n";
	o << "\"Packet Sender 0 Errors\": " << ReadExtROCRegister(ring, roc, 11, 1) << ",\n";
	o << "\"Packet Sender 1 Errors\": " << ReadExtROCRegister(ring, roc, 12, 1) << "\n";
	o << "}";

	return o.str();
}

void DTCLib::DTC::SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet)
{
	DTC_HeartbeatPacket req(ring, when, ReadRingROCCount((DTC_Ring_ID)ring));
	TRACE(19, "DTC::SendReadoutRequestPacket before WriteDMADAQPacket - DTC_HeartbeatPacket");
	if (!quiet) std::cout << req.toJSON() << std::endl;
	WriteDMAPacket(req);
	TRACE(19, "DTC::SendReadoutRequestPacket after  WriteDMADAQPacket - DTC_HeartbeatPacket");
}

void DTCLib::DTC::SendDCSRequestPacket(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_DCSOperationType type, const uint8_t address, const uint16_t data, bool quiet)
{
	DTC_DCSRequestPacket req(ring, roc, type, address, data);
	TRACE(19, "DTC::SendDCSRequestPacket before WriteDMADCSPacket - DTC_DCSRequestPacket");
	if (!quiet) std::cout << req.toJSON() << std::endl;
	WriteDMAPacket(req);
	TRACE(19, "DTC::SendDCSRequestPacket after  WriteDMADCSPacket - DTC_DCSRequestPacket");
}

DTCLib::DTC_DataHeaderPacket* DTCLib::DTC::ReadNextDAQPacket(int tmo_ms)
{
	TRACE(19, "DTC::ReadNextDAQPacket BEGIN");
	if (nextReadPtr_ != nullptr)
	{
		TRACE(19, "DTC::ReadNextDAQPacket BEFORE BUFFER CHECK nextReadPtr_=%p *nextReadPtr_=0x%08x"
			, (void*)nextReadPtr_, *(uint16_t*)nextReadPtr_);
	}
	else
	{
		TRACE(19, "DTC::ReadNextDAQPacket BEFORE BUFFER CHECK nextReadPtr_=nullptr");
	}
	bool newBuffer = false;
	// Check if the nextReadPtr has been initialized, and if its pointing to a valid location
	if (nextReadPtr_ == nullptr
		|| nextReadPtr_ >= (uint8_t*)daqbuffer_ + sizeof(mu2e_databuff_t)
		|| nextReadPtr_ >= (uint8_t*)daqbuffer_ + daqDMAByteCount_
		|| (*((uint16_t*)nextReadPtr_)) == 0)
	{
		newBuffer = true;
		if (first_read_)
		{
			lastReadPtr_ = nullptr;
		}
		TRACE(19, "DTC::ReadNextDAQPacket Obtaining new DAQ Buffer");
		void* oldBufferPtr = &(daqbuffer_[0]);
		int sts = ReadBuffer(DTC_DMA_Engine_DAQ, tmo_ms); // does return code
		if (sts <= 0) {
			TRACE(19, "DTC::ReadNextDAQPacket: ReadBuffer returned %i, returning nullptr", sts);
			return nullptr;
		}
		// MUST BE ABLE TO HANDLE daqbuffer_==nullptr OR retry forever?
		nextReadPtr_ = &(daqbuffer_[0]);
		TRACE(19, "DTC::ReadNextDAQPacket nextReadPtr_=%p *nextReadPtr_=0x%08x lastReadPtr_=%p"
			, (void*)nextReadPtr_, *(unsigned*)nextReadPtr_, (void*)lastReadPtr_);
		void* bufferIndexPointer = (uint8_t*)nextReadPtr_ + 2;
		if (nextReadPtr_ == oldBufferPtr && bufferIndex_ == *(uint32_t*)bufferIndexPointer)
		{
			nextReadPtr_ = nullptr;
			//We didn't actually get a new buffer...this probably means there's no more data
			return nullptr;
		}
		buffers_used_++;
		bufferIndex_++;
	}
	//Read the next packet
	TRACE(19, "DTC::ReadNextDAQPacket reading next packet from buffer: nextReadPtr_=%p:", (void*)nextReadPtr_);
	if (newBuffer)
	{
		daqDMAByteCount_ = static_cast<uint16_t>(*((uint16_t*)nextReadPtr_));
		nextReadPtr_ = (uint8_t*)nextReadPtr_ + 2;
		*(uint32_t*)nextReadPtr_ = bufferIndex_;
		nextReadPtr_ = (uint8_t*)nextReadPtr_ + 6;
	}
	uint16_t blockByteCount = *((uint16_t*)nextReadPtr_);
	TRACE(19, "DTC::ReadNextDAQPacket: blockByteCount=%u, daqDMAByteCount=%u, nextReadPtr_=%p, *nextReadPtr=0x%x", blockByteCount, daqDMAByteCount_, (void*)nextReadPtr_, *((uint8_t*)nextReadPtr_));
	if (blockByteCount == 0)
	{
		TRACE(19, "DTC::ReadNextDAQPacket: blockByteCount is 0, returning NULL!");
		return nullptr;
	}
	DTC_DataPacket test = DTC_DataPacket(nextReadPtr_);
	TRACE(19, test.toJSON().c_str());
	DTC_DataHeaderPacket* output = new DTC_DataHeaderPacket(test);
	TRACE(19, output->toJSON().c_str());
	if (static_cast<uint16_t>((1 + output->GetPacketCount()) * 16) != blockByteCount)
	{
		TRACE(19, "Data Error Detected: PacketCount: %u, ExpectedByteCount: %u, BlockByteCount: %u", output->GetPacketCount(), (1u + output->GetPacketCount()) * 16u, blockByteCount);
		delete output;
		throw DTC_DataCorruptionException();
	}
	first_read_ = false;

	// Update the packet pointers

	// lastReadPtr_ is easy...
	lastReadPtr_ = nextReadPtr_;

	// Increment by the size of the data block
	nextReadPtr_ = (char*)nextReadPtr_ + blockByteCount;

	TRACE(19, "DTC::ReadNextDAQPacket RETURN");
	return output;
}
DTCLib::DTC_DCSReplyPacket* DTCLib::DTC::ReadNextDCSPacket()
{
	TRACE(19, "DTC::ReadNextDCSPacket BEGIN");
	if (dcsReadPtr_ == nullptr || dcsReadPtr_ >= (uint8_t*)dcsbuffer_ + sizeof(mu2e_databuff_t) || (*((uint16_t*)dcsReadPtr_)) == 0)
	{
		TRACE(19, "DTC::ReadNextDCSPacket Obtaining new DCS Buffer");
		int retsts = ReadBuffer(DTC_DMA_Engine_DCS);
		if (retsts > 0) {
			dcsReadPtr_ = &(dcsbuffer_[0]);
			TRACE(19, "DTC::ReadNextDCSPacket dcsReadPtr_=%p dcsBuffer_=%p", (void*)dcsReadPtr_, (void*)dcsbuffer_);
		}
		else {
			TRACE(19, "DTC::ReadNextDCSPacket ReadBuffer returned %i", retsts);
			return nullptr;
		}
	}

	//Read the next packet
	TRACE(19, "DTC::ReadNextDCSPacket Reading packet from buffer: dcsReadPtr_=%p:", (void*)dcsReadPtr_);
	DTC_DCSReplyPacket* output = new DTC_DCSReplyPacket(DTC_DataPacket(dcsReadPtr_));
	TRACE(19, output->toJSON().c_str());

	// Update the packet pointer

	// Increment by the size of the data block
	dcsReadPtr_ = (char*)dcsReadPtr_ + 16;

	TRACE(19, "DTC::ReadNextDCSPacket RETURN");
	return output;
}

void DTCLib::DTC::WriteDetectorEmulatorData(mu2e_databuff_t* buf, size_t sz)
{
	if (sz < dmaSize_)
	{
		sz = dmaSize_;
	}
	uint32_t oldWritePointer = ReadDDRLocalEndAddress();
	SetDDRLocalEndAddress(0xFFFFFFFF);
	int retry = 3;
	int errorCode;
	do
	{
		TRACE(21, "DTC::WriteDetectorEmulatorData: Writing buffer of size %lu", sz);
		errorCode = device_.write_data(DTC_DMA_Engine_DAQ, buf, sz);
		retry--;
	} while (retry > 0 && errorCode != 0);
	SetDDRLocalEndAddress(oldWritePointer);
	IncrementDDRLocalEndAddress(sz);
	TRACE(10, "DTC::WriteDetectorEmulatorData: Incrementing DMACount from %lu", (long unsigned)ReadDetectorEmulationDMACount());
	IncrementDetectorEmulationDMACount();
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
	int retry = 2;
	int errorCode;
	do
	{
		TRACE(19, "DTC::ReadBuffer before device_.read_data");
		errorCode = device_.read_data(channel, (void**)&buffer, tmo_ms);
		retry--;
		//if (errorCode == 0) usleep(1000);
	} while (retry > 0 && errorCode == 0);
	if (errorCode == 0)
	{
		TRACE(16, "DTC::ReadBuffer: Device timeout occurred! ec=%i, rt=%i", errorCode, retry);
	}
	else if (errorCode < 0)
		throw DTC_IOErrorException();
	else {
		TRACE(16, "DTC::ReadDataPacket buffer_=%p errorCode=%d *buffer_=0x%08x"
			, (void*)buffer, errorCode, *(unsigned*)buffer);
		if (channel == DTC_DMA_Engine_DAQ) { daqbuffer_ = buffer; }
		else if (channel == DTC_DMA_Engine_DCS) { dcsbuffer_ = buffer; }
	}
	return errorCode;
}
void DTCLib::DTC::WriteDataPacket(const DTC_DataPacket& packet)
{
	if (packet.GetSize() < dmaSize_)
	{
		DTC_DataPacket thisPacket(packet);
		thisPacket.Resize(dmaSize_);
		int retry = 3;
		int errorCode;
		do
		{
			std::string output = "DTC::WriteDataPacket: Writing packet: " + packet.toJSON();
			TRACE(21, output.c_str());
			errorCode = device_.write_data(DTC_DMA_Engine_DCS, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
			retry--;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			throw DTC_IOErrorException();
		}
	}
	else
	{
		int retry = 3;
		int errorCode;
		do
		{
			std::string output = "DTC::WriteDataPacket: Writing packet: " + packet.toJSON();
			TRACE(21, output.c_str());
			errorCode = device_.write_data(DTC_DMA_Engine_DCS, packet.GetData(), packet.GetSize() * sizeof(uint8_t));
			retry--;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			throw DTC_IOErrorException();
		}
	}
}

void DTCLib::DTC::WriteDMAPacket(const DTC_DMAPacket& packet)
{
	WriteDataPacket(packet.ConvertToDataPacket());
}

