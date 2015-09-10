#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include <cstdlib>
#include <atomic>
#include "mu2edev.hh"

namespace DTCLib {
    class DTC {
    public:
        DTC(DTC_SimMode mode = DTC_SimMode_Disabled);
        virtual ~DTC() = default;

        DTC_SimMode ReadSimMode() { return simMode_; }
        DTC_SimMode SetSimMode(DTC_SimMode mode);

        double GetDeviceTime() { return deviceTime_  / 1000000000.0; }
        void ResetDeviceTime() { deviceTime_ = 0; }


        //
        // DMA Functions
        //
        // Data read-out
        std::vector<void*> GetData(DTC_Timestamp when = DTC_Timestamp());
        std::string GetJSONData(DTC_Timestamp when = DTC_Timestamp());
        mu2edev GetDevice() { return device_; }
        
        // DCS Read/Write Cycle
        void DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t *dataIn);

        // Broadcast Readout
        void SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet = true);

        // For loopback testing...
        void SetFirstRead(bool read) { first_read_ = read; }
        void WriteDMADAQPacket(const DTC_DMAPacket& packet);
        void WriteDMADCSPacket(const DTC_DMAPacket& packet);
        DTC_DataHeaderPacket* ReadNextDAQPacket(int tmo_ms = 0);
        DTC_DCSReplyPacket* ReadNextDCSPacket();
        void ReleaseAllBuffers(const DTC_DMA_Engine& channel) {
            auto start = std::chrono::high_resolution_clock::now();
            device_.release_all(channel); 
            deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now() - start).count();
        }

        //
        // Register IO Functions
        //
        std::string RegDump();
        std::string RingRegDump(const DTC_Ring_ID& ring, std::string id);
        std::string CFORegDump();
        std::string ConsoleFormatRegDump();
        std::string FormatRegister(const DTC_Register& address);
        std::string RegisterRead(const DTC_Register& address);

        std::string ReadDesignVersion();
        std::string ReadDesignDate();
        std::string ReadDesignVersionNumber();

        void ResetDTC();
        bool ReadResetDTC();

		void EnableCFOEmulation();
		void DisableCFOEmulation();
		bool ReadCFOEmulation();

        void ResetSERDESOscillator();
        bool ReadResetSERDESOscillator();

        void ToggleSERDESOscillatorClock();
        bool ReadSERDESOscillatorClock();

        bool SetExternalSystemClock();
        bool SetInternalSystemClock();
        bool ToggleSystemClockEnable();
        bool ReadSystemClock();

        bool EnableTiming();
        bool DisableTiming();
        bool ToggleTimingEnable();
        bool ReadTimingEnable();

        int SetTriggerDMATransferLength(uint16_t length);
        uint16_t ReadTriggerDMATransferLength();

        int SetMinDMATransferLength(uint16_t length);
        uint16_t ReadMinDMATransferLength();

        DTC_SERDESLoopbackMode SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode);
        DTC_SERDESLoopbackMode ReadSERDESLoopback(const DTC_Ring_ID& ring);

        bool ReadSERDESOscillatorIICError();
        bool ReadSERDESOscillatorInitializationComplete();

        bool EnableROCEmulator(const DTC_Ring_ID& ring);
        bool DisableROCEmulator(const DTC_Ring_ID& ring);
        bool ToggleROCEmulator(const DTC_Ring_ID& ring);
        bool ReadROCEmulator(const DTC_Ring_ID& ring);

        DTC_RingEnableMode EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode(), const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
        DTC_RingEnableMode DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
        DTC_RingEnableMode ToggleRingEnabled(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
        DTC_RingEnableMode ReadRingEnabled(const DTC_Ring_ID& ring);

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

        // Set number of ROCs in a Ring
        DTC_ROC_ID SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc);
        DTC_ROC_ID ReadRingROCCount(const DTC_Ring_ID& ring, bool local = true);

        DTC_FIFOFullErrorFlags WriteFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
        DTC_FIFOFullErrorFlags ToggleFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
        DTC_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring);

        uint32_t ReadROCTimeoutPreset();
        int WriteROCTimeoutPreset(uint32_t preset);

        bool ReadROCTimeoutError(const DTC_Ring_ID& ring);
        bool ClearROCTimeoutError(const DTC_Ring_ID& ring);

        bool ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
        bool ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
        bool ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring);
        bool ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring);
        bool ReadPacketError(const DTC_Ring_ID& ring);
        bool ClearPacketError(const DTC_Ring_ID& ring);
        bool ReadPacketCRCError(const DTC_Ring_ID& ring);
        bool ClearPacketCRCError(const DTC_Ring_ID& ring);

        DTC_Timestamp WriteTimestampPreset(const DTC_Timestamp& preset);
        DTC_Timestamp ReadTimestampPreset();

        bool ReadFPGAPROMProgramFIFOFull();
        bool ReadFPGAPROMReady();

        void ReloadFPGAFirmware();
        bool ReadFPGACoreAccessFIFOFull();
        bool ReadFPGACoreAccessFIFOEmpty();

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
        void ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms = 0);
        void WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet);
        void WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet);

        void WriteRegister(uint32_t data, const DTC_Register& address);
        uint32_t ReadRegister(const DTC_Register& address);

        //Register Helper Functions
        uint32_t ReadDesignVersionNumberRegister() { return ReadRegister(DTC_Register_DesignVersion); }
        uint32_t ReadDesignDateRegister() { return ReadRegister(DTC_Register_DesignDate); }
        void WriteControlRegister(uint32_t data){ WriteRegister(data, DTC_Register_DTCControl); }
        uint32_t ReadControlRegister() { return ReadRegister(DTC_Register_DTCControl); }
        void WriteDMATransferLengthRegister(uint32_t data) { WriteRegister(data, DTC_Register_DMATransferLength); }
        uint32_t ReadDMATransferLengthRegister() { return ReadRegister(DTC_Register_DMATransferLength); }
        void WriteSERDESLoopbackEnableRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESLoopbackEnable); }
        uint32_t ReadSERDESLoopbackEnableRegister() { return ReadRegister(DTC_Register_SERDESLoopbackEnable); }
        void WriteSERDESLoopbackEnableTempRegister(uint32_t data){ WriteRegister(data, DTC_Register_SERDESLoopbackEnable_Temp); }
        uint32_t ReadSERDESLoopbackEnableTempRegister() { return ReadRegister(DTC_Register_SERDESLoopbackEnable_Temp); }
        uint32_t ReadSERDESOscillatorStatusRegister() { return ReadRegister(DTC_Register_SERDESOscillatorStatus); }
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
        void WriteROCTimeoutPresetRegister(uint32_t data) { WriteRegister(data, DTC_Register_ROCReplyTimeout); }
        uint32_t ReadROCTimeoutPresetRegister() { return ReadRegister(DTC_Register_ROCReplyTimeout); }
        void WriteROCTimeoutErrorRegister(uint32_t data) { WriteRegister(data, DTC_Register_ROCTimeoutError); }
        uint32_t ReadROCTimeoutErrorRegister() { return ReadRegister(DTC_Register_ROCTimeoutError); }
        void WriteReceivePacketErrorRegister(uint32_t data) { WriteRegister(data, DTC_Register_ReceivePacketError); }
        uint32_t ReadReceivePacketErrorRegister() { return ReadRegister(DTC_Register_ReceivePacketError); }
        void WriteDataPendingTimerRegister(uint32_t data) { WriteRegister(data, DTC_Register_DataPendingTimer); }
        uint32_t ReadDataPendingTimerRegister() { return ReadRegister(DTC_Register_DataPendingTimer); }
        void WriteDMAPacketSizeRegister(uint32_t data) { WriteRegister(data, DTC_Register_PacketSize); }
        uint32_t ReadDMAPacketSizeRegister() { return ReadRegister(DTC_Register_PacketSize); }
        void WriteNUMROCsRegister(uint32_t data){ WriteRegister(data, DTC_Register_NUMROCs); }
        uint32_t ReadNUMROCsRegister() { return ReadRegister(DTC_Register_NUMROCs); }
        void WriteFIFOFullErrorFlag0Register(uint32_t data){ WriteRegister(data, DTC_Register_FIFOFullErrorFlag0); }
        uint32_t ReadFIFOFullErrorFlag0Register() { return ReadRegister(DTC_Register_FIFOFullErrorFlag0); }
        void WriteFIFOFullErrorFlag1Register(uint32_t data){ WriteRegister(data, DTC_Register_FIFOFullErrorFlag1); }
        uint32_t ReadFIFOFullErrorFlag1Register() { return ReadRegister(DTC_Register_FIFOFullErrorFlag1); }
        void WriteFIFOFullErrorFlag2Register(uint32_t data){ WriteRegister(data, DTC_Register_FIFOFullErrorFlag2); }
        uint32_t ReadFIFOFullErrorFlag2Register() { return ReadRegister(DTC_Register_FIFOFullErrorFlag2); }
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
        mu2edev device_;
        mu2e_databuff_t* daqbuffer_;
        mu2e_databuff_t* dcsbuffer_;
        DTC_SimMode simMode_;
        DTC_ROC_ID maxROCs_[6];
        uint16_t dmaSize_;
        uint32_t bufferIndex_;
        bool first_read_;
        uint16_t daqDMAByteCount_;
        uint16_t dcsDMAByteCount_;
        void* lastReadPtr_;
        void* nextReadPtr_;
        void* dcsReadPtr_;
        std::atomic<long long> deviceTime_;
    };
}
#endif
