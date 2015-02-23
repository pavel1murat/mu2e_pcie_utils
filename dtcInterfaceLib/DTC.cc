#include "DTC.h"
#include <sstream> // Convert uint to hex string
#ifndef _WIN32
#include <unistd.h>
#include "trace.h"
#else
#include <chrono>
#include <thread>
#define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#define TRACE(...) 
#endif

DTC::DTC::DTC() : DTC_BUFFSIZE(sizeof(mu2e_databuff_t) / (16 * sizeof(uint8_t))), device_(), buffer_(nullptr)
{
#ifdef _WIN32
	simMode_ = true;
#else
	char* sim = getenv("DTCLIB_SIM_ENABLE");
	simMode_ = sim != NULL && sim[0] == '1';
#endif
	simMode_ = (bool)device_.init(simMode_);
}

//
// DMA Functions
//
std::vector<void*> DTC::DTC::GetData(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_Timestamp& when, int* length)
{
	device_.release_all(0);
	std::vector<void*> output;
	// Send a data request
	DTC_DataRequestPacket req(ring, roc, when);
	WriteDMADAQPacket(req);

	// Read the header packet
	DTC_DMAPacket dmaPacket = ReadDMADAQPacket();
	if (dmaPacket.GetPacketType() != DTC_PacketType_DataHeader)
	{
		throw DTC_WrongPacketTypeException();
	}
	DTC_DataHeaderPacket packet(dmaPacket);

	*length = packet.GetPacketCount() + 1;
	output.push_back(buffer_);

	while (DTC_BUFFSIZE * output.size() < packet.GetPacketCount() + 1U)
	{
		dmaPacket = ReadDMADAQPacket();
		output.push_back(buffer_);
	}

	return output;
}

void DTC::DTC::DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t* dataIn)
{
	device_.release_all(1);
	DTC_DCSRequestPacket req(ring, roc, dataIn);
	WriteDMADCSPacket(req);
	DTC_DMAPacket packet = ReadDMADCSPacket();
	if (packet.GetPacketType() != DTC_PacketType_DCSReply)
	{
		throw DTC_WrongPacketTypeException();
	}

	DTC_DCSReplyPacket dcsPacket(packet);
	for (int ii = 0; ii < 12; ++ii) {
		dataIn[ii] = dcsPacket.GetData()[ii];
	}
}

void DTC::DTC::SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when)
{
	DTC_ReadoutRequestPacket req(ring, when, maxRocs_[ring]);
	WriteDMADAQPacket(req);
}

void DTC::DTC::SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
	maxRocs_[ring] = lastRoc;
}

//
// Register IO Functions
//
std::string DTC::DTC::RegisterRead(const DTC_Register& address)
{
	uint32_t data = ReadRegister(address);
	std::stringstream stream;
	stream << std::hex << data;
	return std::string(stream.str());
}

std::string DTC::DTC::ReadDesignVersion()
{
	return RegisterRead(DTC_Register_DesignVersion);
}

void DTC::DTC::ResetDTC()
{
	std::bitset<32> data = ReadControlRegister();
	data[31] = 1; // DTC Reset bit
	WriteControlRegister(data.to_ulong());
}
bool DTC::DTC::ReadResetDTC()
{
	std::bitset<32> dataSet = ReadControlRegister();
	return dataSet[31];
}

bool DTC::DTC::ToggleCFOEmulation()
{
	std::bitset<32> data = ReadControlRegister();
	data.flip(1); // Clear Latched Errors bit
	WriteControlRegister(data.to_ulong());
	return ReadCFOEmulation();
}
bool DTC::DTC::ReadCFOEmulation()
{
	std::bitset<32> data = ReadControlRegister();
	return data[1];
}

int DTC::DTC::SetTriggerDMATransferLength(uint16_t length)
{
	uint32_t data = ReadDMATransferLengthRegister();
	data = (data & 0x0000FFFF) + (length << 16);
	WriteDMATransferLengthRegister(data);
	return ReadTriggerDMATransferLength();
}
uint16_t DTC::DTC::ReadTriggerDMATransferLength()
{
	uint32_t data = ReadDMATransferLengthRegister();
	data >>= 16;
	return static_cast<uint16_t>(data);
}

