#include "DTC.h"
#include <sstream> // Convert uint to hex string
#include <iomanip> // std::setw, std::setfill
#include <chrono>
#ifndef _WIN32
# include <unistd.h>
# include "trace.h"
#else
#endif
#include <inttypes.h>
#define TRACE_NAME "MU2EDEV"

DTCLib::DTC::DTC(DTCLib::DTC_SimMode mode) : device_(),
daqbuffer_(nullptr), dcsbuffer_(nullptr), simMode_(mode),
maxROCs_(), dmaSize_(16), bufferIndex_(0), first_read_(true), daqDMAByteCount_(0), dcsDMAByteCount_(0),
lastReadPtr_(nullptr), nextReadPtr_(nullptr), dcsReadPtr_(nullptr), deviceTime_(0)
{
#ifdef _WIN32
	simMode_ = DTCLib::DTC_SimMode_Tracker;
#else
	char* sim = getenv("DTCLIB_SIM_ENABLE");
	if (sim != NULL)
	{
		switch (sim[0])
		{
		case '1':
		case 't':
		case 'T':
			simMode_ = DTCLib::DTC_SimMode_Tracker;
			break;
		case '2':
		case 'c':
		case 'C':
			simMode_ = DTCLib::DTC_SimMode_Calorimeter;
			break;
		case '3':
		case 'v':
		case 'V':
			simMode_ = DTCLib::DTC_SimMode_CosmicVeto;
			break;
		case '4':
		case 'n':
		case 'N':
			simMode_ = DTCLib::DTC_SimMode_NoCFO;
			break;
		case '5':
		case 'r':
		case 'R':
			simMode_ = DTCLib::DTC_SimMode_ROCEmulator;
			break;
		case '6':
		case 'l':
		case 'L':
			simMode_ = DTCLib::DTC_SimMode_Loopback;
			break;
		case '7':
		case 'p':
		case 'P':
			simMode_ = DTCLib::DTC_SimMode_Performance;
			break;
		case '0':
		default:
			simMode_ = DTCLib::DTC_SimMode_Disabled;
			break;
		}
	}
#endif
	SetSimMode(simMode_);
}

DTCLib::DTC_SimMode DTCLib::DTC::SetSimMode(DTC_SimMode mode)
{
	simMode_ = mode;

	auto start = std::chrono::high_resolution_clock::now();
	device_.init(simMode_);
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::high_resolution_clock::now() - start).count();

	for (auto ring : DTC_Rings) {
		SetMaxROCNumber(ring, DTC_ROC_Unused);
		DisableROCEmulator(ring);
	}

	if (simMode_ != DTCLib::DTC_SimMode_Disabled)
	{
		// Set up hardware simulation mode: Ring 0 Tx/Rx Enabled, Loopback Enabled, ROC Emulator Enabled. All other rings disabled.
		for (auto ring : DTC_Rings) {
			DisableRing(ring);
		}
		EnableRing(DTC_Ring_0, DTC_RingEnableMode(true, true, false), DTC_ROC_0);
		if (simMode_ == DTC_SimMode_Loopback)
		{
			SetSERDESLoopbackMode(DTC_Ring_0, DTC_SERDESLoopbackMode_NearPCS);
			DisableROCEmulator(DTC_Ring_0);
			SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
		}
		else if(simMode_ == DTC_SimMode_ROCEmulator)
		{
			SetSERDESLoopbackMode(DTC_Ring_0, DTC_SERDESLoopbackMode_Disabled);
			EnableROCEmulator(DTC_Ring_0);
			SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
		}
		SetInternalSystemClock();
		DisableTiming();
	}
	ReadMinDMATransferLength();

	return simMode_;
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
	for (auto ring : DTC_Rings) {
		if (!ringEnabledMode_[ring].TimingEnable)
		{
			if (ringEnabledMode_[ring].TransmitEnable)
			{
				TRACE(19, "DTC::GetData before SendReadoutRequestPacket");
				SendReadoutRequestPacket(ring, when, quiet);
				if (debug) {
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
				//usleep(2000);
			}
		}
}
#endif
	first_read_ = true;
	try {
		// Read the header packet
		DTC_DataHeaderPacket* packet = nullptr;
		int tries = 0;
		while (packet == nullptr && tries < 3) {
			TRACE(19, "DTC::GetData before ReadNextDAQPacket, tries=%i", tries);
			packet = ReadNextDAQPacket(first_read_ ? 10000 : 1);
			TRACE(19, "DTC::GetData after  ReadDMADAQPacket");
			tries++;
			if (packet == nullptr) usleep(1000);
		}
		if (packet == nullptr) {
			TRACE(19, "DTC::GetData: Timeout Occurred! (Lead packet is nullptr after retries)");
			return output;
		}

		if (packet->GetTimestamp() != when && when.GetTimestamp(true) != 0)
		{
			TRACE(0, "DTC::GetData: Error: Lead packet has wrong timestamp! 0x%llX(expected) != 0x%llX"
				, (unsigned long long)when.GetTimestamp(true), (unsigned long long)packet->GetTimestamp().GetTimestamp(true));
			return output;
		}
		else {
			when = packet->GetTimestamp();
		}
		delete packet;

		TRACE(19, "DTC::GetData: Adding pointer %p to the list", (void*)lastReadPtr_);
		output.push_back(lastReadPtr_);

		bool done = false;
		while (!done) {
			try {
				DTC_DataHeaderPacket* thispacket = ReadNextDAQPacket();
				if (thispacket == nullptr)  // End of Data
				{
					done = true;
					nextReadPtr_ = nullptr;
					break;
				}
				if (thispacket->GetTimestamp() != when) {
					done = true;
					nextReadPtr_ = lastReadPtr_;
					break;
				}
				delete thispacket;

				TRACE(19, "DTC::GetData: Adding pointer %p to the list", (void*)lastReadPtr_);
				output.push_back(lastReadPtr_);
			}
			catch (DTC_TimeoutOccurredException ex)
			{
				TRACE(19, "DTC::GetData: Timeout occurred!");
				done = true;
			}
		}
	}
	catch (DTC_WrongPacketTypeException ex)
	{
		TRACE(19, "DTC::GetData: Bad omen: Wrong packet type at the current read position");
		nextReadPtr_ = nullptr;
	}
	catch (DTC_TimeoutOccurredException ex)
	{
		TRACE(19, "DTC::GetData: Timeout occurred!");
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
	return output;
}   // GetData

std::string DTCLib::DTC::GetJSONData(DTC_Timestamp when)
{
	TRACE(19, "DTC::GetJSONData BEGIN");
	std::stringstream ss;
	TRACE(19, "DTC::GetJSONData before call to GetData");
	std::vector<void*> data = GetData(when);
	TRACE(19, "DTC::GetJSONData after call to GetData, data size %zu", data.size());

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

void DTCLib::DTC::DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t* dataIn)
{
	auto start = std::chrono::high_resolution_clock::now();
	device_.release_all(1);
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::high_resolution_clock::now() - start).count();

	DTC_DCSRequestPacket req(ring, roc, dataIn);
	WriteDMADCSPacket(req);
	DTC_DCSReplyPacket* packet = ReadNextDCSPacket();

	TRACE(19, "DTC::DCSReqeuestReply after ReadNextDCSPacket");
	for (int ii = 0; ii < 12; ++ii) {
		dataIn[ii] = packet->GetData()[ii];
	}
}

