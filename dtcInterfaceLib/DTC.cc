#include "DTC.h"
#include "../linux_driver/mymodule/mu2e_mmap_ioctl.h"
#define TRACE_NAME "MU2EDEV"
#include "../linux_driver/include/trace.h"


DTC::DTC::DTC()
{int sts;
    devfd_ = open( "/dev/" MU2E_DEV_FILE, O_RDWR );
    if (devfd_ == -1) { perror("open /dev/" MU2E_DEV_FILE); exit (1); }
    for (unsigned chn=0; chn<MU2E_MAX_CHANNELS; ++chn)
	for (unsigned dir=0; dir<2; ++dir)
	{   m_ioc_get_info_t get_info;
	    get_info.chn = chn; get_info.dir = dir;
	    sts = ioctl( devfd_, M_IOC_GET_INFO, &get_info );
	    if (sts != 0) { perror( "M_IOC_GET_INFO" ); exit (1); }
	    mu2e_channel_info_[chn][dir] = get_info;
	    TRACE( 1, "mu2edev::init %u:%u - num=%u size=%u hwIdx=%u, swIdx=%u"
		  , chn,dir
		  , get_info.num_buffs, get_info.buff_size
		  , get_info.hwIdx, get_info.swIdx );
	    for (unsigned map=0; map<2; ++map)
	    {   mu2e_mmap_ptrs_[chn][dir][map]
		    = mmap( 0 /* hint address */
			   , get_info.num_buffs * ((map==MU2E_MAP_BUFF)
						   ? get_info.buff_size
						   : sizeof(int) )
			   , (((dir==S2C)&&(map==MU2E_MAP_BUFF))
			      ? PROT_WRITE : PROT_READ )
			   , MAP_SHARED
			   , devfd_
			   , chnDirMap2offset( chn, dir, map ) );
		if (mu2e_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
		{   perror( "mmap" ); exit (1);
		}
		TRACE( 1, "mu2edev::init chnDirMap2offset=%lu mu2e_mmap_ptrs_[%d][%d][%d]=%p"
		      , chnDirMap2offset( chn, dir, map )
		      , chn, dir, map, mu2e_mmap_ptrs_[chn][dir][map] );
	    }
	}
}

DTC::DTC_DataPacket DTC::DTC::ReadDataPacket(int channel)
{
  lastError = DTC_ErrorCode_NotImplemented;
	return DTC_DataPacket();
}

uint8_t DTC::DTC::WriteDataPacket(int channel, DTC_DataPacket packet)
{
	return DTC_ErrorCode_NotImplemented;
}

DTC::DTC_DataPacket DTC::DTC::ReadDAQDataPacket()
{
	return ReadDataPacket(0);
}
uint8_t DTC::DTC::WriteDAQDataPacket(DTC_DataPacket packet)
{
	return WriteDataPacket(0, packet);
}

DTC::DTC_DMAPacket DTC::DTC::ReadDMAPacket(int channel)
{
	return DTC_DMAPacket(ReadDataPacket(channel));
}
DTC::DTC_DMAPacket DTC::DTC::ReadDMADAQPacket()
{
	return ReadDMAPacket(0);
}
DTC::DTC_DMAPacket DTC::DTC::ReadDMADCSPacket()
{
	return ReadDMAPacket(1);
}

uint8_t DTC::DTC::WriteDMAPacket(int channel, DTC_DMAPacket packet)
{
	return WriteDataPacket(channel, packet.ConvertToDataPacket());
}
uint8_t DTC::DTC::WriteDMADAQPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(0, packet);
}
uint8_t DTC::DTC::WriteDMADCSPacket(DTC_DMAPacket packet)
{
	return WriteDMAPacket(1, packet);
}

