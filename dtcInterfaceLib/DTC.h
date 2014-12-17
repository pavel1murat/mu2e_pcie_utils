#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include "../linux_driver/user_space/mu2edev.hh"

namespace DTC {
	class DTC {
	public:
		DTC();
		virtual ~DTC() = default;
		//	private:
		//#if DTCLIB_DEBUG
		//	public:
		//#endif
		DTC_ErrorCode ReadDataPacket(DTC_DMA_Engine channel);
		DTC_ErrorCode WriteDataPacket(DTC_DMA_Engine channel, DTC_DataPacket packet);

		DTC_ErrorCode ReadDAQDataPacket();
		DTC_ErrorCode WriteDAQDataPacket(DTC_DataPacket packet);

		DTC_ErrorCode ReadDMAPacket(DTC_DMA_Engine channel);
		DTC_ErrorCode ReadDMADAQPacket();
		DTC_ErrorCode ReadDMADCSPacket();

		DTC_ErrorCode WriteDMAPacket(DTC_DMA_Engine channel, DTC_DMAPacket packet);
		DTC_ErrorCode WriteDMADAQPacket(DTC_DMAPacket packet);
		DTC_ErrorCode WriteDMADCSPacket(DTC_DMAPacket packet);
	public:
		DTC_ErrorCode GetData(const DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp when);
		DTC_ErrorCode DCSRequestReply(const DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t dataIn[12]);
		DTC_ErrorCode SendReadoutRequestPacket(const DTC_Ring_ID ring, DTC_Timestamp when, DTC_ROC_ID roc = DTC_ROC_5);

		//private:
		//#if DTCLIB_DEBUG
		//public:
		//#endif
		DTC_ErrorCode WriteRegister(uint32_t data, uint16_t address);
		DTC_ErrorCode ReadRegister(uint16_t address);

		DTC_ErrorCode WriteControlRegister(uint32_t data);
		DTC_ErrorCode ReadControlRegister();
		DTC_ErrorCode WriteSERDESLoopbackEnableRegister(uint32_t data);
		DTC_ErrorCode ReadSERDESLoopbackEnableRegister();
		DTC_ErrorCode WriteROCEmulationEnableRegister(uint32_t data);
		DTC_ErrorCode ReadROCEmulationEnableRegister();
		DTC_ErrorCode WriteRingEnableRegister(uint32_t data);
		DTC_ErrorCode ReadRingEnableRegister();
		DTC_ErrorCode WriteSERDESResetRegister(uint32_t data);
		DTC_ErrorCode ReadSERDESResetRegister();
		DTC_ErrorCode ReadSERDESRXDisparityErrorRegister();
		DTC_ErrorCode ReadSERDESRXCharacterNotInTableErrorRegister();
		DTC_ErrorCode ReadSERDESUnlockErrorRegister();
		DTC_ErrorCode ReadSERDESPLLLockedRegister();
		DTC_ErrorCode ReadSERDESTXBufferStatusRegister();
		DTC_ErrorCode ReadSERDESRXBufferStatusRegister();
		DTC_ErrorCode ReadSERDESResetDoneRegister();
		DTC_ErrorCode WriteTimestampPreset0Register(uint32_t data);
		DTC_ErrorCode ReadTimestampPreset0Register();
		DTC_ErrorCode WriteTimestampPreset1Register(uint32_t data);
		DTC_ErrorCode ReadTimestampPreset1Register();
		DTC_ErrorCode WriteFPGAPROMProgramDataRegister(uint32_t data);
		DTC_ErrorCode ReadFPGAPROMProgramStatusRegister();

	public:
		DTC_ErrorCode ResetDTC();
		DTC_ErrorCode ReadResetDTC();

		DTC_ErrorCode ClearLatchedErrors();
		DTC_ErrorCode ReadClearLatchedErrors();
		DTC_ErrorCode ClearClearLatchedErrors();

