#ifndef DTC_REGISTERS_H
#define DTC_REGISTERS_H

#include <bitset> // std::bitset


#include <cstdint> // uint8_t, uint16_t


#include <vector> // std::vector


#include "DTC_Types.h"
#include "mu2edev.h"

namespace DTCLib
{
	enum DTC_Register : uint16_t
	{
		DTC_Register_DesignVersion = 0x9000,
		DTC_Register_DesignDate = 0x9004,
		DTC_Register_DesignStatus = 0x9008,
		DTC_Register_PerfMonTXByteCount = 0x900C,
		DTC_Register_PerfMonRXByteCount = 0x9010,
		DTC_Register_PerfMonTXPayloadCount = 0x9014,
		DTC_Register_PerfMonRXPayloadCount = 0x9018,
		DTC_Register_PerfMonInitCDC = 0x901C,
		DTC_Register_PerfMonInitCHC = 0x9020,
		DTC_Register_PerfMonInitNPDC = 0x9024,
		DTC_Register_PerfMonInitNPHC = 0x9028,
		DTC_Register_PerfMonInitPDC = 0x902C,
		DTC_Register_PerfMonInitPHC = 0x9030,
		DTC_Register_DTCControl = 0x9100,
		DTC_Register_DMATransferLength = 0x9104,
		DTC_Register_SERDESLoopbackEnable = 0x9108,
		DTC_Register_SERDESOscillatorStatus = 0x910C,
		DTC_Register_ROCEmulationEnable = 0x9110,
		DTC_Register_RingEnable = 0x9114,
		DTC_Register_SERDESReset = 0x9118,
		DTC_Register_SERDESRXDisparityError = 0x911C,
		DTC_Register_SERDESRXCharacterNotInTableError = 0x9120,
		DTC_Register_SERDESUnlockError = 0x9124,
		DTC_Register_SERDESPLLLocked = 0x9128,
		DTC_Register_SERDESTXBufferStatus = 0x912C,
		DTC_Register_SERDESRXBufferStatus = 0x9130,
		DTC_Register_SERDESRXStatus = 0x9134,
		DTC_Register_SERDESResetDone = 0x9138,
		DTC_Register_SERDESEyescanData = 0x913C,
		DTC_Register_SERDESRXCDRLock = 0x9140,
		DTC_Register_DMATimeoutPreset = 0x9144,
		DTC_Register_ROCReplyTimeout = 0x9148,
		DTC_Register_ROCReplyTimeoutError = 0x914C,
		DTC_Register_RingPacketLength = 0x9150,
		DTC_Register_TimestampPreset0 = 0x9180,
		DTC_Register_TimestampPreset1 = 0x9184,
		DTC_Register_DataPendingTimer = 0x9188,
		DTC_Register_NUMROCs = 0x918C,
		DTC_Register_FIFOFullErrorFlag0 = 0x9190,
		DTC_Register_FIFOFullErrorFlag1 = 0x9194,
		DTC_Register_FIFOFullErrorFlag2 = 0x9198,
		DTC_Register_ReceivePacketError = 0x919C,
		DTC_Register_CFOEmulationTimestampLow = 0x91A0,
		DTC_Register_CFOEmulationTimestampHigh = 0x91A4,
		DTC_Register_CFOEmulationRequestInterval = 0x91A8,
		DTC_Register_CFOEmulationNumRequests = 0x91AC,
		DTC_Register_CFOEmulationNumPacketsRing0 = 0x91B0,
		DTC_Register_CFOEmulationNumPacketsRing1 = 0x91B4,
		DTC_Register_CFOEmulationNumPacketsRing2 = 0x91B8,
		DTC_Register_CFOEmulationNumPacketsRing3 = 0x91BC,
		DTC_Register_CFOEmulationNumPacketsRing4 = 0x91C0,
		DTC_Register_CFOEmulationNumPacketsRing5 = 0x91C4,
		DTC_Register_CFOEmulationDebugPacketType = 0x91C8,
		DTC_Register_DetEmulationDMACount = 0x91D0,
		DTC_Register_DetEmulationDelayCount = 0x91D4,
		DTC_Register_ReceiveByteCountDataRing0 = 0x9200,
		DTC_Register_ReceiveByteCountDataRing1 = 0x9204,
		DTC_Register_ReceiveByteCountDataRing2 = 0x9208,
		DTC_Register_ReceiveByteCountDataRing3 = 0x920C,
		DTC_Register_ReceiveByteCountDataRing4 = 0x9210,
		DTC_Register_ReceiveByteCountDataRing5 = 0x9214,
		DTC_Register_ReceiveByteCountDataCFO = 0x9218,
		DTC_Register_ReceivePacketCountDataRing0 = 0x9220,
		DTC_Register_ReceivePacketCountDataRing1 = 0x9224,
		DTC_Register_ReceivePacketCountDataRing2 = 0x9228,
		DTC_Register_ReceivePacketCountDataRing3 = 0x922C,
		DTC_Register_ReceivePacketCountDataRing4 = 0x9230,
		DTC_Register_ReceivePacketCountDataRing5 = 0x9234,
		DTC_Register_ReceivePacketCountDataCFO = 0x9238,
		DTC_Register_TransmitByteCountDataRing0 = 0x9240,
		DTC_Register_TransmitByteCountDataRing1 = 0x9244,
		DTC_Register_TransmitByteCountDataRing2 = 0x9248,
		DTC_Register_TransmitByteCountDataRing3 = 0x924C,
		DTC_Register_TransmitByteCountDataRing4 = 0x9250,
		DTC_Register_TransmitByteCountDataRing5 = 0x9254,
		DTC_Register_TransmitByteCountDataCFO = 0x9258,
		DTC_Register_TransmitPacketCountDataRing0 = 0x9260,
		DTC_Register_TransmitPacketCountDataRing1 = 0x9264,
		DTC_Register_TransmitPacketCountDataRing2 = 0x9268,
		DTC_Register_TransmitPacketCountDataRing3 = 0x926C,
		DTC_Register_TransmitPacketCountDataRing4 = 0x9270,
		DTC_Register_TransmitPacketCountDataRing5 = 0x9274,
		DTC_Register_TransmitPacketCountDataCFO = 0x9278,
		DTC_Register_DDRLocalStartAddress = 0x9300,
		DTC_Register_DDRLocalEndAddress = 0x9304,
		DTC_Regsiter_DDRWriteBurstSize = 0x9308,
		DTC_Register_DDRReadBurstSize = 0x930C,
		DTC_Register_FPGAProgramData = 0x9400,
		DTC_Register_FPGAPROMProgramStatus = 0x9404,
		DTC_Register_FPGACoreAccess = 0x9408,
		DTC_Register_Invalid,
	};

