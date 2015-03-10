#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include <cstdlib>
#include "mu2edev.hh"

namespace DTC {
	class DTC {
	public:
		DTC();
		virtual ~DTC() = default;

		const int DTC_BUFFSIZE;

		bool IsSimulatedDTC() { return simMode_; }

		//
		// DMA Functions
		//
		// Data read-out
		std::vector<void*> GetData(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_Timestamp& when, int* length);

		// DCS Read/Write Cycle
		void DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t *dataIn);

		// Broadcast Readout
		void SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when);

		// Set number of ROCs in a Ring
		void SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc);

		// For loopback testing...
		void WriteDMADAQPacket(const DTC_DMAPacket& packet);
		void WriteDMADCSPacket(const DTC_DMAPacket& packet);
		DTC_DMAPacket ReadDMADAQPacket();
		DTC_DMAPacket ReadDMADCSPacket();

		//
		// Register IO Functions
		//
		std::string RegisterRead(const DTC_Register& address);

		std::string ReadDesignVersion();

		void ResetDTC();
		bool ReadResetDTC();

		bool ToggleCFOEmulation();
		bool ReadCFOEmulation();

		int SetTriggerDMATransferLength(uint16_t length);
		uint16_t ReadTriggerDMATransferLength();

		int SetMinDMATransferLength(uint16_t length);
		uint16_t ReadMinDMATransferLength();

		DTC_SERDESLoopbackMode SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode);
		DTC_SERDESLoopbackMode ReadSERDESLoopback(const DTC_Ring_ID& ring);

		bool ToggleROCEmulator();
		bool ReadROCEmulator();

		bool EnableRing(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
		bool DisableRing(const DTC_Ring_ID& ring);
		bool ToggleRingEnabled(const DTC_Ring_ID& ring);
		bool ReadRingEnabled(const DTC_Ring_ID& ring);

		bool ResetSERDES(const DTC_Ring_ID& ring, int interval = 100);
		bool ReadResetSERDES(const DTC_Ring_ID& ring);
		bool ReadResetSERDESDone(const DTC_Ring_ID& ring);

		DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(const DTC_Ring_ID& ring);
		DTC_SERDESRXDisparityError ClearSERDESRXDisparityError(const DTC_Ring_ID& ring);
		DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);
		DTC_CharacterNotInTableError ClearSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);

		bool ReadSERDESUnlockError(const DTC_Ring_ID& ring);
		bool ClearSERDESUnlockError(const DTC_Ring_ID& ring);
		bool ReadSERDESPLLLocked(const DTC_Ring_ID& ring);
		bool ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring);
		bool ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring);

		DTC_RXBufferStatus ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring);

		DTC_RXStatus ReadSERDESRXStatus(const DTC_Ring_ID& ring);

		bool ReadSERDESEyescanError(const DTC_Ring_ID& ring);
		bool ClearSERDESEyescanError(const DTC_Ring_ID& ring);
		bool ReadSERDESRXCDRLock(const DTC_Ring_ID& ring);

		int WriteDMATimeoutPreset(uint32_t preset);
		uint32_t ReadDMATimeoutPreset();
		int WriteDataPendingTimer(uint32_t timer);
		uint32_t ReadDataPendingTimer();
		int SetPacketSize(uint16_t packetSize);
		uint16_t ReadPacketSize();

		DTC_Timestamp WriteTimestampPreset(const DTC_Timestamp& preset);
		DTC_Timestamp ReadTimestampPreset();

		bool ReadFPGAPROMProgramFIFOFull();
		bool ReadFPGAPROMReady();

		void ReloadFPGAFirmware();
		bool ReadFPGACoreAccessFIFOFull();

		//
		// PCIe/DMA Status and Performance
		// DMA Testing Engine
		//
		DTC_TestMode StartTest(const DTC_DMA_Engine& dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator);
		DTC_TestMode StopTest(const DTC_DMA_Engine& dma);

		DTC_DMAState ReadDMAState(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir);
		DTC_DMAStats ReadDMAStats(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir);

		DTC_PCIeState ReadPCIeState();
		DTC_PCIeStat ReadPCIeStats();

	private:
		DTC_DataPacket ReadDataPacket(const DTC_DMA_Engine& channel);
		void WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet);
		DTC_DMAPacket ReadDMAPacket(const DTC_DMA_Engine& channel);
		void WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet);

		void WriteRegister(uint32_t data, const DTC_Register& address);
		uint32_t ReadRegister(const DTC_Register& address);

		//Register Helper Functions
		void WriteControlRegister(uint32_t data){ WriteRegister(data, DTC_Register_DTCControl); }
		uint32_t ReadControlRegister() { return ReadRegister(DTC_Register_DTCControl); }
		void WriteDMATransferLengthRegister(uint32_t data) { WriteRegister(data, DTC_Register_DMATransferLength); }
		uint32_t ReadDMATransferLengthRegister() { return ReadRegister(DTC_Register_DMATransferLength); }
		void WriteSERDESLoopbackEnableRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESLoopbackEnable); }
		uint32_t ReadSERDESLoopbackEnableRegister() { return ReadRegister(DTC_Register_SERDESLoopbackEnable); }
		void WriteSERDESLoopbackEnableTempRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESLoopbackEnable_Temp); }
		uint32_t ReadSERDESLoopbackEnableTempRegister() { return ReadRegister(DTC_Register_SERDESLoopbackEnable_Temp); }
		void WriteROCEmulationEnableRegister(uint32_t data){ WriteRegister(data, DTC_Register_ROCEmulationEnable); }
		uint32_t ReadROCEmulationEnableRegister() { return ReadRegister(DTC_Register_ROCEmulationEnable); }
		void WriteRingEnableRegister(uint32_t data){ WriteRegister(data, DTC_Register_RingEnable); }
		uint32_t ReadRingEnableRegister() { return ReadRegister(DTC_Register_RingEnable); }
		void WriteSERDESResetRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESReset); }
		uint32_t ReadSERDESResetRegister() { return ReadRegister(DTC_Register_SERDESReset); }
		bool ReadSERDESResetDone(const DTC_Ring_ID& ring);
		uint32_t ReadSERDESRXDisparityErrorRegister() { return ReadRegister(DTC_Register_SERDESRXDisparityError); }
		void WriteSERDESRXDisparityErrorRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESRXDisparityError); }
		uint32_t ReadSERDESRXCharacterNotInTableErrorRegister() { return ReadRegister(DTC_Register_SERDESRXCharacterNotInTableError); }
		void WriteSERDESRXCharacterNotInTableErrorRegister(uint32_t data) { WriteRegister(data, DTC_Register_SERDESRXCharacterNotInTableError); }
		uint32_t ReadSERDESUnlockErrorRegister() { return ReadRegister(DTC_Register_SERDESUnlockError); }
		void WriteSERDESUnlockErrorRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESUnlockError); }
		uint32_t ReadSERDESPLLLockedRegister() { return ReadRegister(DTC_Register_SERDESPLLLocked); }
		uint32_t ReadSERDESTXBufferStatusRegister() { return ReadRegister(DTC_Register_SERDESTXBufferStatus); }
		uint32_t ReadSERDESRXBufferStatusRegister() { return ReadRegister(DTC_Register_SERDESRXBufferStatus); }
		uint32_t ReadSERDESRXStatusRegister() { return ReadRegister(DTC_Register_SERDESRXStatus); }
		uint32_t ReadSERDESResetDoneRegister() { return ReadRegister(DTC_Register_SERDESResetDone); }
		uint32_t ReadSERDESEyescanErrorRegister() { return ReadRegister(DTC_Register_SERDESEyescanData); }
		void WriteSERDESEyescanErrorRegister(uint32_t data) { WriteRegister(data, DTC_Register_SERDESEyescanData); }
		uint32_t ReadSERDESRXCDRLockRegister() { return ReadRegister(DTC_Register_SERDESRXCDRLock); }
		void WriteDMATimeoutPresetRegister(uint32_t data) { WriteRegister(data, DTC_Register_DMATimeoutPreset); }
		uint32_t ReadDMATimeoutPresetRegister() { return ReadRegister(DTC_Register_DMATimeoutPreset); }
		void WriteDataPendingTimerRegister(uint32_t data) { WriteRegister(data, DTC_Register_DataPendingTimer); }
		uint32_t ReadDataPendingTimerRegister() { return ReadRegister(DTC_Register_DataPendingTimer); }
		void WriteDMAPacketSizetRegister(uint32_t data) { WriteRegister(data, DTC_Register_DMATransferLength); }
		uint32_t ReadDMAPacketSizeRegister() { return ReadRegister(DTC_Register_DMATransferLength); }
		void WriteTimestampPreset0Register(uint32_t data){ WriteRegister(data, DTC_Register_TimestampPreset0); }
		uint32_t ReadTimestampPreset0Register() { return ReadRegister(DTC_Register_TimestampPreset0); }
		void WriteTimestampPreset1Register(uint32_t data){ WriteRegister(data, DTC_Register_TimestampPreset1); }
		uint32_t ReadTimestampPreset1Register() { return ReadRegister(DTC_Register_TimestampPreset1); }
		uint32_t ReadFPGAPROMProgramStatusRegister() { return ReadRegister(DTC_Register_FPGAPROMProgramStatus); }
		void WriteFPGACoreAccessRegister(uint32_t data){ WriteRegister(data, DTC_Register_FPGACoreAccess); }
		uint32_t ReadFPGACoreAccessRegister() { return ReadRegister(DTC_Register_FPGACoreAccess); }

		void WriteTestCommand(const DTC_TestCommand& comm, bool start);
		DTC_TestCommand ReadTestCommand();

	private:
		DTC_ROC_ID maxRocs_[7];
		mu2edev device_;
		mu2e_databuff_t* buffer_;
		bool simMode_;
	};
}
#endif
