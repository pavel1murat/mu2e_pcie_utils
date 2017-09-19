#ifndef DTC_REGISTERS_H
#define DTC_REGISTERS_H

//#include <bitset> // std::bitset
//#include <cstdint> // uint8_t, uint16_t
#include <functional> // std::bind, std::function
#include <vector> // std::vector

#include "DTC_Types.h"
#include "mu2edev.h"

namespace DTCLib
{
	enum DTC_Register : uint16_t
	{
		DTC_Register_DesignVersion = 0x9000,
		DTC_Register_DesignDate = 0x9004,
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
		DTC_Register_ClockOscillatorStatus = 0x910C,
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
		DTC_Register_SFPSERDESStatus = 0x9140,
		DTC_Register_DMATimeoutPreset = 0x9144,
		DTC_Register_ROCReplyTimeout = 0x9148,
		DTC_Register_ROCReplyTimeoutError = 0x914C,
		DTC_Register_RingPacketLength = 0x9150,
		DTC_Register_EVBPartitionID = 0x9154,
		DTC_Register_EVBDestCount = 0x9158,
		DTC_Register_HeartbeatErrorFlags = 0x915c,
		DTC_Register_SERDESOscillatorFrequency = 0x9160,
		DTC_Register_SERDESOscillatorControl = 0x9164,
		DTC_Register_SERDESOscillatorParameterLow = 0x9168,
		DTC_Register_SERDESOscillatorParameterHigh = 0x916C,
		DTC_Register_DDROscillatorFrequency = 0x9170,
		DTC_Register_DDROscillatorControl = 0x9174,
		DTC_Register_DDROscillatorParameterLow = 0x9178,
		DTC_Register_DDROscillatorParameterHigh = 0x917C,
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
		DTC_Register_CFOEmulationNumPacketsRings10 = 0x91B0,
		DTC_Register_CFOEmulationNumPacketsRings32 = 0x91B4,
		DTC_Register_CFOEmulationNumPacketsRings54 = 0x91B8,
		DTC_Register_CFOEmulationEventMode1 = 0x91C0,
		DTC_Register_CFOEmulationEventMode2 = 0x91C4,
		DTC_Register_CFOEmulationDebugPacketType = 0x91C8,
		DTC_Register_DetEmulationDMACount = 0x91D0,
		DTC_Register_DetEmulationDelayCount = 0x91D4,
		DTC_Register_DetEmulationControl0 = 0x91D8,
		DTC_Register_DetEmulationControl1 = 0x91DC,
		DTC_Register_DetEmulationDataStartAddress = 0x91E0,
		DTC_Register_DetEmulationDataEndAddress = 0x91E4,
		DTC_Register_ROCDRPDataSyncError = 0x91E8,
		DTC_Register_EthernetFramePayloadSize = 0x91EC,
		DTC_Register_ReceiveByteCountDataRing0 = 0x9200,
		DTC_Register_ReceiveByteCountDataRing1 = 0x9204,
		DTC_Register_ReceiveByteCountDataRing2 = 0x9208,
		DTC_Register_ReceiveByteCountDataRing3 = 0x920C,
		DTC_Register_ReceiveByteCountDataRing4 = 0x9210,
		DTC_Register_ReceiveByteCountDataRing5 = 0x9214,
		DTC_Register_ReceiveByteCountDataCFO = 0x9218,
		DTC_Register_ReceiveByteCountDataEVB = 0x921C,
		DTC_Register_ReceivePacketCountDataRing0 = 0x9220,
		DTC_Register_ReceivePacketCountDataRing1 = 0x9224,
		DTC_Register_ReceivePacketCountDataRing2 = 0x9228,
		DTC_Register_ReceivePacketCountDataRing3 = 0x922C,
		DTC_Register_ReceivePacketCountDataRing4 = 0x9230,
		DTC_Register_ReceivePacketCountDataRing5 = 0x9234,
		DTC_Register_ReceivePacketCountDataCFO = 0x9238,
		DTC_Register_ReceivePacketCountDataEVB = 0x923C,
		DTC_Register_TransmitByteCountDataRing0 = 0x9240,
		DTC_Register_TransmitByteCountDataRing1 = 0x9244,
		DTC_Register_TransmitByteCountDataRing2 = 0x9248,
		DTC_Register_TransmitByteCountDataRing3 = 0x924C,
		DTC_Register_TransmitByteCountDataRing4 = 0x9250,
		DTC_Register_TransmitByteCountDataRing5 = 0x9254,
		DTC_Register_TransmitByteCountDataCFO = 0x9258,
		DTC_Register_TransmitByteCountDataEVB = 0x925C,
		DTC_Register_TransmitPacketCountDataRing0 = 0x9260,
		DTC_Register_TransmitPacketCountDataRing1 = 0x9264,
		DTC_Register_TransmitPacketCountDataRing2 = 0x9268,
		DTC_Register_TransmitPacketCountDataRing3 = 0x926C,
		DTC_Register_TransmitPacketCountDataRing4 = 0x9270,
		DTC_Register_TransmitPacketCountDataRing5 = 0x9274,
		DTC_Register_TransmitPacketCountDataCFO = 0x9278,
		DTC_Register_TransmitPacketCountDataEVB = 0x927C,
		DTC_Register_DDRRingBufferFullFlags1 = 0x92B0,
		DTC_Register_DDRRingBufferFullFlags2 = 0x92B4,
		DTC_Register_DDRRingBufferFullErrorFlags1 = 0x92B8,
		DTC_Register_DDRRingBufferFullErrorFlags2 = 0x92BC,
		DTC_Register_DDRRingBufferEmptyFlags1 = 0x92C0,
		DTC_Register_DDRRingBufferEmptyFlags2 = 0x92C4,
		DTC_Register_DDRRingBufferHalfFullFlags1 = 0x92C8,
		DTC_Register_DDRRingBufferHalfFullFlags2 = 0x92CC,
		DTC_Register_EventBuilderBufferFullFlags1 = 0x92D0,
		DTC_Register_EventBuilderBufferFullFlags2 = 0x92D4,
		DTC_Register_EventBuilderBufferFullErrorFlags1 = 0x92D8,
		DTC_Register_EventBuilderBufferFullErrorFlags2 = 0x92DC,
		DTC_Register_EventBuilderBufferEmptyFlags1 = 0x92E0,
		DTC_Register_EventBuilderBufferEmptyFlags2 = 0x92E4,
		DTC_Register_EventBuilderBufferHalfFullFlags1 = 0x92E8,
		DTC_Register_EventBuilderBufferHalfFullFlags2 = 0x92EC,
		DTC_Register_EVBSERDESPRBSControlStatus = 0x9330,
		DTC_Register_MissedCFOPacketCountRing0 = 0x9340,
		DTC_Register_MissedCFOPacketCountRing1 = 0x9344,
		DTC_Register_MissedCFOPacketCountRing2 = 0x9348,
		DTC_Register_MissedCFOPacketCountRing3 = 0x934C,
		DTC_Register_MissedCFOPacketCountRing4 = 0x9350,
		DTC_Register_MissedCFOPacketCountRing5 = 0x9354,
		DTC_Register_LocalFragmentDropCount = 0x9360,
		DTC_Register_FPGAProgramData = 0x9400,
		DTC_Register_FPGAPROMProgramStatus = 0x9404,
		DTC_Register_FPGACoreAccess = 0x9408,
		DTC_Register_EventModeLookupTableStart = 0x9500,
		DTC_Register_EventModeLookupTableEnd = 0x98FC,
		DTC_Register_Invalid,
	};