	static const std::vector<DTC_Register> DTC_Readable_Registers = {
		DTC_Register_DesignVersion ,
		DTC_Register_DesignDate ,
		DTC_Register_DesignStatus ,
		DTC_Register_DTCControl ,
		DTC_Register_DMATransferLength ,
		DTC_Register_SERDESLoopbackEnable ,
		DTC_Register_SERDESOscillatorStatus ,
		DTC_Register_ROCEmulationEnable ,
		DTC_Register_RingEnable ,
		DTC_Register_SERDESReset ,
		DTC_Register_SERDESRXDisparityError,
		DTC_Register_SERDESRXCharacterNotInTableError,
		DTC_Register_SERDESUnlockError,
		DTC_Register_SERDESPLLLocked,
		DTC_Register_SERDESTXBufferStatus,
		DTC_Register_SERDESRXBufferStatus,
		DTC_Register_SERDESRXStatus,
		DTC_Register_SERDESResetDone,
		DTC_Register_SERDESEyescanData,
		DTC_Register_SERDESRXCDRLock,
		DTC_Register_DMATimeoutPreset,
		DTC_Register_ROCReplyTimeout,
		DTC_Register_ROCReplyTimeoutError,
		DTC_Register_RingPacketLength,
		DTC_Register_TimestampPreset0,
		DTC_Register_TimestampPreset1,
		DTC_Register_DataPendingTimer,
		DTC_Register_NUMROCs,
		DTC_Register_FIFOFullErrorFlag0,
		DTC_Register_FIFOFullErrorFlag1,
		DTC_Register_FIFOFullErrorFlag2,
		DTC_Register_ReceivePacketError,
		DTC_Register_CFOEmulationTimestampLow,
		DTC_Register_CFOEmulationTimestampHigh,
		DTC_Register_CFOEmulationRequestInterval,
		DTC_Register_CFOEmulationNumRequests,
		DTC_Register_CFOEmulationNumPacketsRing0,
		DTC_Register_CFOEmulationNumPacketsRing1,
		DTC_Register_CFOEmulationNumPacketsRing2,
		DTC_Register_CFOEmulationNumPacketsRing3,
		DTC_Register_CFOEmulationNumPacketsRing4,
		DTC_Register_CFOEmulationNumPacketsRing5,
		DTC_Register_CFOEmulationDebugPacketType,
		DTC_Register_DetEmulationDMACount,
		DTC_Register_DetEmulationDelayCount,
		DTC_Register_ReceiveByteCountDataRing0,
		DTC_Register_ReceiveByteCountDataRing1,
		DTC_Register_ReceiveByteCountDataRing2,
		DTC_Register_ReceiveByteCountDataRing3,
		DTC_Register_ReceiveByteCountDataRing4,
		DTC_Register_ReceiveByteCountDataRing5,
		DTC_Register_ReceiveByteCountDataCFO,
		DTC_Register_ReceivePacketCountDataRing0,
		DTC_Register_ReceivePacketCountDataRing1,
		DTC_Register_ReceivePacketCountDataRing2,
		DTC_Register_ReceivePacketCountDataRing3,
		DTC_Register_ReceivePacketCountDataRing4,
		DTC_Register_ReceivePacketCountDataRing5,
		DTC_Register_ReceivePacketCountDataCFO,
		DTC_Register_TransmitByteCountDataRing0,
		DTC_Register_TransmitByteCountDataRing1,
		DTC_Register_TransmitByteCountDataRing2,
		DTC_Register_TransmitByteCountDataRing3,
		DTC_Register_TransmitByteCountDataRing4,
		DTC_Register_TransmitByteCountDataRing5,
		DTC_Register_TransmitByteCountDataCFO,
		DTC_Register_TransmitPacketCountDataRing0,
		DTC_Register_TransmitPacketCountDataRing1,
		DTC_Register_TransmitPacketCountDataRing2,
		DTC_Register_TransmitPacketCountDataRing3,
		DTC_Register_TransmitPacketCountDataRing4,
		DTC_Register_TransmitPacketCountDataRing5,
		DTC_Register_TransmitPacketCountDataCFO,
		DTC_Register_DDRLocalStartAddress,
		DTC_Register_DDRLocalEndAddress,
		DTC_Regsiter_DDRWriteBurstSize,
		DTC_Register_DDRReadBurstSize,
		DTC_Register_FPGAPROMProgramStatus,
		DTC_Register_FPGACoreAccess
	};