void DTCLib::DTC::SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet)
{
	DTC_ReadoutRequestPacket req(ring, when, ReadRingROCCount((DTC_Ring_ID)ring));
	TRACE(19, "DTC::SendReadoutRequestPacket before WriteDMADAQPacket - DTC_ReadoutRequestPacket");
	if (!quiet) std::cout << req.toJSON() << std::endl;
	WriteDMADAQPacket(req);
	TRACE(19, "DTC::SendReadoutRequestPacket after  WriteDMADAQPacket - DTC_ReadoutRequestPacket");
}

void DTCLib::DTC::WriteDMADAQPacket(const DTC_DMAPacket& packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DAQ, packet);
}
void DTCLib::DTC::WriteDMADCSPacket(const DTC_DMAPacket& packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DCS, packet);
}

DTCLib::DTC_DataHeaderPacket* DTCLib::DTC::ReadNextDAQPacket(int tmo_ms)
{
	TRACE(19, "DTC::ReadNextDAQPacket BEGIN");
	if (nextReadPtr_ != nullptr) {
		TRACE(19, "DTC::ReadNextDAQPacket BEFORE BUFFER CHECK nextReadPtr_=%p *nextReadPtr_=0x%08x"
			, (void*)nextReadPtr_, *(uint16_t*)nextReadPtr_);
	}
	else {
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
		if (first_read_) {
			TRACE(19, "DTC::ReadNextDAQPacket: calling device_.release_all");
			auto start = std::chrono::high_resolution_clock::now();
			device_.release_all(DTC_DMA_Engine_DAQ);
			deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
				(std::chrono::high_resolution_clock::now() - start).count();

			lastReadPtr_ = nullptr;
		}
		TRACE(19, "DTC::ReadNextDAQPacket Obtaining new DAQ Buffer");
		void* oldBufferPtr = &(daqbuffer_[0]);
		ReadBuffer(DTC_DMA_Engine_DAQ, tmo_ms); // does return val of type DTCLib::DTC_DataPacket
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
		bufferIndex_++;
	}
	//Read the next packet
	TRACE(19, "DTC::ReadNextDAQPacket reading next packet from buffer: nextReadPtr_=%p:", (void*)nextReadPtr_);
	if (newBuffer) {
		daqDMAByteCount_ = static_cast<uint16_t>(*((uint16_t*)nextReadPtr_));
		nextReadPtr_ = (uint8_t*)nextReadPtr_ + 2;
		*(uint32_t*)nextReadPtr_ = bufferIndex_;
		nextReadPtr_ = (uint8_t*)nextReadPtr_ + 6;
	}
	uint16_t blockByteCount = *((uint16_t*)nextReadPtr_);
	TRACE(19, "DTC::ReadNextDAQPacket: blockByteCount=%u, daqDMAByteCount=%u, nextReadPtr_=%p, *nextReadPtr=0x%x", blockByteCount, daqDMAByteCount_, (void*)nextReadPtr_, *((uint8_t*)nextReadPtr_));
	if (blockByteCount == 0) {
		TRACE(19, "DTC::ReadNextDAQPacket: blockByteCount is 0, returning NULL!");
		return nullptr;
	}
	DTC_DataPacket test = DTC_DataPacket(nextReadPtr_);
	TRACE(19, test.toJSON().c_str());
	DTC_DataHeaderPacket* output = new DTC_DataHeaderPacket(test);
	TRACE(19, output->toJSON().c_str());
	if (static_cast<uint16_t>((1 + output->GetPacketCount()) * 16) != blockByteCount) {
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
	if (dcsReadPtr_ == nullptr || dcsReadPtr_ >= (uint8_t*)dcsbuffer_ + sizeof(mu2e_databuff_t) || (*((uint16_t*)dcsReadPtr_)) == 0) {
		TRACE(19, "DTC::ReadNextDCSPacket Obtaining new DCS Buffer");
		ReadBuffer(DTC_DMA_Engine_DCS);
		dcsReadPtr_ = &(dcsbuffer_[0]);
		TRACE(19, "DTC::ReadNextDCSPacket dcsReadPtr_=%p dcsBuffer_=%p", (void*)dcsReadPtr_, (void*)dcsbuffer_);
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

//
// Register IO Functions
//
std::string DTCLib::DTC::RegDump()
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);
	o << "{";
	o << "\"SimMode\":" << DTC_SimModeConverter(simMode_) << ",\n";
	o << "\"Version\":\"" << ReadDesignVersion() << "\",\n";
	o << "\"ResetDTC\":" << ReadResetDTC() << ",\n";
	o << "\"CFOEmulation\":" << ReadCFOEmulation() << ",\n";
	o << "\"ResetSERDESOscillator\":" << ReadResetSERDESOscillator() << ",\n";
	o << "\"SERDESOscillatorClock\":" << ReadSERDESOscillatorClock() << ",\n";
	o << "\"SystemClock\":" << ReadSystemClock() << ",\n";
	o << "\"TimingEnable\":" << ReadTimingEnable() << ",\n";
	o << "\"TriggerDMALength\":" << ReadTriggerDMATransferLength() << ",\n";
	o << "\"MinDMALength\":" << ReadMinDMATransferLength() << ",\n";
	o << "\"SERDESOscillatorIICError\":" << ReadSERDESOscillatorIICError() << ",\n";
	o << "\"SERDESOscillatorInitComplete\":" << ReadSERDESOscillatorInitializationComplete() << ",\n";
	o << "\"DMATimeout\":" << ReadDMATimeoutPreset() << ",\n";
	o << "\"ROC DataBlock Timeout\":" << ReadROCTimeoutPreset() << ",\n";
	o << "\"Timestamp\":" << ReadTimestampPreset().GetTimestamp(true) << ",\n";
	o << "\"DataPendingTimer\":" << ReadDataPendingTimer() << ",\n";
	o << "\"PacketSize\":" << ReadPacketSize() << ",\n";
	o << "\"PROMFIFOFull\":" << ReadFPGAPROMProgramFIFOFull() << ",\n";
	o << "\"PROMReady\":" << ReadFPGAPROMReady() << ",\n";
	o << "\"FPGACoreFIFOFull\":" << ReadFPGACoreAccessFIFOFull() << ",\n";
	o << RingRegDump(DTC_Ring_0, "\"Ring0\"") << ",\n";
	o << RingRegDump(DTC_Ring_1, "\"Ring1\"") << ",\n";
	o << RingRegDump(DTC_Ring_2, "\"Ring2\"") << ",\n";
	o << RingRegDump(DTC_Ring_3, "\"Ring3\"") << ",\n";
	o << RingRegDump(DTC_Ring_4, "\"Ring4\"") << ",\n";
	o << RingRegDump(DTC_Ring_5, "\"Ring5\"") << ",\n";
	o << CFORegDump() << "\n";
	o << "}";

	return o.str();
}
std::string DTCLib::DTC::RingRegDump(const DTC_Ring_ID& ring, std::string id)
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);

	o << id << ":{\n";

	DTC_ROC_ID ringROCs = ReadRingROCCount(ring);
	switch (ringROCs) {
	case DTC_ROC_Unused:
	default:
		o << "\t\"ROC0Enabled\":false,\n";
		o << "\t\"ROC1Enabled\":false,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_0:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":false,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_1:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_2:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_3:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_4:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":true,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_5:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":true,\n";
		o << "\t\"ROC5Enabled\":true,\n";
		break;
	}

	o << "\t\"Enabled\":" << ReadRingEnabled(ring) << ",\n";
	o << "\t\"ROCEmulator\":" << ReadROCEmulator(ring) << ",\n";
	o << "\t\"ResetSERDES\":" << ReadResetSERDES(ring) << ",\n";
	o << "\t\"SERDESLoopback\":" << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(ring)) << ",\n";
	o << "\t\"EyescanError\":" << ReadSERDESEyescanError(ring) << ",\n";
	o << "\t\"FIFOFullFlags\":" << ReadFIFOFullErrorFlags(ring) << ",\n";
	o << "\t\"FIFOHalfFull\":" << ReadSERDESBufferFIFOHalfFull(ring) << ",\n";
	o << "\t\"OverflowOrUnderflow\":" << ReadSERDESOverflowOrUnderflow(ring) << ",\n";
	o << "\t\"PLLLocked\":" << ReadSERDESPLLLocked(ring) << ",\n";
	o << "\t\"RXCDRLock\":" << ReadSERDESRXCDRLock(ring) << ",\n";
	o << "\t\"ResetDone\":" << ReadResetSERDESDone(ring) << ",\n";
	o << "\t\"UnlockError\":" << ReadSERDESUnlockError(ring) << ",\n";
	o << "\t\"RXBufferStatus\":" << DTC_RXBufferStatusConverter(ReadSERDESRXBufferStatus(ring)) << ",\n";
	o << "\t\"RXStatus\":" << DTC_RXStatusConverter(ReadSERDESRXStatus(ring)) << ",\n";
	o << "\t\"SERDESRXDisparity\":" << ReadSERDESRXDisparityError(ring) << ",\n";
	o << "\t\"CharacterError\":" << ReadSERDESRXCharacterNotInTableError(ring) << "\n";
	o << "\t\"TimeoutError\":" << ReadROCTimeoutError(ring) << "\n";
	o << "\t\"PacketCRCError\":" << ReadPacketCRCError(ring) << "\n";
	o << "\t\"PacketError\":" << ReadPacketError(ring) << "\n";
	o << "\t\"RXElasticBufferOverrun\":" << ReadRXElasticBufferOverrun(ring) << "\n";
	o << "\t\"RXElasticBufferUnderrun\":" << ReadRXElasticBufferUnderrun(ring) << "\n";
	o << "}";

	return o.str();
}
std::string DTCLib::DTC::CFORegDump()
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);

	o << "\"CFO\":{";

	o << "\t\"Enabled\":" << ReadRingEnabled(DTC_Ring_CFO) << ",\n";
	o << "\t\"SERDESLoopback\":" << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_CFO)) << ",\n";
	o << "\t\"CharacterError\":" << ReadSERDESRXCharacterNotInTableError(DTC_Ring_CFO) << ",\n";
	o << "\t\"EyescanError\":" << ReadSERDESEyescanError(DTC_Ring_CFO) << ",\n";
	o << "\t\"FIFOFullFlags\":" << ReadFIFOFullErrorFlags(DTC_Ring_CFO) << ",\n";
	o << "\t\"FIFOHalfFull\":" << ReadSERDESBufferFIFOHalfFull(DTC_Ring_CFO) << ",\n";
	o << "\t\"OverflowOrUnderflow\":" << ReadSERDESOverflowOrUnderflow(DTC_Ring_CFO) << ",\n";
	o << "\t\"PLLLocked\":" << ReadSERDESPLLLocked(DTC_Ring_CFO) << ",\n";
	o << "\t\"RXBufferStatus\":" << DTC_RXBufferStatusConverter(ReadSERDESRXBufferStatus(DTC_Ring_CFO)) << ",\n";
	o << "\t\"RXCDRLock\":" << ReadSERDESRXCDRLock(DTC_Ring_CFO) << ",\n";
	o << "\t\"RXStatus\":" << DTC_RXStatusConverter(ReadSERDESRXStatus(DTC_Ring_CFO)) << ",\n";
	o << "\t\"ResetDone\":" << ReadResetSERDESDone(DTC_Ring_CFO) << ",\n";
	o << "\t\"ResetSERDES\":" << ReadResetSERDES(DTC_Ring_CFO) << ",\n";
	o << "\t\"SERDESRXDisparity\":" << ReadSERDESRXDisparityError(DTC_Ring_CFO) << ",\n";
	o << "\t\"UnlockError\":" << ReadSERDESUnlockError(DTC_Ring_CFO) << "\n";
	o << "\t\"PacketCRCError\":" << ReadPacketCRCError(DTC_Ring_CFO) << "\n";
	o << "\t\"PacketError\":" << ReadPacketError(DTC_Ring_CFO) << "\n";
	o << "\t\"RXElasticBufferOverrun\":" << ReadRXElasticBufferOverrun(DTC_Ring_CFO) << "\n";
	o << "\t\"RXElasticBufferUnderrun\":" << ReadRXElasticBufferUnderrun(DTC_Ring_CFO) << "\n";

	o << "}";

	return o.str();
}

