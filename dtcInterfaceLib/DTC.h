#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#ifndef _WIN32
#include "../linux_driver/mymodule/mu2e_mmap_ioctl.h"
#endif

namespace DTC {
	class DTC {
	public:
		DTC();
		virtual ~DTC() = default;
		//	private:
		//#if DTCLIB_DEBUG
		//	public:
		//#endif
		virtual DTC_ErrorCode ReadDataPacket(int channel);
		virtual DTC_ErrorCode WriteDataPacket(int channel, DTC_DataPacket packet);

		virtual DTC_ErrorCode ReadDAQDataPacket();
		virtual DTC_ErrorCode WriteDAQDataPacket(DTC_DataPacket packet);

		virtual DTC_ErrorCode ReadDMAPacket(int channel);
		virtual DTC_ErrorCode ReadDMADAQPacket();
		virtual DTC_ErrorCode ReadDMADCSPacket();

		virtual DTC_ErrorCode WriteDMAPacket(int channel, DTC_DMAPacket packet);
		virtual DTC_ErrorCode WriteDMADAQPacket(DTC_DMAPacket packet);
		virtual DTC_ErrorCode WriteDMADCSPacket(DTC_DMAPacket packet);
	public:
		virtual DTC_ErrorCode GetData(const DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp when);
		virtual DTC_ErrorCode DCSRequestReply(const DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t dataIn[12]);
		virtual DTC_ErrorCode SendReadoutRequestPacket(const DTC_Ring_ID ring, DTC_Timestamp when, DTC_ROC_ID roc = DTC_ROC_5);

		//private:
		//#if DTCLIB_DEBUG
		//public:
		//#endif
		virtual DTC_ErrorCode WriteRegister(uint32_t data, uint16_t address);
		virtual DTC_ErrorCode ReadRegister(uint16_t address);

		virtual DTC_ErrorCode WriteControlRegister(uint32_t data);
		virtual DTC_ErrorCode ReadControlRegister();
		virtual DTC_ErrorCode WriteSERDESLoopbackEnableRegister(uint32_t data);
		virtual DTC_ErrorCode ReadSERDESLoopbackEnableRegister();
		virtual DTC_ErrorCode WriteROCEmulationEnableRegister(uint32_t data);
		virtual DTC_ErrorCode ReadROCEmulationEnableRegister();
		virtual DTC_ErrorCode WriteRingEnableRegister(uint32_t data);
		virtual DTC_ErrorCode ReadRingEnableRegister();
		virtual DTC_ErrorCode WriteSERDESResetRegister(uint32_t data);
		virtual DTC_ErrorCode ReadSERDESResetRegister();
		virtual DTC_ErrorCode ReadSERDESRXDisparityError();
		virtual DTC_ErrorCode ReadSERDESRXCharacterNotInTableError();
		virtual DTC_ErrorCode ReadSERDESUnlockError();
		virtual DTC_ErrorCode ReadSERDESPLLLocked();
		virtual DTC_ErrorCode ReadSERDESTXBufferStatus();
		virtual DTC_ErrorCode ReadSERDESRXBufferStatus();
		virtual DTC_ErrorCode ReadSERDESResetDone();
		virtual DTC_ErrorCode WriteTimestampPreset0Register(uint32_t data);
		virtual DTC_ErrorCode ReadTimestampPreset0Register();
		virtual DTC_ErrorCode WriteTimestampPreset1Register(uint32_t data);
		virtual DTC_ErrorCode ReadTimestampPreset1Register();
		virtual DTC_ErrorCode WriteTimestampPreset1(uint16_t data);
		virtual DTC_ErrorCode ReadTimestampPreset1();
		virtual DTC_ErrorCode WriteFPGAPROMProgramDataRegister(uint32_t data);
		virtual DTC_ErrorCode ReadFPGAPROMProgramStatusRegister();

	public:
		virtual DTC_ErrorCode ResetDTC();
		virtual DTC_ErrorCode ReadResetDTC();

		virtual DTC_ErrorCode ClearLatchedErrors();
		virtual DTC_ErrorCode ReadClearLatchedErrors();
		virtual DTC_ErrorCode ClearClearLatchedErrors();

		virtual DTC_ErrorCode EnableSERDESLoopback(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode DisableSERDESLoopback(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode ReadSERDESLoopback(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode EnableROCEmulator();
		virtual DTC_ErrorCode DisableROCEmulator();
		virtual DTC_ErrorCode ReadROCEmulatorEnabled();

		virtual DTC_ErrorCode EnableRing(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode DisableRing(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode ReadRingEnabled(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ResetSERDES(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode ReadResetSERDES(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode ClearResetSERDES(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESRXDisparityError(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESUnlockError(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESPLLLocked(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID ring);
		virtual DTC_ErrorCode ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESRXBufferStatus(const DTC_Ring_ID ring);

		virtual DTC_ErrorCode ReadSERDESResetDone(const DTC_Ring_ID ring);


		virtual DTC_ErrorCode WriteTimestampPreset(DTC_Timestamp preset);
		virtual DTC_ErrorCode ReadTimestampPreset();

		virtual DTC_ErrorCode ReadFPGAPROMProgramFIFOFull();
		virtual DTC_ErrorCode ReadFPGAPROMReady();

		virtual DTC_DataPacket ReadDataPacket() { return dataPacket_; }
		virtual DTC_DMAPacket ReadDMAPacket() { return dmaPacket_; }
		virtual DTC_Timestamp ReadTimestamp() { return timestampPreset_; }
		virtual DTC_SERDESRXBufferStatus ReadRXBufferStatus() { return SERDESRXBufferStatus_; }
		virtual DTC_CharacterNotInTableError ReadCNITError() { return CharacterNotInTableError_; }
		virtual DTC_SERDESRXDisparityError ReadRXDisparityError() { return SERDESRXDisparityError_; }
		virtual bool ReadBooleanValue() { return booleanValue_; }
		virtual uint32_t ReadDataWord() { return dataWord_; }
		virtual std::vector<uint8_t> ReadDataVector() { return dataVector_; }

	private:
#ifndef _WIN32
		int	             devfd_;
		volatile void *  mu2e_mmap_ptrs_[MU2E_MAX_CHANNELS][2][2];
		m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];
#endif

		DTC_DataPacket dataPacket_;
		DTC_DMAPacket dmaPacket_;
		DTC_Timestamp timestampPreset_;
		DTC_SERDESRXBufferStatus SERDESRXBufferStatus_;
		DTC_CharacterNotInTableError CharacterNotInTableError_;
		DTC_SERDESRXDisparityError SERDESRXDisparityError_;
		bool booleanValue_;
		uint32_t dataWord_;
		std::vector<uint8_t> dataVector_;
	};
};

#endif
