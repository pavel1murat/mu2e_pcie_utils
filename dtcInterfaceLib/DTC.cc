#include "DTC.h"
#ifndef _WIN32
#include "../linux_driver/mymodule/mu2e_mmap_ioctl.h"
#define TRACE_NAME "MU2EDEV"
#include "../linux_driver/include/trace.h"
#endif


DTC::DTC::DTC()
{
#ifndef _WIN32
	int sts;
	devfd_ = open("/dev/" MU2E_DEV_FILE, O_RDWR);
	if (devfd_ == -1) { perror("open /dev/" MU2E_DEV_FILE); exit(1); }
	for (unsigned chn = 0; chn < MU2E_MAX_CHANNELS; ++chn)
		for (unsigned dir = 0; dir < 2; ++dir)
		{
			m_ioc_get_info_t get_info;
			get_info.chn = chn; get_info.dir = dir;
			sts = ioctl(devfd_, M_IOC_GET_INFO, &get_info);
			if (sts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
			mu2e_channel_info_[chn][dir] = get_info;
			TRACE(1, "mu2edev::init %u:%u - num=%u size=%u hwIdx=%u, swIdx=%u"
				, chn, dir
				, get_info.num_buffs, get_info.buff_size
				, get_info.hwIdx, get_info.swIdx);
			for (unsigned map = 0; map < 2; ++map)
			{
				mu2e_mmap_ptrs_[chn][dir][map]
					= mmap(0 /* hint address */
					, get_info.num_buffs * ((map == MU2E_MAP_BUFF)
					? get_info.buff_size
					: sizeof(int))
					, (((dir == S2C) && (map == MU2E_MAP_BUFF))
					? PROT_WRITE : PROT_READ)
					, MAP_SHARED
					, devfd_
					, chnDirMap2offset(chn, dir, map));
				if (mu2e_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
				{
					perror("mmap"); exit(1);
				}
				TRACE(1, "mu2edev::init chnDirMap2offset=%lu mu2e_mmap_ptrs_[%d][%d][%d]=%p"
					, chnDirMap2offset(chn, dir, map)
					, chn, dir, map, mu2e_mmap_ptrs_[chn][dir][map]);
			}
		}
#endif
}

DTC::DTC_ErrorCode DTC::DTC::ReadDataPacket(int channel)
{
	return DTC_ErrorCode_NotImplemented;
}

DTC::DTC_ErrorCode DTC::DTC::WriteDataPacket(int channel, DTC_DataPacket packet)
{
	return DTC_ErrorCode_NotImplemented;
}

DTC::DTC_ErrorCode DTC::DTC::ReadDAQDataPacket()
{
	return ReadDataPacket(0);
}
DTC::DTC_ErrorCode DTC::DTC::WriteDAQDataPacket(DTC_DataPacket packet)
{
	return WriteDataPacket(0, packet);
}

DTC::DTC_ErrorCode DTC::DTC::ReadDMAPacket(int channel)
{
	DTC_ErrorCode err = ReadDataPacket(channel);
	if (err != DTC_ErrorCode_Success) { return err; }
	dmaPacket_ = DTC_DMAPacket(dataPacket_);
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadDMADAQPacket()
{
	return ReadDMAPacket(0);
}
DTC::DTC_ErrorCode DTC::DTC::ReadDMADCSPacket()
{
	return ReadDMAPacket(1);
}

DTC::DTC_ErrorCode DTC::DTC::WriteDMAPacket(int channel, DTC_DMAPacket packet)
{
	return WriteDataPacket(channel, packet.ConvertToDataPacket());
}
DTC::DTC_ErrorCode DTC::DTC::WriteDMADAQPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(0, packet);
}
DTC::DTC_ErrorCode DTC::DTC::WriteDMADCSPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(1, packet);
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
#ifndef _WIN32
	m_ioc_reg_access_t reg;
	reg.reg_offset = address;
	reg.access_type = 1;
	reg.val = data;
	return static_cast<DTC_ErrorCode>(-1 * ioctl(devfd_, M_IOC_REG_ACCESS, &reg));
#else
	return DTC_ErrorCode_NotImplemented;
#endif
}
DTC::DTC_ErrorCode DTC::DTC::ReadRegister(uint16_t address)
{
#ifndef _WIN32
	m_ioc_reg_access_t reg;
	reg.reg_offset = address;
	reg.access_type = 0;
	int errCode = -1 * ioctl(devfd_, M_IOC_REG_ACCESS, &reg);
	dataWord_ = reg.val;
	return static_cast<DTC_ErrorCode>(errCode);
#else
	return DTC_ErrorCode_NotImplemented;
#endif
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
DTC::DTC_ErrorCode DTC::DTC::ResetSERDES(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 1;
	return WriteSERDESResetRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ClearResetSERDES(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord_;
	data[(uint8_t)ring] = 0;
	return WriteSERDESResetRegister(data.to_ulong());
}
DTC::DTC_ErrorCode DTC::DTC::ReadResetSERDES(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetRegister();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXDisparityError()
{
	return ReadRegister(SERDESRXDisparityErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXDisparityError();
	SERDESRXDisparityError_ = std::move(DTC_SERDESRXDisparityError(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXCharacterNotInTableError()
{
	return ReadRegister(SERDESRXCharacterNotInTableErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXCharacterNotInTableError();
	CharacterNotInTableError_ = std::move(DTC_CharacterNotInTableError(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESUnlockError()
{
	return ReadRegister(SERDESUnlockErrorRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESUnlockError(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESUnlockError();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESPLLLocked()
{
	return ReadRegister(SERDESPLLLockedRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESPLLLocked(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESPLLLocked();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESTXBufferStatus()
{
	return ReadRegister(SERDESTXBufferStatusRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESTXBufferStatus();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring * 2 + 1];
	return err;
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESTXBufferStatus();
	std::bitset<32> dataSet = dataWord_;
	booleanValue_ = dataSet[(uint8_t)ring * 2];
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXBufferStatus()
{
	return ReadRegister(SERDESRXBufferStatusRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESRXBufferStatus();
	SERDESRXBufferStatus_ = std::move(DTC_SERDESRXBufferStatus(dataWord_, ring));
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::ReadSERDESResetDone()
{
	return ReadRegister(SERDESResetDoneRegister);
}
DTC::DTC_ErrorCode DTC::DTC::ReadSERDESResetDone(const DTC_Ring_ID ring)
{
	DTC_ErrorCode err = ReadSERDESResetDone();
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
DTC::DTC_ErrorCode DTC::DTC::WriteTimestampPreset1(uint16_t data)
{
	return WriteTimestampPreset1Register(data);
}
DTC::DTC_ErrorCode DTC::DTC::ReadTimestampPreset1()
{
	DTC_ErrorCode err = ReadTimestampPreset1Register();
	return err;
}

DTC::DTC_ErrorCode DTC::DTC::WriteTimestampPreset(DTC_Timestamp preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	DTC_ErrorCode err = WriteTimestampPreset0Register(timestampLow);
	if (err != DTC_ErrorCode_Success)
	{
		return err;
	}

	return WriteTimestampPreset1(timestampHigh);
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