std::string DTCLib::DTC::ConsoleFormatRegDump()
{
	std::ostringstream o;
	o << "Memory Map: " << std::endl;
	o << "    Address | Value      | Name                         | Translation" << std::endl;
	for (auto i : DTC_Registers)
	{
		o << "================================================================================" << std::endl;
		o << FormatRegister(i);
	}
	return o.str();
}

std::string DTCLib::DTC::FormatRegister(const DTC_Register& address)
{
	std::ostringstream o;
	o << std::hex << std::setfill('0');
	o << "    0x" << (int)address << "  | 0x" << std::setw(8) << (int)ReadRegister(address) << " ";

	switch (address) {
	case DTC_Register_DesignVersion:
		o << "| DTC Firmware Design Version  | " << ReadDesignVersionNumber();
		break;
	case DTC_Register_DesignDate:
		o << "| DTC Firmware Design Date     | " << ReadDesignDate();
		break;
	case DTC_Register_DTCControl:
		o << "| DTC Control                  | ";
		o << "Reset: [" << (ReadResetDTC() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "CFO Emulation Enable: [" << (ReadCFOEmulation() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "SERDES Oscillator Reset: [" << (ReadResetSERDESOscillator() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "SERDES Oscillator Clock Select : [" << (ReadSERDESOscillatorClock() ? " 2.5Gbs" : "3.125Gbs") << "], " << std::endl;
		o << "                                                        | ";
		o << "System Clock Select : [" << (ReadSystemClock() ? "Ext" : "Int") << "], " << std::endl;
		o << "                                                        | ";
		o << "Timing Enable : [" << (ReadTimingEnable() ? "x" : " ") << "]";
		break;
	case DTC_Register_DMATransferLength:
		o << "| DMA Transfer Length         | ";
		o << "Trigger Length: 0x" << ReadTriggerDMATransferLength() << "," << std::endl;
		o << "                                                       | ";
		o << "Minimum Length : 0x" << ReadMinDMATransferLength();
		break;
	case DTC_Register_SERDESLoopbackEnable:
		o << "| SERDES Loopback Enable       | ";
		for (auto r : DTC_Rings) {
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": " << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString() << "," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    " << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_CFO)).toString();
		break;
	case DTC_Register_SERDESOscillatorStatus:
		o << "| SERDES Oscillator Status     | ";
		o << "IIC Error: [" << (ReadSERDESOscillatorIICError() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "Init.Complete: [" << (ReadSERDESOscillatorInitializationComplete() ? "x" : " ") << "]";
		break;
	case DTC_Register_ROCEmulationEnable:
		o << "| ROC Emulator Enable          | ";
		for (auto r : DTC_Rings) {
			if ((int)r > 0) { o << "," << std::endl << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadROCEmulator(r) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_RingEnable:
		o << "| Ring Enable                  | ([TX,RX,Timing])" << std::endl;
		for (auto r : DTC_Rings) {
			DTC_RingEnableMode re = ReadRingEnabled(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (re.TransmitEnable ? "x" : " ") << ",";
			o << (re.ReceiveEnable ? "x" : " ") << ",";
			o << (re.TimingEnable ? "x" : " ") << "]," << std::endl;
		}
		{
			DTC_RingEnableMode ce = ReadRingEnabled(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << "TX:[" << (ce.TransmitEnable ? "x" : " ") << "], ";
			o << "RX:[" << (ce.ReceiveEnable ? "x" : " ") << "]]";
		}
		break;
	case DTC_Register_SERDESReset:
		o << "| SERDES Reset                 | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadResetSERDES(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadResetSERDES(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXDisparityError:
		o << "| SERDES RX Disparity Error    | ([H,L])" << std::endl;
		for (auto r : DTC_Rings) {
			o << "                                                        | ";
			DTC_SERDESRXDisparityError re = ReadSERDESRXDisparityError(r);
			o << "Ring " << (int)r << ": [";
			o << re.GetData()[1] << ",";
			o << re.GetData()[0] << "]," << std::endl;
		}
		{
			DTC_SERDESRXDisparityError ce = ReadSERDESRXDisparityError(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << ce.GetData()[1] << ",";
			o << ce.GetData()[0] << "]";
		}
		break;
	case DTC_Register_SERDESRXCharacterNotInTableError:
		o << "| SERDES RX CNIT Error         | ([H,L])" << std::endl;
		for (auto r : DTC_Rings) {
			auto re = ReadSERDESRXCharacterNotInTableError(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << re.GetData()[1] << ",";
			o << re.GetData()[0] << "]," << std::endl;
		}
		{
			auto ce = ReadSERDESRXCharacterNotInTableError(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << ce.GetData()[1] << ",";
			o << ce.GetData()[0] << "]";
		}
		break;
	case DTC_Register_SERDESUnlockError:
		o << "| SERDES Unlock Error          | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadSERDESUnlockError(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESUnlockError(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESPLLLocked:
		o << "| SERDES PLL Locked            | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadSERDESPLLLocked(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESPLLLocked(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESTXBufferStatus:
		o << "| SERDES TX Buffer Status      | ([OF or UF, FIFO Half Full])" << std::endl;
		for (auto r : DTC_Rings)
		{
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (ReadSERDESOverflowOrUnderflow(r) ? "x" : " ") << ",";
			o << (ReadSERDESBufferFIFOHalfFull(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [";
		o << (ReadSERDESOverflowOrUnderflow(DTC_Ring_CFO) ? "x" : " ") << ",";
		o << (ReadSERDESBufferFIFOHalfFull(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXBufferStatus:
		o << "| SERDES RX Buffer Status      | ";
		for (auto r : DTC_Rings) {
			auto re = ReadSERDESRXBufferStatus(r);
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": " << DTC_RXBufferStatusConverter(re).toString() << "," << std::endl;
		}
		{
			auto ce = ReadSERDESRXBufferStatus(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    " << DTC_RXBufferStatusConverter(ce).toString();
		}
		break;
	case DTC_Register_SERDESRXStatus:
		o << "| SERDES RX Status             | ";
		for (auto r : DTC_Rings) {
			if ((int)r > 0) { o << "                                                        | "; }
			auto re = ReadSERDESRXStatus(r);
			o << "Ring " << (int)r << ": " << DTC_RXStatusConverter(re).toString() << "," << std::endl;
		}
		{
			auto ce = ReadSERDESRXStatus(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    " << DTC_RXStatusConverter(ce).toString();
		}
		break;
	case DTC_Register_SERDESResetDone:
		o << "| SERDES Reset Done            | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadResetSERDESDone(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadResetSERDESDone(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESEyescanData:
		o << "| SERDES Eyescan Data Error    | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadSERDESEyescanError(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESEyescanError(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXCDRLock:
		o << "| SERDES RX CDR Lock           | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) { o << "                                                        | "; }
			o << "Ring " << (int)r << ": [" << (ReadSERDESRXCDRLock(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESRXCDRLock(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_DMATimeoutPreset:
		o << "| DMA Timeout                  | ";
		o << "0x" << ReadDMATimeoutPreset();
		break;
	case DTC_Register_ROCReplyTimeout:
		o << "| ROC Reply Timeout            | ";
		o << "0x" << ReadROCTimeoutPreset();
		break;
	case DTC_Register_ROCTimeoutError:
		o << "| ROC Reply Timeout Error      | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0) {
				o << ", " << std::endl;
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadROCTimeoutError(r) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_ReceivePacketError:
		o << "| Receive Packet Error         | ([CRC, PacketError, RX Overrun, RX Underrun])" << std::endl;
		for (auto r : DTC_Rings) {
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (ReadPacketCRCError(r) ? "x" : " ") << ",";
			o << (ReadPacketError(r) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferOverrun(r) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferUnderrun(r) ? "x" : " ") << "]," << std::endl;
		}
		{
			o << "                                                        | ";
			o << "CFO:    [";
			o << (ReadPacketCRCError(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadPacketError(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferOverrun(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferUnderrun(DTC_Ring_CFO) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_TimestampPreset0:
		o << "| Timestamp Preset 0           | ";
		o << "0x" << ReadRegister(DTC_Register_TimestampPreset0);
		break;
	case DTC_Register_TimestampPreset1:
		o << "| Timestamp Preset 1           | ";
		o << "0x" << ReadRegister(DTC_Register_TimestampPreset1);
		break;
	case DTC_Register_DataPendingTimer:
		o << "| DMA Data Pending Timer       | ";
		o << "0x" << ReadDataPendingTimer();
		break;
	case DTC_Register_NUMROCs:
		o << "| NUMROCs                      | ";
		for (auto r : DTC_Rings) {
			if ((int)r > 0) {
				o << ", " << std::endl;
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": " << DTC_ROCIDConverter(ReadRingROCCount(r, false));
		}
		break;
	case DTC_Register_FIFOFullErrorFlag0:
		o << "| FIFO Full Error Flags 0      | ([DataRequest, ReadoutRequest, CFOLink, OutputData])";
		for (auto r : DTC_Rings) {
			o << "," << std::endl;
			o << "                                                        | ";
			auto re = ReadFIFOFullErrorFlags(r);
			o << "Ring " << (int)r << ": [";
			o << (re.DataRequestOutput ? "x" : " ") << ",";
			o << (re.ReadoutRequestOutput ? "x" : " ") << ",";
			o << (re.CFOLinkInput ? "x" : " ") << ",";
			o << (re.OutputData ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_FIFOFullErrorFlag1:
		o << "| FIFO Full Error Flags 1      | ([DataInput, OutputDCSStage2, OutputDCS, OtherOutput]) " << std::endl;
		for (auto r : DTC_Rings) {
			auto re = ReadFIFOFullErrorFlags(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (re.DataInput ? "x" : " ") << ",";
			o << (re.OutputDCSStage2 ? "x" : " ") << ",";
			o << (re.OutputDCS ? "x" : " ") << ",";
			o << (re.OtherOutput ? "x" : " ") << "]," << std::endl;
		}
		{
			auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << (ce.DataInput ? "x" : " ") << ",";
			o << (ce.OutputDCSStage2 ? "x" : " ") << ",";
			o << (ce.OutputDCS ? "x" : " ") << ",";
			o << (ce.OtherOutput ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_FIFOFullErrorFlag2:
		o << "| FIFO Full Error Flags 2      | ([DCSStatusInput])" << std::endl;
		for (auto r : DTC_Rings) {
			auto re = ReadFIFOFullErrorFlags(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [" << (re.DCSStatusInput ? "x" : " ") << "]," << std::endl;
		}
		{
			auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [" << (ce.DCSStatusInput ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_CFOEmulationTimestampLow:
		o << "| CFO Emulation Timestamp Low  | ";
		o << "0x" << ReadRegister(DTC_Register_CFOEmulationTimestampLow);
		break;
	case DTC_Register_CFOEmulationTimestampHigh:
		o << "| CFO Emulation Timestamp High | ";
		o << "0x" << ReadRegister(DTC_Register_CFOEmulationTimestampHigh);
		break;
	case DTC_Register_CFOEmulationRequestInterval:
		o << "| CFO Emu. Request Interval    | ";
		o << "0x" << ReadCFOEmulationRequestInterval();
		break;
	case DTC_Register_CFOEmulationNumRequests:
		o << "| CFO Emulator Number Requests | ";
		o << "0x" << ReadCFOEmulationNumRequests();
		break;
	case DTC_Register_CFOEmulationNumPackets:
		o << "| CFO Emulator Number Packets  | ";
		o << "0x" << ReadCFOEmulationNumPackets();
		break;
	case DTC_Register_PacketSize:
		o << "| DMA Packet Size              | ";
		o << "0x" << ReadPacketSize();
		break;
	case DTC_Register_FPGAPROMProgramStatus:
		o << "| FPGA PROM Program Status     | ";
		o << "FPGA PROM Program FIFO Full: [" << (ReadFPGAPROMProgramFIFOFull() ? "x" : " ") << "]" << std::endl;
		o << "                                                        | ";
		o << "FPGA PROM Ready: [" << (ReadFPGAPROMReady() ? "x" : " ") << "]";
		break;
	case DTC_Register_FPGACoreAccess:
		o << "| FPGA Core Access             | ";
		o << "FPGA Core Access FIFO Full: [" << (ReadFPGACoreAccessFIFOFull() ? "x" : " ") << "]" << std::endl;
		o << "                                                        | ";
		o << "FPGA Core Access FIFO Empty: [" << (ReadFPGACoreAccessFIFOEmpty() ? "x" : " ") << "]";
		break;
	case DTC_Register_Invalid:
	default:
		o << "| Invalid Register             | !!!";
		break;
	}
	o << std::endl;
	return o.str();
}

std::string DTCLib::DTC::RegisterRead(const DTC_Register& address)
{
	uint32_t data = ReadRegister(address);
	std::stringstream stream;
	stream << std::hex << data;
	return std::string(stream.str());
}

std::string DTCLib::DTC::ReadDesignVersion()
{
	return ReadDesignVersionNumber() + "_" + ReadDesignDate();
}
std::string DTCLib::DTC::ReadDesignDate()
{
	uint32_t data = ReadRegister(DTC_Register_DesignDate);
	std::ostringstream o;
	int yearHex = (data & 0xFF000000) >> 24;
	int year = ((yearHex & 0xF0) >> 4) * 10 + (yearHex & 0xF);
	int monthHex = (data & 0xFF0000) >> 16;
	int month = ((monthHex & 0xF0) >> 4) * 10 + (monthHex & 0xF);
	int dayHex = (data & 0xFF00) >> 8;
	int day = ((dayHex & 0xF0) >> 4) * 10 + (dayHex & 0xF);
	int hour = ((data & 0xF0) >> 4) * 10 + (data & 0xF);
	o << "20" << std::setfill('0') << std::setw(2) << year << "-";
	o << std::setfill('0') << std::setw(2) << month << "-";
	o << std::setfill('0') << std::setw(2) << day << "-";
	o << std::setfill('0') << std::setw(2) << hour;
	//std::cout << o.str() << std::endl;
	return o.str();
}
std::string DTCLib::DTC::ReadDesignVersionNumber()
{
	uint32_t data = ReadRegister(DTC_Register_DesignVersion);
	int minor = data & 0xFF;
	int major = (data & 0xFF00) >> 8;
	return "v" + std::to_string(major) + "." + std::to_string(minor);
}

void DTCLib::DTC::ResetDTC()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[31] = 1; // DTC Reset bit
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}
bool DTCLib::DTC::ReadResetDTC()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_DTCControl);
	return dataSet[31];
}

void DTCLib::DTC::EnableCFOEmulation()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[30] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}
void DTCLib::DTC::DisableCFOEmulation()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[30] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}
bool DTCLib::DTC::ReadCFOEmulation()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_DTCControl);
	return dataSet[30];
}

void DTCLib::DTC::ResetSERDESOscillator() {
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[29] = 1; //SERDES Oscillator Reset bit
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	usleep(2);
	data[29] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	for (auto ring : DTC_Rings)
	{
		ResetSERDES(ring);
	}
}
bool DTCLib::DTC::ReadResetSERDESOscillator()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[29];
}
void DTCLib::DTC::ToggleSERDESOscillatorClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(28);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);

	ResetSERDESOscillator();
}
bool DTCLib::DTC::ReadSERDESOscillatorClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[28];
}

bool DTCLib::DTC::SetExternalSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[1] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}
bool DTCLib::DTC::SetInternalSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[1] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}
bool DTCLib::DTC::ToggleSystemClockEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(1);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}
bool DTCLib::DTC::ReadSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[1];
}
bool DTCLib::DTC::EnableTiming()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[0] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}
bool DTCLib::DTC::DisableTiming()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[0] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}
bool DTCLib::DTC::ToggleTimingEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(0);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}
bool DTCLib::DTC::ReadTimingEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[0];
}

int DTCLib::DTC::SetTriggerDMATransferLength(uint16_t length)
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = (data & 0x0000FFFF) + (length << 16);
	WriteRegister(data, DTC_Register_DMATransferLength);
	return ReadTriggerDMATransferLength();
}
uint16_t DTCLib::DTC::ReadTriggerDMATransferLength()
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data >>= 16;
	return static_cast<uint16_t>(data);
}

int DTCLib::DTC::SetMinDMATransferLength(uint16_t length)
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = (data & 0xFFFF0000) + length;
	WriteRegister(data, DTC_Register_DMATransferLength);
	return ReadMinDMATransferLength();
}
uint16_t DTCLib::DTC::ReadMinDMATransferLength()
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = data & 0x0000FFFF;
	dmaSize_ = static_cast<uint16_t>(data);
	return dmaSize_;
}

DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC::SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESLoopbackEnable);
	std::bitset<3> modeSet = mode;
	data[3 * ring] = modeSet[0];
	data[3 * ring + 1] = modeSet[1];
	data[3 * ring + 2] = modeSet[2];
	WriteRegister(data.to_ulong(), DTC_Register_SERDESLoopbackEnable);
	return ReadSERDESLoopback(ring);
}
DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESLoopbackEnable) >> (3 * ring));
	return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}

bool DTCLib::DTC::ReadSERDESOscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESOscillatorStatus);
	return dataSet[2];
}

bool DTCLib::DTC::ReadSERDESOscillatorInitializationComplete()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESOscillatorStatus);
	return dataSet[1];
}


bool DTCLib::DTC::EnableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 1;
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}
bool DTCLib::DTC::DisableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 0;
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}
bool DTCLib::DTC::ToggleROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = !dataSet[ring];
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}
bool DTCLib::DTC::ReadROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	return dataSet[ring];
}

DTCLib::DTC_RingEnableMode DTCLib::DTC::EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	data[ring] = mode.TransmitEnable;
	data[ring + 8] = mode.ReceiveEnable;
	data[ring + 16] = mode.TimingEnable;
	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	SetMaxROCNumber(ring, lastRoc);
	return ReadRingEnabled(ring);
}
DTCLib::DTC_RingEnableMode DTCLib::DTC::DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	data[ring] = data[ring] && !mode.TransmitEnable;
	data[ring + 8] = data[ring + 8] && !mode.ReceiveEnable;
	data[ring + 16] = data[ring + 16] && !mode.TimingEnable;
	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	return ReadRingEnabled(ring);
}
DTCLib::DTC_RingEnableMode DTCLib::DTC::ToggleRingEnabled(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	if (mode.TransmitEnable) { data.flip((uint8_t)ring); }
	if (mode.ReceiveEnable) { data.flip((uint8_t)ring + 8); }
	if (mode.TimingEnable) { data.flip((uint8_t)ring + 16); }

	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	return ReadRingEnabled(ring);
}
DTCLib::DTC_RingEnableMode DTCLib::DTC::ReadRingEnabled(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_RingEnable);
	return DTC_RingEnableMode(dataSet[ring], dataSet[ring + 8], dataSet[ring + 16]);
}

