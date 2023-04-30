#ifndef DTC_REGISTERS_H
#define DTC_REGISTERS_H

//#include <bitset> // std::bitset
//#include <cstdint> // uint8_t, uint16_t
#include <functional>  // std::bind, std::function
#include <vector>      // std::vector

#include "DTC_Types.h"
#include "mu2edev.h"

namespace DTCLib {
enum DTC_Register : uint16_t
{
	DTC_Register_DesignVersion = 0x9000,
	DTC_Register_DesignDate = 0x9004,
	DTC_Register_DesignStatus = 0x9008,
	DTC_Register_VivadoVersion = 0x900C,
	DTC_Register_FPGA_Temperature = 0x9010,
	DTC_Register_FPGA_VCCINT = 0x9014,
	DTC_Register_FPGA_VCCAUX = 0x9018,
	DTC_Register_FPGA_VCCBRAM = 0x901C,
	DTC_Register_FPGA_MonitorAlarm = 0x9020,
	DTC_Register_Scratch = 0x9030,
	DTC_Register_KernelDriverVersion = 0x9040,
	DTC_Register_DTCControl = 0x9100,
	DTC_Register_DMATransferLength = 0x9104,
	DTC_Register_SERDESLoopbackEnable = 0x9108,
	DTC_Register_SERDESDDRClockStatus = 0x910C,
	DTC_Register_ROCEmulationEnable = 0x9110,
	DTC_Register_LinkEnable = 0x9114,
	DTC_Register_SERDES_Reset = 0x9118,
	DTC_Register_SERDES_RXDisparityError = 0x911C,
	DTC_Register_SERDES_RXCharacterNotInTableError = 0x9120,
	DTC_Register_SERDES_UnlockError = 0x9124,
	DTC_Register_SERDES_PLLLocked = 0x9128,
	DTC_Register_SERDES_PLLPowerDown = 0x912C,
	DTC_Register_SERDES_CDRLockCommaCount = 0x9130,
	DTC_Register_SERDES_RXStatus = 0x9134,
	DTC_Register_SERDES_ResetDone = 0x9138,
	// 0x913C Reserved
	DTC_Register_SERDES_RXCDRLockStatus = 0x9140,
	DTC_Register_DMATimeoutPreset = 0x9144,
	DTC_Register_ROCReplyTimeout = 0x9148,
	DTC_Register_ROCReplyTimeoutError = 0x914C,
	// 0x9150 Reserved
	DTC_Register_EVBPartitionID = 0x9154,
	DTC_Register_EVBConfiguration = 0x9158,
	DTC_Register_SERDESTimingCardOscillatorFrequency = 0x915C,
	DTC_Register_SERDESReferenceClockFrequency = 0x9160,
	DTC_Register_SERDESClock_IICBusControl = 0x9164,
	DTC_Register_SERDESClock_IICBusLow = 0x9168,
	DTC_Register_SERDESClock_IICBusHigh = 0x916C,
	DTC_Register_DDRReferenceClockFrequency = 0x9170,
	DTC_Register_DDRClock_IICBusControl = 0x9174,
	DTC_Register_DDRClock_IICBusLow = 0x9178,
	DTC_Register_DDRClock_IICBusHigh = 0x917C,
	DTC_Register_DDRWriteResponseTimer = 0x9180,
	DTC_Register_CFOEmulation_NumDebugDataPackets = 0x9184,
	DTC_Register_DataPendingTimer = 0x9188,
	// 0x918C Reserved
	DTC_Register_FIFOFullErrorFlag0 = 0x9190,
	DTC_Register_FIFOFullErrorFlag1 = 0x9194,
	DTC_Register_FIFOFullErrorFlag2 = 0x9198,
	DTC_Register_ReceivePacketError = 0x919C,
	DTC_Register_CFOEmulation_TimestampLow = 0x91A0,
	DTC_Register_CFOEmulation_TimestampHigh = 0x91A4,
	DTC_Register_CFOEmulation_HeartbeatInterval = 0x91A8,
	DTC_Register_CFOEmulation_NumHeartbeats = 0x91AC,
	DTC_Register_CFOEmulation_NumPacketsLinks10 = 0x91B0,
	DTC_Register_CFOEmulation_NumPacketsLinks32 = 0x91B4,
	DTC_Register_CFOEmulation_NumPacketsLinks54 = 0x91B8,
	DTC_Register_CFOEmulation_NumNullHeartbeats = 0x91BC,
	DTC_Register_CFOEmulation_EventMode1 = 0x91C0,
	DTC_Register_CFOEmulation_EventMode2 = 0x91C4,
	DTC_Register_DebugPacketType = 0x91C8,
	DTC_Register_RXPacketCountErrorFlags = 0x91CC,
	DTC_Register_DetEmulation_DMACount = 0x91D0,
	DTC_Register_DetEmulation_DelayCount = 0x91D4,
	DTC_Register_DetEmulation_Control0 = 0x91D8,
	DTC_Register_DetEmulation_Control1 = 0x91DC,
	DTC_Register_DetEmulation_DataStartAddress = 0x91E0,
	DTC_Register_DetEmulation_DataEndAddress = 0x91E4,
	DTC_Register_EthernetFramePayloadSize = 0x91EC,
	DTC_Register_CFOEmulation_DataRequestDelay = 0x91E8,
	DTC_Register_EthernetFrameMaxPayload = 0x91EC,
	// 0x91F0 Reserved
	DTC_Register_CFOEmulation_40MHzClockMarkerInterval = 0x91F4,
	DTC_Register_CFOMarkerEnables = 0x91F8,
	DTC_Register_ROCFinishThreshold = 0x91FC,
	DTC_Register_ReceiveByteCount_Link0 = 0x9200,
	DTC_Register_ReceiveByteCount_Link1 = 0x9204,
	DTC_Register_ReceiveByteCount_Link2 = 0x9208,
	DTC_Register_ReceiveByteCount_Link3 = 0x920C,
	DTC_Register_ReceiveByteCount_Link4 = 0x9210,
	DTC_Register_ReceiveByteCount_Link5 = 0x9214,
	DTC_Register_ReceiveByteCount_CFOLink = 0x9218,
	// 0x921C Reserved
	DTC_Register_ReceivePacketCount_Link0 = 0x9220,
	DTC_Register_ReceivePacketCount_Link1 = 0x9224,
	DTC_Register_ReceivePacketCount_Link2 = 0x9228,
	DTC_Register_ReceivePacketCount_Link3 = 0x922C,
	DTC_Register_ReceivePacketCount_Link4 = 0x9230,
	DTC_Register_ReceivePacketCount_Link5 = 0x9234,
	DTC_Register_ReceivePacketCount_CFOLink = 0x9238,
	// 0x923C Reserved
	DTC_Register_TransmitByteCount_Link0 = 0x9240,
	DTC_Register_TransmitByteCount_Link1 = 0x9244,
	DTC_Register_TransmitByteCount_Link2 = 0x9248,
	DTC_Register_TransmitByteCount_Link3 = 0x924C,
	DTC_Register_TransmitByteCount_Link4 = 0x9250,
	DTC_Register_TransmitByteCount_Link5 = 0x9254,
	DTC_Register_TransmitByteCount_CFOLink = 0x9258,
	// 0x925C Reserved
	DTC_Register_TransmitPacketCount_Link0 = 0x9260,
	DTC_Register_TransmitPacketCount_Link1 = 0x9264,
	DTC_Register_TransmitPacketCount_Link2 = 0x9268,
	DTC_Register_TransmitPacketCount_Link3 = 0x926C,
	DTC_Register_TransmitPacketCount_Link4 = 0x9270,
	DTC_Register_TransmitPacketCount_Link5 = 0x9274,
	DTC_Register_TransmitPacketCount_CFOLink = 0x9278,
	// 0x927C Reserved
	// 0x9280 Reserved
	DTC_Register_FireflyTX_IICBusControl = 0x9284,
	DTC_Register_FireflyTX_IICBusConfigLow = 0x9288,
	DTC_Register_FireflyTX_IICBusConfigHigh = 0x928C,
	// 0x9290 Reserved
	DTC_Register_FireflyRX_IICBusControl = 0x9294,
	DTC_Register_FireflyRX_IICBusConfigLow = 0x9298,
	DTC_Register_FireflyRX_IICBusConfigHigh = 0x929C,
	// 0x92A0 Reserved
	DTC_Register_FireflyTXRX_IICBusControl = 0x92A4,
	DTC_Register_FireflyTXRX_IICBusConfigLow = 0x92A8,
	DTC_Register_FireflyTXRX_IICBusConfigHigh = 0x92AC,
	DTC_Register_TXPRBSControl = 0x92B0,
	DTC_Register_RXPRBSControl = 0x92B4,
	// 0x92B8 Reserved
	// 0x92BC Reserved
	DTC_Register_EventModeLookupTableControl = 0x92C0,
	// 0x92C4-0x92EC Reserved
	DTC_Register_DD3TestRegister = 0x92F0,
	// 0x92F4 Reserved
	// 0x92F8 Reserved
	// 0x92FC Reserved
	DTC_Register_SERDESTXRXInvertEnable = 0x9300,
	// 0x9304 Reserved
	DTC_Register_JitterAttenuatorCSR = 0x9308,
	// 0x930C Reserved
	// 0x9310 Reserved
	DTC_Register_SFP_IICBusControl = 0x9314,
	DTC_Register_SFP_IICBusLow = 0x9318,
	DTC_Register_SFP_IICBusHigh = 0x931C,
	DTC_Register_RetransmitRequestCount_Link0 = 0x9320,
	DTC_Register_RetransmitRequestCount_Link1 = 0x9324,
	DTC_Register_RetransmitRequestCount_Link2 = 0x9328,
	DTC_Register_RetransmitRequestCount_Link3 = 0x932C,
	DTC_Register_RetransmitRequestCount_Link4 = 0x9330,
	DTC_Register_RetransmitRequestCount_Link5 = 0x9334,
	// 0x9338 Reserved
	// 0x933C Reserved
	DTC_Register_MissedCFOPacketCount_Link0 = 0x9340,
	DTC_Register_MissedCFOPacketCount_Link1 = 0x9344,
	DTC_Register_MissedCFOPacketCount_Link2 = 0x9348,
	DTC_Register_MissedCFOPacketCount_Link3 = 0x934C,
	DTC_Register_MissedCFOPacketCount_Link4 = 0x9350,
	DTC_Register_MissedCFOPacketCount_Link5 = 0x9354,
	// 0x9358 Reserved
	// 0x935C Reserved
	DTC_Register_LocalFragmentDropCount = 0x9360,
	DTC_Register_EVBSubEventReceiveTimerPreset = 0x9364,
	DTC_Register_EVBSERDESPRBSControlStatus = 0x9368,
	// 0x936C Reserved
	DTC_Register_EventBuilderErrorFlags = 0x9370,
	DTC_Register_InputBufferErrorFlags = 0x9374,
	DTC_Register_OutputBufferErrorFlags = 0x9378,
	DTC_Register_Link0ErrorFlags = 0x9380,
	DTC_Register_Link1ErrorFlags = 0x9384,
	DTC_Register_Link2ErrorFlags = 0x9388,
	DTC_Register_Link3ErrorFlags = 0x938C,
	DTC_Register_Link4ErrorFlags = 0x9390,
	DTC_Register_Link5ErrorFlags = 0x9394,
	DTC_Register_CFOLinkErrorFlags = 0x9398,
	DTC_Register_LinkMuxErrorFlags = 0x939C,
	DTC_Register_FireFlyControlStatus = 0x93A0,
	DTC_Register_SFPControlStatus = 0x93A4,
	// 0x93A8 Reserved
	// 0x93AC Reserved
	DTC_Register_RXCDRUnlockCount_Link0 = 0x93B0,
	DTC_Register_RXCDRUnlockCount_Link1 = 0x93B4,
	DTC_Register_RXCDRUnlockCount_Link2 = 0x93B8,
	DTC_Register_RXCDRUnlockCount_Link3 = 0x93BC,
	DTC_Register_RXCDRUnlockCount_Link4 = 0x93C0,
	DTC_Register_RXCDRUnlockCount_Link5 = 0x93C4,
	DTC_Register_RXCDRUnlockCount_CFOLink = 0x93C8,
	DTC_Register_JitterAttenuatorLossOfLockCount = 0x93CC,
	DTC_Register_CFOLinkEventStartErrorCount = 0x93D0,
	DTC_Register_CFOLink40MHzErrorCount = 0x93D4,
	DTC_Register_InputBufferDropCount = 0x93D8,
	DTC_Register_OutputBufferDropCount = 0x93DC,
	DTC_Register_ROCDCSTimerPreset = 0x93E0,
	// 0x93E4 - 0x93FC Reserved
	DTC_Register_FPGAProgramData = 0x9400,
	DTC_Register_FPGAPROMProgramStatus = 0x9404,
	DTC_Register_FPGACoreAccess = 0x9408,
	// 0x940C Reserved
	DTC_Register_SlowOpticalLinksDiag = 0x9410,
	DTC_Register_DiagSERDESErrorEnable = 0x9414,
	DTC_Register_DiagSERDESPacket0 = 0x9418,
	DTC_Register_DiagSERDESPacket1 = 0x941C,
	DTC_Register_DiagSERDESPacket2 = 0x9420,
	DTC_Register_DiagSERDESPacket3 = 0x9424,
	DTC_Register_DiagSERDESPacket4 = 0x9428,
	DTC_Register_DiagSERDESPacket5 = 0x942C,
	DTC_Register_DDR3LinkBufferEmptyFlags0 = 0x9430,
	DTC_Register_DDR3LinkBufferEmptyFlags1 = 0x9434,
	DTC_Register_DDR3LinkBufferEmptyFlags2 = 0x9438,
	DTC_Register_DDR3LinkBufferEmptyFlags3 = 0x943C,
	DTC_Register_DDR3LinkBufferHalfFullFlags0 = 0x9440,
	DTC_Register_DDR3LinkBufferHalfFullFlags1 = 0x9444,
	DTC_Register_DDR3LinkBufferHalfFullFlags2 = 0x9448,
	DTC_Register_DDR3LinkBufferHalfFullFlags3 = 0x944C,
	DTC_Register_DDR3LinkBufferFullFlags0 = 0x9450,
	DTC_Register_DDR3LinkBufferFullFlags1 = 0x9454,
	DTC_Register_DDR3LinkBufferFullFlags2 = 0x9458,
	DTC_Register_DDR3LinkBufferFullFlags3 = 0x945C,
	DTC_Register_DDR3EVBBufferEmptyFlags0 = 0x9460,
	DTC_Register_DDR3EVBBufferEmptyFlags1 = 0x9464,
	DTC_Register_DDR3EVBBufferEmptyFlags2 = 0x9468,
	DTC_Register_DDR3EVBBufferEmptyFlags3 = 0x946C,
	DTC_Register_DDR3EVBBufferHalfFullFlags0 = 0x9470,
	DTC_Register_DDR3EVBBufferHalfFullFlags1 = 0x9474,
	DTC_Register_DDR3EVBBufferHalfFullFlags2 = 0x9478,
	DTC_Register_DDR3EVBBufferHalfFullFlags3 = 0x947C,
	DTC_Register_DDR3EVBBufferFullFlags0 = 0x9480,
	DTC_Register_DDR3EVBBufferFullFlags1 = 0x9484,
	DTC_Register_DDR3EVBBufferFullFlags2 = 0x9488,
	DTC_Register_DDR3EVBBufferFullFlags3 = 0x948C,
	DTC_Register_DataPendingDiagTimer_Link0 = 0x9490,
	DTC_Register_DataPendingDiagTimer_Link1 = 0x9494,
	DTC_Register_DataPendingDiagTimer_Link2 = 0x9498,
	DTC_Register_DataPendingDiagTimer_Link3 = 0x949C,
	DTC_Register_DataPendingDiagTimer_Link4 = 0x94A0,
	DTC_Register_DataPendingDiagTimer_Link5 = 0x94A4,
	// 0x94A8 Reserved
	// 0x94AC Reserved
	DTC_Register_ROCEmulator_InduceTimeoutError_Link0 = 0x94B0,
	DTC_Register_ROCEmulator_InduceTimeoutError_Link1 = 0x94B4,
	DTC_Register_ROCEmulator_InduceTimeoutError_Link2 = 0x94B8,
	DTC_Register_ROCEmulator_InduceTimeoutError_Link3 = 0x94BC,
	DTC_Register_ROCEmulator_InduceTimeoutError_Link4 = 0x94C0,
	DTC_Register_ROCEmulator_InduceTimeoutError_Link5 = 0x94C4,
	// 0x94C8 Reserved
	// 0x94CC Reserved
	DTC_Register_ROCEmulator_InduceExtraWordError_Link0 = 0x94D0,
	DTC_Register_ROCEmulator_InduceExtraWordError_Link1 = 0x94D4,
	DTC_Register_ROCEmulator_InduceExtraWordError_Link2 = 0x94D8,
	DTC_Register_ROCEmulator_InduceExtraWordError_Link3 = 0x94DC,
	DTC_Register_ROCEmulator_InduceExtraWordError_Link4 = 0x94E0,
	DTC_Register_ROCEmulator_InduceExtraWordError_Link5 = 0x94E4,
	// 0x94E8 - 0x94FC Reserved
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link0 = 0x9500,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link1 = 0x9504,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link2 = 0x9508,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link3 = 0x950C,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link4 = 0x9510,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_Link5 = 0x9514,
	DTC_Register_SERDES_CharacterNotInTableErrorCount_CFOLink = 0x9518,
	// 0x951C Reserved
	DTC_Register_SERDES_RXDisparityErrorCount_Link0 = 0x9520,
	DTC_Register_SERDES_RXDisparityErrorCount_Link1 = 0x9524,
	DTC_Register_SERDES_RXDisparityErrorCount_Link2 = 0x9528,
	DTC_Register_SERDES_RXDisparityErrorCount_Link3 = 0x952C,
	DTC_Register_SERDES_RXDisparityErrorCount_Link4 = 0x9530,
	DTC_Register_SERDES_RXDisparityErrorCount_Link5 = 0x9534,
	DTC_Register_SERDES_RXDisparityErrorCount_CFOLink = 0x9538,
	// 0x953C Reserved
	DTC_Register_SERDES_RXPRBSErrorCount_Link0 = 0x9540,
	DTC_Register_SERDES_RXPRBSErrorCount_Link1 = 0x9544,
	DTC_Register_SERDES_RXPRBSErrorCount_Link2 = 0x9548,
	DTC_Register_SERDES_RXPRBSErrorCount_Link3 = 0x954C,
	DTC_Register_SERDES_RXPRBSErrorCount_Link4 = 0x9550,
	DTC_Register_SERDES_RXPRBSErrorCount_Link5 = 0x9554,
	DTC_Register_SERDES_RXPRBSErrorCount_CFOLink = 0x9558,
	// 0x955C Reserved
	DTC_Register_SERDES_RXCRCErrorCount_Link0 = 0x9560,
	DTC_Register_SERDES_RXCRCErrorCount_Link1 = 0x9564,
	DTC_Register_SERDES_RXCRCErrorCount_Link2 = 0x9568,
	DTC_Register_SERDES_RXCRCErrorCount_Link3 = 0x956C,
	DTC_Register_SERDES_RXCRCErrorCount_Link4 = 0x9570,
	DTC_Register_SERDES_RXCRCErrorCount_Link5 = 0x9574,
	DTC_Register_SERDES_RXCRCErrorCount_CFOLink = 0x9578,
	DTC_Register_SERDES_RXCRCErrorControl = 0x957C,
	// 0x9580 - 0x958C Reserved
	DTC_Register_EBVSERDES_RXPacketErrorCount = 0x9590,
	// 0x9594 - 0x959C Reserved
	DTC_Register_JitterAttenuator_SERDES_RXRecoveredClockLOSCount = 0x95A0,
	DTC_Register_JitterAttenuator_SERDES_RXExternalClockLOSCount = 0x95A4,
	// 0x9598 - 0x960C Reserved
	DTC_Register_ROCEmulator_InterpacketDelay_Link0 = 0x9610,
	DTC_Register_ROCEmulator_InterpacketDelay_Link1 = 0x9614,
	DTC_Register_ROCEmulator_InterpacketDelay_Link2 = 0x9618,
	DTC_Register_ROCEmulator_InterpacketDelay_Link3 = 0x961C,
	DTC_Register_ROCEmulator_InterpacketDelay_Link4 = 0x9620,
	DTC_Register_ROCEmulator_InterpacketDelay_Link5 = 0x9624,
	// 0x9628 Reserved
	// 0x962C Reserved
	DTC_Register_TXDataRequestPacketCount_Link0 = 0x9630,
	DTC_Register_TXDataRequestPacketCount_Link1 = 0x9634,
	DTC_Register_TXDataRequestPacketCount_Link2 = 0x9638,
	DTC_Register_TXDataRequestPacketCount_Link3 = 0x963C,
	DTC_Register_TXDataRequestPacketCount_Link4 = 0x9640,
	DTC_Register_TXDataRequestPacketCount_Link5 = 0x9644,
	// 0x9648 Reserved
	// 0x964C Reserved
	DTC_Register_TXHeartbeatPacketCount_Link0 = 0x9650,
	DTC_Register_TXHeartbeatPacketCount_Link1 = 0x9654,
	DTC_Register_TXHeartbeatPacketCount_Link2 = 0x9658,
	DTC_Register_TXHeartbeatPacketCount_Link3 = 0x965C,
	DTC_Register_TXHeartbeatPacketCount_Link4 = 0x9660,
	DTC_Register_TXHeartbeatPacketCount_Link5 = 0x9664,
	// 0x9668 Reserved
	// 0x966C Reserved
	DTC_Register_RXDataHeaderPacketCount_Link0 = 0x9670,
	DTC_Register_RXDataHeaderPacketCount_Link1 = 0x9674,
	DTC_Register_RXDataHeaderPacketCount_Link2 = 0x9678,
	DTC_Register_RXDataHeaderPacketCount_Link3 = 0x967C,
	DTC_Register_RXDataHeaderPacketCount_Link4 = 0x9680,
	DTC_Register_RXDataHeaderPacketCount_Link5 = 0x9684,
	// 0x9688 Reserved
	// 0x968C Reserved
	DTC_Register_RXDataPacketCount_Link0 = 0x9690,
	DTC_Register_RXDataPacketCount_Link1 = 0x9694,
	DTC_Register_RXDataPacketCount_Link2 = 0x9698,
	DTC_Register_RXDataPacketCount_Link3 = 0x969C,
	DTC_Register_RXDataPacketCount_Link4 = 0x96A0,
	DTC_Register_RXDataPacketCount_Link5 = 0x96A4,
	// 0x96A8 Reserved
	// 0x96AC Reserved
	DTC_Register_EVBDiagnosticRXPacket_Low = 0x96B0,
	DTC_Register_EVBDiagnosticRXPacket_High = 0x96B4,
	DTC_Register_EventModeLookupTableStart = 0xA000,
	DTC_Register_EventModeLookupTableEnd = 0xA3FC,
	DTC_Register_Invalid,
};

/// <summary>
/// The DTC_Registers class represents the DTC Register space, and all the methods necessary to read and write those
/// registers. Each register has, at the very least, a read method, a write method, and a DTC_RegisterFormatter method
/// which formats the register value in a human-readable way.
/// </summary>
class DTC_Registers
{
public:
	explicit DTC_Registers(DTC_SimMode mode, int dtc, std::string simFileName, unsigned linkMask = 0x1, std::string expectedDesignVersion = "",
						   bool skipInit = false);