	class DTC_Registers
	{
	public:
		explicit DTC_Registers(DTC_SimMode mode = DTC_SimMode_Disabled, unsigned rocMask = 0x1, bool skipInit = false);
		virtual ~DTC_Registers();

		//
		// Device Access
		//
		mu2edev* GetDevice()
		{
			return &device_;
		}

		//
		// DTC Sim Mode Virtual Register
		//
		DTC_SimMode ReadSimMode() const
		{
			return simMode_;
		}

		DTC_SimMode SetSimMode(DTC_SimMode mode, unsigned rocMask);

		//
		// DTC Register Dumps

		std::string FormattedRegDump(int width);
		std::string PerformanceMonitorRegDump(int width);
		std::string RingCountersRegDump(int width);

		DTC_RegisterFormatter CreateFormatter(const DTC_Register& address)
		{
			DTC_RegisterFormatter form;
			form.descWidth = formatterWidth_;
			form.address = address;
			form.value = ReadRegister_(address);
			return form;
		}

		//
		// Register IO Functions
		//

		// Desgin Version/Date Registers
		std::string ReadDesignVersion();
		DTC_RegisterFormatter FormatDesignVersion();
		std::string ReadDesignDate();
		DTC_RegisterFormatter FormatDesignDate();
		std::string ReadDesignVersionNumber();

		// PCIE Performance Monitor Registers
		uint32_t ReadPerfMonTXByteCount();
		DTC_RegisterFormatter FormatPerfMonTXByteCount();
		uint32_t ReadPerfMonRXByteCount();
		DTC_RegisterFormatter FormatPerfMonRXByteCount();
		uint32_t ReadPerfMonTXPayloadCount();
		DTC_RegisterFormatter FormatPerfMonTXPayloadCount();
		uint32_t ReadPerfMonRXPayloadCount();
		DTC_RegisterFormatter FormatPerfMonRXPayloadCount();
		uint16_t ReadPerfMonInitCDC();
		DTC_RegisterFormatter FormatPerfMonInitCDC();
		uint8_t ReadPerfMonInitCHC();
		DTC_RegisterFormatter FormatPerfMonInitCHC();
		uint16_t ReadPerfMonInitNPDC();
		DTC_RegisterFormatter FormatPerfMonInitNPDC();
		uint8_t ReadPerfMonInitNPHC();
		DTC_RegisterFormatter FormatPerfMonInitNPHC();
		uint16_t ReadPerfMonInitPDC();
		DTC_RegisterFormatter FormatPerfMonInitPDC();
		uint8_t ReadPerfMonInitPHC();
		DTC_RegisterFormatter FormatPerfMonInitPHC();