bool DTCLib::DTC::ResetSERDES(const DTC_Ring_ID& ring, int interval)
{
	bool resetDone = false;
	while (!resetDone)
	{
		TRACE(0, "Entering SERDES Reset Loop");
		std::bitset<32> data = ReadRegister(DTC_Register_SERDESReset);
		data[ring] = 1;
		WriteRegister(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		data = ReadRegister(DTC_Register_SERDESReset);
		data[ring] = 0;
		WriteRegister(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		resetDone = ReadResetSERDESDone(ring);
		TRACE(0, "End of SERDES Reset loop, done %s", (resetDone ? "true" : "false"));
	}
	return resetDone;
}
bool DTCLib::DTC::ReadResetSERDES(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESReset);
	return dataSet[ring];
}
bool DTCLib::DTC::ReadResetSERDESDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESResetDone);
	return dataSet[ring];
}

DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXDisparityError(ReadRegister(DTC_Register_SERDESRXDisparityError), ring);
}
DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC::ClearSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESRXDisparityError);
	data[ring * 2] = 1;
	data[ring * 2 + 1] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_SERDESRXDisparityError);
	return ReadSERDESRXDisparityError(ring);
}
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	return DTC_CharacterNotInTableError(ReadRegister(DTC_Register_SERDESRXCharacterNotInTableError), ring);
}
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC::ClearSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESRXCharacterNotInTableError);
	data[ring * 2] = 1;
	data[ring * 2 + 1] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_SERDESRXCharacterNotInTableError);

	return ReadSERDESRXCharacterNotInTableError(ring);
}

