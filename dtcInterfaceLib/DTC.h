#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include "../linux_driver/mymodule/mu2e_mmap_ioctl.h"

namespace DTC {
	class DTC {
	public:
		DTC();
                virtual ~DTC() = default;
		//	private:
		//#if DTCLIB_DEBUG
		//	public:
		//#endif
		DTC_DataPacket ReadDataPacket(int channel);
		uint8_t WriteDataPacket(int channel, DTC_DataPacket packet);

		DTC_DataPacket ReadDAQDataPacket();
		uint8_t WriteDAQDataPacket(DTC_DataPacket packet);

		DTC_DMAPacket ReadDMAPacket(int channel);
		DTC_DMAPacket ReadDMADAQPacket();
		DTC_DMAPacket ReadDMADCSPacket();

		uint8_t WriteDMAPacket(int channel, DTC_DMAPacket packet);
		uint8_t WriteDMADAQPacket(DTC_DMAPacket packet);
		uint8_t WriteDMADCSPacket(DTC_DMAPacket packet);
	public:
		uint8_t GetData(const uint8_t ring, uint8_t roc, DTC_Timestamp when);
		uint8_t DCSRequestReply(const uint8_t ring, uint8_t roc, uint8_t dataIn[12]);
		uint8_t SendReadoutRequestPacket(const uint8_t ring, DTC_Timestamp when, uint8_t roc = DTC_ROC_5);

		//private:
		//#if DTCLIB_DEBUG
		//public:
		//#endif
		uint8_t WriteRegister(uint32_t data, uint16_t address);
		uint8_t ReadRegister(uint16_t address);

		uint8_t WriteControlRegister(uint32_t data);
		uint8_t ReadControlRegister();
		uint8_t WriteSERDESLoopbackEnableRegister(uint32_t data);
		uint8_t ReadSERDESLoopbackEnableRegister();
		uint8_t WriteROCEmulationEnableRegister(uint32_t data);
		uint8_t ReadROCEmulationEnableRegister();
		uint8_t WriteRingEnableRegister(uint32_t data);
		uint8_t ReadRingEnableRegister();
		uint8_t WriteSERDESResetRegister(uint32_t data);
		uint8_t ReadSERDESResetRegister();
		uint8_t ReadSERDESRXDisparityError();
		uint8_t ReadSERDESRXCharacterNotInTableError();
		uint8_t ReadSERDESUnlockError();
		uint8_t ReadSERDESPLLLocked();
		uint8_t ReadSERDESTXBufferStatus();
		uint8_t ReadSERDESRXBufferStatus();
		uint8_t ReadSERDESResetDone();
		uint8_t WriteTimestampPreset0Register(uint32_t data);
		uint8_t ReadTimestampPreset0Register();
		uint8_t WriteTimestampPreset1Register(uint32_t data);
		uint8_t ReadTimestampPreset1Register();
		uint8_t WriteTimestampPreset1(uint16_t data);
		uint8_t ReadTimestampPreset1();
		uint8_t WriteFPGAPROMProgramDataRegister(uint32_t data);
		uint8_t ReadFPGAPROMProgramStatusRegister();

	public:
		uint8_t ResetDTC();
                uint8_t ReadResetDTC();
		uint8_t ClearLatchedErrors();
                uint8_t ReadClearLatchedErrors();

		uint8_t EnableSERDESLoopback(const uint8_t ring);
		uint8_t DisableSERDESLoopback(const uint8_t ring);
		uint8_t ReadSERDESLoopback(const uint8_t ring);

		uint8_t EnableROCEmulator();
		uint8_t DisableROCEmulator();
		uint8_t ReadROCEmulatorEnabled();

		uint8_t EnableRing(const uint8_t ring);
		uint8_t DisableRing(const uint8_t ring);
		uint8_t ReadRingEnabled(const uint8_t ring);

		uint8_t ResetSERDES(const uint8_t ring);
		uint8_t ReadResetSERDES(const uint8_t ring);

		uint8_t ReadSERDESRXDisparityError(const uint8_t ring);

		uint8_t ReadSERDESRXCharacterNotInTableError(const uint8_t ring);

		uint8_t ReadSERDESUnlockError(const uint8_t ring);

		uint8_t ReadSERDESPLLLocked(const uint8_t ring);

		uint8_t ReadSERDESOverflowOrUnderflow(const uint8_t ring);
		uint8_t ReadSERDESBufferFIFOHalfFull(const uint8_t ring);

		uint8_t ReadSERDESRXBufferStatus(const uint8_t ring);

		uint8_t ReadSERDESResetDone(const uint8_t ring);


		uint8_t WriteTimestampPreset(DTC_Timestamp preset);
		uint8_t ReadTimestampPreset();

		uint8_t ReadFPGAPROMProgramFIFOFull();
		uint8_t ReadFPGAPROMReady();

                DTC_Timestamp timestampPreset;
                uint8_t lastError;
                DTC_SERDESRXBufferStatus SERDESRXBufferStatus;
                DTC_CharacterNotInTableError CharacterNotInTableError;
                DTC_SERDESRXDisparityError SERDESRXDisparityError;
                bool booleanValue;
                uint32_t dataWord;
                std::vector<uint8_t> dataVector;
	private:
    int	             devfd_;
    volatile void *  mu2e_mmap_ptrs_[MU2E_MAX_CHANNELS][2][2];
    m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];
	};
};

#endif