	virtual ~DTC_Registers();

	/// <summary>
	/// Get a pointer to the device handle
	/// </summary>
	/// <returns>mu2edev* pointer</returns>
	mu2edev* GetDevice() { return &device_; }

	//
	// DTC Sim Mode Virtual Register
	//
	/// <summary>
	/// Get the current DTC_SimMode of this DTC_Registers object
	/// </summary>
	/// <returns></returns>
	DTC_SimMode ReadSimMode() const { return simMode_; }

	DTC_SimMode SetSimMode(std::string expectedDesignVersion, DTC_SimMode mode, int dtc, std::string simMemoryFile, unsigned linkMask,
						   bool skipInit = false);

	//
	// DTC Register Dumps
	//
	std::string FormattedRegDump(int width);
	std::string LinkCountersRegDump(int width);
	std::string PerformanceCountersRegDump(int width);
	std::string SERDESErrorsRegDump(int width);
	std::string PacketCountersRegDump(int width);

	/// <summary>
	/// Initializes a DTC_RegisterFormatter for the given DTC_Register
	/// </summary>
	/// <param name="address">Address of register to format</param>
	/// <returns>DTC_RegisterFormatter with address and raw value set</returns>
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

	// Design Status Register
	bool ReadDDRInterfaceReset();
	void SetDDRInterfaceReset(bool reset);
	void ResetDDRInterface();
	bool ReadDDRAutoCalibrationDone();
	DTC_RegisterFormatter FormatDesignStatus();