bool DTCLib::DTC::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESUnlockError);
	return dataSet[ring];
}
bool DTCLib::DTC::ClearSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESUnlockError);
	data[ring] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_SERDESUnlockError);
	return ReadSERDESUnlockError(ring);
}
bool DTCLib::DTC::ReadSERDESPLLLocked(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESPLLLocked);
	return dataSet[ring];
}
bool DTCLib::DTC::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2 + 1];
}
bool DTCLib::DTC::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2];
}

DTCLib::DTC_RXBufferStatus DTCLib::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESRXBufferStatus) >> (3 * ring));
	return static_cast<DTC_RXBufferStatus>(dataSet.to_ulong());
}

DTCLib::DTC_RXStatus DTCLib::DTC::ReadSERDESRXStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESRXStatus) >> (3 * ring));
	return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

bool DTCLib::DTC::ReadSERDESEyescanError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESEyescanData);
	return dataSet[ring];
}
bool DTCLib::DTC::ClearSERDESEyescanError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESEyescanData);
	data[ring] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_SERDESEyescanData);
	return ReadSERDESEyescanError(ring);
}
bool DTCLib::DTC::ReadSERDESRXCDRLock(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESRXCDRLock);
	return dataSet[ring];
}

int DTCLib::DTC::WriteDMATimeoutPreset(uint32_t preset)
{
	WriteRegister(preset, DTC_Register_DMATimeoutPreset);
	return ReadDMATimeoutPreset();
}
uint32_t DTCLib::DTC::ReadDMATimeoutPreset()
{
	return ReadRegister(DTC_Register_DMATimeoutPreset);
}
int DTCLib::DTC::WriteDataPendingTimer(uint32_t timer)
{
	WriteRegister(timer, DTC_Register_DataPendingTimer);
	return ReadDataPendingTimer();
}
uint32_t DTCLib::DTC::ReadDataPendingTimer()
{
	return ReadRegister(DTC_Register_DataPendingTimer);
}
int DTCLib::DTC::SetPacketSize(uint16_t packetSize)
{
	WriteRegister(0x00000000 + packetSize, DTC_Register_PacketSize);
	return ReadPacketSize();
}
uint16_t DTCLib::DTC::ReadPacketSize()
{
	return static_cast<uint16_t>(ReadRegister(DTC_Register_PacketSize));
}