		// DTC Control Register
		void ResetDTC();
		bool ReadResetDTC();
		void EnableCFOEmulation();
		void DisableCFOEmulation();
		bool ReadCFOEmulation();
		void ResetDDRWriteAddress();
		bool ReadResetDDRWriteAddress();
		void ResetDDRReadAddress();
		bool ReadResetDDRReadAddress();
		void ResetDDR();
		bool ReadResetDDR();
		void EnableCFOEmulatorDRP();
		void DisableCFOEmulatorDRP();
		bool ReadCFOEmulatorDRP();
		void EnableAutogenDRP();
		void DisableAutogenDRP();
		bool ReadAutogenDRP();
		void EnableSoftwareDRP();
		void DisableSoftwareDRP();
		bool ReadSoftwareDRP();
		void EnableDCSReception();
		void DisableDCSReception();
		bool ReadDCSReception();
		void SetExternalSystemClock();
		void SetInternalSystemClock();
		bool ReadSystemClock();
		void EnableTiming();
		void DisableTiming();
		bool ReadTimingEnable();
		DTC_RegisterFormatter FormatDTCControl();

		// DMA Transfer Length Register
		void SetTriggerDMATransferLength(uint16_t length);
		uint16_t ReadTriggerDMATransferLength();
		void SetMinDMATransferLength(uint16_t length);
		uint16_t ReadMinDMATransferLength();
		DTC_RegisterFormatter FormatDMATransferLength();