int DTC::DTC::SetMinDMATransferLength(uint16_t length)
{
	uint32_t data = ReadDMATransferLengthRegister();
	data = (data & 0xFFFF0000) + length;
	WriteDMATransferLengthRegister(data);
	return ReadMinDMATransferLength();
}
uint16_t DTC::DTC::ReadMinDMATransferLength()
{
	uint32_t data = ReadDMATransferLengthRegister();
	data = data & 0x0000FFFF;
	return static_cast<uint16_t>(data);
}

DTC::DTC_SERDESLoopbackMode DTC::DTC::SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode)
{
	std::bitset<32> data = ReadSERDESLoopbackEnableRegister();
	std::bitset<3> modeSet = mode;
	data[3 * ring] = modeSet[0];
	data[3 * ring + 1] = modeSet[1];
	data[3 * ring + 2] = modeSet[2];
	WriteSERDESLoopbackEnableRegister(data.to_ulong());

	// Now do the temp register
	data = ReadSERDESLoopbackEnableTempRegister();
	modeSet = mode;
	data[3 * ring] = modeSet[0];
	data[3 * ring + 1] = modeSet[1];
	data[3 * ring + 2] = modeSet[2];
	WriteSERDESLoopbackEnableTempRegister(data.to_ulong());
	return ReadSERDESLoopback(ring);
}
DTC::DTC_SERDESLoopbackMode DTC::DTC::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadSERDESLoopbackEnableRegister() >> (3 * ring));
	if (dataSet == 0) {
		dataSet = (ReadSERDESLoopbackEnableTempRegister() >> (3 * ring));
	}
	return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}

bool DTC::DTC::ToggleROCEmulator()
{
	bool enabled = ReadROCEmulator();
	uint32_t value = enabled ? 0UL : 1UL;
	WriteROCEmulationEnableRegister(value);
	return ReadROCEmulator();
}
bool DTC::DTC::ReadROCEmulator()
{
	std::bitset<32> dataSet = ReadROCEmulationEnableRegister();
	return dataSet[0];
}

bool DTC::DTC::EnableRing(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
	if (lastRoc != DTC_ROC_Unused)
	{
		maxRocs_[ring] = lastRoc;
	}
	std::bitset<32> data = ReadRingEnableRegister();
	data[ring] = 1;
	WriteRingEnableRegister(data.to_ulong());
	return ReadRingEnabled(ring);
}
bool DTC::DTC::DisableRing(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRingEnableRegister();
	data[ring] = 0;
	WriteRingEnableRegister(data.to_ulong());
	return ReadRingEnabled(ring);
}
bool DTC::DTC::ToggleRingEnabled(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRingEnableRegister();
	data.flip((uint8_t)ring);
	WriteRingEnableRegister(data.to_ulong());
	return ReadRingEnabled(ring);
}
bool DTC::DTC::ReadRingEnabled(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRingEnableRegister();
	return dataSet[ring];
}

bool DTC::DTC::ResetSERDES(const DTC_Ring_ID& ring, int interval)
{
	bool resetDone = false;
	while (!resetDone)
	{
		TRACE(0, "Entering SERDES Reset Loop");
		std::bitset<32> data = ReadSERDESResetRegister();
		data[ring] = 1;
		WriteSERDESResetRegister(data.to_ulong());

		usleep(interval);

		data = ReadSERDESResetRegister();
		data[ring] = 0;
		WriteSERDESResetRegister(data.to_ulong());

		resetDone = ReadSERDESResetDone(ring);
		TRACE(0, "End of SERDES Reset loop, done %d", resetDone);
	}
	return resetDone;
}
bool DTC::DTC::ReadResetSERDES(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESResetRegister();
	return dataSet[ring];
}
bool DTC::DTC::ReadResetSERDESDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
	return dataSet[ring];
}

DTC::DTC_SERDESRXDisparityError DTC::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXDisparityError(ReadSERDESRXDisparityErrorRegister(), ring);
}
DTC::DTC_SERDESRXDisparityError DTC::DTC::ClearSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadSERDESRXDisparityErrorRegister();
	data[ring * 2] = 1;
	data[ring * 2 + 1] = 1;
	WriteSERDESRXDisparityErrorRegister(data.to_ulong());
	return ReadSERDESRXDisparityError(ring);
}
DTC::DTC_CharacterNotInTableError DTC::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	return DTC_CharacterNotInTableError(ReadSERDESRXCharacterNotInTableErrorRegister(), ring);
}
DTC::DTC_CharacterNotInTableError DTC::DTC::ClearSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadSERDESRXCharacterNotInTableErrorRegister();
	data[ring * 2] = 1;
	data[ring * 2 + 1] = 1;
	WriteSERDESRXCharacterNotInTableErrorRegister(data.to_ulong());

	return ReadSERDESRXCharacterNotInTableError(ring);
}

