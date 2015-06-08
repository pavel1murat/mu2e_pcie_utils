#ifndef CFO_H
#define CFO_H

#include "CFO_Types.h"
#include <vector>
#include <cstdlib>
#include "mu2edev.hh"

namespace CFOLib {
    class CFO {
    public:
        CFO(CFO_SimMode mode = CFO_SimMode_Disabled);
        virtual ~CFO() = default;

        const int CFO_BUFFSIZE;

        CFO_SimMode ReadSimMode() { return simMode_; }
        CFO_SimMode SetSimMode(CFO_SimMode mode);

        //
        // DMA Functions
        //
        void WriteCFOTable();

        //
        // Register IO Functions
        //
        std::string RegDump();
        std::string RingRegDump(const CFO_Ring_ID& ring, std::string id);
        std::string ConsoleFormatRegDump();
        std::string FormatRegister(const CFO_Register& address);

        std::string RegisterRead(const CFO_Register& address);

        std::string ReadDesignVersion();
        std::string ReadDesignDate();
        std::string ReadDesignVersionNumber();

        void ResetCFO();
        bool ReadResetCFO();

        void ResetSERDESOscillator();
        bool ReadResetSERDESOscillator();

        void ToggleSERDESOscillatorClock();
        bool ReadSERDESOscillatorClock();

        void ToggleRRPTableStartAddress();
        bool ReadResetRRPTableStartAddress();

        bool SetExternalSystemClock();
        bool SetInternalSystemClock();
        bool ToggleSystemClockEnable();
        bool ReadSystemClock();

        int SetTriggerDMATransferLength(uint16_t length);
        uint16_t ReadTriggerDMATransferLength();

        int SetMinDMATransferLength(uint16_t length);
        uint16_t ReadMinDMATransferLength();

        CFO_SERDESLoopbackMode SetSERDESLoopbackMode(const CFO_Ring_ID& ring, const CFO_SERDESLoopbackMode& mode);
        CFO_SERDESLoopbackMode ReadSERDESLoopback(const CFO_Ring_ID& ring);

        bool ReadSERDESOscillatorIICError();
        bool ReadSERDESOscillatorInitializationComplete();