		DTC_ErrorCode EnableSERDESLoopback(const DTC_Ring_ID ring);
		DTC_ErrorCode DisableSERDESLoopback(const DTC_Ring_ID ring);
		DTC_ErrorCode ReadSERDESLoopback(const DTC_Ring_ID ring);

		DTC_ErrorCode EnableROCEmulator();
		DTC_ErrorCode DisableROCEmulator();
		DTC_ErrorCode ReadROCEmulatorEnabled();

		DTC_ErrorCode EnableRing(const DTC_Ring_ID ring);
		DTC_ErrorCode DisableRing(const DTC_Ring_ID ring);
		DTC_ErrorCode ReadRingEnabled(const DTC_Ring_ID ring);

		DTC_ErrorCode ResetSERDES(const DTC_Ring_ID ring, int interval);
		DTC_ErrorCode ReadResetSERDES(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESRXDisparityError(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESUnlockError(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESPLLLocked(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID ring);
		DTC_ErrorCode ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESRXBufferStatus(const DTC_Ring_ID ring);

		DTC_ErrorCode ReadSERDESResetDone(const DTC_Ring_ID ring);


		DTC_ErrorCode WriteTimestampPreset(DTC_Timestamp preset);
		DTC_ErrorCode ReadTimestampPreset();

		DTC_ErrorCode ReadFPGAPROMProgramFIFOFull();
		DTC_ErrorCode ReadFPGAPROMReady();

		//DMA/PCIe Monitoring Methods
	public:
		DTC_ErrorCode WriteTestCommand(DTC_TestCommand comm);
		DTC_ErrorCode ReadTestCommand();
	public:
		DTC_ErrorCode StartTest(DTC_DMA_Engine dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator);
		DTC_ErrorCode StopTest(DTC_DMA_Engine dma);
		DTC_ErrorCode ReadDMAStateData(DTC_DMA_Engine dma, DTC_DMA_Direction dir);
		DTC_ErrorCode ReadDMAStatsData();
			DTC_ErrorCode ReadPCIeStateData();
			DTC_ErrorCode ReadPCIeStatsData();

	public:
		DTC_DataPacket ReadDataPacket() { return dataPacket_; }
		DTC_DMAPacket ReadDMAPacket() { return dmaPacket_; }
		DTC_Timestamp ReadTimestamp() { return timestampPreset_; }
		DTC_SERDESRXBufferStatus ReadRXBufferStatus() { return SERDESRXBufferStatus_; }
		DTC_CharacterNotInTableError ReadCNITError() { return CharacterNotInTableError_; }
		DTC_SERDESRXDisparityError ReadRXDisparityError() { return SERDESRXDisparityError_; }
		bool ReadBooleanValue() { return booleanValue_; }
		uint32_t ReadDataWord() { return dataWord_; }
		std::vector<uint8_t> ReadDataVector() { return dataVector_; }
		DTC_DMAState ReadDMAState() { return dmaState_; }
		std::vector<DTC_DMAStat> ReadDMAStats(DTC_DMA_Engine dma, DTC_DMA_Direction dir) { return dmaStats_.getData(dma, dir); }
		DTC_PCIeState ReadPCIeState() { return pcieState_; }
		DTC_PCIeStats ReadPCIeStats() { return pcieStats_; }

	private:
		DTC_DataPacket dataPacket_;
		DTC_DMAPacket dmaPacket_;
		DTC_Timestamp timestampPreset_;
		DTC_SERDESRXBufferStatus SERDESRXBufferStatus_;
		DTC_CharacterNotInTableError CharacterNotInTableError_;
		DTC_SERDESRXDisparityError SERDESRXDisparityError_;
		DTC_TestCommand testCommand_;
		DTC_DMAState dmaState_;
		DTC_DMAStats dmaStats_;
		DTC_PCIeState pcieState_;
		DTC_PCIeStats pcieStats_;
		bool booleanValue_;
		uint32_t dataWord_;
		std::vector<uint8_t> dataVector_;
		mu2edev device_;
	};
};

#endif