uint8_t DTC::DTC::GetData(uint8_t ring, uint8_t roc, DTC_Timestamp when)
{
  dataVector.clear();
	uint8_t err;
	DTC_DataRequestPacket req(ring, roc, when);
	err = WriteDMADAQPacket(req);
	if (err != DTC_ErrorCode_Success) { return err; }

	DTC_DataHeaderPacket packet = ReadDMADAQPacket();
	if (lastError != DTC_ErrorCode_Success) { return lastError; }
	if (packet.GetPacketType() != uint8_t_DataHeader)
	{
		return DTC_ErrorCode_WrongPacketType;
	}
	for (int i = 0; i < 6; ++i)
	{
		dataVector.push_back(packet.GetData()[i]);
	}
	for (int ii = 0; ii < packet.GetPacketCount(); ++ii)
	{
		DTC_DataPacket dataPacket = ReadDAQDataPacket();
		if (lastError != DTC_ErrorCode_Success) { return lastError; }
		for (int i = 0; i < 16; ++i)
		{
			dataVector.push_back(dataPacket.GetWord(i));
		}
	}
	return err;
}

uint8_t DTC::DTC::DCSRequestReply(uint8_t ring, uint8_t roc, uint8_t dataIn[12])
{
  dataVector.clear();
	uint8_t err;
	DTC_DCSRequestPacket req(ring, roc, dataIn);
	err = WriteDMADCSPacket(req);
	if (err != DTC_ErrorCode_Success) { return err; }
	DTC_DCSReplyPacket packet = static_cast<DTC_DCSReplyPacket>(ReadDMADCSPacket());
	for(int ii = 0; ii < 12; ++ii) {
	  dataVector.push_back(packet.GetData()[ii]);
        }
	return lastError;
}

uint8_t DTC::DTC::SendReadoutRequestPacket(uint8_t ring, DTC_Timestamp when, uint8_t roc)
{
	DTC_ReadoutRequestPacket req(ring, when, roc);
	return WriteDMADAQPacket(req);
}

uint8_t DTC::DTC::WriteRegister(uint32_t data, uint16_t address)
{
	m_ioc_reg_access_t reg;
	reg.reg_offset = address;
	reg.access_type = 1;
	reg.val = data;
	return static_cast<uint8_t>(-1 * ioctl( devfd_, M_IOC_REG_ACCESS, &reg));
	//return DTC_ErrorCode_NotImplemented;
}
uint8_t DTC::DTC::ReadRegister(uint16_t address)
{
	m_ioc_reg_access_t reg;
	reg.reg_offset = address;
	reg.access_type = 0;
	int errCode = -1 * ioctl( devfd_, M_IOC_REG_ACCESS, &reg);
	dataWord = reg.val;
	return static_cast<uint8_t>(errCode);
	//return DTC_ErrorCode_NotImplemented;
}

uint8_t DTC::DTC::WriteControlRegister(uint32_t data)
{
	return WriteRegister(data, DTCControlRegister);
}
uint8_t DTC::DTC::ReadControlRegister()
{
	return ReadRegister(DTCControlRegister);
}
uint8_t DTC::DTC::ReadResetDTC()
{
        uint8_t err = ReadControlRegister();
	std::bitset<32> data = dataWord;
        booleanValue = data[31];
        return err;
}
uint8_t DTC::DTC::ResetDTC()
{
  uint8_t err = ReadControlRegister();
  if(err != DTC_ErrorCode_Success){return err;}
  std::bitset<32> data = dataWord;
	data[31] = 1; // DTC Reset bit
	return WriteControlRegister(data.to_ulong());
}
uint8_t DTC::DTC::ClearLatchedErrors()
{
  uint8_t err = ReadControlRegister();
  if(err != DTC_ErrorCode_Success) { return err; }
  std::bitset<32> data = dataWord;
	data[30] = 1; // Clear Latched Errors bit
	return WriteControlRegister(data.to_ulong());
}
uint8_t DTC::DTC::ReadClearLatchedErrors() 
{
  uint8_t err = ReadControlRegister();
  std::bitset<32> data = dataWord;
  booleanValue = data[30];
  return err;
}