DTCLib::DTC_ROC_ID DTCLib::DTC::SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> ringRocs = ReadRegister(DTC_Register_NUMROCs);
	maxROCs_[ring] = lastRoc;
	int numRocs = (lastRoc == DTC_ROC_Unused) ? 0 : lastRoc + 1;
	ringRocs[ring * 3] = numRocs & 1;
	ringRocs[ring * 3 + 1] = ((numRocs & 2) >> 1) & 1;
	ringRocs[ring * 3 + 2] = ((numRocs & 4) >> 2) & 1;
	WriteRegister(ringRocs.to_ulong(), DTC_Register_NUMROCs);
	return ReadRingROCCount(ring);
}

DTCLib::DTC_ROC_ID DTCLib::DTC::ReadRingROCCount(const DTC_Ring_ID& ring, bool local)
{
	if (local) {
		return maxROCs_[ring];
	}
	std::bitset<32> ringRocs = ReadRegister(DTC_Register_NUMROCs);
	int number = ringRocs[ring * 3] + (ringRocs[ring * 3 + 1] << 1) + (ringRocs[ring * 3 + 2] << 2);
	return DTC_ROCS[number];
}


DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC::WriteFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);

	data0[ring] = flags.OutputData;
	data0[ring + 8] = flags.CFOLinkInput;
	data0[ring + 16] = flags.ReadoutRequestOutput;
	data0[ring + 24] = flags.DataRequestOutput;
	data1[ring] = flags.OtherOutput;
	data1[ring + 8] = flags.OutputDCS;
	data1[ring + 16] = flags.OutputDCSStage2;
	data1[ring + 24] = flags.DataInput;
	data2[ring] = flags.DCSStatusInput;

	WriteRegister(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);

	return ReadFIFOFullErrorFlags(ring);
}

DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC::ToggleFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);

	data0[ring] = flags.OutputData ? !data0[ring] : data0[ring];
	data0[ring + 8] = flags.CFOLinkInput ? !data0[ring + 8] : data0[ring + 8];
	data0[ring + 16] = flags.ReadoutRequestOutput ? !data0[ring + 16] : data0[ring + 16];
	data0[ring + 24] = flags.DataRequestOutput ? !data0[ring + 24] : data0[ring + 24];
	data1[ring] = flags.OtherOutput ? !data1[ring] : data1[ring];
	data1[ring + 8] = flags.OutputDCS ? !data1[ring + 8] : data1[ring + 8];
	data1[ring + 16] = flags.OutputDCSStage2 ? !data1[ring + 16] : data1[ring + 16];
	data1[ring + 24] = flags.DataInput ? !data1[ring + 24] : data1[ring + 24];
	data2[ring] = flags.DCSStatusInput ? !data2[ring] : data2[ring];

	WriteRegister(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);

	return ReadFIFOFullErrorFlags(ring);
}

DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC::ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);
	DTC_FIFOFullErrorFlags flags;

	flags.OutputData = data0[ring];
	flags.CFOLinkInput = data0[ring + 8];
	flags.ReadoutRequestOutput = data0[ring + 16];
	flags.DataRequestOutput = data0[ring + 24];
	flags.OtherOutput = data1[ring];
	flags.OutputDCS = data1[ring + 8];
	flags.OutputDCSStage2 = data1[ring + 16];
	flags.DataInput = data1[ring + 24];
	flags.DCSStatusInput = data2[ring];

	return flags;

}

