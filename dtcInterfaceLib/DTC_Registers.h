#ifndef DTC_REGISTERS_H
#define DTC_REGISTERS_H

#include <bitset> // std::bitset



#include <cstdint> // uint8_t, uint16_t


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

	class DTC_Registers
	{
	public:
		explicit DTC_Registers(DTC_SimMode mode = DTC_SimMode_Disabled);

		//
		// Device Access
		//
		mu2edev* GetDevice()
		{
			return &device_;
		}

		double GetDeviceTime() const
		{
			return device_.GetDeviceTime();
		}

		void ResetDeviceTime()
		{
			device_.ResetDeviceTime();
		}

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
		std::string FormattedRegDump() const;
		std::string PerformanceMonitorRegDump() const;

		DTC_RegisterFormatter CreateFormatter(const DTC_Register& address)
		{
			DTC_RegisterFormatter form;
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
		void ResetSERDESOscillator();
		bool ReadResetSERDESOscillator();
		void SetSERDESOscillatorClock_25Gbps();
		void SetSERDESOscillatorClock_3125Gbps();
		bool ReadSERDESOscillatorClock();
		void ResetDDRWriteAddress();
		bool ReadResetDDRWriteAddress();
		void EnableDetectorEmulator();
		void DisableDetectorEmulator();
		bool ReadDetectorEmulatorEnable();
		void EnableDetectorEmulatorMode();
		void DisableDetectorEmulatorMode();
		bool ReadDetectorEmulatorMode();
		void SetExternalSystemClock();
		void SetInternalSystemClock();
		bool ReadSystemClock();
		void EnableTiming();
		void DisableTiming();
		bool ReadTimingEnable();
		DTC_RegisterFormatter FormatDTCControl();
		void DTC_Reset();

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

		// SERDES Status Register
		bool ReadSERDESOscillatorIICError();
		bool ReadSERDESOscillatorInitializationComplete();
		DTC_RegisterFormatter FormatSERDESOscillatorStatus();

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

		// SERDES RX CDR Lock Register
		bool ReadSERDESRXCDRLock(const DTC_Ring_ID& ring);
		DTC_RegisterFormatter FormatSERDESRXCDRLock();

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
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing0();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing1();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing2();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing3();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing4();
		DTC_RegisterFormatter FormatCFOEmulationNumPacketsRing5();

		// CFO Emulation Debug Packet Type Register
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

		// SERDES Counter Registers
		/*
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
		DTC_RegisterFormatter FromatReceivePacketCountRing0();
		DTC_RegisterFormatter FromatReceivePacketCountRing1();
		DTC_RegisterFormatter FromatReceivePacketCountRing2();
		DTC_RegisterFormatter FromatReceivePacketCountRing3();
		DTC_RegisterFormatter FromatReceivePacketCountRing4();
		DTC_RegisterFormatter FromatReceivePacketCountRing5();
		DTC_RegisterFormatter FromatReceivePacketCountCFO();
		DTC_RegisterFormatter FormatTramsitByteCountRing0();
		DTC_RegisterFormatter FormatTramsitByteCountRing1();
		DTC_RegisterFormatter FormatTramsitByteCountRing2();
		DTC_RegisterFormatter FormatTramsitByteCountRing3();
		DTC_RegisterFormatter FormatTramsitByteCountRing4();
		DTC_RegisterFormatter FormatTramsitByteCountRing5();
		DTC_RegisterFormatter FormatTramsitByteCountCFO();
		DTC_RegisterFormatter FormatTransmitPacketCountRing0();
		DTC_RegisterFormatter FormatTransmitPacketCountRing1();
		DTC_RegisterFormatter FormatTransmitPacketCountRing2();
		DTC_RegisterFormatter FormatTransmitPacketCountRing3();
		DTC_RegisterFormatter FormatTransmitPacketCountRing4();
		DTC_RegisterFormatter FormatTransmitPacketCountRing5();
		DTC_RegisterFormatter FormatTransmitPacketCountCFO);
		*/

		// DDR Local Start Address Register
		uint32_t ReadDDRLocalStartAddress();
		void SetDDRLocalStartAddress(uint32_t address);
		DTC_RegisterFormatter FormatDDRLocalStartAddress();


		// DDR Local End Address Register
		uint32_t ReadDDRLocalEndAddress();
		void SetDDRLocalEndAddress(uint32_t address);
		DTC_RegisterFormatter FormatDDRLocalEndAddress();

		// DDR Write Burst Size Register

		// DDR Read Burst Size Register

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


	private:
		void WriteRegister_(uint32_t data, const DTC_Register& address);
		uint32_t ReadRegister_(const DTC_Register& address);

	protected:
		mu2edev device_;
		DTC_SimMode simMode_;
		DTC_ROC_ID maxROCs_[6];
		uint16_t dmaSize_;

		const std::vector<std::function<DTC_RegisterFormatter()>> formattedDumpFunctions_{
			[this]()
			{
				return this->FormatDesignVersion();
			},
			[this]()
			{
				return this->FormatDesignDate();
			},
			[this]()
			{
				return this->FormatDTCControl();
			},
			[this]()
			{
				return this->FormatDMATransferLength();
			},
			[this]()
			{
				return this->FormatSERDESLoopbackEnable();
			},
			[this]()
			{
				return this->FormatSERDESOscillatorStatus();
			},
			[this]()
			{
				return this->FormatROCEmulationEnable();
			},
			[this]()
			{
				return this->FormatRingEnable();
			},
			[this]()
			{
				return this->FormatSERDESReset();
			},
			[this]()
			{
				return this->FormatSERDESRXDisparityError();
			},
			[this]()
			{
				return this->FormatSERDESRXCharacterNotInTableError();
			},
			[this]()
			{
				return this->FormatSERDESUnlockError();
			},
			[this]()
			{
				return this->FormatSERDESPLLLocked();
			},
			[this]()
			{
				return this->FormatSERDESTXBufferStatus();
			},
			[this]()
			{
				return this->FormatSERDESRXBufferStatus();
			},
			[this]()
			{
				return this->FormatSERDESRXStatus();
			},
			[this]()
			{
				return this->FormatSERDESResetDone();
			},
			[this]()
			{
				return this->FormatSERDESEyescanData();
			},
			[this]()
			{
				return this->FormatSERDESRXCDRLock();
			},
			[this]()
			{
				return this->FormatDMATimeoutPreset();
			},
			[this]()
			{
				return this->FormatROCReplyTimeout();
			},
			[this]()
			{
				return this->FormatROCReplyTimeoutError();
			},
			[this]()
			{
				return this->FormatReceivePacketError();
			},
			[this]()
			{
				return this->FormatTimestampPreset0();
			},
			[this]()
			{
				return this->FormatTimestampPreset1();
			},
			[this]()
			{
				return this->FormatDataPendingTimer();
			},
			[this]()
			{
				return this->FormatNUMROCs();
			},
			[this]()
			{
				return this->FormatFIFOFullErrorFlag0();
			},
			[this]()
			{
				return this->FormatFIFOFullErrorFlag1();
			},
			[this]()
			{
				return this->FormatFIFOFullErrorFlag2();
			},
			[this]()
			{
				return this->FormatCFOEmulationTimestampLow();
			},
			[this]()
			{
				return this->FormatCFOEmulationTimestampHigh();
			},
			[this]()
			{
				return this->FormatCFOEmulationRequestInterval();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumRequests();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing0();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing1();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing2();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing3();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing4();
			},
			[this]()
			{
				return this->FormatCFOEmulationNumPacketsRing5();
			},

			[this]()
			{
				return this->FormatCFOEmulationDebugPacketType();
			},

			[this]()
			{
				return this->FormatDetectorEmulationDMACount();
			},

			[this]()
			{
				return this->FormatDetectorEmulationDMADelayCount();
			},
			[this]()
			{
				return this->FormatDDRLocalStartAddress();
			},
			[this]()
			{
				return this->FormatDDRLocalEndAddress();
			},
			[this]()
			{
				return this->FormatRingPacketLength();
			},
			[this]()
			{
				return this->FormatFPGAPROMProgramStatus();
			},
			[this]()
			{
				return this->FormatFPGACoreAccess();
			}
		};

		const std::vector<std::function<DTC_RegisterFormatter()>> formattedPerfMonFunctions_{
			[this]()
			{
				return this->FormatPerfMonTXByteCount();
			},
			[this]()
			{
				return this->FormatPerfMonRXByteCount();
			},
			[this]()
			{
				return this->FormatPerfMonTXPayloadCount();
			},
			[this]()
			{
				return this->FormatPerfMonRXPayloadCount();
			},
			[this]()
			{
				return this->FormatPerfMonInitCDC();
			},
			[this]()
			{
				return this->FormatPerfMonInitCHC();
			},
			[this]()
			{
				return this->FormatPerfMonInitNPDC();
			},
			[this]()
			{
				return this->FormatPerfMonInitNPHC();
			},
			[this]()
			{
				return this->FormatPerfMonInitPDC();
			},
			[this]()
			{
				return this->FormatPerfMonInitPHC();
			}
		};
	};
}

#endif //DTC_REGISTERS_H