	class DTC_Registers
	{
	public:
		DTC_Registers(DTC_SimMode mode = DTC_SimMode_Disabled);

		//
		// Device Access
		//
		mu2edev* GetDevice()
		{
			return &device_;
		}
		double GetDeviceTime() const { return device_.GetDeviceTime(); }
		void ResetDeviceTime() { device_.ResetDeviceTime(); }

		//
		// DTC Sim Mode Virtual Register
		//
		DTC_SimMode ReadSimMode() const
		{
			return simMode_;
		}

		DTC_SimMode SetSimMode(DTC_SimMode mode);

		//
		// DTC Register Dumps
		//
		std::string RegDump();
		std::string RingRegDump(const DTC_Ring_ID& ring, std::string id);
		std::string CFORegDump();
		std::string ConsoleFormatRegDump();
		std::string FormatRegister(const DTC_Register& address);
		std::string RegisterRead(const DTC_Register& address);

		//
		// Register IO Functions
		//

		// Desgin Version/Date Registers
		std::string ReadDesignVersion();
		std::string ReadDesignDate();
		std::string ReadDesignVersionNumber();

		// Collect PCIE Performance Metrics
		DTC_PerfMonCounters ReadPCIEPerformanceMonitor()
		{
			DTC_PerfMonCounters output;
			output.DesignStatus = ReadRegister(DTC_Register_DesignStatus);
			output.TXPCIEByteCount = ReadRegister(DTC_Register_PerfMonTXByteCount);
			output.RXPCIEByteCount = (DTC_Register_PerfMonRXByteCount);
			output.TXPCIEPayloadCount = ReadRegister(DTC_Register_PerfMonTXPayloadCount);
			output.RXPCIEPayloadCount = ReadRegister(DTC_Register_PerfMonRXPayloadCount);
			output.InitialCompletionDataCredits = ReadRegister(DTC_Register_PerfMonInitCDC);
			output.InitialCompletionHeaderCredits = ReadRegister(DTC_Register_PerfMonInitCHC);
			output.InitialNPDCredits = ReadRegister(DTC_Register_PerfMonInitNPDC);
			output.InitialNPHCredits = ReadRegister(DTC_Register_PerfMonInitNPHC);
			output.InitialPDCredits = ReadRegister(DTC_Register_PerfMonInitPDC);
			output.InitialPHCredits = ReadRegister(DTC_Register_PerfMonInitPHC);

			return output;
		}

