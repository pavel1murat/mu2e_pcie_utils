#include "DTC.h"
#ifndef _WIN32
#include <unistd.h>
#else
#include <chrono>
#include <thread>
#define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#endif


DTC::DTC::DTC() : device_()
{
	device_.init();
}

DTC::DTC_ErrorCode DTC::DTC::ReadDataPacket(DTC_DMA_Engine channel)
{
	mu2e_databuff_t buffer;
	int errorCode = device_.read_data(channel, (void**)&buffer, 1000);
	dataPacket_ = DTC_DataPacket(buffer, true);
	return errorCode == 0 ? DTC_ErrorCode_Success : DTC_ErrorCode_IOError;
}

DTC::DTC_ErrorCode DTC::DTC::WriteDataPacket(DTC_DMA_Engine channel, DTC_DataPacket packet)
{
	return DTC_ErrorCode_NotImplemented;
}

DTC::DTC_ErrorCode DTC::DTC::ReadDAQDataPacket()
{
	return ReadDataPacket(DTC_DMA_Engine_DAQ);
}
DTC::DTC_ErrorCode DTC::DTC::WriteDAQDataPacket(DTC_DataPacket packet)
{
	return WriteDataPacket(DTC_DMA_Engine_DAQ, packet);
}

DTC::DTC_ErrorCode DTC::DTC::ReadDMAPacket(DTC_DMA_Engine channel)
{
	DTC_ErrorCode err = ReadDataPacket(channel);
	if (err != DTC_ErrorCode_Success) { return err; }
	dmaPacket_ = DTC_DMAPacket(dataPacket_);
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadDMADAQPacket()
{
	return ReadDMAPacket(DTC_DMA_Engine_DAQ);
}
DTC::DTC_ErrorCode DTC::DTC::ReadDMADCSPacket()
{
	return ReadDMAPacket(DTC_DMA_Engine_DCS);
}

DTC::DTC_ErrorCode DTC::DTC::WriteDMAPacket(DTC_DMA_Engine channel, DTC_DMAPacket packet)
{
	return WriteDataPacket(channel, packet.ConvertToDataPacket());
}
DTC::DTC_ErrorCode DTC::DTC::WriteDMADAQPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DAQ, packet);
}
DTC::DTC_ErrorCode DTC::DTC::WriteDMADCSPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(DTC_DMA_Engine_DCS, packet);
}