	// Vivado Version Register
	std::string ReadVivadoVersionNumber();
	DTC_RegisterFormatter FormatVivadoVersion();

	// FPGA Temperature Register
	double ReadFPGATemperature();
	DTC_RegisterFormatter FormatFPGATemperature();

	// FPGA VCCINT Voltage Register
	double ReadFPGAVCCINTVoltage();
	DTC_RegisterFormatter FormatFPGAVCCINT();

	// FPGA VCCAUX Voltage Register
	double ReadFPGAVCCAUXVoltage();
	DTC_RegisterFormatter FormatFPGAVCCAUX();

	// FPGA VCCBRAM Voltage Register
	double ReadFPGAVCCBRAMVoltage();
	DTC_RegisterFormatter FormatFPGAVCCBRAM();

	// FPGA Monitor Alarm Register
	bool ReadFPGADieTemperatureAlarm();
	void ResetFPGADieTemperatureAlarm();
	bool ReadFPGAAlarms();
	void ResetFPGAAlarms();
	bool ReadVCCBRAMAlarm();
	void ResetVCCBRAMAlarm();
	bool ReadVCCAUXAlarm();
	void ResetVCCAUXAlarm();
	bool ReadVCCINTAlarm();
	void ResetVCCINTAlarm();
	bool ReadFPGAUserTemperatureAlarm();
	void ResetFPGAUserTemperatureAlarm();
	DTC_RegisterFormatter FormatFPGAAlarms();

	// DTC Control Register
	void ResetDTC();             // B31
	bool ReadResetDTC();         // B31
	void EnableCFOEmulation();   // B30
	void DisableCFOEmulation();  // B30
	bool ReadCFOEmulation();     // B30
	// Bit 29 Reserved
	void EnableCFOLoopback();         // B28
	void DisableCFOLoopback();        // B28
	bool ReadCFOLoopback();           // B28
	void ResetDDRWriteAddress();      // B27
	bool ReadResetDDRWriteAddress();  // B27
	void ResetDDRReadAddress();       // B26
	bool ReadResetDDRReadAddress();   // B26
	void ResetDDR();                  // B25
	bool ReadResetDDR();              // B25
	void EnableCFOEmulatorDRP();      // B24
	void DisableCFOEmulatorDRP();     // B24
	bool ReadCFOEmulatorDRP();        // B24
	void EnableAutogenDRP();          // B23
	void DisableAutogenDRP();         // B23
	bool ReadAutogenDRP();            // B23
	void EnableSoftwareDRP();         // B22
	void DisableSoftwareDRP();        // B22
	bool ReadSoftwareDRP();           // B22
	// Bit 21 Reserved
	// Bit 20 Reserved
	void EnableDownLED0();         // B19
	void DisableDownLED0();        // B19
	bool ReadDownLED0State();      // B19
	void EnableUpLED1();           // B18
	void DisableUpLED1();          // B18
	bool ReadUpLED1State();        // B18
	void EnableUpLED0();           // B17
	void DisableUpLED0();          // B17
	bool ReadUpLED0State();        // B17
	void EnableLED6();             // B16
	void DisableLED6();            // B16
	bool ReadLED6State();          // B16
	void SetCFOEmulationMode();    // B15
	void ClearCFOEmulationMode();  // B15
	bool ReadCFOEmulationMode();   // B15
	// Bit 14 Reserved
	void SetDataFilterEnable();               // B13
	void ClearDataFilterEnable();             // B13
	bool ReadDataFilterEnable();              // B13
	void SetDRPPrefetchEnable();              // B12
	void ClearDRPPrefetchEnable();            // B12
	bool ReadDRPPrefetchEnable();             // B12
	void ROCInterfaceSoftReset();             // B11
	bool ReadROCInterfaceSoftReset();         // B11
	void SetSequenceNumberDisable();          // B10
	void ClearSequenceNumberDisable();        // B10
	bool ReadSequenceNumberDisable();         // B10
	void SetPunchEnable();                    // B9
	void ClearPunchEnable();                  // B9
	bool ReadPunchEnable();                   // B9
	void ResetSERDES();                       // B8
	bool ReadResetSERDES();                   // B8
	void SetRxPacketErrorFeedbackEnable();    // B6
	void ClearRxPacketErrorFeedbackEnable();  // B6
	bool ReadRxPacketErrorFeedbackEnable();   // B6
	void SetCommaToleranceEnable();           // B5
	void ClearCommaToleranceEnable();         // B5
	bool ReadCommaToleranceEnable();          // B5
	void SetExternalFanoutClockInput();       // B4
	void SetInternalFanoutClockInput();       // B4
	bool ReadFanoutClockInput();              // B4
	// Bit 3 Reserved
	void EnableDCSReception();   // B2
	void DisableDCSReception();  // B2
	bool ReadDCSReception();     // B2
	// Bit 1 Reserved
	// Bit 0 Reserved
	DTC_RegisterFormatter FormatDTCControl();

	// DMA Transfer Length Register
	void SetTriggerDMATransferLength(uint16_t length);
	uint16_t ReadTriggerDMATransferLength();
	void SetMinDMATransferLength(uint16_t length);
	uint16_t ReadMinDMATransferLength();
	DTC_RegisterFormatter FormatDMATransferLength();

