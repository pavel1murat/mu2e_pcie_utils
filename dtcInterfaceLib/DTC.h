#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include "../linux_driver/user_space/mu2edev.hh"
#ifndef _WIN32
#include "../linux_driver/include/trace.h"
#endif

namespace DTC {
	class DTC {
	public:
		DTC();
		virtual ~DTC() = default;

		const int DTC_BUFFSIZE;

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

		//
		// Register IO Functions
		//
		uint32_t ReadDesignVersion();

		void ResetDTC();

		bool ToggleClearLatchedErrors();
		bool ReadClearLatchedErrors();

		bool ToggleSERDESLoopback(const DTC_Ring_ID& ring);
		bool ReadSERDESLoopback(const DTC_Ring_ID& ring);

		bool ToggleROCEmulator();
		bool ReadROCEmulator();

		bool EnableRing(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
		bool DisableRing(const DTC_Ring_ID& ring);
		bool ReadRingEnabled(const DTC_Ring_ID& ring);

		bool ResetSERDES(const DTC_Ring_ID& ring, int interval = 100);
		
		DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(const DTC_Ring_ID& ring);
		DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);

		bool ReadSERDESUnlockError(const DTC_Ring_ID& ring);
		bool ReadSERDESPLLLocked(const DTC_Ring_ID& ring);
		bool ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring);
		bool ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring);

		DTC_SERDESRXBufferStatus ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring);

		DTC_Timestamp WriteTimestampPreset(const DTC_Timestamp& preset);
		DTC_Timestamp ReadTimestampPreset();

		bool ReadFPGAPROMProgramFIFOFull();
		bool ReadFPGAPROMReady();

	    //
		// PCIe/DMA Status and Performance
		// DMA Testing Engine
		//
		DTC_TestMode StartTest(const DTC_DMA_Engine& dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator);
		DTC_TestMode StopTest(const DTC_DMA_Engine& dma);
		
		DTC_DMAState ReadDMAState(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir);
		std::vector<DTC_DMAStat> ReadDMAStats(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir);
		
		DTC_PCIeState ReadPCIeState();
		DTC_PCIeStat ReadPCIeStats();

	private:
		DTC_DataPacket ReadDataPacket(const DTC_DMA_Engine& channel);
		void WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet);
		DTC_DMAPacket ReadDMAPacket(const DTC_DMA_Engine& channel);
		DTC_DMAPacket ReadDMADAQPacket();
		DTC_DMAPacket ReadDMADCSPacket();
		void WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet);
		void WriteDMADAQPacket(const DTC_DMAPacket& packet);
		void WriteDMADCSPacket(const DTC_DMAPacket& packet);
		
		void WriteRegister(uint32_t data, uint16_t address);
		uint32_t ReadRegister(uint16_t address);
		void WriteControlRegister(uint32_t data);
		uint32_t ReadControlRegister();
		void WriteSERDESLoopbackEnableRegister(uint32_t data);
		uint32_t ReadSERDESLoopbackEnableRegister();
		void WriteROCEmulationEnableRegister(uint32_t data);
		uint32_t ReadROCEmulationEnableRegister();
		void WriteRingEnableRegister(uint32_t data);
		uint32_t ReadRingEnableRegister();
		void WriteSERDESResetRegister(uint32_t data);
		uint32_t ReadSERDESResetRegister();
		bool ReadSERDESResetDone(const DTC_Ring_ID& ring);
		uint32_t ReadSERDESRXDisparityErrorRegister();
		uint32_t ReadSERDESRXCharacterNotInTableErrorRegister();
		uint32_t ReadSERDESUnlockErrorRegister();
		uint32_t ReadSERDESPLLLockedRegister();
		uint32_t ReadSERDESTXBufferStatusRegister();
		uint32_t ReadSERDESRXBufferStatusRegister();
		uint32_t ReadSERDESResetDoneRegister();
		void WriteTimestampPreset0Register(uint32_t data);
		uint32_t ReadTimestampPreset0Register();
		void WriteTimestampPreset1Register(uint32_t data);
		uint32_t ReadTimestampPreset1Register();
		void WriteFPGAPROMProgramDataRegister(uint32_t data);
		uint32_t ReadFPGAPROMProgramStatusRegister();

		void WriteTestCommand(const DTC_TestCommand& comm, bool start);
		DTC_TestCommand ReadTestCommand();

	private:
		DTC_ROC_ID maxRocs_[6];
		mu2edev device_;
		mu2e_databuff_t* buffer_;
	};
};

#endif