        CFO_RingEnableMode EnableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode = CFO_RingEnableMode(), const int cfoCount = 0);
        CFO_RingEnableMode DisableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode = CFO_RingEnableMode());
        CFO_RingEnableMode ToggleRingEnabled(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode = CFO_RingEnableMode());
        CFO_RingEnableMode ReadRingEnabled(const CFO_Ring_ID& ring);

        bool ResetSERDES(const CFO_Ring_ID& ring, int interval = 100);
        bool ReadResetSERDES(const CFO_Ring_ID& ring);
        bool ReadResetSERDESDone(const CFO_Ring_ID& ring);

        CFO_SERDESRXDisparityError ReadSERDESRXDisparityError(const CFO_Ring_ID& ring);
        CFO_SERDESRXDisparityError ClearSERDESRXDisparityError(const CFO_Ring_ID& ring);
        CFO_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const CFO_Ring_ID& ring);
        CFO_CharacterNotInTableError ClearSERDESRXCharacterNotInTableError(const CFO_Ring_ID& ring);

        bool ReadSERDESUnlockError(const CFO_Ring_ID& ring);
        bool ClearSERDESUnlockError(const CFO_Ring_ID& ring);
        bool ReadSERDESPLLLocked(const CFO_Ring_ID& ring);
        bool ReadSERDESOverflowOrUnderflow(const CFO_Ring_ID& ring);
        bool ReadSERDESBufferFIFOHalfFull(const CFO_Ring_ID& ring);

        CFO_RXBufferStatus ReadSERDESRXBufferStatus(const CFO_Ring_ID& ring);

        CFO_RXStatus ReadSERDESRXStatus(const CFO_Ring_ID& ring);

        bool ReadSERDESEyescanError(const CFO_Ring_ID& ring);
        bool ClearSERDESEyescanError(const CFO_Ring_ID& ring);
        bool ReadSERDESRXCDRLock(const CFO_Ring_ID& ring);

        int WriteDMATimeoutPreset(uint32_t preset);
        uint32_t ReadDMATimeoutPreset();
        int WriteDataPendingTimer(uint32_t timer);
        uint32_t ReadDataPendingTimer();
        int SetPacketSize(uint16_t packetSize);
        uint16_t ReadPacketSize();

        uint32_t ReadReadoutRequestInfoTableSize();
        uint32_t WriteReadoutRequestInfoTableSize(uint32_t size);

        // Set number of CFOs in a Ring
        int WriteRingCFOCount(const CFO_Ring_ID& ring, const int count);
        int ReadRingCFOCount(const CFO_Ring_ID& ring);

        CFO_FIFOFullErrorFlags WriteFIFOFullErrorFlags(const CFO_Ring_ID& ring, const CFO_FIFOFullErrorFlags& flags);
        CFO_FIFOFullErrorFlags ToggleFIFOFullErrorFlags(const CFO_Ring_ID& ring, const CFO_FIFOFullErrorFlags& flags);
        CFO_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const CFO_Ring_ID& ring);

        CFO_Timestamp WriteTimestampPreset(const CFO_Timestamp& preset);
        CFO_Timestamp ReadTimestampPreset();

        bool ReadFPGAPROMProgramFIFOFull();
        bool ReadFPGAPROMReady();

        void ReloadFPGAFirmware();
        bool ReadFPGACoreAccessFIFOFull();
        bool ReadFPGACoreAccessFIFOEmpty();

        //
        // PCIe/DMA Status and Performance
        // DMA Testing Engine
        //
        CFO_TestMode StartTest(int packetSize, bool loopback, bool txChecker, bool rxGenerator);
        CFO_TestMode StopTest();

        CFO_DMAState ReadDMAState(const DTC_DMA_Direction& dir);
        CFO_DMAStats ReadDMAStats(const DTC_DMA_Direction& dir);

        CFO_PCIeState ReadPCIeState();
        CFO_PCIeStat ReadPCIeStats();

    private:
        mu2e_databuff_t* ReadBuffer(int tmo_ms = 0);

        void WriteRegister(uint32_t data, const CFO_Register& address);
        uint32_t ReadRegister(const CFO_Register& address);

        bool ReadSERDESResetDone(const CFO_Ring_ID& ring);

        //Register Helper Functions
        uint32_t ReadDesignVersionNumberRegister() { return ReadRegister(CFO_Register_DesignVersion); }
        uint32_t ReadDesignDateRegister() { return ReadRegister(CFO_Register_DesignDate); }
        void WriteControlRegister(uint32_t data){ WriteRegister(data, CFO_Register_CFOControl); }
        uint32_t ReadControlRegister() { return ReadRegister(CFO_Register_CFOControl); }
        void WriteDMATransferLengthRegister(uint32_t data) { WriteRegister(data, CFO_Register_DMATransferLength); }
        uint32_t ReadDMATransferLengthRegister() { return ReadRegister(CFO_Register_DMATransferLength); }
        void WriteSERDESLoopbackEnableRegister(uint32_t data){ WriteRegister(data, CFO_Register_SERDESLoopbackEnable); }
        uint32_t ReadSERDESLoopbackEnableRegister() { return ReadRegister(CFO_Register_SERDESLoopbackEnable); }
        void WriteSERDESLoopbackEnableTempRegister(uint32_t data){ WriteRegister(data, CFO_Register_SERDESLoopbackEnable_Temp); }
        uint32_t ReadSERDESLoopbackEnableTempRegister() { return ReadRegister(CFO_Register_SERDESLoopbackEnable_Temp); }
        void WriteRingEnableRegister(uint32_t data){ WriteRegister(data, CFO_Register_RingEnable); }
        uint32_t ReadRingEnableRegister() { return ReadRegister(CFO_Register_RingEnable); }
        void WriteSERDESResetRegister(uint32_t data){ WriteRegister(data, CFO_Register_SERDESReset); }
        uint32_t ReadSERDESResetRegister() { return ReadRegister(CFO_Register_SERDESReset); }
        uint32_t ReadSERDESRXDisparityErrorRegister() { return ReadRegister(CFO_Register_SERDESRXDisparityError); }
        void WriteSERDESRXDisparityErrorRegister(uint32_t data){ WriteRegister(data, CFO_Register_SERDESRXDisparityError); }
        uint32_t ReadSERDESRXCharacterNotInTableErrorRegister() { return ReadRegister(CFO_Register_SERDESRXCharacterNotInTableError); }
        void WriteSERDESRXCharacterNotInTableErrorRegister(uint32_t data) { WriteRegister(data, CFO_Register_SERDESRXCharacterNotInTableError); }
        uint32_t ReadSERDESUnlockErrorRegister() { return ReadRegister(CFO_Register_SERDESUnlockError); }
        void WriteSERDESUnlockErrorRegister(uint32_t data){ WriteRegister(data, CFO_Register_SERDESUnlockError); }
        uint32_t ReadSERDESPLLLockedRegister() { return ReadRegister(CFO_Register_SERDESPLLLocked); }
        uint32_t ReadSERDESTXBufferStatusRegister() { return ReadRegister(CFO_Register_SERDESTXBufferStatus); }
        uint32_t ReadSERDESRXBufferStatusRegister() { return ReadRegister(CFO_Register_SERDESRXBufferStatus); }
        uint32_t ReadSERDESRXStatusRegister() { return ReadRegister(CFO_Register_SERDESRXStatus); }
        uint32_t ReadSERDESResetDoneRegister() { return ReadRegister(CFO_Register_SERDESResetDone); }
        uint32_t ReadSERDESEyescanErrorRegister() { return ReadRegister(CFO_Register_SERDESEyescanData); }
        void WriteSERDESEyescanErrorRegister(uint32_t data) { WriteRegister(data, CFO_Register_SERDESEyescanData); }
        uint32_t ReadSERDESRXCDRLockRegister() { return ReadRegister(CFO_Register_SERDESRXCDRLock); }
        void WriteDMATimeoutPresetRegister(uint32_t data) { WriteRegister(data, CFO_Register_DMATimeoutPreset); }
        uint32_t ReadDMATimeoutPresetRegister() { return ReadRegister(CFO_Register_DMATimeoutPreset); }
        void WriteDataPendingTimerRegister(uint32_t data) { WriteRegister(data, CFO_Register_DataPendingTimer); }
        uint32_t ReadDataPendingTimerRegister() { return ReadRegister(CFO_Register_DataPendingTimer); }
        void WriteDMAPacketSizetRegister(uint32_t data) { WriteRegister(data, CFO_Register_PacketSize); }
        uint32_t ReadDMAPacketSizeRegister() { return ReadRegister(CFO_Register_PacketSize); }
        void WriteRRInfoTableSizeRegister(uint32_t data) { return WriteRegister(data, CFO_Register_RRInfoTableSize); }
        uint32_t ReadRRInfoTableSizeRegister() { return ReadRegister(CFO_Register_RRInfoTableSize); }
        void WriteNUMCFOsRegister(uint32_t data){ WriteRegister(data, CFO_Register_NUMCFOs); }
        uint32_t ReadNUMCFOsRegister() { return ReadRegister(CFO_Register_NUMCFOs); }
        void WriteFIFOFullErrorFlag0Register(uint32_t data){ WriteRegister(data, CFO_Register_FIFOFullErrorFlag0); }
        uint32_t ReadFIFOFullErrorFlag0Register() { return ReadRegister(CFO_Register_FIFOFullErrorFlag0); }
        void WriteFIFOFullErrorFlag1Register(uint32_t data){ WriteRegister(data, CFO_Register_FIFOFullErrorFlag1); }
        uint32_t ReadFIFOFullErrorFlag1Register() { return ReadRegister(CFO_Register_FIFOFullErrorFlag1); }
        void WriteFIFOFullErrorFlag2Register(uint32_t data){ WriteRegister(data, CFO_Register_FIFOFullErrorFlag2); }
        uint32_t ReadFIFOFullErrorFlag2Register() { return ReadRegister(CFO_Register_FIFOFullErrorFlag2); }
        void WriteTimestampPreset0Register(uint32_t data){ WriteRegister(data, CFO_Register_TimestampPreset0); }
        uint32_t ReadTimestampPreset0Register() { return ReadRegister(CFO_Register_TimestampPreset0); }
        void WriteTimestampPreset1Register(uint32_t data){ WriteRegister(data, CFO_Register_TimestampPreset1); }
        uint32_t ReadTimestampPreset1Register() { return ReadRegister(CFO_Register_TimestampPreset1); }
        uint32_t ReadFPGAPROMProgramStatusRegister() { return ReadRegister(CFO_Register_FPGAPROMProgramStatus); }
        void WriteFPGACoreAccessRegister(uint32_t data){ WriteRegister(data, CFO_Register_FPGACoreAccess); }
        uint32_t ReadFPGACoreAccessRegister() { return ReadRegister(CFO_Register_FPGACoreAccess); }

        void WriteTestCommand(const CFO_TestCommand& comm, bool start);
        CFO_TestCommand ReadTestCommand();

    private:
        mu2edev device_;
        mu2e_databuff_t* rrqbuffer_;
        CFO_SimMode simMode_;

        bool first_read_;
        void* lastReadPtr_;
        void* nextReadPtr_;
    };
}
#endif