uint32_t DTCLib::DTC::ReadROCTimeoutPreset()
{
	return ReadRegister(DTC_Register_ROCReplyTimeout);
}

int DTCLib::DTC::WriteROCTimeoutPreset(uint32_t preset)
{
	WriteRegister(preset, DTC_Register_ROCReplyTimeout);
	return ReadROCTimeoutPreset();
}

bool DTCLib::DTC::ReadROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ROCTimeoutError);
	return data[(int)ring];
}

bool DTCLib::DTC::ClearROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = 0x0;
	data[ring] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_ROCTimeoutError);
	return ReadROCTimeoutError(ring);
}

bool DTCLib::DTC::ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 24];
}

bool DTCLib::DTC::ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 24] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC::ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 16];
}

bool DTCLib::DTC::ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 16] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC::ReadPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 8];
}

bool DTCLib::DTC::ClearPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 8] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC::ReadPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring];
}

bool DTCLib::DTC::ClearPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

DTCLib::DTC_Timestamp DTCLib::DTC::WriteTimestampPreset(const DTC_Timestamp& preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister(timestampLow, DTC_Register_TimestampPreset0);
	WriteRegister(timestampHigh, DTC_Register_TimestampPreset1);
	return ReadTimestampPreset();
}
DTCLib::DTC_Timestamp DTCLib::DTC::ReadTimestampPreset()
{
	uint32_t timestampLow = ReadRegister(DTC_Register_TimestampPreset0);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister(DTC_Register_TimestampPreset1)));
	return output;
}

void DTCLib::DTC::SetCFOEmulationTimestamp(const DTC_Timestamp& ts)
{
	std::bitset<48> timestamp = ts.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister(timestampLow, DTC_Register_CFOEmulationTimestampLow);
	WriteRegister(timestampHigh, DTC_Register_CFOEmulationTimestampHigh);
}

DTCLib::DTC_Timestamp DTCLib::DTC::ReadCFOEmulationTimestamp()
{
	uint32_t timestampLow = ReadRegister(DTC_Register_CFOEmulationTimestampLow);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationTimestampHigh)));
	return output;
}

bool DTCLib::DTC::ReadFPGAPROMProgramFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[1];
}
bool DTCLib::DTC::ReadFPGAPROMReady()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[0];
}

void DTCLib::DTC::ReloadFPGAFirmware()
{
	WriteRegister(0xFFFFFFFF, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0xAA995566, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x20000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x30020001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x00000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x30008001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x0000000F, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteRegister(0x20000000, DTC_Register_FPGACoreAccess);
}
bool DTCLib::DTC::ReadFPGACoreAccessFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGACoreAccess);
	return dataSet[1];
}
bool DTCLib::DTC::ReadFPGACoreAccessFIFOEmpty()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGACoreAccess);
	return dataSet[0];
}

//
// Private Functions.
//
void DTCLib::DTC::ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms)
{
	mu2e_databuff_t* buffer;
	int retry = 2;
	int errorCode;
	do {
		TRACE(19, "DTC::ReadBuffer before device_.read_data");
		auto start = std::chrono::high_resolution_clock::now();
		errorCode = device_.read_data(channel, (void**)&buffer, tmo_ms);
		deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
			(std::chrono::high_resolution_clock::now() - start).count();
		retry--;
	} while (retry > 0 && errorCode == 0);
	if (errorCode == 0) // timeout
		throw DTC_TimeoutOccurredException();
	else if (errorCode < 0)
		throw DTC_IOErrorException();
	TRACE(16, "DTC::ReadDataPacket buffer_=%p errorCode=%d *buffer_=0x%08x"
		, (void*)buffer, errorCode, *(unsigned*)buffer);
	if (channel == DTC_DMA_Engine_DAQ) { daqbuffer_ = buffer; }
	else if (channel == DTC_DMA_Engine_DCS) { dcsbuffer_ = buffer; }
	//return DTC_DataPacket(buffer);
}
void DTCLib::DTC::WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet)
{
	if (packet.GetSize() < dmaSize_)
	{
		DTC_DataPacket thisPacket(packet);
		thisPacket.Resize(dmaSize_);
		int retry = 3;
		int errorCode = 0;
		do {
			auto start = std::chrono::high_resolution_clock::now();
			errorCode = device_.write_data(channel, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
			deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
				(std::chrono::high_resolution_clock::now() - start).count();
			retry--;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			throw DTC_IOErrorException();
		}
	}
	else {
		int retry = 3;
		int errorCode = 0;
		do {
			auto start = std::chrono::high_resolution_clock::now();
			errorCode = device_.write_data(channel, packet.GetData(), packet.GetSize() * sizeof(uint8_t));
			deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
				(std::chrono::high_resolution_clock::now() - start).count();
			retry--;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			throw DTC_IOErrorException();
		}
	}
}
void DTCLib::DTC::WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet)
{
	DTC_DataPacket dp = packet.ConvertToDataPacket();
	WriteDataPacket(channel, dp);
}

void DTCLib::DTC::WriteRegister(uint32_t data, const DTC_Register& address)
{
	int retry = 3;
	int errorCode = 0;
	do {
		auto start = std::chrono::high_resolution_clock::now();
		errorCode = device_.write_register(address, 100, data);
		deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
			(std::chrono::high_resolution_clock::now() - start).count();
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}
}
uint32_t DTCLib::DTC::ReadRegister(const DTC_Register& address)
{
	int retry = 3;
	int errorCode = 0;
	uint32_t data;
	do {
		auto start = std::chrono::high_resolution_clock::now();
		errorCode = device_.read_register(address, 100, &data);
		deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
			(std::chrono::high_resolution_clock::now() - start).count();
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}

	return data;
}