	// SERDES Loopback Enable Register
	void SetSERDESLoopbackMode(DTC_Link_ID const& link, const DTC_SERDESLoopbackMode& mode);
	DTC_SERDESLoopbackMode ReadSERDESLoopback(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESLoopbackEnable();

	// Clock Status Register
	bool ReadSERDESOscillatorIICError();
	bool ReadDDROscillatorIICError();
	DTC_RegisterFormatter FormatClockOscillatorStatus();

	// ROC Emulation Enable Register
	void EnableROCEmulator(DTC_Link_ID const& link);
	void DisableROCEmulator(DTC_Link_ID const& link);
	bool ReadROCEmulator(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatROCEmulationEnable();

	// Link Enable Register
	void EnableLink(DTC_Link_ID const& link, const DTC_LinkEnableMode& mode = DTC_LinkEnableMode());
	void DisableLink(DTC_Link_ID const& link, const DTC_LinkEnableMode& mode = DTC_LinkEnableMode());
	DTC_LinkEnableMode ReadLinkEnabled(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatLinkEnable();

	// SERDES Reset Register
	void ResetSERDESTX(DTC_Link_ID const& link, int interval = 100);
	bool ReadResetSERDESTX(DTC_Link_ID const& link);
	void ResetSERDESRX(DTC_Link_ID const& link, int interval = 100);
	bool ReadResetSERDESRX(DTC_Link_ID const& link);
	void ResetSERDESPLL(const DTC_PLL_ID& pll, int interval = 100);
	bool ReadResetSERDESPLL(const DTC_PLL_ID& pll);
	void ResetSERDES(DTC_Link_ID const& link, int interval = 100);
	bool ReadResetSERDES(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESReset();

	// SERDES RX Disparity Error Register
	DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXDisparityError();

	// SERDES Character Not In Table Error Register
	DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXCharacterNotInTableError();

	// SERDES Unlock Error Register
	bool ReadSERDESCDRUnlockError(DTC_Link_ID const& link);
	bool ReadSERDESPLLUnlockError(const DTC_PLL_ID& pll);
	DTC_RegisterFormatter FormatSERDESUnlockError();

	// SERDES PLL Locked Register
	bool ReadSERDESPLLLocked(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESPLLLocked();

	// SERDES PLL Power Down
	void EnableSERDESPLL(DTC_Link_ID const& link);
	void DisableSERDESPLL(DTC_Link_ID const& link);
	bool ReadSERDESPLLPowerDown(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESPLLPowerDown();

	// SERDES RX Status Register
	DTC_RXStatus ReadSERDESRXStatus(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXStatus();

	// SERDES Reset Done Register
	bool ReadResetRXFSMSERDESDone(DTC_Link_ID const& link);
	bool ReadResetRXSERDESDone(DTC_Link_ID const& link);
	bool ReadResetTXFSMSERDESDone(DTC_Link_ID const& link);
	bool ReadResetTXSERDESDone(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESResetDone();

	// SERDES CDR Lock Register
	bool ReadSERDESRXCDRLock(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRXCDRLockStatus();

	// DMA Timeout Preset Regsiter
	void SetDMATimeoutPreset(uint32_t preset);
	uint32_t ReadDMATimeoutPreset();
	DTC_RegisterFormatter FormatDMATimeoutPreset();

	// ROC Timeout (Header Packet to All Packets Received) Preset Register
	void SetROCTimeoutPreset(uint32_t preset);
	uint32_t ReadROCTimeoutPreset();
	DTC_RegisterFormatter FormatROCReplyTimeout();

	// ROC Timeout Error Register
	void ClearROCTimeoutError(DTC_Link_ID const& link);
	bool ReadROCTimeoutError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatROCReplyTimeoutError();

	// EVB Network Partition ID / EVB Network Local MAC Index Register
	uint8_t ReadDTCID();
	void SetEVBMode(uint8_t mode);
	uint8_t ReadEVBMode();
	void SetEVBLocalParitionID(uint8_t id);
	uint8_t ReadEVBLocalParitionID();
	void SetEVBLocalMACAddress(uint8_t macByte);
	uint8_t ReadEVBLocalMACAddress();
	DTC_RegisterFormatter FormatEVBLocalParitionIDMACIndex();

	// EVB Buffer Config
	void SetEVBNumberInputBuffers(uint8_t count);
	uint8_t ReadEVBNumberInputBuffers();
	void SetEVBStartNode(uint8_t node);
	uint8_t ReadEVBStartNode();
	void SetEVBNumberOfDestinationNodes(uint8_t number);
	uint8_t ReadEVBNumberOfDestinationNodes();
	DTC_RegisterFormatter FormatEVBNumberOfDestinationNodes();

	// SERDES Oscillator Registers
	uint32_t ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress device);
	void SetSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress device, uint32_t freq);

	bool ReadSERDESOscillatorIICInterfaceReset();
	void ResetSERDESOscillatorIICInterface();

	void WriteSERDESIICInterface(DTC_IICSERDESBusAddress device, uint8_t address, uint8_t data);
	uint8_t ReadSERDESIICInterface(DTC_IICSERDESBusAddress device, uint8_t address);

	DTC_SerdesClockSpeed ReadSERDESOscillatorClock();
	void SetSERDESOscillatorClock(DTC_SerdesClockSpeed speed);
	void SetTimingOscillatorClock(uint32_t freq);
	DTC_RegisterFormatter FormatTimingSERDESOscillatorFrequency();
	DTC_RegisterFormatter FormatMainBoardSERDESOscillatorFrequency();
	DTC_RegisterFormatter FormatSERDESOscillatorControl();
	DTC_RegisterFormatter FormatSERDESOscillatorParameterLow();
	DTC_RegisterFormatter FormatSERDESOscillatorParameterHigh();

	// DDR Oscillator Registers
	uint32_t ReadDDROscillatorReferenceFrequency();
	void SetDDROscillatorReferenceFrequency(uint32_t freq);
	bool ReadDDROscillatorIICInterfaceReset();
	void ResetDDROscillatorIICInterface();

	void WriteDDRIICInterface(DTC_IICDDRBusAddress device, uint8_t address, uint8_t data);
	uint8_t ReadDDRIICInterface(DTC_IICDDRBusAddress device, uint8_t address);
	DTC_RegisterFormatter FormatDDROscillatorFrequency();
	DTC_RegisterFormatter FormatDDROscillatorControl();
	DTC_RegisterFormatter FormatDDROscillatorParameterLow();
	DTC_RegisterFormatter FormatDDROscillatorParameterHigh();

	// Data Pending Timer Register
	void SetDataPendingTimer(uint32_t timer);
	uint32_t ReadDataPendingTimer();
	DTC_RegisterFormatter FormatDataPendingTimer();

	// FIFO Full Error Flags Registers
	void ClearFIFOFullErrorFlags(DTC_Link_ID const& link);
	DTC_FIFOFullErrorFlags ReadFIFOFullErrorFlags(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatFIFOFullErrorFlag0();
	DTC_RegisterFormatter FormatFIFOFullErrorFlag1();
	DTC_RegisterFormatter FormatFIFOFullErrorFlag2();

	// Receive Packet Error Register
	void ClearPacketError(DTC_Link_ID const& link);
	bool ReadPacketError(DTC_Link_ID const& link);
	void ClearPacketCRCError(DTC_Link_ID const& link);
	bool ReadPacketCRCError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatReceivePacketError();

	// CFO Emulation Timestamp Registers
	void SetCFOEmulationTimestamp(const DTC_EventWindowTag& ts);
	DTC_EventWindowTag ReadCFOEmulationTimestamp();
	DTC_RegisterFormatter FormatCFOEmulationTimestampLow();
	DTC_RegisterFormatter FormatCFOEmulationTimestampHigh();

	// CFO Emulation Heartbeat Interval Regsister
	void SetCFOEmulationHeartbeatInterval(uint32_t interval);
	uint32_t ReadCFOEmulationHeartbeatInterval();
	DTC_RegisterFormatter FormatCFOEmulationHeartbeatInterval();

	// CFO Emulation Number of Heartbeats Register
	void SetCFOEmulationNumHeartbeats(uint32_t numHeartbeats);
	uint32_t ReadCFOEmulationNumHeartbeats();
	DTC_RegisterFormatter FormatCFOEmulationNumHeartbeats();

	// CFO Emulation Number of Packets Registers
	void SetCFOEmulationNumPackets(DTC_Link_ID const& link, uint16_t numPackets);
	uint16_t ReadCFOEmulationNumPackets(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatCFOEmulationNumPacketsLink01();
	DTC_RegisterFormatter FormatCFOEmulationNumPacketsLink23();
	DTC_RegisterFormatter FormatCFOEmulationNumPacketsLink45();

	// CFO Emulation Number of Null Heartbeats Register
	void SetCFOEmulationNumNullHeartbeats(const uint32_t& count);
	uint32_t ReadCFOEmulationNumNullHeartbeats();
	DTC_RegisterFormatter FormatCFOEmulationNumNullHeartbeats();

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

	// RX Packet Count Error Flags Register
	bool ReadRXPacketCountErrorFlags(DTC_Link_ID const& link);
	void ClearRXPacketCountErrorFlags(DTC_Link_ID const& link);
	void ClearRXPacketCountErrorFlags();
	DTC_RegisterFormatter FormatRXPacketCountErrorFlags();

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
	/// <summary>
	/// Return the current value of the "Detector Emulator In Use" virtual register
	/// </summary>
	/// <returns>Whether the DTC Detector Emulator has been set up</returns>
	bool IsDetectorEmulatorInUse() const { return usingDetectorEmulator_; }
	/// <summary>
	/// Set the "Detector Emulator In Use" virtual register to true
	/// </summary>
	void SetDetectorEmulatorInUse()
	{
		TLOG(TLVL_WARNING) << "DTC_Registers::SetDetectorEmulatorInUse: Enabling Detector Emulator!";
		usingDetectorEmulator_ = true;
	}
	void ClearDetectorEmulatorInUse();
	DTC_RegisterFormatter FormatDetectorEmulationControl0();
	DTC_RegisterFormatter FormatDetectorEmulationControl1();

	// DDR Event Data Local Start Address Register
	void SetDDRDataLocalStartAddress(uint32_t address);
	uint32_t ReadDDRDataLocalStartAddress();
	DTC_RegisterFormatter FormatDDRDataLocalStartAddress();

	// DDR Event Data Local End Address Register
	void SetDDRDataLocalEndAddress(uint32_t address);
	uint32_t ReadDDRDataLocalEndAddress();
	DTC_RegisterFormatter FormatDDRDataLocalEndAddress();

	// CFO Emulator Data Request Interpacket Delay
	uint32_t ReadCFOEmulatorInterpacketDelay();
	DTC_RegisterFormatter FormatCFOEmulatorInterpacketDelay();

	// Ethernet Frame Payload Max Size
	uint32_t ReadEthernetPayloadSize();
	void SetEthernetPayloadSize(uint32_t size);
	DTC_RegisterFormatter FormatEthernetPayloadSize();

	// CFO Emulation 40 MHz Clock Marker Interval
	uint32_t ReadCFOEmulation40MHzMarkerInterval();
	void SetCFOEmulation40MHzMarkerInterval(uint32_t interval);
	DTC_RegisterFormatter FormatCFOEmulation40MHzMarkerInterval();

	// CFO Emulation Marker Enables
	bool ReadCFOEmulationEventStartMarkerEnable(DTC_Link_ID const& link);
	void SetCFOEmulationEventStartMarkerEnable(DTC_Link_ID const& link, bool enable);
	bool ReadCFOEmulation40MHzClockMarkerEnable(DTC_Link_ID const& link);
	void SetCFOEmulation40MHzClockMarkerEnable(DTC_Link_ID const& link, bool enable);
	DTC_RegisterFormatter FormatCFOEmulationMarkerEnables();

	// ROC Finish Threshold Register
	uint8_t ReadROCCommaLimit();
	void SetROCCommaLimit(uint8_t limit);
	DTC_RegisterFormatter FormatROCFinishThreshold();

	// SERDES Counter Registers
	void ClearReceiveByteCount(DTC_Link_ID const& link);
	uint32_t ReadReceiveByteCount(DTC_Link_ID const& link);
	void ClearReceivePacketCount(DTC_Link_ID const& link);
	uint32_t ReadReceivePacketCount(DTC_Link_ID const& link);
	void ClearTransmitByteCount(DTC_Link_ID const& link);
	uint32_t ReadTransmitByteCount(DTC_Link_ID const& link);
	void ClearTransmitPacketCount(DTC_Link_ID const& link);
	uint32_t ReadTransmitPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatReceiveByteCountLink0();
	DTC_RegisterFormatter FormatReceiveByteCountLink1();
	DTC_RegisterFormatter FormatReceiveByteCountLink2();
	DTC_RegisterFormatter FormatReceiveByteCountLink3();
	DTC_RegisterFormatter FormatReceiveByteCountLink4();
	DTC_RegisterFormatter FormatReceiveByteCountLink5();
	DTC_RegisterFormatter FormatReceiveByteCountCFO();
	DTC_RegisterFormatter FormatReceivePacketCountLink0();
	DTC_RegisterFormatter FormatReceivePacketCountLink1();
	DTC_RegisterFormatter FormatReceivePacketCountLink2();
	DTC_RegisterFormatter FormatReceivePacketCountLink3();
	DTC_RegisterFormatter FormatReceivePacketCountLink4();
	DTC_RegisterFormatter FormatReceivePacketCountLink5();
	DTC_RegisterFormatter FormatReceivePacketCountCFO();
	DTC_RegisterFormatter FormatTramsitByteCountLink0();
	DTC_RegisterFormatter FormatTramsitByteCountLink1();
	DTC_RegisterFormatter FormatTramsitByteCountLink2();
	DTC_RegisterFormatter FormatTramsitByteCountLink3();
	DTC_RegisterFormatter FormatTramsitByteCountLink4();
	DTC_RegisterFormatter FormatTramsitByteCountLink5();
	DTC_RegisterFormatter FormatTramsitByteCountCFO();
	DTC_RegisterFormatter FormatTransmitPacketCountLink0();
	DTC_RegisterFormatter FormatTransmitPacketCountLink1();
	DTC_RegisterFormatter FormatTransmitPacketCountLink2();
	DTC_RegisterFormatter FormatTransmitPacketCountLink3();
	DTC_RegisterFormatter FormatTransmitPacketCountLink4();
	DTC_RegisterFormatter FormatTransmitPacketCountLink5();
	DTC_RegisterFormatter FormatTransmitPacketCountCFO();

	// Firefly TX IIC Registers
	bool ReadFireflyTXIICInterfaceReset();
	void ResetFireflyTXIICInterface();
	void WriteFireflyTXIICInterface(uint8_t device, uint8_t address, uint8_t data);
	uint8_t ReadFireflyTXIICInterface(uint8_t device, uint8_t address);
	DTC_RegisterFormatter FormatFireflyTXIICControl();
	DTC_RegisterFormatter FormatFireflyTXIICParameterLow();
	DTC_RegisterFormatter FormatFireflyTXIICParameterHigh();

	// Firefly RX IIC Registers
	bool ReadFireflyRXIICInterfaceReset();
	void ResetFireflyRXIICInterface();
	void WriteFireflyRXIICInterface(uint8_t device, uint8_t address, uint8_t data);
	uint8_t ReadFireflyRXIICInterface(uint8_t device, uint8_t address);
	DTC_RegisterFormatter FormatFireflyRXIICControl();
	DTC_RegisterFormatter FormatFireflyRXIICParameterLow();
	DTC_RegisterFormatter FormatFireflyRXIICParameterHigh();

	// Firefly TXRX IIC Registers
	bool ReadFireflyTXRXIICInterfaceReset();
	void ResetFireflyTXRXIICInterface();
	void WriteFireflyTXRXIICInterface(uint8_t device, uint8_t address, uint8_t data);
	uint8_t ReadFireflyTXRXIICInterface(uint8_t device, uint8_t address);
	DTC_RegisterFormatter FormatFireflyTXRXIICControl();
	DTC_RegisterFormatter FormatFireflyTXRXIICParameterLow();
	DTC_RegisterFormatter FormatFireflyTXRXIICParameterHigh();

	// SERDES TX PRBS Control
	bool ReadTXPRBSForceError(DTC_Link_ID const& link);
	void SetTXPRBSForceError(DTC_Link_ID const& link);
	void ClearTXPRBSForceError(DTC_Link_ID const& link);
	DTC_PRBSMode ReadTXPRBSMode(DTC_Link_ID const& link);
	void SetTXPRBSMode(DTC_Link_ID const& link, DTC_PRBSMode mode);
	DTC_RegisterFormatter FormatSERDESTXPRBSControl();

	// SERDES RX PRBS Control
	bool ReadRXPRBSError(DTC_Link_ID const& link);
	DTC_PRBSMode ReadRXPRBSMode(DTC_Link_ID const& link);
	void SetRXPRBSMode(DTC_Link_ID const& link, DTC_PRBSMode mode);
	DTC_RegisterFormatter FormatSERDESRXPRBSControl();

	// DTC Mode Lookup
	bool ReadEventModeTableEnable();
	void SetEventModeTableEnable();
	void ClearEventModeTableEnable();
	uint8_t ReadEventModeLookupByteSelect();
	void SetEventModeLookupByteSelect(uint8_t byte);
	DTC_RegisterFormatter FormatEventModeLookupTableControl();

	// DDR Memory Test Register
	bool ReadDDRMemoryTestComplete();
	bool ReadDDRMemoryTestError();
	void ClearDDRMemoryTestError();
	DTC_RegisterFormatter FormatDDRMemoryTestRegister();

	// SERDES Serial Inversion Enable Register
	bool ReadInvertSERDESRXInput(DTC_Link_ID const& link);
	void SetInvertSERDESRXInput(DTC_Link_ID const& link, bool invert);
	bool ReadInvertSERDESTXOutput(DTC_Link_ID const& link);
	void SetInvertSERDESTXOutput(DTC_Link_ID const& link, bool invert);
	DTC_RegisterFormatter FormatSERDESSerialInversionEnable();

	// Jitter Attenuator CSR Register
	std::bitset<2> ReadJitterAttenuatorSelect();
	void SetJitterAttenuatorSelect(std::bitset<2> data);
	bool ReadJitterAttenuatorReset();
	void ResetJitterAttenuator();
	DTC_RegisterFormatter FormatJitterAttenuatorCSR();

	// SFP IIC Registers
	bool ReadSFPIICInterfaceReset();
	void ResetSFPIICInterface();
	void WriteSFPIICInterface(uint8_t device, uint8_t address, uint8_t data);
	uint8_t ReadSFPIICInterface(uint8_t device, uint8_t address);
	DTC_RegisterFormatter FormatSFPIICControl();
	DTC_RegisterFormatter FormatSFPIICParameterLow();
	DTC_RegisterFormatter FormatSFPIICParameterHigh();

	// Retransmit Request Count Registers
	uint32_t ReadRetransmitRequestCount(DTC_Link_ID const& link);
	void ClearRetransmitRequestCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRetransmitRequestCountLink0();
	DTC_RegisterFormatter FormatRetransmitRequestCountLink1();
	DTC_RegisterFormatter FormatRetransmitRequestCountLink2();
	DTC_RegisterFormatter FormatRetransmitRequestCountLink3();
	DTC_RegisterFormatter FormatRetransmitRequestCountLink4();
	DTC_RegisterFormatter FormatRetransmitRequestCountLink5();

	// Missed CFO Packet Count Registers
	uint32_t ReadMissedCFOPacketCount(DTC_Link_ID const& link);
	void ClearMissedCFOPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink0();
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink1();
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink2();
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink3();
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink4();
	DTC_RegisterFormatter FormatMissedCFOPacketCountLink5();

	// Local Fragment Drop Count Register
	uint32_t ReadLocalFragmentDropCount();
	void ClearLocalFragmentDropCount();
	DTC_RegisterFormatter FormatLocalFragmentDropCount();

	// EVB SubEvent Receive Timer Preset
	uint32_t ReadEVBSubEventReceiveTimer();
	void SetEVBSubEventReceiveTimer(uint32_t timer);
	DTC_RegisterFormatter FormatEVBSubEventReceiveTimer();

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

	// Event Builder Error Register
	bool ReadEventBuilder_SubEventReceiverFlagsBufferError();
	bool ReadEventBuilder_EthernetInputFIFOFull();
	bool ReadEventBuilder_LinkError();
	bool ReadEventBuilder_TXPacketError();
	bool ReadEventBuilder_LocalDataPointerFIFOQueueError();
	bool ReadEventBuilder_TransmitDMAByteCountFIFOFull();
	DTC_RegisterFormatter FormatEventBuilderErrorRegister();

	// SERDES VFIFO Error Register
	bool ReadSERDESVFIFO_EgressFIFOFull();
	bool ReadSERDESVFIFO_IngressFIFOFull();
	bool ReadSERDESVFIFO_EventByteCountTotalError();
	bool ReadSERDESVFIFO_LastWordWrittenTimeoutError();
	bool ReadSERDESVFIFO_FragmentCountError();
	bool ReadSERDESVFIFO_DDRFullError();
	DTC_RegisterFormatter FormatSERDESVFIFOError();

	// PCI VFIFO Error Register
	bool ReadPCIVFIFO_DDRFull();
	bool ReadPCIVFIFO_MemoryMappedWriteCompleteFIFOFull();
	bool ReadPCIVFIFO_PCIWriteEventFIFOFull();
	bool ReadPCIVFIFO_LocalDataPointerFIFOFull();
	bool ReadPCIVFIFO_EgressFIFOFull();
	bool ReadPCIVFIFO_RXBufferSelectFIFOFull();
	bool ReadPCIVFIFO_IngressFIFOFull();
	bool ReadPCIVFIFO_EventByteCountTotalError();
	DTC_RegisterFormatter FormatPCIVFIFOError();

	// ROC Link Error Registers
	bool ReadROCLink_ROCDataRequestSyncError(DTC_Link_ID const& link);
	bool ReadROCLink_RXPacketCountError(DTC_Link_ID const& link);
	bool ReadROCLink_RXPacketError(DTC_Link_ID const& link);
	bool ReadROCLink_RXPacketCRCError(DTC_Link_ID const& link);
	bool ReadROCLink_DataPendingTimeoutError(DTC_Link_ID const& link);
	bool ReadROCLink_ReceiveDataPacketCountError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRocLink0Error();
	DTC_RegisterFormatter FormatRocLink1Error();
	DTC_RegisterFormatter FormatRocLink2Error();
	DTC_RegisterFormatter FormatRocLink3Error();
	DTC_RegisterFormatter FormatRocLink4Error();
	DTC_RegisterFormatter FormatRocLink5Error();

	// CFO Link Error Register
	DTC_RegisterFormatter FormatCFOLinkError();

	// Link Mux Error Register
	bool ReadDCSMuxDecodeError();
	bool ReadDataMuxDecodeError();
	DTC_RegisterFormatter FormatLinkMuxError();

	// Firefly CSR Register
	bool ReadTXRXFireflyPresent();
	bool ReadRXFireflyPresent();
	bool ReadTXFireflyPresent();
	bool ReadTXRXFireflyInterrupt();
	bool ReadRXFireflyInterrupt();
	bool ReadTXFireflyInterrupt();
	bool ReadTXRXFireflySelect();
	void SetTXRXFireflySelect(bool select);
	bool ReadTXFireflySelect();
	void SetTXFireflySelect(bool select);
	bool ReadRXFireflySelect();
	void SetRXFireflySelect(bool select);
	bool ReadResetTXRXFirefly();
	void ResetTXRXFirefly();
	bool ReadResetTXFirefly();
	void ResetTXFirefly();
	bool ReadResetRXFirefly();
	void ResetRXFirefly();
	DTC_RegisterFormatter FormatFireflyCSR();

	// SFP Control Status Register
	bool ReadSFPPresent();
	bool ReadSFPLOS();
	bool ReadSFPTXFault();
	void EnableSFPRateSelect();
	void DisableSFPRateSelect();
	bool ReadSFPRateSelect();
	void DisableSFPTX();
	void EnableSFPTX();
	bool ReadSFPTXDisable();
	DTC_RegisterFormatter FormatSFPControlStatus();

	// RX CDR Unlock Count Registers
	uint32_t ReadRXCDRUnlockCount(DTC_Link_ID const& link);
	void ClearRXCDRUnlockCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink0();
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink1();
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink2();
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink3();
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink4();
	DTC_RegisterFormatter FormatRXCDRUnlockCountLink5();
	DTC_RegisterFormatter FormatRXCDRUnlockCountCFOLink();

	// RX Jitter Attenuator Unlock Count Register
	uint32_t ReadJitterAttenuatorUnlockCuont();
	void ClearJitterAttenuatorUnlockCount();
	DTC_RegisterFormatter FormatJitterAttenuatorUnlockCount();

	// RX CFO Link Event Start Character Error Count Register
	uint32_t ReadRXCFOLinkEventStartCharacterErrorCount();
	void ClearRXCFOLinkEventStartCharacterErrorCount();
	DTC_RegisterFormatter FormatRXCFOLinkEventStartCharacterErrorCount();

	// RX CFO Link 40MHz Clock Character Error Count Register
	uint32_t ReadRXCFOLink40MHzCharacterErrorCount();
	void ClearRXCFOLink40MHzCharacterErrorCount();
	DTC_RegisterFormatter FormatRXCFOLink40MHzCharacterErrorCount();

	// Input Buffer Fragment Dump Count
	uint32_t ReadInputBufferFragmentDumpCount();
	void ClearInputBufferFragmentDumpCount();
	DTC_RegisterFormatter FormatInputBufferFragmentDumpCount();

	// Output Buffer Fragment Dump Count
	uint32_t ReadOutputBufferFragmentDumpCount();
	void ClearOutputBufferFragmentDumpCount();
	DTC_RegisterFormatter FormatOutputBufferFragmentDumpCount();

	// ROC DCS Response Timer Preset
	uint32_t ReadROCDCSResponseTimer();
	void SetROCDCSResponseTimer(uint32_t timer);
	DTC_RegisterFormatter FormatROCDCSResponseTimerPreset();

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

	// Slow Optical Links Control/Status Register
	bool ReadRXOKErrorSlowOpticalLink3();
	bool ReadRXOKErrorSlowOpticalLink2();
	bool ReadRXOKErrorSlowOpticalLink1();
	bool ReadRXOKErrorSlowOpticalLink0();
	bool ReadLatchedSpareSMAInputOKError();
	void ClearLatchedSpareSMAInputOKError();
	bool ReadLatchedEventMarkerSMAInputOKError();
	void ClearLatchedEventMarkerSMASMAInputOKError();
	bool ReadLatchedRXOKErrorSlowOpticalLink3();
	void ClearLatchedRXOKErrorSlowOpticalLink3();
	bool ReadLatchedRXOKErrorSlowOpticalLink2();
	void ClearLatchedRXOKErrorSlowOpticalLink2();
	bool ReadLatchedRXOKErrorSlowOpticalLink1();
	void ClearLatchedRXOKErrorSlowOpticalLink1();
	bool ReadLatchedRXOKErrorSlowOpticalLink0();
	void ClearLatchedRXOKErrorSlowOpticalLink0();
	DTC_RegisterFormatter FormatSlowOpticalLinkControlStatus();

	// Diagnostic SERDES Induce Error Enable Register
	bool ReadSERDESInduceErrorEnable(DTC_Link_ID const& link);
	void EnableSERDESInduceError(DTC_Link_ID const& link);
	void DisableSERDESInduceError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESInduceErrorEnable();

	// Diagnostic SERDES Incude Error Link Registers
	uint32_t ReadSERDESInduceErrorSequenceNumber(DTC_Link_ID const& link);
	void SetSERDESInduceErrorSequenceNumber(DTC_Link_ID const& link, uint32_t sequence);
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink0();
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink1();
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink2();
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink3();
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink4();
	DTC_RegisterFormatter FormatSERDESInduceErrorSequenceNumberLink5();

	// DDR Memory Flags Registers
	DTC_DDRFlags ReadDDRFlags(uint8_t buffer_id);
	std::bitset<128> ReadDDRLinkBufferFullFlags();
	std::bitset<128> ReadDDRLinkBufferEmptyFlags();
	std::bitset<128> ReadDDRLinkBufferHalfFullFlags();
	std::bitset<128> ReadDDREventBuilderBufferFullFlags();
	std::bitset<128> ReadDDREventBuilderBufferEmptyFlags();
	std::bitset<128> ReadDDREventBuilderBufferHalfFullFlags();
	DTC_RegisterFormatter FormatDDRLinkBufferEmptyFlags0();
	DTC_RegisterFormatter FormatDDRLinkBufferEmptyFlags1();
	DTC_RegisterFormatter FormatDDRLinkBufferEmptyFlags2();
	DTC_RegisterFormatter FormatDDRLinkBufferEmptyFlags3();
	DTC_RegisterFormatter FormatDDRLinkBufferHalfFullFlags0();
	DTC_RegisterFormatter FormatDDRLinkBufferHalfFullFlags1();
	DTC_RegisterFormatter FormatDDRLinkBufferHalfFullFlags2();
	DTC_RegisterFormatter FormatDDRLinkBufferHalfFullFlags3();
	DTC_RegisterFormatter FormatDDRLinkBufferFullFlags0();
	DTC_RegisterFormatter FormatDDRLinkBufferFullFlags1();
	DTC_RegisterFormatter FormatDDRLinkBufferFullFlags2();
	DTC_RegisterFormatter FormatDDRLinkBufferFullFlags3();
	DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlags0();
	DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlags1();
	DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlags2();
	DTC_RegisterFormatter FormatDDREventBuilderBufferEmptyFlags3();
	DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlags0();
	DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlags1();
	DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlags2();
	DTC_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlags3();
	DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlags0();
	DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlags1();
	DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlags2();
	DTC_RegisterFormatter FormatDDREventBuilderBufferFullFlags3();

	// Data Pending Diagnostic Timer Registers
	uint32_t ReadDataPendingDiagnosticTimer(DTC_Link_ID const& link);
	void ResetDataPendingDiagnosticTimerFIFO(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink0();
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink1();
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink2();
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink3();
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink4();
	DTC_RegisterFormatter FormatDataPendingDiagnosticTimerLink5();

	// ROC Emulator Induce Timeout Error Control
	bool ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link);
	void EnableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link);
	void DisableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link);
	bool ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link);
	void EnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link);
	void DisableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link);
	uint32_t ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_ID const& link);
	void SetROCEmulatorTimeoutErrorTimestamp(DTC_Link_ID const& link, uint32_t timestamp);
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink0();
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink1();
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink2();
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink3();
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink4();
	DTC_RegisterFormatter FormatROCEmulatorInduceTimeoutErrorLink5();

	// ROC Emulator Induce Extra Word Error
	bool ReadEnableROCEmulatorExtraWordError(DTC_Link_ID const& link);
	void EnableROCEmulatorExtraWordError(DTC_Link_ID const& link);
	void DisableROCEmulatorExtraWordError(DTC_Link_ID const& link);
	uint32_t ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_ID const& link);
	void SetROCEmulatorExtraWordErrorTimestamp(DTC_Link_ID const& link, uint32_t timestamp);
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink0();
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink1();
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink2();
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink3();
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink4();
	DTC_RegisterFormatter FormatROCEmulatorExtraWordErrorLink5();

	// SERDES CNIT Error Count
	uint32_t ReadSERDESCharacterNotInTableErrorCount(DTC_Link_ID const& link);
	void ClearSERDESCharacterNotInTableErrorCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink0();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink1();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink2();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink3();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink4();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountLink5();
	DTC_RegisterFormatter FormatSERDESCharacterNotInTableErrorCountCFOLink();

	// SERDES RX Disparity Error Count
	uint32_t ReadSERDESRXDisparityErrorCount(DTC_Link_ID const& link);
	void ClearSERDESRXDisparityErrorCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink0();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink1();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink2();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink3();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink4();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountLink5();
	DTC_RegisterFormatter FormatSERDESRXDisparityErrorCountCFOLink();

	// SERDES RX PRBS Error Count
	uint32_t ReadSERDESRXPRBSErrorCount(DTC_Link_ID const& link);
	void ClearSERDESRXPRBSErrorCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink0();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink1();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink2();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink3();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink4();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountLink5();
	DTC_RegisterFormatter FormatSERDESRXPRBSErrorCountCFOLink();

	// SERDES RX CRC Error Count
	uint32_t ReadSERDESRXCRCErrorCount(DTC_Link_ID const& link);
	void ClearSERDESRXCRCErrorCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink0();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink1();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink2();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink3();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink4();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountLink5();
	DTC_RegisterFormatter FormatSERDESRXCRCErrorCountCFOLink();

	// SERDES RX CRC Error Control
	bool ReadEnableInduceSERDESRXCRCError(DTC_Link_ID const& link);
	void EnableInduceSERDESRXCRCError(DTC_Link_ID const& link);
	void DisableInduceSERDESRXCRCError(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatSERDESRXCRCErrorControl();

	uint32_t ReadEVBSERDESRXPacketErrorCounter();
	void ClearEVBSERDESRXPacketErrorCounter();
	DTC_RegisterFormatter FormatEVBSERDESRXPacketErrorCounter();

	// Jitter Attenuator SERDES RX Recovered Clock LOS Counter
	uint32_t ReadJitterAttenuatorRecoveredClockLOSCount();
	void ClearJitterAttenuatorRecoeveredClockLOSCount();
	DTC_RegisterFormatter FormatJitterAttenuatorRecoveredClockLOSCount();

	// Jitter Attenuator SERDES RX External Clock LOS Counter
	uint32_t ReadJitterAttenuatorExternalClockLOSCount();
	void ClearJitterAttenuatorExternalClockLOSCount();
	DTC_RegisterFormatter FormatJitterAttenuatorExternalClockLOSCount();

	// ROC Emulator Interpacket Delay
	uint32_t ReadROCEmulatorInterpacketDelay(DTC_Link_ID const& link);
	void SetROCEmulatorInterpacketDelay(DTC_Link_ID const& link, uint32_t delay);
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink0();
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink1();
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink2();
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink3();
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink4();
	DTC_RegisterFormatter FormatROCEmulatorInterpacketDelayLink5();

	// TX Data Request Packet Count
	uint32_t ReadTXDataRequestPacketCount(DTC_Link_ID const& link);
	void ClearTXDataRequetsPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink0();
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink1();
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink2();
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink3();
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink4();
	DTC_RegisterFormatter FormatTXDataRequestPacketCountLink5();

	// TX Heartbeat Packet Count
	uint32_t ReadTXHeartbeatPacketCount(DTC_Link_ID const& link);
	void ClearTXHeartbeatPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink0();
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink1();
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink2();
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink3();
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink4();
	DTC_RegisterFormatter FormatTXHeartbeatPacketCountLink5();

	// RX Data Header Packet Count
	uint32_t ReadRXDataHeaderPacketCount(DTC_Link_ID const& link);
	void ClearRXDataHeaderPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink0();
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink1();
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink2();
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink3();
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink4();
	DTC_RegisterFormatter FormatRXDataHeaderPacketCountLink5();

	// RX Data Packet Count
	uint32_t ReadRXDataPacketCount(DTC_Link_ID const& link);
	void ClearRXDataPacketCount(DTC_Link_ID const& link);
	DTC_RegisterFormatter FormatRXDataPacketCountLink0();
	DTC_RegisterFormatter FormatRXDataPacketCountLink1();
	DTC_RegisterFormatter FormatRXDataPacketCountLink2();
	DTC_RegisterFormatter FormatRXDataPacketCountLink3();
	DTC_RegisterFormatter FormatRXDataPacketCountLink4();
	DTC_RegisterFormatter FormatRXDataPacketCountLink5();

	// EVB Diagnostic RX Packet FIFO
	uint64_t ReadEVBDiagnosticFIFO();
	void ClearEVBDiagnosticFIFO();
	// No formatters for FIFO!

	// Event Mode Lookup Table
	void SetAllEventModeWords(uint32_t data);
	void SetEventModeWord(uint8_t which, uint32_t data);
	uint32_t ReadEventModeWord(uint8_t which);

	// Oscillator Programming (DDR and SERDES)
	bool SetNewOscillatorFrequency(DTC_OscillatorType oscillator, double targetFrequency);
	double ReadCurrentFrequency(DTC_OscillatorType oscillator);
	uint64_t ReadCurrentProgram(DTC_OscillatorType oscillator);
	void WriteCurrentFrequency(double freq, DTC_OscillatorType oscillator);
	void WriteCurrentProgram(uint64_t program, DTC_OscillatorType oscillator);
//-----------------------------------------------------------------------------
// 2023-04-22 P.Murat: to avoid development of parallel low-level interfaces, 
//                     the functions below should be kept public
//-----------------------------------------------------------------------------
	void WriteRegister_(uint32_t data, const DTC_Register& address);
	uint32_t ReadRegister_(const DTC_Register& address);

	bool GetBit_(const DTC_Register& address, size_t bit);
	void SetBit_(const DTC_Register& address, size_t bit, bool value);
	bool ToggleBit_(const DTC_Register& address, size_t bit)
	{
		auto val = GetBit_(address, bit);
		SetBit_(address, bit, !val);
		return !val;
	}

private:
	int DecodeHighSpeedDivider_(int input);
	int DecodeOutputDivider_(int input) { return input + 1; }
	double DecodeRFREQ_(uint64_t input) { return input / 268435456.0; }
	int EncodeHighSpeedDivider_(int input);
	int EncodeOutputDivider_(int input);
	uint64_t EncodeRFREQ_(double input) { return static_cast<uint64_t>(input * 268435456) & 0x3FFFFFFFFF; }
	uint64_t CalculateFrequencyForProgramming_(double targetFrequency, double currentFrequency,
											   uint64_t currentProgram);
	uint64_t ReadSERDESOscillatorParameters_();
	uint64_t ReadTimingOscillatorParameters_();
	uint64_t ReadDDROscillatorParameters_();
	void SetSERDESOscillatorParameters_(uint64_t program);
	void SetTimingOscillatorParameters_(uint64_t program);
	void SetDDROscillatorParameters_(uint64_t program);

	bool WaitForLinkReady_(DTC_Link_ID const& link, size_t interval, double timeout = 2.0 /*seconds*/);

protected:
	mu2edev device_;                     ///< Device handle
	DTC_SimMode simMode_;                ///< Simulation mode
	bool usingDetectorEmulator_{false};  ///< Whether Detector Emulation mode is enabled
	uint16_t dmaSize_;                   ///< Size of DMAs, in bytes (default 32k)
	int formatterWidth_;                 ///< Description field width, in characters

	/// <summary>
	/// Functions needed to print regular register map
	/// </summary>
	const std::vector<std::function<DTC_RegisterFormatter()>> formattedDumpFunctions_{
		[this] { return this->FormatDesignVersion(); },
		[this] { return this->FormatDesignDate(); },
		[this] { return this->FormatDesignStatus(); },
		[this] { return this->FormatVivadoVersion(); },
		[this] { return this->FormatFPGATemperature(); },
		[this] { return this->FormatFPGAVCCINT(); },
		[this] { return this->FormatFPGAVCCAUX(); },
		[this] { return this->FormatFPGAVCCBRAM(); },
		[this] { return this->FormatFPGAAlarms(); },
		[this] { return this->FormatDTCControl(); },
		[this] { return this->FormatDMATransferLength(); },
		[this] { return this->FormatSERDESLoopbackEnable(); },
		[this] { return this->FormatClockOscillatorStatus(); },
		[this] { return this->FormatROCEmulationEnable(); },
		[this] { return this->FormatLinkEnable(); },
		[this] { return this->FormatSERDESReset(); },
		[this] { return this->FormatSERDESRXDisparityError(); },
		[this] { return this->FormatSERDESRXCharacterNotInTableError(); },
		[this] { return this->FormatSERDESUnlockError(); },
		[this] { return this->FormatSERDESPLLLocked(); },
		[this] { return this->FormatSERDESPLLPowerDown(); },
		[this] { return this->FormatSERDESRXStatus(); },
		[this] { return this->FormatSERDESResetDone(); },
		[this] { return this->FormatRXCDRLockStatus(); },
		[this] { return this->FormatDMATimeoutPreset(); },
		[this] { return this->FormatROCReplyTimeout(); },
		[this] { return this->FormatROCReplyTimeoutError(); },
		[this] { return this->FormatEVBLocalParitionIDMACIndex(); },
		[this] { return this->FormatEVBNumberOfDestinationNodes(); },
		[this] { return this->FormatTimingSERDESOscillatorFrequency(); },
		[this] { return this->FormatMainBoardSERDESOscillatorFrequency(); },
		[this] { return this->FormatSERDESOscillatorControl(); },
		[this] { return this->FormatSERDESOscillatorParameterLow(); },
		[this] { return this->FormatSERDESOscillatorParameterHigh(); },
		[this] { return this->FormatDDROscillatorFrequency(); },
		[this] { return this->FormatDDROscillatorControl(); },
		[this] { return this->FormatDDROscillatorParameterLow(); },
		[this] { return this->FormatDDROscillatorParameterHigh(); },
		[this] { return this->FormatDataPendingTimer(); },
		[this] { return this->FormatFIFOFullErrorFlag0(); },
		[this] { return this->FormatFIFOFullErrorFlag1(); },
		[this] { return this->FormatFIFOFullErrorFlag2(); },
		[this] { return this->FormatReceivePacketError(); },
		[this] { return this->FormatCFOEmulationTimestampLow(); },
		[this] { return this->FormatCFOEmulationTimestampHigh(); },
		[this] { return this->FormatCFOEmulationHeartbeatInterval(); },
		[this] { return this->FormatCFOEmulationNumHeartbeats(); },
		[this] { return this->FormatCFOEmulationNumPacketsLink01(); },
		[this] { return this->FormatCFOEmulationNumPacketsLink23(); },
		[this] { return this->FormatCFOEmulationNumPacketsLink45(); },
		[this] { return this->FormatCFOEmulationNumNullHeartbeats(); },
		[this] { return this->FormatCFOEmulationModeBytes03(); },
		[this] { return this->FormatCFOEmulationModeBytes45(); },
		[this] { return this->FormatCFOEmulationDebugPacketType(); },
		[this] { return this->FormatRXPacketCountErrorFlags(); },
		[this] { return this->FormatDetectorEmulationDMACount(); },
		[this] { return this->FormatDetectorEmulationDMADelayCount(); },
		[this] { return this->FormatDetectorEmulationControl0(); },
		[this] { return this->FormatDetectorEmulationControl1(); },
		[this] { return this->FormatDDRDataLocalStartAddress(); },
		[this] { return this->FormatDDRDataLocalEndAddress(); },
		[this] { return this->FormatCFOEmulatorInterpacketDelay(); },
		[this] { return this->FormatEthernetPayloadSize(); },
		[this] { return this->FormatCFOEmulation40MHzMarkerInterval(); },
		[this] { return this->FormatCFOEmulationMarkerEnables(); },
		[this] { return this->FormatROCFinishThreshold(); },
		[this] { return this->FormatFireflyTXIICControl(); },
		[this] { return this->FormatFireflyTXIICParameterLow(); },
		[this] { return this->FormatFireflyTXIICParameterHigh(); },
		[this] { return this->FormatFireflyRXIICControl(); },
		[this] { return this->FormatFireflyRXIICParameterLow(); },
		[this] { return this->FormatFireflyRXIICParameterHigh(); },
		[this] { return this->FormatFireflyTXRXIICControl(); },
		[this] { return this->FormatFireflyTXRXIICParameterLow(); },
		[this] { return this->FormatFireflyTXRXIICParameterHigh(); },
		[this] { return this->FormatSERDESTXPRBSControl(); },
		[this] { return this->FormatSERDESRXPRBSControl(); },
		[this] { return this->FormatEventModeLookupTableControl(); },
		[this] { return this->FormatDDRMemoryTestRegister(); },
		[this] { return this->FormatSERDESSerialInversionEnable(); },
		[this] { return this->FormatJitterAttenuatorCSR(); },
		[this] { return this->FormatSFPIICControl(); },
		[this] { return this->FormatSFPIICParameterLow(); },
		[this] { return this->FormatSFPIICParameterHigh(); },
		[this] { return this->FormatEVBSubEventReceiveTimer(); },
		[this] { return this->FormatEVBSERDESPRBSControl(); },
		[this] { return this->FormatEventBuilderErrorRegister(); },
		[this] { return this->FormatSERDESVFIFOError(); },
		[this] { return this->FormatPCIVFIFOError(); },
		[this] { return this->FormatRocLink0Error(); },
		[this] { return this->FormatRocLink1Error(); },
		[this] { return this->FormatRocLink2Error(); },
		[this] { return this->FormatRocLink3Error(); },
		[this] { return this->FormatRocLink4Error(); },
		[this] { return this->FormatRocLink5Error(); },
		[this] { return this->FormatCFOLinkError(); },
		[this] { return this->FormatLinkMuxError(); },
		[this] { return this->FormatFireflyCSR(); },
		[this] { return this->FormatSFPControlStatus(); },
		[this] { return this->FormatROCDCSResponseTimerPreset(); },
		[this] { return this->FormatFPGAPROMProgramStatus(); },
		[this] { return this->FormatFPGACoreAccess(); },
		[this] { return this->FormatSlowOpticalLinkControlStatus(); },
		[this] { return this->FormatSERDESInduceErrorEnable(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink0(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink1(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink2(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink3(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink4(); },
		[this] { return this->FormatSERDESInduceErrorSequenceNumberLink5(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink0(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink1(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink2(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink3(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink4(); },
		[this] { return this->FormatROCEmulatorInduceTimeoutErrorLink5(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink0(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink1(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink2(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink3(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink4(); },
		[this] { return this->FormatROCEmulatorExtraWordErrorLink5(); },
		[this] { return this->FormatSERDESRXCRCErrorControl(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink0(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink1(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink2(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink3(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink4(); },
		[this] { return this->FormatROCEmulatorInterpacketDelayLink5(); },
	};

	/// <summary>
	/// Dump Byte/Packet Counter Registers
	/// </summary>
	const std::vector<std::function<DTC_RegisterFormatter()>> formattedSERDESCounterFunctions_{
		[this] { return this->FormatReceiveByteCountLink0(); },
		[this] { return this->FormatReceiveByteCountLink1(); },
		[this] { return this->FormatReceiveByteCountLink2(); },
		[this] { return this->FormatReceiveByteCountLink3(); },
		[this] { return this->FormatReceiveByteCountLink4(); },
		[this] { return this->FormatReceiveByteCountLink5(); },
		[this] { return this->FormatReceiveByteCountCFO(); },
		[this] { return this->FormatReceivePacketCountLink0(); },
		[this] { return this->FormatReceivePacketCountLink1(); },
		[this] { return this->FormatReceivePacketCountLink2(); },
		[this] { return this->FormatReceivePacketCountLink3(); },
		[this] { return this->FormatReceivePacketCountLink4(); },
		[this] { return this->FormatReceivePacketCountLink5(); },
		[this] { return this->FormatReceivePacketCountCFO(); },
		[this] { return this->FormatTramsitByteCountLink0(); },
		[this] { return this->FormatTramsitByteCountLink1(); },
		[this] { return this->FormatTramsitByteCountLink2(); },
		[this] { return this->FormatTramsitByteCountLink3(); },
		[this] { return this->FormatTramsitByteCountLink4(); },
		[this] { return this->FormatTramsitByteCountLink5(); },
		[this] { return this->FormatTramsitByteCountCFO(); },
		[this] { return this->FormatTransmitPacketCountLink0(); },
		[this] { return this->FormatTransmitPacketCountLink1(); },
		[this] { return this->FormatTransmitPacketCountLink2(); },
		[this] { return this->FormatTransmitPacketCountLink3(); },
		[this] { return this->FormatTransmitPacketCountLink4(); },
		[this] { return this->FormatTransmitPacketCountLink5(); },
		[this] { return this->FormatTransmitPacketCountCFO(); }

	};

	const std::vector<std::function<DTC_RegisterFormatter()>> formattedPerformanceCounterFunctions_{
		[this] { return this->FormatDDRLinkBufferEmptyFlags0(); },
		[this] { return this->FormatDDRLinkBufferEmptyFlags1(); },
		[this] { return this->FormatDDRLinkBufferEmptyFlags2(); },
		[this] { return this->FormatDDRLinkBufferEmptyFlags3(); },
		[this] { return this->FormatDDRLinkBufferHalfFullFlags0(); },
		[this] { return this->FormatDDRLinkBufferHalfFullFlags1(); },
		[this] { return this->FormatDDRLinkBufferHalfFullFlags2(); },
		[this] { return this->FormatDDRLinkBufferHalfFullFlags3(); },
		[this] { return this->FormatDDRLinkBufferFullFlags0(); },
		[this] { return this->FormatDDRLinkBufferFullFlags1(); },
		[this] { return this->FormatDDRLinkBufferFullFlags2(); },
		[this] { return this->FormatDDRLinkBufferFullFlags3(); },
		[this] { return this->FormatDDREventBuilderBufferEmptyFlags0(); },
		[this] { return this->FormatDDREventBuilderBufferEmptyFlags1(); },
		[this] { return this->FormatDDREventBuilderBufferEmptyFlags2(); },
		[this] { return this->FormatDDREventBuilderBufferEmptyFlags3(); },
		[this] { return this->FormatDDREventBuilderBufferHalfFullFlags0(); },
		[this] { return this->FormatDDREventBuilderBufferHalfFullFlags1(); },
		[this] { return this->FormatDDREventBuilderBufferHalfFullFlags2(); },
		[this] { return this->FormatDDREventBuilderBufferHalfFullFlags3(); },
		[this] { return this->FormatDDREventBuilderBufferFullFlags0(); },
		[this] { return this->FormatDDREventBuilderBufferFullFlags1(); },
		[this] { return this->FormatDDREventBuilderBufferFullFlags2(); },
		[this] { return this->FormatDDREventBuilderBufferFullFlags3(); },
		[this] { return this->FormatRetransmitRequestCountLink0(); },
		[this] { return this->FormatRetransmitRequestCountLink1(); },
		[this] { return this->FormatRetransmitRequestCountLink2(); },
		[this] { return this->FormatRetransmitRequestCountLink3(); },
		[this] { return this->FormatRetransmitRequestCountLink4(); },
		[this] { return this->FormatRetransmitRequestCountLink5(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink0(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink1(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink2(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink3(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink4(); },
		[this] { return this->FormatDataPendingDiagnosticTimerLink5(); }

	};

	const std::vector<std::function<DTC_RegisterFormatter()>> formattedSERDESErrorFunctions_{
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink0(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink1(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink2(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink3(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink4(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountLink5(); },
		[this] { return this->FormatSERDESCharacterNotInTableErrorCountCFOLink(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink0(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink1(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink2(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink3(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink4(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountLink5(); },
		[this] { return this->FormatSERDESRXDisparityErrorCountCFOLink(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink0(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink1(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink2(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink3(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink4(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountLink5(); },
		[this] { return this->FormatSERDESRXPRBSErrorCountCFOLink(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink0(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink1(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink2(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink3(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink4(); },
		[this] { return this->FormatSERDESRXCRCErrorCountLink5(); },
		[this] { return this->FormatSERDESRXCRCErrorCountCFOLink(); },
		[this] { return this->FormatEVBSERDESRXPacketErrorCounter(); },
		[this] { return this->FormatJitterAttenuatorRecoveredClockLOSCount(); },
		[this] { return this->FormatJitterAttenuatorExternalClockLOSCount(); }

	};

	const std::vector<std::function<DTC_RegisterFormatter()>> formattedPacketCounterFunctions_{
		[this] { return this->FormatMissedCFOPacketCountLink0(); },
		[this] { return this->FormatMissedCFOPacketCountLink1(); },
		[this] { return this->FormatMissedCFOPacketCountLink2(); },
		[this] { return this->FormatMissedCFOPacketCountLink3(); },
		[this] { return this->FormatMissedCFOPacketCountLink4(); },
		[this] { return this->FormatMissedCFOPacketCountLink5(); },
		[this] { return this->FormatLocalFragmentDropCount(); },
		[this] { return this->FormatOutputBufferFragmentDumpCount(); },
		[this] { return this->FormatTXDataRequestPacketCountLink0(); },
		[this] { return this->FormatTXDataRequestPacketCountLink1(); },
		[this] { return this->FormatTXDataRequestPacketCountLink2(); },
		[this] { return this->FormatTXDataRequestPacketCountLink3(); },
		[this] { return this->FormatTXDataRequestPacketCountLink4(); },
		[this] { return this->FormatTXDataRequestPacketCountLink5(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink0(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink1(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink2(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink3(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink4(); },
		[this] { return this->FormatTXHeartbeatPacketCountLink5(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink0(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink1(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink2(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink3(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink4(); },
		[this] { return this->FormatRXDataHeaderPacketCountLink5(); },
		[this] { return this->FormatRXDataPacketCountLink0(); },
		[this] { return this->FormatRXDataPacketCountLink1(); },
		[this] { return this->FormatRXDataPacketCountLink2(); },
		[this] { return this->FormatRXDataPacketCountLink3(); },
		[this] { return this->FormatRXDataPacketCountLink4(); },
		[this] { return this->FormatRXDataPacketCountLink5(); }

	};
};
}  // namespace DTCLib

#endif  // DTC_REGISTERS_H