bool DTC::DTC::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESUnlockErrorRegister();
	return dataSet[ring];
}
bool DTC::DTC::ClearSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadSERDESUnlockErrorRegister();
	data[ring] = 1;
	WriteSERDESUnlockErrorRegister(data.to_ulong());
	return ReadSERDESUnlockError(ring);
}
bool DTC::DTC::ReadSERDESPLLLocked(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESPLLLockedRegister();
	return dataSet[ring];
}
bool DTC::DTC::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
	return dataSet[ring * 2 + 1];
}
bool DTC::DTC::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
	return dataSet[ring * 2];
}

DTC::DTC_RXBufferStatus DTC::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadSERDESRXBufferStatusRegister() >> (3 * ring));
	return static_cast<DTC_RXBufferStatus>(dataSet.to_ulong());
}

DTC::DTC_RXStatus DTC::DTC::ReadSERDESRXStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadSERDESRXStatusRegister() >> (3 * ring));
	return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

bool DTC::DTC::ReadSERDESEyescanError(const DTC_Ring_ID& ring) 
{
	std::bitset<32> dataSet = ReadSERDESEyescanErrorRegister();
	return dataSet[ring];
}
bool DTC::DTC::ClearSERDESEyescanError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadSERDESEyescanErrorRegister();
	data[ring] = 1;
	WriteSERDESEyescanErrorRegister(data.to_ulong());
	return ReadSERDESEyescanError(ring);
}
bool DTC::DTC::ReadSERDESRXCDRLock(const DTC_Ring_ID& ring) 
{
	std::bitset<32> dataSet = ReadSERDESRXCDRLockRegister();
	return dataSet[ring];
}

int DTC::DTC::WriteDMATimeoutPreset(uint32_t preset) 
{
	WriteDMATimeoutPresetRegister(preset);
	return ReadDMATimeoutPreset();
}
uint32_t DTC::DTC::ReadDMATimeoutPreset()
{
	return ReadDMATimeoutPresetRegister();
}
int DTC::DTC::WriteDataPendingTimer(uint32_t timer)
{
	WriteDataPendingTimerRegister(timer);
	return ReadDataPendingTimer();
}
uint32_t DTC::DTC::ReadDataPendingTimer()
{
	return ReadDataPendingTimerRegister();
}
int DTC::DTC::SetPacketSize(uint16_t packetSize)
{
	WriteDMAPacketSizetRegister(0x00000000 + packetSize);
	return ReadPacketSize();
}
uint16_t DTC::DTC::ReadPacketSize()
{
	return static_cast<uint16_t>(ReadDMAPacketSizeRegister());
}

DTC::DTC_Timestamp DTC::DTC::WriteTimestampPreset(const DTC_Timestamp& preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteTimestampPreset0Register(timestampLow);
	WriteTimestampPreset1Register(timestampHigh);
	return ReadTimestampPreset();
}
DTC::DTC_Timestamp DTC::DTC::ReadTimestampPreset()
{
	uint32_t timestampLow = ReadTimestampPreset0Register();
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadTimestampPreset1Register()));
	return output;
}

bool DTC::DTC::ReadFPGAPROMProgramFIFOFull()
{
	std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
	return dataSet[1];
}
bool DTC::DTC::ReadFPGAPROMReady()
{
	std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
	return dataSet[0];
}

void DTC::DTC::ReloadFPGAFirmware() 
{
	WriteFPGACoreAccessRegister(0xFFFFFFFF);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0xAA995566);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x20000000);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x30020001);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x00000000);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x30008001);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x0000000F);
	while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
	WriteFPGACoreAccessRegister(0x20000000);
}
bool DTC::DTC::ReadFPGACoreAccessFIFOFull()
{
	std::bitset<32> dataSet = ReadFPGACoreAccessRegister();
	return dataSet[0];
}

//
// PCIe/DMA Status and Performance
// DMA Testing Engine
//
DTC::DTC_TestMode DTC::DTC::StartTest(const DTC_DMA_Engine& dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator)
{
	DTC_TestCommand testCommand(dma, true, packetSize, loopback, txChecker, rxGenerator);
	WriteTestCommand(testCommand, true);
	return ReadTestCommand().GetMode();
}
DTC::DTC_TestMode DTC::DTC::StopTest(const DTC_DMA_Engine& dma)
{
	WriteTestCommand(DTC_TestCommand(dma), false);
	return ReadTestCommand().GetMode();
}

