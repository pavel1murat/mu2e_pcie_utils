#include "DTC.h"
#ifndef _WIN32
#include <unistd.h>
#else
#include <chrono>
#include <thread>
#define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#endif

DTC::DTC::DTC() : DTC_BUFFSIZE(sizeof(mu2e_databuff_t) / (16 * sizeof(uint8_t))), device_()
{
	device_.init();
}

//
// DMA Functions
//
std::vector<void*> DTC::DTC::GetData(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_Timestamp& when, int* length)
{
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

	*length = packet.GetPacketCount();
	output.push_back(buffer_);

	while (DTC_BUFFSIZE * output.size() < packet.GetPacketCount())
	{
		dmaPacket = ReadDMADAQPacket();
		output.push_back(buffer_);
	}

	return output;
}

void DTC::DTC::DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t* dataIn)
{
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
uint32_t DTC::DTC::ReadDesignVersion()
{
	return ReadRegister(DesignVersionRegister);
}

void DTC::DTC::ResetDTC()
{
	std::bitset<32> data = ReadControlRegister();
	data[31] = 1; // DTC Reset bit
	WriteControlRegister(data.to_ulong());
}

bool DTC::DTC::ToggleClearLatchedErrors()
{
	std::bitset<32> data = ReadControlRegister();
	data.flip(30); // Clear Latched Errors bit
	WriteControlRegister(data.to_ulong());
	return ReadClearLatchedErrors();
}
bool DTC::DTC::ReadClearLatchedErrors()
{
	std::bitset<32> data = ReadControlRegister();
	return data[30];
}

bool DTC::DTC::ToggleSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadSERDESLoopbackEnableRegister();
	data.flip((uint8_t)ring);
	WriteSERDESLoopbackEnableRegister(data.to_ulong());
	return ReadSERDESLoopback(ring);
}
bool DTC::DTC::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESLoopbackEnableRegister();
	return dataSet[ring];
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
		std::bitset<32> data = ReadSERDESResetRegister();
		data[ring] = 1;
		WriteSERDESResetRegister(data.to_ulong());

		usleep(interval);

		data = ReadSERDESResetRegister();
		data[ring] = 0;
		WriteSERDESResetRegister(data.to_ulong());

		resetDone = ReadSERDESResetDone(ring);
	}
	return resetDone;
}

DTC::DTC_SERDESRXDisparityError DTC::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXDisparityError(ReadSERDESRXDisparityErrorRegister(), ring);
}
DTC::DTC_CharacterNotInTableError DTC::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	return DTC_CharacterNotInTableError(ReadSERDESRXCharacterNotInTableErrorRegister(), ring);
}

bool DTC::DTC::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESUnlockErrorRegister();
	return dataSet[ring];
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

DTC::DTC_SERDESRXBufferStatus DTC::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXBufferStatus(ReadSERDESRXBufferStatusRegister(), ring);
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
std::vector<DTC::DTC_DMAStat> DTC::DTC::ReadDMAStats(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir)
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

	return DTC_DMAStats(stats).getData(dma, dir);
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
		errorCode = device_.read_data(channel, (void**)buffer_, 1000);
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
	throw new DTC_NotImplementedException();
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

void DTC::DTC::WriteRegister(uint32_t data, uint16_t address)
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
uint32_t DTC::DTC::ReadRegister(uint16_t address)
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
void DTC::DTC::WriteControlRegister(uint32_t data)
{
	WriteRegister(data, DTCControlRegister);
}
uint32_t DTC::DTC::ReadControlRegister()
{
	return ReadRegister(DTCControlRegister);
}
void DTC::DTC::WriteSERDESLoopbackEnableRegister(uint32_t data)
{
	WriteRegister(data, SERDESLoopbackEnableRegister);
}
uint32_t DTC::DTC::ReadSERDESLoopbackEnableRegister()
{
	return ReadRegister(SERDESLoopbackEnableRegister);
}
void DTC::DTC::WriteROCEmulationEnableRegister(uint32_t data)
{
	WriteRegister(data, ROCEmulationEnableRegister);
}
uint32_t DTC::DTC::ReadROCEmulationEnableRegister()
{
	return ReadRegister(ROCEmulationEnableRegister);
}
void DTC::DTC::WriteRingEnableRegister(uint32_t data)
{
	WriteRegister(data, RingEnableRegister);
}
uint32_t DTC::DTC::ReadRingEnableRegister()
{
	return ReadRegister(RingEnableRegister);
}
void DTC::DTC::WriteSERDESResetRegister(uint32_t data)
{
	WriteRegister(data, SERDESResetRegister);
}
uint32_t DTC::DTC::ReadSERDESResetRegister()
{
	return ReadRegister(SERDESResetRegister);
}
uint32_t DTC::DTC::ReadSERDESRXDisparityErrorRegister()
{
	return ReadRegister(SERDESRXDisparityErrorRegister);
}
uint32_t DTC::DTC::ReadSERDESRXCharacterNotInTableErrorRegister()
{
	return ReadRegister(SERDESRXCharacterNotInTableErrorRegister);
}
uint32_t DTC::DTC::ReadSERDESUnlockErrorRegister()
{
	return ReadRegister(SERDESUnlockErrorRegister);
}
uint32_t DTC::DTC::ReadSERDESPLLLockedRegister()
{
	return ReadRegister(SERDESPLLLockedRegister);
}
uint32_t DTC::DTC::ReadSERDESTXBufferStatusRegister()
{
	return ReadRegister(SERDESTXBufferStatusRegister);
}
uint32_t DTC::DTC::ReadSERDESRXBufferStatusRegister()
{
	return ReadRegister(SERDESRXBufferStatusRegister);
}
uint32_t DTC::DTC::ReadSERDESResetDoneRegister()
{
	return ReadRegister(SERDESResetDoneRegister);
}
bool DTC::DTC::ReadSERDESResetDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
	return dataSet[ring];
}
void DTC::DTC::WriteTimestampPreset0Register(uint32_t data)
{
	WriteRegister(data, TimestampPreset0Register);
}
uint32_t DTC::DTC::ReadTimestampPreset0Register()
{
	return ReadRegister(TimestampPreset0Register);
}
void DTC::DTC::WriteTimestampPreset1Register(uint32_t data)
{
	WriteRegister(data, TimestampPreset1Register);
}
uint32_t DTC::DTC::ReadTimestampPreset1Register()
{
	return ReadRegister(TimestampPreset1Register);
}
void DTC::DTC::WriteFPGAPROMProgramDataRegister(uint32_t data)
{
	WriteRegister(data, FPGAPROMProgramDataRegister);
}
uint32_t DTC::DTC::ReadFPGAPROMProgramStatusRegister()
{
	return ReadRegister(FPGAPROMProgramStatusRegister);
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