		// DTC Control Register
		void ResetDTC();
		bool ReadResetDTC();
		void EnableCFOEmulation();
		void DisableCFOEmulation();
		bool ReadCFOEmulation();
		void ResetSERDESOscillator();
		bool ReadResetSERDESOscillator();
		void ToggleSERDESOscillatorClock();
		bool ReadSERDESOscillatorClock();
		void ResetDDRWriteAddress();
		bool ReadResetDDRWriteAddress();
		bool EnableDetectorEmulator();
		bool DisableDetectorEmulator();
		bool ReadDetectorEmulatorEnable();
		bool SetExternalSystemClock();
		bool SetInternalSystemClock();
		bool ToggleSystemClockEnable();
		bool ReadSystemClock();
		bool EnableTiming();
		bool DisableTiming();
		bool ToggleTimingEnable();
		bool ReadTimingEnable();

		// DMA Transfer Length Register
		int SetTriggerDMATransferLength(uint16_t length);
		uint16_t ReadTriggerDMATransferLength();
		int SetMinDMATransferLength(uint16_t length);
		uint16_t ReadMinDMATransferLength();

		// SERDES Loopback Enable Register
		DTC_SERDESLoopbackMode SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode);
		DTC_SERDESLoopbackMode ReadSERDESLoopback(const DTC_Ring_ID& ring);

		// SERDES Status Register
		bool ReadSERDESOscillatorIICError();
		bool ReadSERDESOscillatorInitializationComplete();

		// ROC Emulation Enable Register
		bool EnableROCEmulator(const DTC_Ring_ID& ring);
		bool DisableROCEmulator(const DTC_Ring_ID& ring);
		bool ToggleROCEmulator(const DTC_Ring_ID& ring);
		bool ReadROCEmulator(const DTC_Ring_ID& ring);

		// Ring Enable Register
		DTC_RingEnableMode EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode(), const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
		DTC_RingEnableMode DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
		DTC_RingEnableMode ToggleRingEnabled(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
		DTC_RingEnableMode ReadRingEnabled(const DTC_Ring_ID& ring);

		// SERDES Reset Register
		bool ResetSERDES(const DTC_Ring_ID& ring, int interval = 100);
		bool ReadResetSERDES(const DTC_Ring_ID& ring);

		// SERDES RX Disparity Error Register
		DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(const DTC_Ring_ID& ring);

		// SERDES Character Not In Table Error Register
		DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);

		// SERDES Unlock Error Register
		bool ReadSERDESUnlockError(const DTC_Ring_ID& ring);

		// SERDES PLL Locked Register
		bool ReadSERDESPLLLocked(const DTC_Ring_ID& ring);