DTC::DTC_DMAState DTC::DTC::ReadDMAState(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir)
{
	m_ioc_engstate_t state;
	int errorCode = 0;
	int retry = 3;
	do {
		errorCode = device_.read_dma_state(dma, dir, &state);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw DTC_IOErrorException();
	}

	return DTC_DMAState(state);
}
DTC::DTC_DMAStats DTC::DTC::ReadDMAStats(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir)
{
	DMAStatistics statData[100];
	m_ioc_engstats_t stats;
	stats.Count = 100;
	stats.engptr = statData;

	int errorCode = 0;
	int retry = 3;
	do {
		errorCode = device_.read_dma_stats(&stats);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw DTC_IOErrorException();
	}

	return DTC_DMAStats(stats).getData(dma,dir);
}

DTC::DTC_PCIeState DTC::DTC::ReadPCIeState()
{
	m_ioc_pcistate_t state;
	int errorCode = 0;
	int retry = 3;
	do {
		errorCode = device_.read_pcie_state(&state);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0) { throw DTC_IOErrorException(); }
	return DTC_PCIeState(state);
}
DTC::DTC_PCIeStat DTC::DTC::ReadPCIeStats()
{
	TRNStatistics statData[1];
	TRNStatsArray stats;
	stats.Count = 1;
	stats.trnptr = statData;
	int errorCode = 0;
	int retry = 3;
	do {
		errorCode = device_.read_trn_stats(&stats);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0) { throw DTC_IOErrorException(); }
	return DTC_PCIeStat(statData[0]);
}

//
// Private Functions.
//
DTC::DTC_DataPacket DTC::DTC::ReadDataPacket(const DTC_DMA_Engine& channel)
{
	int retry = 3;
	int errorCode = 0;
	do {
		errorCode = device_.read_data(channel, (void**)&buffer_, 1000);
		retry--;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw DTC_IOErrorException();
	}
	return DTC_DataPacket(buffer_);
}
void DTC::DTC::WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet)
{
	uint8_t packetData[16];
	for (int i = 0; i < 16; ++i)
	{
		packetData[i] = packet.GetWord(i);
	}

	int retry = 3;
	int errorCode = 0;
	do {
		errorCode = device_.write_loopback_data(channel, packetData, sizeof(packetData));
		retry--;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw DTC_IOErrorException();
	}
}
DTC::DTC_DMAPacket DTC::DTC::ReadDMAPacket(const DTC_DMA_Engine& channel)
{
	return DTC_DMAPacket(ReadDataPacket(channel));
}
DTC::DTC_DMAPacket DTC::DTC::ReadDMADAQPacket()
{
	return ReadDMAPacket(DTC_DMA_Engine_DAQ);
}
DTC::DTC_DMAPacket DTC::DTC::ReadDMADCSPacket()
{
	return ReadDMAPacket(DTC_DMA_Engine_DCS);
}
void DTC::DTC::WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet)
{
	return WriteDataPacket(channel, packet.ConvertToDataPacket());
}
void DTC::DTC::WriteDMADAQPacket(const DTC_DMAPacket& packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DAQ, packet);
}
void DTC::DTC::WriteDMADCSPacket(const DTC_DMAPacket& packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DCS, packet);
}

void DTC::DTC::WriteRegister(uint32_t data, const DTC_Register& address)
{
	int retry = 3;
	int errorCode = 0;
	do {
		errorCode = device_.write_register(address, 100, data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}
}
uint32_t DTC::DTC::ReadRegister(const DTC_Register& address)
{
	int retry = 3;
	int errorCode = 0;
	uint32_t data;
	do {
		errorCode = device_.read_register(address, 100, &data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}

	return data;
}

bool DTC::DTC::ReadSERDESResetDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
	return dataSet[ring];
}


void DTC::DTC::WriteTestCommand(const DTC_TestCommand& comm, bool start)
{
	int retry = 3;
	int errorCode = 0;
	do {
		errorCode = device_.write_test_command(comm.GetCommand(), start);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}
}
DTC::DTC_TestCommand DTC::DTC::ReadTestCommand()
{
	m_ioc_cmd_t comm;
	int retry = 3;
	int errorCode = 0;
	do {
		errorCode = device_.read_test_command(&comm);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException;
	}
	return DTC_TestCommand(comm);
}