		// SERDES Loopback Enable Register
		void SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode);
		DTC_SERDESLoopbackMode ReadSERDESLoopback(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESLoopbackEnable();

		// Clock Status Register
		bool ReadSERDESOscillatorIICError();
		bool ReadSERDESOscillatorInitializationComplete();
		bool WaitForSERDESOscillatorInitializationComplete(double max_wait = 1.0);
		bool ReadDDROscillatorIICError();
		bool ReadDDROscillatorInitializationComplete();
		bool WaitForDDROscillatorInitializationComplete(double max_wait = 1.0);
		DTC_RegisterFormatter FormatClockOscillatorStatus();

		// ROC Emulation Enable Register
		void EnableROCEmulator(const DTC_Ring_ID& ring);
		void DisableROCEmulator(const DTC_Ring_ID& ring);
		bool ReadROCEmulator(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatROCEmulationEnable();

		// Ring Enable Register
		void EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode(), const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
		void DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
		DTC_RingEnableMode ReadRingEnabled(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatRingEnable();

		// SERDES Reset Register
		void ResetSERDES(const DTC_Ring_ID& ring, int interval = 100);
		bool ReadResetSERDES(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESReset();

		// SERDES RX Disparity Error Register
		DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESRXDisparityError();

		// SERDES Character Not In Table Error Register
		DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESRXCharacterNotInTableError();

		// SERDES Unlock Error Register
		bool ReadSERDESUnlockError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESUnlockError();

		// SERDES PLL Locked Register
		bool ReadSERDESPLLLocked(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESPLLLocked();

		// SERDES TX Buffer Status Register
		bool ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring);
		bool ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESTXBufferStatus();

		// SERDES RX Buffer Status Register
		DTC_RXBufferStatus ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESRXBufferStatus();

		// SERDES RX Status Register
		DTC_RXStatus ReadSERDESRXStatus(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESRXStatus();

		// SERDES Reset Done Register
		bool ReadResetSERDESDone(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESResetDone();

		// Eyescan Data Error Register
		bool ReadSERDESEyescanError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESEyescanData();

		// SFP / SERDES Status Register
		bool ReadSERDESSFPPresent(const DTC_Ring_ID& ring);
		bool ReadSERDESSFPLOS(const DTC_Ring_ID& ring);
		bool ReadSERDESSFPTXFault(const DTC_Ring_ID& ring);
		bool ReadSERDESRXCDRLock(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSFPSERDESStatus();

		// DMA Timeout Preset Regsiter
		void SetDMATimeoutPreset(uint32_t preset);
		uint32_t ReadDMATimeoutPreset();
		DTC_RegisterFormatter FormatDMATimeoutPreset();

		// ROC Timeout (Header Packet to All Packets Received) Preset Register
		void SetROCTimeoutPreset(uint32_t preset);
		uint32_t ReadROCTimeoutPreset();
		DTC_RegisterFormatter FormatROCReplyTimeout();

		// ROC Timeout Error Register
		void ClearROCTimeoutError(const DTC_Ring_ID& ring);
		bool ReadROCTimeoutError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatROCReplyTimeoutError();

		// Ring Packet Length Register
		void SetPacketSize(uint16_t packetSize);
		uint16_t ReadPacketSize();
		DTC_RegisterFormatter FormatRingPacketLength();

		// EVB Network Partition ID / EVB Network Local MAC Index Register
		void SetEVBMode(uint8_t mode);
		uint8_t ReadEVBMode();
		void SetEVBLocalParitionID(uint8_t id);
		uint8_t ReadEVBLocalParitionID();
		void SetEVBLocalMACAddress(uint8_t macByte);
		uint8_t ReadEVBLocalMACAddress();
		DTC_RegisterFormatter FormatEVBLocalParitionIDMACIndex();

		// EVB Number of Destination Nodes Register
		void SetEVBStartNode(uint8_t node);
		uint8_t ReadEVBStartNode();
		void SetEVBNumberOfDestinationNodes(uint8_t number);
		uint8_t ReadEVBNumberOfDestinationNodes();
		DTC_RegisterFormatter FormatEVBNumberOfDestinationNodes();

		// Heartbeat Error Register
		bool ReadHeartbeatTimeout(const DTC_Ring_ID& ring);
		bool ReadHeartbeat20Mismatch(const DTC_Ring_ID& ring);
		bool ReadHeartbeat12Mismatch(const DTC_Ring_ID& ring);
		bool ReadHeartbeat01Mismatch(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatHeartbeatError();

		// SERDES Oscillator Registers
		uint32_t ReadSERDESOscillatorFrequency();
		void SetSERDESOscillatorFrequency(uint32_t freq);
		bool ReadSERDESOscillaotrIICFSMEnable();
		void EnableSERDESOscillatorIICFSM();
		void DisableSERDESOscillatorIICFSM();
		bool ReadSERDESOscillatorReadWriteMode();
		void SetSERDESOscillatorWriteMode();
		void SetSERDESOscillatorReadMode();
		uint64_t ReadSERDESOscillatorParameters();
		void SetSERDESOscillatorParameters(uint64_t parameters);
		DTC_SerdesClockSpeed ReadSERDESOscillatorClock();
		void SetSERDESOscillatorClock(DTC_SerdesClockSpeed speed);
		DTC_RegisterFormatter FormatSERDESOscillatorFrequency();
		DTC_RegisterFormatter FormatSERDESOscillatorControl();
		DTC_RegisterFormatter FormatSERDESOscillatorParameterLow();
		DTC_RegisterFormatter FormatSERDESOscillatorParameterHigh();

		// DDR Oscillator Registers
		uint32_t ReadDDROscillatorFrequency();
		void SetDDROscillatorFrequency(uint32_t freq);
		bool ReadDDROscillaotrIICFSMEnable();
		void EnableDDROscillatorIICFSM();
		void DisableDDROscillatorIICFSM();
		bool ReadDDROscillatorReadWriteMode();
		void SetDDROscillatorWriteMode();
		void SetDDROscillatorReadMode();
		uint64_t ReadDDROscillatorParameters();
		void SetDDROscillatorParameters(uint64_t parameters);
		DTC_RegisterFormatter FormatDDROscillatorFrequency();
		DTC_RegisterFormatter FormatDDROscillatorControl();
		DTC_RegisterFormatter FormatDDROscillatorParameterLow();
		DTC_RegisterFormatter FormatDDROscillatorParameterHigh();

		// Timestamp Preset Registers
		void SetTimestampPreset(const DTC_Timestamp& preset);
		DTC_Timestamp ReadTimestampPreset();
		DTC_RegisterFormatter FormatTimestampPreset0();
		DTC_RegisterFormatter FormatTimestampPreset1();

		// Data Pending Timer Register
		void SetDataPendingTimer(uint32_t timer);
		uint32_t ReadDataPendingTimer();
		DTC_RegisterFormatter FormatDataPendingTimer();

		// NUMROCs Register
		void SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc);
		DTC_ROC_ID ReadRingROCCount(const DTC_Ring_ID& ring, bool local = true);
		DTC_RegisterFormatter FormatNUMROCs();

		// FIFO Full Error Flags Registers
		void ClearFIFOFullErrorFlags(const DTC_Ring_ID& ring);
		DTC_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatFIFOFullErrorFlag0();
		DTC_RegisterFormatter FormatFIFOFullErrorFlag1();
		DTC_RegisterFormatter FormatFIFOFullErrorFlag2();

		// Receive Packet Error Register
		void ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
		bool ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
		void ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring);
		bool ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring);
		void ClearPacketError(const DTC_Ring_ID& ring);
		bool ReadPacketError(const DTC_Ring_ID& ring);
		void ClearPacketCRCError(const DTC_Ring_ID& ring);
		bool ReadPacketCRCError(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatReceivePacketError();

		// CFO Emulation Timestamp Registers 
		void SetCFOEmulationTimestamp(const DTC_Timestamp& ts);
		DTC_Timestamp ReadCFOEmulationTimestamp();
		DTC_RegisterFormatter FormatCFOEmulationTimestampLow();
		DTC_RegisterFormatter FormatCFOEmulationTimestampHigh();

		// CFO Emulation Request Interval Regsister
		void SetCFOEmulationRequestInterval(uint32_t interval);
		uint32_t ReadCFOEmulationRequestInterval();
		DTC_RegisterFormatter FormatCFOEmulationRequestInterval();

		// CFO Emulation Number of Requests Register
		void SetCFOEmulationNumRequests(uint32_t numRequests);
		uint32_t ReadCFOEmulationNumRequests();
		DTC_RegisterFormatter FormatCFOEmulationNumRequests();

		// CFO Emulation Number of Packets Registers
		void SetCFOEmulationNumPackets(const DTC_Ring_ID& ring, uint16_t numPackets);
		uint16_t ReadCFOEmulationNumPackets(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing01();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing23();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing45();

		// CFO Emulation Event Mode Bytes Registers
		void SetCFOEmulationModeByte(const uint8_t& byteNum, uint8_t data);
		uint8_t ReadCFOEmulationModeByte(const uint8_t& byteNum);
		DTC_RegisterFormatter FormatCFOEmulationModeBytes03();
		DTC_RegisterFormatter FormatCFOEmulationModeBytes45();

		// CFO Emulation Debug Packet Type Register
		void EnableDebugPacketMode();
		void DisableDebugPacketMode();
		bool ReadDebugPacketMode();
		void SetCFOEmulationDebugType(DTC_DebugType type);
		DTC_DebugType ReadCFOEmulationDebugType();
		DTC_RegisterFormatter FormatCFOEmulationDebugPacketType();

		// Detector Emulation DMA Count Register
		void SetDetectorEmulationDMACount(uint32_t count);
		uint32_t ReadDetectorEmulationDMACount();
		DTC_RegisterFormatter FormatDetectorEmulationDMACount();

		// Detector Emulation DMA Delay Count Register
		void SetDetectorEmulationDMADelayCount(uint32_t count);
		uint32_t ReadDetectorEmulationDMADelayCount();
		DTC_RegisterFormatter FormatDetectorEmulationDMADelayCount();

		// Detector Emulation Control Registers
		void EnableDetectorEmulatorMode();
		void DisableDetectorEmulatorMode();
		bool ReadDetectorEmulatorMode();
		void EnableDetectorEmulator();
		void DisableDetectorEmulator();
		bool ReadDetectorEmulatorEnable();
		bool ReadDetectorEmulatorEnableClear();
		bool IsDetectorEmulatorInUse() const
		{
			return usingDetectorEmulator_;
		}
		void SetDetectorEmulatorInUse() { usingDetectorEmulator_ = true; }
		void ClearDetectorEmulatorInUse();
		DTC_RegisterFormatter FormatDetectorEmulationControl0();
		DTC_RegisterFormatter FormatDetectorEmulationControl1();

		// SERDES Counter Registers
		// DDR Event Data Local Start Address Register
		void SetDDRDataLocalStartAddress(uint32_t address);
		uint32_t ReadDDRDataLocalStartAddress();
		DTC_RegisterFormatter FormatDDRDataLocalStartAddress();

		// DDR Event Data Local End Address Register
		void SetDDRDataLocalEndAddress(uint32_t address);
		uint32_t ReadDDRDataLocalEndAddress();
		DTC_RegisterFormatter FormatDDRDataLocalEndAddress();

		// ROC DRP Sync Error Register
		uint32_t ReadROCDRPSyncErrors();
		DTC_RegisterFormatter FormatROCDRPSyncError();

		// Ethernet Frame Payload Max Size
		uint32_t ReadEthernetPayloadSize();
		void SetEthernetPayloadSize(uint32_t size);
		DTC_RegisterFormatter FormatEthernetPayloadSize();


		void ClearReceiveByteCount(const DTC_Ring_ID& ring);
		uint32_t ReadReceiveByteCount(const DTC_Ring_ID& ring);
		void ClearReceivePacketCount(const DTC_Ring_ID& ring);
		uint32_t ReadReceivePacketCount(const DTC_Ring_ID& ring);
		void ClearTransmitByteCount(const DTC_Ring_ID& ring);
		uint32_t ReadTransmitByteCount(const DTC_Ring_ID& ring);
		void ClearTransmitPacketCount(const DTC_Ring_ID& ring);
		uint32_t ReadTransmitPacketCount(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatReceiveByteCountRing0();
		DTC_RegisterFormatter FormatReceiveByteCountRing1();
		DTC_RegisterFormatter FormatReceiveByteCountRing2();
		DTC_RegisterFormatter FormatReceiveByteCountRing3();
		DTC_RegisterFormatter FormatReceiveByteCountRing4();
		DTC_RegisterFormatter FormatReceiveByteCountRing5();
		DTC_RegisterFormatter FormatReceiveByteCountCFO();
		DTC_RegisterFormatter FormatReceiveByteCountEVB();
		DTC_RegisterFormatter FormatReceivePacketCountRing0();
		DTC_RegisterFormatter FormatReceivePacketCountRing1();
		DTC_RegisterFormatter FormatReceivePacketCountRing2();
		DTC_RegisterFormatter FormatReceivePacketCountRing3();
		DTC_RegisterFormatter FormatReceivePacketCountRing4();
		DTC_RegisterFormatter FormatReceivePacketCountRing5();
		DTC_RegisterFormatter FormatReceivePacketCountCFO();
		DTC_RegisterFormatter FormatReceivePacketCountEVB();
		DTC_RegisterFormatter FormatTramsitByteCountRing0();
		DTC_RegisterFormatter FormatTramsitByteCountRing1();
		DTC_RegisterFormatter FormatTramsitByteCountRing2();
		DTC_RegisterFormatter FormatTramsitByteCountRing3();
		DTC_RegisterFormatter FormatTramsitByteCountRing4();
		DTC_RegisterFormatter FormatTramsitByteCountRing5();
		DTC_RegisterFormatter FormatTramsitByteCountCFO();
		DTC_RegisterFormatter FormatTramsitByteCountEVB();
		DTC_RegisterFormatter FormatTransmitPacketCountRing0();
		DTC_RegisterFormatter FormatTransmitPacketCountRing1();
		DTC_RegisterFormatter FormatTransmitPacketCountRing2();
		DTC_RegisterFormatter FormatTransmitPacketCountRing3();
		DTC_RegisterFormatter FormatTransmitPacketCountRing4();
		DTC_RegisterFormatter FormatTransmitPacketCountRing5();
		DTC_RegisterFormatter FormatTransmitPacketCountCFO();
		DTC_RegisterFormatter FormatTransmitPacketCountEVB();

		// DDR Memory Flags Registers
		std::bitset<64> ReadDDRRingBufferFullFlags();
		std::bitset<64> ReadDDRRingBufferFullErrorFlags();
		std::bitset<64> ReadDDRRingBufferEmptyFlags();
		std::bitset<64> ReadDDRRingBufferHalfFullFlags();
		std::bitset<64> ReadDDREventBuilderBufferFullFlags();
		std::bitset<64> ReadDDREventBuilderBufferFullErrorFlags();
		std::bitset<64> ReadDDREventBuilderBufferEmptyFlags();
		std::bitset<64> ReadDDREventBuilderBufferHalfFullFlags();
		DTC_RegisterFormatter FormatDDRRingBufferFullFlagsLow();
		DTC_RegisterFormatter FormatDDRRingBufferFullErrorFlagsLow();
		DTC_RegisterFormatter FormatDDRRingBufferEmptyFlagsLow();
		DTC_RegisterFormatter FormatDDRRingBufferHalfFullFlagsLow();
		DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlagsLow();
		DTC_RegisterFormatter FormatDDREventBuilderBufferFullErrorFlagsLow();
		DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlagsLow();
		DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlagsLow();
		DTC_RegisterFormatter FormatDDRRingBufferFullFlagsHigh();
		DTC_RegisterFormatter FormatDDRRingBufferFullErrorFlagsHigh();
		DTC_RegisterFormatter FormatDDRRingBufferEmptyFlagsHigh();
		DTC_RegisterFormatter FormatDDRRingBufferHalfFullFlagsHigh();
		DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlagsHigh();
		DTC_RegisterFormatter FormatDDREventBuilderBufferFullErrorFlagsHigh();
		DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlagsHigh();
		DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlagsHigh();

		// EVB SERDES PRBS Control / Status Register
		bool ReadEVBSERDESPRBSErrorFlag();
		uint8_t ReadEVBSERDESTXPRBSSEL();
		void SetEVBSERDESTXPRBSSEL(uint8_t byte);
		uint8_t ReadEVBSERDESRXPRBSSEL();
		void SetEVBSERDESRXPRBSSEL(uint8_t byte);
		bool ReadEVBSERDESPRBSForceError();
		void SetEVBSERDESPRBSForceError(bool flag);
		void ToggleEVBSERDESPRBSForceError();
		bool ReadEVBSERDESPRBSReset();
		void SetEVBSERDESPRBSReset(bool flag);
		void ToggleEVBSERDESPRBSReset();
		DTC_RegisterFormatter FormatEVBSERDESPRBSControl();

		// Missed CFO Packet Count Registers
		uint32_t ReadMissedCFOPacketCountRing0();
		uint32_t ReadMissedCFOPacketCountRing1();
		uint32_t ReadMissedCFOPacketCountRing2();
		uint32_t ReadMissedCFOPacketCountRing3();
		uint32_t ReadMissedCFOPacketCountRing4();
		uint32_t ReadMissedCFOPacketCountRing5();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing0();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing1();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing2();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing3();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing4();
		DTC_RegisterFormatter FormatMissedCFOPacketCountRing5();

		// Local Fragment Drop Count
		uint32_t ReadLocalFragmentDropCount();
		DTC_RegisterFormatter FormatLocalFragmentDropCount();

		// FPGA PROM Program Data Register

		// FPGA PROM Program Status Register
		bool ReadFPGAPROMProgramFIFOFull();
		bool ReadFPGAPROMReady();
		DTC_RegisterFormatter FormatFPGAPROMProgramStatus();

		// FPGA Core Access Register
		void ReloadFPGAFirmware();
		bool ReadFPGACoreAccessFIFOFull();
		bool ReadFPGACoreAccessFIFOEmpty();
		DTC_RegisterFormatter FormatFPGACoreAccess();

		// Event Mode Lookup Table
		void SetAllEventModeWords(uint32_t data);
		void SetEventModeWord(uint8_t which, uint32_t data);
		uint32_t ReadEventModeWord(uint8_t which);


		// Oscillator Programming (DDR and SERDES)
		void SetNewOscillatorFrequency(DTC_OscillatorType oscillator, double targetFrequency);
		double ReadCurrentFrequency(DTC_OscillatorType oscillator);
		uint64_t ReadCurrentProgram(DTC_OscillatorType oscillator);
		void WriteCurrentFrequency(double freq, DTC_OscillatorType oscillator);
		void WriteCurrentProgram(uint64_t program, DTC_OscillatorType oscillator);

	private:
		void WriteRegister_(uint32_t data, const DTC_Register& address);
		uint32_t ReadRegister_(const DTC_Register& address);

		static int DecodeHighSpeedDivider_(int input);
		static int DecodeOutputDivider_(int input) { return input + 1; }
		static double DecodeRFREQ_(uint64_t input) { return input / 268435456.0; }
		static int EncodeHighSpeedDivider_(int input);
		static int EncodeOutputDivider_(int input);
		static uint64_t EncodeRFREQ_(double input) { return static_cast<uint64_t>(input * 268435456) & 0x3FFFFFFFFF; }
		static uint64_t CalculateFrequencyForProgramming_(double targetFrequency, double currentFrequency, uint64_t currentProgram);


	protected:
		mu2edev device_;
		DTC_SimMode simMode_;
		DTC_ROC_ID maxROCs_[6];
		bool usingDetectorEmulator_;
		uint16_t dmaSize_;
		int formatterWidth_;

		const std::vector<std::function<DTC_RegisterFormatter()>> formattedDumpFunctions_{
			[this]() { return this->FormatDesignVersion(); },
			[this]() { return this->FormatDesignDate(); },
			[this]() { return this->FormatDTCControl(); },
			[this]() { return this->FormatDMATransferLength(); },
			[this]() { return this->FormatSERDESLoopbackEnable(); },
			[this]() { return this->FormatClockOscillatorStatus(); },
			[this]() { return this->FormatROCEmulationEnable(); },
			[this]() { return this->FormatRingEnable(); },
			[this]() { return this->FormatSERDESReset(); },
			[this]() { return this->FormatSERDESRXDisparityError(); },
			[this]() { return this->FormatSERDESRXCharacterNotInTableError(); },
			[this]() { return this->FormatSERDESUnlockError(); },
			[this]() { return this->FormatSERDESPLLLocked(); },
			[this]() { return this->FormatSERDESTXBufferStatus(); },
			[this]() { return this->FormatSERDESRXBufferStatus(); },
			[this]() { return this->FormatSERDESRXStatus(); },
			[this]() { return this->FormatSERDESResetDone(); },
			[this]() { return this->FormatSERDESEyescanData(); },
			[this]() { return this->FormatSFPSERDESStatus(); },
			[this]() { return this->FormatDMATimeoutPreset(); },
			[this]() { return this->FormatROCReplyTimeout(); },
			[this]() { return this->FormatROCReplyTimeoutError(); },
			[this]() { return this->FormatRingPacketLength(); },
			[this]() { return this->FormatEVBLocalParitionIDMACIndex(); },
			[this]() { return this->FormatEVBNumberOfDestinationNodes(); },
			[this]() { return this->FormatHeartbeatError(); },
			[this]() { return this->FormatSERDESOscillatorFrequency(); },
			[this]() { return this->FormatSERDESOscillatorControl(); },
			[this]() { return this->FormatSERDESOscillatorParameterLow(); },
			[this]() { return this->FormatSERDESOscillatorParameterHigh(); },
			[this]() { return this->FormatDDROscillatorFrequency(); },
			[this]() { return this->FormatDDROscillatorControl(); },
			[this]() { return this->FormatDDROscillatorParameterLow(); },
			[this]() { return this->FormatDDROscillatorParameterHigh(); },
			[this]() { return this->FormatTimestampPreset0(); },
			[this]() { return this->FormatTimestampPreset1(); },
			[this]() { return this->FormatDataPendingTimer(); },
			[this]() { return this->FormatNUMROCs(); },
			[this]() { return this->FormatFIFOFullErrorFlag0(); },
			[this]() { return this->FormatFIFOFullErrorFlag1(); },
			[this]() { return this->FormatFIFOFullErrorFlag2(); },
			[this]() { return this->FormatReceivePacketError(); },
			[this]() { return this->FormatCFOEmulationTimestampLow(); },
			[this]() { return this->FormatCFOEmulationTimestampHigh(); },
			[this]() { return this->FormatCFOEmulationRequestInterval(); },
			[this]() { return this->FormatCFOEmulationNumRequests(); },
			[this]() { return this->FormatCFOEmulationNumPacketsRing01(); },
			[this]() { return this->FormatCFOEmulationNumPacketsRing23(); },
			[this]() { return this->FormatCFOEmulationNumPacketsRing45(); },
			[this]() { return this->FormatCFOEmulationModeBytes03(); },
			[this]() { return this->FormatCFOEmulationModeBytes45(); },
			[this]() { return this->FormatCFOEmulationDebugPacketType(); },
			[this]() { return this->FormatDetectorEmulationDMACount(); },
			[this]() { return this->FormatDetectorEmulationDMADelayCount(); },
			[this]() { return this->FormatDetectorEmulationControl0(); },
			[this]() { return this->FormatDetectorEmulationControl1(); },
			[this]() { return this->FormatDDRDataLocalStartAddress(); },
			[this]() { return this->FormatDDRDataLocalEndAddress(); },
			[this]() { return this->FormatROCDRPSyncError(); },
			[this]() { return this->FormatEthernetPayloadSize(); },
			[this]() { return this->FormatDDRRingBufferFullFlagsLow(); },
			[this]() { return this->FormatDDRRingBufferFullFlagsHigh(); },
			[this]() { return this->FormatDDRRingBufferFullErrorFlagsLow(); },
			[this]() { return this->FormatDDRRingBufferFullErrorFlagsHigh(); },
			[this]() { return this->FormatDDRRingBufferEmptyFlagsLow(); },
			[this]() { return this->FormatDDRRingBufferEmptyFlagsHigh(); },
			[this]() { return this->FormatDDRRingBufferHalfFullFlagsLow(); },
			[this]() { return this->FormatDDRRingBufferHalfFullFlagsHigh(); },
			[this]() { return this->FormatDDREventBuilderBufferFullFlagsLow(); },
			[this]() { return this->FormatDDREventBuilderBufferFullFlagsHigh(); },
			[this]() { return this->FormatDDREventBuilderBufferFullErrorFlagsLow(); },
			[this]() { return this->FormatDDREventBuilderBufferFullErrorFlagsHigh(); },
			[this]() { return this->FormatDDREventBuilderBufferEmptyFlagsLow(); },
			[this]() { return this->FormatDDREventBuilderBufferEmptyFlagsHigh(); },
			[this]() { return this->FormatDDREventBuilderBufferHalfFullFlagsLow(); },
			[this]() { return this->FormatDDREventBuilderBufferHalfFullFlagsHigh(); },
			[this]() { return this->FormatEVBSERDESPRBSControl(); },
			[this]() { return this->FormatMissedCFOPacketCountRing0(); },
			[this]() { return this->FormatMissedCFOPacketCountRing1(); },
			[this]() { return this->FormatMissedCFOPacketCountRing2(); },
			[this]() { return this->FormatMissedCFOPacketCountRing3(); },
			[this]() { return this->FormatMissedCFOPacketCountRing4(); },
			[this]() { return this->FormatMissedCFOPacketCountRing5(); },
			[this]() { return this->FormatLocalFragmentDropCount(); },
			[this]() { return this->FormatFPGAPROMProgramStatus(); },
			[this]() { return this->FormatFPGACoreAccess(); }
		};

		const std::vector<std::function<DTC_RegisterFormatter()>> formattedPerfMonFunctions_{
			[this]() { return this->FormatPerfMonTXByteCount(); },
			[this]() { return this->FormatPerfMonRXByteCount(); },
			[this]() { return this->FormatPerfMonTXPayloadCount(); },
			[this]() { return this->FormatPerfMonRXPayloadCount(); },
			[this]() { return this->FormatPerfMonInitCDC(); },
			[this]() { return this->FormatPerfMonInitCHC(); },
			[this]() { return this->FormatPerfMonInitNPDC(); },
			[this]() { return this->FormatPerfMonInitNPHC(); },
			[this]() { return this->FormatPerfMonInitPDC(); },
			[this]() { return this->FormatPerfMonInitPHC(); }
		};

		const std::vector<std::function<DTC_RegisterFormatter()>> formattedCounterFunctions_{
			[this]() { return this->FormatReceiveByteCountRing0(); },
			[this]() { return this->FormatReceiveByteCountRing1(); },
			[this]() { return this->FormatReceiveByteCountRing2(); },
			[this]() { return this->FormatReceiveByteCountRing3(); },
			[this]() { return this->FormatReceiveByteCountRing4(); },
			[this]() { return this->FormatReceiveByteCountRing5(); },
			[this]() { return this->FormatReceiveByteCountCFO(); },
			[this]() { return this->FormatReceiveByteCountEVB(); },
			[this]() { return this->FormatReceivePacketCountRing0(); },
			[this]() { return this->FormatReceivePacketCountRing1(); },
			[this]() { return this->FormatReceivePacketCountRing2(); },
			[this]() { return this->FormatReceivePacketCountRing3(); },
			[this]() { return this->FormatReceivePacketCountRing4(); },
			[this]() { return this->FormatReceivePacketCountRing5(); },
			[this]() { return this->FormatReceivePacketCountCFO(); },
			[this]() { return this->FormatReceivePacketCountEVB(); },
			[this]() { return this->FormatTramsitByteCountRing0(); },
			[this]() { return this->FormatTramsitByteCountRing1(); },
			[this]() { return this->FormatTramsitByteCountRing2(); },
			[this]() { return this->FormatTramsitByteCountRing3(); },
			[this]() { return this->FormatTramsitByteCountRing4(); },
			[this]() { return this->FormatTramsitByteCountRing5(); },
			[this]() { return this->FormatTramsitByteCountCFO(); },
			[this]() { return this->FormatTramsitByteCountEVB(); },
			[this]() { return this->FormatTransmitPacketCountRing0(); },
			[this]() { return this->FormatTransmitPacketCountRing1(); },
			[this]() { return this->FormatTransmitPacketCountRing2(); },
			[this]() { return this->FormatTransmitPacketCountRing3(); },
			[this]() { return this->FormatTransmitPacketCountRing4(); },
			[this]() { return this->FormatTransmitPacketCountRing5(); },
			[this]() { return this->FormatTransmitPacketCountCFO(); },
			[this]() { return this->FormatTransmitPacketCountEVB(); }
		};
	};
}

#endif //DTC_REGISTERS_H