		// SERDES TX Buffer Status Register
		bool ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring);
		bool ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring);

		// SERDES RX Buffer Status Register
		DTC_RXBufferStatus ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring);

		// SERDES RX Status Register
		DTC_RXStatus ReadSERDESRXStatus(const DTC_Ring_ID& ring);

		// SERDES Reset Done Register
		bool ReadResetSERDESDone(const DTC_Ring_ID& ring);

		// Eyescan Data Error Register
		bool ReadSERDESEyescanError(const DTC_Ring_ID& ring);

		// SERDES RX CDR Lock Register
		bool ReadSERDESRXCDRLock(const DTC_Ring_ID& ring);

		// DMA Timeout Preset Regsiter
		int WriteDMATimeoutPreset(uint32_t preset);
		uint32_t ReadDMATimeoutPreset();

		// ROC Timeout (Header Packet to All Packets Received) Preset Register
		uint32_t ReadROCTimeoutPreset();
		int WriteROCTimeoutPreset(uint32_t preset);

		// ROC Timeout Error Register
		bool ReadROCTimeoutError(const DTC_Ring_ID& ring);
		bool ClearROCTimeoutError(const DTC_Ring_ID& ring);

		// Ring Packet Length Register
		int SetPacketSize(uint16_t packetSize);
		uint16_t ReadPacketSize();

		// Timestamp Preset Registers
		DTC_Timestamp WriteTimestampPreset(const DTC_Timestamp& preset);
		DTC_Timestamp ReadTimestampPreset();

		// Data Pending Timer Register
		int WriteDataPendingTimer(uint32_t timer);
		uint32_t ReadDataPendingTimer();

		// NUMROCs Register
		DTC_ROC_ID SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc);
		DTC_ROC_ID ReadRingROCCount(const DTC_Ring_ID& ring, bool local = true);

		// FIFO Full Error Flags Registers
		DTC_FIFOFullErrorFlags WriteFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
		DTC_FIFOFullErrorFlags ToggleFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
		DTC_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring);

		// Receive Packet Error Register
		bool ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
		bool ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
		bool ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring);
		bool ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring);
		bool ReadPacketError(const DTC_Ring_ID& ring);
		bool ClearPacketError(const DTC_Ring_ID& ring);
		bool ReadPacketCRCError(const DTC_Ring_ID& ring);
		bool ClearPacketCRCError(const DTC_Ring_ID& ring);

		// CFO Emulation Timestamp Registers 
		void SetCFOEmulationTimestamp(const DTC_Timestamp& ts);
		DTC_Timestamp ReadCFOEmulationTimestamp();

		// CFO Emulation Request Interval Regsister
		void SetCFOEmulationRequestInterval(uint32_t interval);
		uint32_t ReadCFOEmulationRequestInterval();

		// CFO Emulation Number of Requests Register
		void SetCFOEmulationNumRequests(uint32_t numRequests);
		uint32_t ReadCFOEmulationNumRequests();

		// CFO Emulation Number of Packets Registers
		void SetCFOEmulationNumPackets(const DTC_Ring_ID& ring, uint16_t numPackets);
		uint16_t ReadCFOEmulationNumPackets(const DTC_Ring_ID& ring);

		// CFO Emulation Debug Packet Type Register
		void SetCFOEmulationDebugType(DTC_DebugType type);
		DTC_DebugType ReadCFOEmulationDebugType();
		// Detector Emulation DMA Count Register
		void SetDetectorEmulationDMACount(uint32_t count);
		uint32_t ReadDetectorEmulationDMACount();
		void IncrementDetectorEmulationDMACount();

		// Detector Emulation DMA Delay Count Register
		void SetDetectorEmulationDMADelayCount(uint32_t count);
		uint32_t ReadDetectorEmulationDMADelayCount();

		// DDR Local End Address Register
		void SetDDRLocalEndAddress(uint32_t address);
		uint32_t ReadDDRLocalEndAddress();
		void IncrementDDRLocalEndAddress(size_t sz);

		// FPGA PROM Program Status Register
		bool ReadFPGAPROMProgramFIFOFull();
		bool ReadFPGAPROMReady();

		// FPGA Core Access Register
		void ReloadFPGAFirmware();
		bool ReadFPGACoreAccessFIFOFull();
		bool ReadFPGACoreAccessFIFOEmpty();

	private:
		void WriteRegister(uint32_t data, const DTC_Register& address);
		uint32_t ReadRegister(const DTC_Register& address);

	protected:
		mu2edev device_;
		DTC_SimMode simMode_;
		DTC_ROC_ID maxROCs_[6];
		uint16_t dmaSize_;
	};
}

#endif //DTC_REGISTERS_H