uint8_t DTC::DTC::WriteSERDESLoopbackEnableRegister(uint32_t data)
{
	return WriteRegister(data, SERDESLoopbackEnableRegister);
}
uint8_t DTC::DTC::ReadSERDESLoopbackEnableRegister()
{
	return ReadRegister(SERDESLoopbackEnableRegister);
}
uint8_t DTC::DTC::EnableSERDESLoopback(uint8_t ring)
{
	uint8_t err = ReadSERDESLoopbackEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord;
	data[(uint8_t)ring] = 1;
	return WriteSERDESLoopbackEnableRegister(data.to_ulong());
}
uint8_t DTC::DTC::DisableSERDESLoopback(uint8_t ring)
{
	uint8_t err = ReadSERDESLoopbackEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord;
	data[(uint8_t)ring] = 0;
	return WriteSERDESLoopbackEnableRegister(data.to_ulong());
}
uint8_t DTC::DTC::ReadSERDESLoopback(uint8_t ring)
{
	uint8_t err = ReadSERDESLoopbackEnableRegister();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::WriteROCEmulationEnableRegister(uint32_t data)
{
	return WriteRegister(data, ROCEmulationEnableRegister);
}
uint8_t DTC::DTC::ReadROCEmulationEnableRegister()
{
	return ReadRegister(ROCEmulationEnableRegister);
}
uint8_t DTC::DTC::EnableROCEmulator()
{
	return WriteROCEmulationEnableRegister(1UL);
}
uint8_t DTC::DTC::DisableROCEmulator()
{
	return WriteROCEmulationEnableRegister(0UL);
}
uint8_t DTC::DTC::ReadROCEmulatorEnabled()
{
	uint8_t err = ReadROCEmulationEnableRegister();
	std::bitset<32> dataSet = dataWord;
	if (dataSet[0] == 0)
	{
		booleanValue = false;
	}
	else
	{
		booleanValue = true;
	}

	return err;
}

uint8_t DTC::DTC::WriteRingEnableRegister(uint32_t data)
{
	return WriteRegister(data, RingEnableRegister);
}
uint8_t DTC::DTC::ReadRingEnableRegister()
{
	return ReadRegister(RingEnableRegister);
}
uint8_t DTC::DTC::EnableRing(uint8_t ring)
{
	uint8_t err = ReadRingEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord;
	data[(uint8_t)ring] = 1;
	return WriteRingEnableRegister(data.to_ulong());
}
uint8_t DTC::DTC::DisableRing(uint8_t ring)
{
	uint8_t err = ReadRingEnableRegister();
	if (err != DTC_ErrorCode_Success) { return err; }
	std::bitset<32> data = dataWord;
	data[(uint8_t)ring] = 0;
	return WriteRingEnableRegister(data.to_ulong());
}
uint8_t DTC::DTC::ReadRingEnabled(uint8_t ring)
{
	uint8_t err = ReadRingEnableRegister();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::WriteSERDESResetRegister(uint32_t data)
{
	return WriteRegister(data, SERDESResetRegister);
}
uint8_t DTC::DTC::ReadSERDESResetRegister()
{
	return ReadRegister(SERDESResetRegister);
}
uint8_t DTC::DTC::ResetSERDES(uint8_t ring)
{
	std::bitset<32> data;
	data[(uint8_t)ring] = 1;
	return WriteSERDESResetRegister(data.to_ulong());
}
uint8_t DTC::DTC::ReadResetSERDES(uint8_t ring)
{
	uint8_t err = ReadSERDESResetRegister();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::ReadSERDESRXDisparityError()
{
	return ReadRegister(SERDESRXDisparityErrorRegister);
}
uint8_t DTC::DTC::ReadSERDESRXDisparityError(uint8_t ring)
{
	uint8_t err = ReadSERDESRXDisparityError();
	SERDESRXDisparityError = std::move(DTC_SERDESRXDisparityError(dataWord, ring));
	return err;
}

uint8_t DTC::DTC::ReadSERDESRXCharacterNotInTableError()
{
	return ReadRegister(SERDESRXCharacterNotInTableErrorRegister);
}
uint8_t DTC::DTC::ReadSERDESRXCharacterNotInTableError(uint8_t ring)
{
	uint8_t err = ReadSERDESRXCharacterNotInTableError();
	CharacterNotInTableError = std::move(DTC_CharacterNotInTableError(dataWord, ring));
	return err;
}

uint8_t DTC::DTC::ReadSERDESUnlockError()
{
	return ReadRegister(SERDESUnlockErrorRegister);
}
uint8_t DTC::DTC::ReadSERDESUnlockError(uint8_t ring)
{
	uint8_t err = ReadSERDESUnlockError();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::ReadSERDESPLLLocked()
{
	return ReadRegister(SERDESPLLLockedRegister);
}
uint8_t DTC::DTC::ReadSERDESPLLLocked(uint8_t ring)
{
	uint8_t err = ReadSERDESPLLLocked();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::ReadSERDESTXBufferStatus()
{
	return ReadRegister(SERDESTXBufferStatusRegister);
}
uint8_t DTC::DTC::ReadSERDESOverflowOrUnderflow(uint8_t ring)
{
	uint8_t err = ReadSERDESTXBufferStatus();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring * 2 + 1];
	return err;
}
uint8_t DTC::DTC::ReadSERDESBufferFIFOHalfFull(uint8_t ring)
{
	uint8_t err = ReadSERDESTXBufferStatus();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring * 2];
	return err;
}

uint8_t DTC::DTC::ReadSERDESRXBufferStatus()
{
	return ReadRegister(SERDESRXBufferStatusRegister);
}
uint8_t DTC::DTC::ReadSERDESRXBufferStatus(uint8_t ring)
{
	uint8_t err = ReadSERDESRXBufferStatus();
	SERDESRXBufferStatus = std::move(DTC_SERDESRXBufferStatus(dataWord, ring));
	return err;
}

uint8_t DTC::DTC::ReadSERDESResetDone()
{
	return ReadRegister(SERDESResetDoneRegister);
}
uint8_t DTC::DTC::ReadSERDESResetDone(uint8_t ring)
{
	uint8_t err = ReadSERDESResetDone();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[(uint8_t)ring];
	return err;
}

uint8_t DTC::DTC::WriteTimestampPreset0Register(uint32_t data)
{
	return WriteRegister(data, TimestampPreset0Register);
}
uint8_t DTC::DTC::ReadTimestampPreset0Register()
{
	return ReadRegister(TimestampPreset0Register);
}

uint8_t DTC::DTC::WriteTimestampPreset1Register(uint32_t data)
{
	return WriteRegister(data, TimestampPreset1Register);
}
uint8_t DTC::DTC::ReadTimestampPreset1Register()
{
	return ReadRegister(TimestampPreset1Register);
}
uint8_t DTC::DTC::WriteTimestampPreset1(uint16_t data)
{
	return WriteTimestampPreset1Register(data);
}
uint8_t DTC::DTC::ReadTimestampPreset1()
{
	uint8_t err = ReadTimestampPreset1Register();
	return err;
}

uint8_t DTC::DTC::WriteTimestampPreset(DTC_Timestamp preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	uint8_t err = WriteTimestampPreset0Register(timestampLow);
	if (err != DTC_ErrorCode_Success)
	{
		return err;
	}

	return WriteTimestampPreset1(timestampHigh);
}
uint8_t DTC::DTC::ReadTimestampPreset()
{
	uint8_t err = ReadTimestampPreset0Register();
        uint32_t timestampLow = dataWord;
	if (err != DTC_ErrorCode_Success)
	{
		return err;
	}

	err = ReadTimestampPreset1Register();
	timestampPreset.SetTimestamp(timestampLow, dataWord);

	return err;
}

uint8_t DTC::DTC::WriteFPGAPROMProgramDataRegister(uint32_t data)
{
	return WriteRegister(data, FPGAPROMProgramDataRegister);
}
uint8_t DTC::DTC::ReadFPGAPROMProgramStatusRegister()
{
	return ReadRegister(FPGAPROMProgramStatusRegister);
}
uint8_t DTC::DTC::ReadFPGAPROMProgramFIFOFull()
{
	uint8_t err = ReadFPGAPROMProgramStatusRegister();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[1];
	return err;
}
uint8_t DTC::DTC::ReadFPGAPROMReady()
{
	uint8_t err = ReadFPGAPROMProgramStatusRegister();
	std::bitset<32> dataSet = dataWord;
	booleanValue = dataSet[0];
	return err;
}