DTC::DTC_ErrorCode DTC::DTC::GetData(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp when)
{
	dataVector_.clear();
	DTC_ErrorCode err;
	DTC_DataRequestPacket req(ring, roc, when);
	err = WriteDMADAQPacket(req);
	if (err != DTC_ErrorCode_Success) { return err; }

	err = ReadDMADAQPacket();
	if (err != DTC_ErrorCode_Success) { return err; }
	DTC_DataHeaderPacket packet(dmaPacket_);
	if (packet.GetPacketType() != DTC_PacketType_DataHeader)
	{
		return DTC_ErrorCode_WrongPacketType;
	}
	for (int i = 0; i < 6; ++i)
	{
		dataVector_.push_back(packet.GetData()[i]);
	}
	for (int ii = 0; ii < packet.GetPacketCount(); ++ii)
	{
		err = ReadDAQDataPacket();
		if (err != DTC_ErrorCode_Success) { return err; }
		for (int i = 0; i < 16; ++i)
		{
			dataVector_.push_back(dataPacket_.GetWord(i));
		}
	}
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::DCSRequestReply(DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t dataIn[12])
{
	dataVector_.clear();
	DTC_ErrorCode err;
	DTC_DCSRequestPacket req(ring, roc, dataIn);
	err = WriteDMADCSPacket(req);
	if (err != DTC_ErrorCode_Success) { return err; }
	err = ReadDMADCSPacket();
	DTC_DCSReplyPacket packet(dmaPacket_);
	for (int ii = 0; ii < 12; ++ii) {
		dataVector_.push_back(packet.GetData()[ii]);
	}
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::SendReadoutRequestPacket(DTC_Ring_ID ring, DTC_Timestamp when, DTC_ROC_ID roc)
{
	DTC_ReadoutRequestPacket req(ring, when, roc);
	return WriteDMADAQPacket(req);
}

DTC::DTC_ErrorCode DTC::DTC::WriteRegister(uint32_t data, uint16_t address)
{
	int errorCode = device_.write_register(address, 100, data);
	return errorCode == 0 ? DTC_ErrorCode_Success : DTC_ErrorCode_IOError;
}
DTC::DTC_ErrorCode DTC::DTC::ReadRegister(uint16_t address)
{
	int err = device_.read_register(address, 100, &dataWord_);
	return err == 0 ? DTC_ErrorCode_Success : DTC_ErrorCode_IOError;
}

DTC::DTC_ErrorCode DTC::DTC::WriteControlRegister(uint32_t data)
{
	return WriteRegister(data, DTCControlRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadControlRegister()
{
	return ReadRegister(DTCControlRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadResetDTC()
{
	DTC_ErrorCode err = ReadControlRegister();
	std::bitset<32> data = dataWord_;
	booleanValue_ = data[31];
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ResetDTC()
{
	DTC_ErrorCode err = ReadControlRegister();
	if (err != DTC_ErrorCode_Success){ return err; }
	std::bitset<32> data = dataWord_;
	data[31] = 1; // DTC Reset bit
	return WriteControlRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ClearLatchedErrors()
{
	DTC_ErrorCode err = ReadControlRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[30] = 1; // Clear Latched Errors bit
	return WriteControlRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ReadClearLatchedErrors()
{
	DTC_ErrorCode err = ReadControlRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	booleanValue_ = data[30];
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ClearClearLatchedErrors()
{
	DTC_ErrorCode err = ReadControlRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[30] = 0;
	return WriteControlRegister(data.to_ulong());
}

DTC::DTC_ErrorCode DTC::DTC::WriteSERDESLoopbackEnableRegister(uint32_t data)
{
	return WriteRegister(data, SERDESLoopbackEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESLoopbackEnableRegister()
{
	return ReadRegister(SERDESLoopbackEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::EnableSERDESLoopback(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESLoopbackEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 1;
	return WriteSERDESLoopbackEnableRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::DisableSERDESLoopback(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESLoopbackEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 0;
	return WriteSERDESLoopbackEnableRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESLoopback(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESLoopbackEnableRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteROCEmulationEnableRegister(uint32_t data)
{
	return WriteRegister(data, ROCEmulationEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadROCEmulationEnableRegister()
{
	return ReadRegister(ROCEmulationEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::EnableROCEmulator()
{
	return WriteROCEmulationEnableRegister(1UL);
}
DTC::DTC_ErrorCode DTC::DTC::DisableROCEmulator()
{
	return WriteROCEmulationEnableRegister(0UL);
}
DTC::DTC_ErrorCode DTC::DTC::ReadROCEmulatorEnabled()
{
	DTC_ErrorCode err = ReadROCEmulationEnableRegister();
	std::bitset<32> dataSet = dataWord_;
	if (dataSet[0] == 0)
	{
		booleanValue_ = false;
	}
	else
	{
		booleanValue_ = true;
	}

	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteRingEnableRegister(uint32_t data)
{
	return WriteRegister(data, RingEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadRingEnableRegister()
{
	return ReadRegister(RingEnableRegister);
}
DTC::DTC_ErrorCode DTC::DTC::EnableRing(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadRingEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 1;
	return WriteRingEnableRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::DisableRing(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadRingEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 0;
	return WriteRingEnableRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ReadRingEnabled(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadRingEnableRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteSERDESResetRegister(uint32_t data)
{
	return WriteRegister(data, SERDESResetRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESResetRegister()
{
	return ReadRegister(SERDESResetRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ResetSERDES(const DTC_Ring_ID ring, int interval)
{
	bool resetDone = false;
	DTC_ErrorCode err;
	while (!resetDone)
	{
		err = ReadSERDESResetRegister();
		if (err != DTC_ErrorCode_Success) { return DTC_ErrorCode_ResetFailed; }
		std::bitset<32> data = dataWord_;
		data[(uint8_t)ring] = 1;
		err = WriteSERDESResetRegister(data.to_ulong());
		if (err != DTC_ErrorCode_Success) { return DTC_ErrorCode_ResetFailed; }
		usleep(interval);
		data[(uint8_t)ring] = 0;
		err = WriteSERDESResetRegister(data.to_ulong());
		if (err != DTC_ErrorCode_Success) { return DTC_ErrorCode_ResetFailed; }
		err = ReadSERDESResetDone(ring);
		resetDone = booleanValue_;
	}
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadResetSERDES(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXDisparityErrorRegister()
{
	return ReadRegister(SERDESRXDisparityErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXDisparityErrorRegister();
	SERDESRXDisparityError_ = std::move(DTC_SERDESRXDisparityError(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXCharacterNotInTableErrorRegister()
{
	return ReadRegister(SERDESRXCharacterNotInTableErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXCharacterNotInTableErrorRegister();
	CharacterNotInTableError_ = std::move(DTC_CharacterNotInTableError(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESUnlockErrorRegister()
{
	return ReadRegister(SERDESUnlockErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESUnlockError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESUnlockErrorRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESPLLLockedRegister()
{
	return ReadRegister(SERDESPLLLockedRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESPLLLocked(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESPLLLockedRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESTXBufferStatusRegister()
{
	return ReadRegister(SERDESTXBufferStatusRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESTXBufferStatusRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring * 2 + 1];
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESTXBufferStatusRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring * 2];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXBufferStatusRegister()
{
	return ReadRegister(SERDESRXBufferStatusRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXBufferStatusRegister();
	SERDESRXBufferStatus_ = std::move(DTC_SERDESRXBufferStatus(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESResetDoneRegister()
{
	return ReadRegister(SERDESResetDoneRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESResetDone(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetDoneRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteTimestampPreset0Register(uint32_t data)
{
	return WriteRegister(data, TimestampPreset0Register);
}
DTC::DTC_ErrorCode DTC::DTC::ReadTimestampPreset0Register()
{
	return ReadRegister(TimestampPreset0Register);
}

DTC::DTC_ErrorCode DTC::DTC::WriteTimestampPreset1Register(uint32_t data)
{
	return WriteRegister(data, TimestampPreset1Register);
}
DTC::DTC_ErrorCode DTC::DTC::ReadTimestampPreset1Register()
{
	return ReadRegister(TimestampPreset1Register);
}

DTC::DTC_ErrorCode DTC::DTC::WriteTimestampPreset(DTC_Timestamp preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint32_t>(timestamp.to_ulong());

	DTC_ErrorCode err = WriteTimestampPreset0Register(timestampLow);
	if (err != DTC_ErrorCode_Success)
	{
		return err;
	}

	return WriteTimestampPreset1Register(timestampHigh);
}
DTC::DTC_ErrorCode DTC::DTC::ReadTimestampPreset()
{
	DTC_ErrorCode err = ReadTimestampPreset0Register();
	uint32_t timestampLow = dataWord_;
	if (err != DTC_ErrorCode_Success)
	{
		return err;
	}

	err = ReadTimestampPreset1Register();
	timestampPreset_.SetTimestamp(timestampLow, static_cast<uint16_t>(dataWord_));

	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteFPGAPROMProgramDataRegister(uint32_t data)
{
	return WriteRegister(data, FPGAPROMProgramDataRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadFPGAPROMProgramStatusRegister()
{
	return ReadRegister(FPGAPROMProgramStatusRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadFPGAPROMProgramFIFOFull()
{
	DTC_ErrorCode err = ReadFPGAPROMProgramStatusRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[1];
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadFPGAPROMReady()
{
	DTC_ErrorCode err = ReadFPGAPROMProgramStatusRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[0];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteTestCommand(DTC_TestCommand comm)
{
	int err = device_.write_test_command(comm.GetCommand());
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}
DTC::DTC_ErrorCode DTC::DTC::ReadTestCommand()
{
	m_ioc_cmd_t comm;
	int err = device_.read_test_command(&comm);
	testCommand_ = DTC_TestCommand(comm);
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}
DTC::DTC_ErrorCode DTC::DTC::StartTest(DTC_DMA_Engine dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator)
{
	testCommand_ = DTC_TestCommand(dma, true, packetSize, loopback, txChecker, rxGenerator);
	DTC_ErrorCode err = WriteTestCommand(testCommand_);
	if (err != DTC_ErrorCode_Success){ return err; }
	return ReadTestCommand();
}
DTC::DTC_ErrorCode DTC::DTC::StopTest(DTC_DMA_Engine dma)
{
	testCommand_ = DTC_TestCommand(dma);
	DTC_ErrorCode err = WriteTestCommand(testCommand_);
	if (err != DTC_ErrorCode_Success){ return err; }
	return ReadTestCommand();
}

DTC::DTC_ErrorCode DTC::DTC::ReadDMAStateData(DTC_DMA_Engine dma, DTC_DMA_Direction dir)
{
	m_ioc_engstate_t state;
	int err = device_.read_dma_state(dma, dir, &state);
	dmaState_ = DTC_DMAState(state);
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}
DTC::DTC_ErrorCode DTC::DTC::ReadDMAStatsData()
{
	DMAStatistics statData[350];
	m_ioc_engstats_t stats;
	stats.Count = 350;
	stats.engptr = statData;
	int err = device_.read_dma_stats(&stats);
	dmaStats_ = DTC_DMAStats(stats);
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}

DTC::DTC_ErrorCode DTC::DTC::ReadPCIeStateData()
{
	m_ioc_pcistate_t state;
	int err = device_.read_pcie_state(&state);
	pcieState_ = DTC_PCIeState(state);
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}
DTC::DTC_ErrorCode DTC::DTC::ReadPCIeStatsData()
{
	TRNStatistics statData[1];
	TRNStatsArray stats;
	stats.Count = 1;
	stats.trnptr = statData;
	int err = device_.read_trn_stats(&stats);
	pcieStats_ = DTC_PCIeStats(stats);
	if (err != 0) { return DTC_ErrorCode_IOError; }
	return DTC_ErrorCode_Success;
}