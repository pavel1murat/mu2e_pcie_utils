#ifndef CFO_REGISTERS_H
#define CFO_REGISTERS_H

//#include <bitset> // std::bitset
//#include <cstdint> // uint8_t, uint16_t
#include <functional> // std::bind, std::function
#include <vector> // std::vector

#include "CFO_Types.h"
#include "cfodev.h"

namespace CFOLib
{
	enum CFO_Register : uint16_t
	{
		CFO_Register_DesignVersion = 0x9000,
		CFO_Register_DesignDate = 0x9004,
		CFO_Register_DesignStatus = 0x9008,
		CFO_Register_PerfMonTXByteCount = 0x900C,
		CFO_Register_PerfMonRXByteCount = 0x9010,
		CFO_Register_PerfMonTXPayloadCount = 0x9014,
		CFO_Register_PerfMonRXPayloadCount = 0x9018,
		CFO_Register_PerfMonInitCDC = 0x901C,
		CFO_Register_PerfMonInitCHC = 0x9020,
		CFO_Register_PerfMonInitNPDC = 0x9024,
		CFO_Register_PerfMonInitNPHC = 0x9028,
		CFO_Register_PerfMonInitPDC = 0x902C,
		CFO_Register_PerfMonInitPHC = 0x9030,
		CFO_Register_CFOControl = 0x9100,
		CFO_Register_DMATransferLength = 0x9104,
		CFO_Register_SERDESLoopbackEnable = 0x9108,
		CFO_Register_ClockOscillatorStatus = 0x910C,
		CFO_Register_RingEnable = 0x9114,
		CFO_Register_SERDESReset = 0x9118,
		CFO_Register_SERDESRXDisparityError = 0x911C,
		CFO_Register_SERDESRXCharacterNotInTableError = 0x9120,
		CFO_Register_SERDESUnlockError = 0x9124,
		CFO_Register_SERDESPLLLocked = 0x9128,
		CFO_Register_SERDESTXBufferStatus = 0x912C,
		CFO_Register_SERDESRXBufferStatus = 0x9130,
		CFO_Register_SERDESRXStatus = 0x9134,
		CFO_Register_SERDESResetDone = 0x9138,
		CFO_Register_SERDESEyescanData = 0x913C,
		CFO_Register_SFPSERDESStatus = 0x9140,
		CFO_Register_BeamOnTimeoutPreset = 0x9144,
		CFO_Register_EnableBeamOnMode = 0x9148,
		CFO_Register_EnableBeamOffMode = 0x914C,
		CFO_Register_RingPacketLength = 0x9150,
		CFO_Register_TimestampPreset0 = 0x9180,
		CFO_Register_TimestampPreset1 = 0x9184,
		CFO_Register_NUMDTCs = 0x918C,
		CFO_Register_FIFOFullErrorFlag0 = 0x9190,
		CFO_Register_ReceivePacketError = 0x919C,
		CFO_Register_EventWindowHoldoffTime = 0x91A4,
		CFO_Register_EventWindowEmulatorInterval = 0x91A8,
		CFO_Register_EventWindowTimeout = 0x91AC,
		CFO_Register_EventWindowTimeoutError = 0x91B0,
		CFO_Register_ReceiveByteCountDataRing0 = 0x9200,
		CFO_Register_ReceiveByteCountDataRing1 = 0x9204,
		CFO_Register_ReceiveByteCountDataRing2 = 0x9208,
		CFO_Register_ReceiveByteCountDataRing3 = 0x920C,
		CFO_Register_ReceiveByteCountDataRing4 = 0x9210,
		CFO_Register_ReceiveByteCountDataRing5 = 0x9214,
		CFO_Register_ReceiveByteCountDataRing6 = 0x9218,
		CFO_Register_ReceiveByteCountDataRing7 = 0x921C,
		CFO_Register_ReceivePacketCountDataRing0 = 0x9220,
		CFO_Register_ReceivePacketCountDataRing1 = 0x9224,
		CFO_Register_ReceivePacketCountDataRing2 = 0x9228,
		CFO_Register_ReceivePacketCountDataRing3 = 0x922C,
		CFO_Register_ReceivePacketCountDataRing4 = 0x9230,
		CFO_Register_ReceivePacketCountDataRing5 = 0x9234,
		CFO_Register_ReceivePacketCountDataRing6 = 0x9238,
		CFO_Register_ReceivePacketCountDataRing7 = 0x923C,
		CFO_Register_TransmitByteCountDataRing0 = 0x9240,
		CFO_Register_TransmitByteCountDataRing1 = 0x9244,
		CFO_Register_TransmitByteCountDataRing2 = 0x9248,
		CFO_Register_TransmitByteCountDataRing3 = 0x924C,
		CFO_Register_TransmitByteCountDataRing4 = 0x9250,
		CFO_Register_TransmitByteCountDataRing5 = 0x9254,
		CFO_Register_TransmitByteCountDataRing6 = 0x9258,
		CFO_Register_TransmitByteCountDataRing7 = 0x925C,
		CFO_Register_TransmitPacketCountDataRing0 = 0x9260,
		CFO_Register_TransmitPacketCountDataRing1 = 0x9264,
		CFO_Register_TransmitPacketCountDataRing2 = 0x9268,
		CFO_Register_TransmitPacketCountDataRing3 = 0x926C,
		CFO_Register_TransmitPacketCountDataRing4 = 0x9270,
		CFO_Register_TransmitPacketCountDataRing5 = 0x9274,
		CFO_Register_TransmitPacketCountDataRing6 = 0x9278,
		CFO_Register_TransmitPacketCountDataRing7 = 0x927C,
		CFO_Register_DDRWriteStartAddress = 0x9300,
		CFO_Register_DDRReadStartAddress = 0x9304,
		CFO_Register_DDRReadByteCount = 0x9308,
		CFO_Register_DDRBeamOnBaseAddress = 0x930C,
		CFO_Register_DDRBeamOffBaseAddress = 0x9310,
		CFO_Register_FPGAProgramData = 0x9400,
		CFO_Register_FPGAPROMProgramStatus = 0x9404,
		CFO_Register_FPGACoreAccess = 0x9408,
		CFO_Register_Invalid,
	};

	/// <summary>
	/// The CFO_Registers class represents the CFO Register space, and all the methods necessary to read and write those registers.
	/// Each register has, at the very least, a read method, a write method, and a CFO_RegisterFormatter method which
	/// formats the register value in a human-readable way.
	/// </summary>
	class CFO_Registers
	{
	public:
		/// <summary>
		/// Construct an instance of the CFO register map
		/// </summary>
		/// <param name="expectedDesignVersion">Expected CFO Firmware Design Version. If set, will throw an exception if the CFO firmware does not match (Default: "")</param>
		/// <param name="mode">Default: CFO_SimMode_Disabled; The simulation mode of the CFO</param>
		/// <param name="rocMask">Default 0x1; The initially-enabled ROCs. Each digit corresponds to a ring, so all ROCs = 0x666666</param>
		/// <param name="skipInit">Default: false; Whether to skip initializing the CFO using the SimMode. Used to read state.</param>
		explicit CFO_Registers(std::string expectedDesignVersion = "", CFO_SimMode mode = CFO_SimMode_Disabled, unsigned rocMask = 0x1, bool skipInit = false);
		/// <summary>
		/// CFO_Registers destructor
		/// </summary>
		virtual ~CFO_Registers();

		/// <summary>
		/// Get a pointer to the device handle
		/// </summary>
		/// <returns>cfodev* pointer</returns>
		cfodev* GetDevice()
		{
			return &device_;
		}

		//
		// CFO Sim Mode Virtual Register
		//
		/// <summary>
		/// Get the current CFO_SimMode of this CFO_Registers object
		/// </summary>
		/// <returns></returns>
		CFO_SimMode ReadSimMode() const
		{
			return simMode_;
		}

		/// <summary>
		/// Initialize the CFO in the given SimMode. 
		/// </summary>
		/// <param name="expectedDesignVersion">Expected CFO Firmware Design Version. If set, will throw an exception if the CFO firmware does not match</param>
		/// <param name="mode">Mode to set</param>
		/// <param name="rocMask">The initially-enabled ROCs. Each digit corresponds to a ring, so all ROCs = 0x666666</param>
		/// <param name="skipInit">Whether to skip initializing the CFO using the SimMode. Used to read state.</param>
		/// <returns></returns>
		CFO_SimMode SetSimMode(std::string expectedDesignVersion, CFO_SimMode mode, unsigned rocMask, bool skipInit = false);

		//
		// CFO Register Dumps
		//
		/// <summary>
		/// Perform a register dump
		/// </summary>
		/// <param name="width">Printable width of description fields</param>
		/// <returns>String containing all registers, with their human-readable representations</returns>
		std::string FormattedRegDump(int width);
		/// <summary>
		/// Dump the Performance Monitor registers
		/// </summary>
		/// <param name="width">Printable width of description fields</param>
		/// <returns>String containing the performance monitor registers, with their human-readable representations</returns>
		std::string PerformanceMonitorRegDump(int width);
		/// <summary>
		/// Dump the ring byte/packet counters
		/// </summary>
		/// <param name="width">Printable width of description fields</param>
		/// <returns>String containing the ring counter registers, with their human-readable representations</returns>
		std::string RingCountersRegDump(int width);

		/// <summary>
		/// Initializes a CFO_RegisterFormatter for the given CFO_Register
		/// </summary>
		/// <param name="address">Address of register to format</param>
		/// <returns>CFO_RegisterFormatter with address and raw value set</returns>
		CFO_RegisterFormatter CreateFormatter(const CFO_Register& address)
		{
			CFO_RegisterFormatter form;
			form.descWidth = formatterWidth_;
			form.address = address;
			form.value = ReadRegister_(address);
			return form;
		}

		//
		// Register IO Functions
		//

		// Desgin Version/Date Registers
		/// <summary>
		/// Read the design version
		/// </summary>
		/// <returns>Design version, in VersionNumber_Date format</returns>
		std::string ReadDesignVersion();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDesignVersion();
		/// <summary>
		/// Read the modification date of the CFO firmware
		/// </summary>
		/// <returns>Design date in 20YY-MM-DD-HH format</returns>
		std::string ReadDesignDate();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDesignDate();
		/// <summary>
		/// Read the design version number
		/// </summary>
		/// <returns>The design version number, in vMM.mm format</returns>
		std::string ReadDesignVersionNumber();

		// PCIE Performance Monitor Registers
		/// <summary>
		/// Read the TX Byte count for the PCIe bus
		/// </summary>
		/// <returns>The TX byte count</returns>
		uint32_t ReadPerfMonTXByteCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonTXByteCount();
		/// <summary>
		/// Read the RX Byte count for the PCIe bus
		/// </summary>
		/// <returns>The RX byte count</returns>
		uint32_t ReadPerfMonRXByteCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonRXByteCount();
		/// <summary>
		/// Read the TX Payload count for the PCIe bus
		/// </summary>
		/// <returns>The TX payload count</returns>
		uint32_t ReadPerfMonTXPayloadCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonTXPayloadCount();
		/// <summary>
		/// Read the RX Payload count for the PCIe bus
		/// </summary>
		/// <returns>The RX payload count</returns>
		uint32_t ReadPerfMonRXPayloadCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonRXPayloadCount();
		/// <summary>
		/// Read the Initial Complete Data Credits counter
		/// </summary>
		/// <returns>the Initial Complete Data Credits counter</returns>
		uint16_t ReadPerfMonInitCDC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitCDC();
		/// <summary>
		/// Read the Initial Complete Header Credits counter
		/// </summary>
		/// <returns>the Initial Complete Header Credits counter</returns>
		uint8_t ReadPerfMonInitCHC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitCHC();
		/// <summary>
		/// Read the Initial Non-posted Data Credits counter
		/// </summary>
		/// <returns>the Initial Non-posted Data Credits counter</returns>
		uint16_t ReadPerfMonInitNPDC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitNPDC();
		/// <summary>
		/// Read the Initial Non-posted Header Credits counter
		/// </summary>
		/// <returns>the Initial Non-posted Header Credits counter</returns>
		uint8_t ReadPerfMonInitNPHC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitNPHC();
		/// <summary>
		/// Read the Initial Posted Data Credits counter
		/// </summary>
		/// <returns>the Initial Posted Data Credits counter</returns>
		uint16_t ReadPerfMonInitPDC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitPDC();
		/// <summary>
		/// Read the Initial Posted Header Credits counter
		/// </summary>
		/// <returns>the Initial Posted Header Credits counter</returns>
		uint8_t ReadPerfMonInitPHC();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatPerfMonInitPHC();

		// CFO Control Register
		/// <summary>
		/// Perform a CFO Reset
		/// </summary>
		void ResetCFO();
		/// <summary>
		/// Read the Reset CFO Bit
		/// </summary>
		/// <returns>True if the CFO is currently resetting, false otherwise</returns>
		bool ReadResetCFO();
		/// <summary>
		/// Enable automatically generating Data Request packets from the CFO CFO Emulator
		/// </summary>
		void EnableAutogenDRP();
		/// <summary>
		/// Disable automatically generating Data Request packets from the CFO CFO Emulator
		/// </summary>
		void DisableAutogenDRP();
		/// <summary>
		/// Read whether Data Request packets are generated by the CFO CFO Emulator
		/// </summary>
		/// <returns>True if Data Request packets are generated by the CFO CFO Emulator, false otherwise</returns>
		bool ReadAutogenDRP();
		void EnableEventWindowInput();
		void DisableEventWindowInput();
		bool ReadEventWindowInput();
		/// <summary>
		/// Set the CFO to External timing mode
		/// </summary>
		void SetExternalSystemClock();
		/// <summary>
		/// Set the CFO to Internal timing mode
		/// </summary>
		void SetInternalSystemClock();
		/// <summary>
		/// Read the status of the System timing bit
		/// </summary>
		/// <returns>True if the clock is in External mode, false for Internal mode</returns>
		bool ReadSystemClock();
		/// <summary>
		/// Enable Timing Synchronization for the ROC control packets
		/// </summary>
		void EnableTiming();
		/// <summary>
		/// Disable timing synchronization for ROC control packets
		/// </summary>
		void DisableTiming();
		/// <summary>
		/// Read the status of the timing sync enable bit
		/// </summary>
		/// <returns>The current value of the timing sync enable bit</returns>
		bool ReadTimingEnable();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOControl();

		// DMA Transfer Length Register
		/// <summary>
		/// Set the DMA buffer size which will automatically trigger a DMA
		/// </summary>
		/// <param name="length">Size, in bytes of buffer that will trigger a DMA</param>
		void SetTriggerDMATransferLength(uint16_t length);
		/// <summary>
		/// Read the DMA buffer size which will automatically trigger a DMA
		/// </summary>
		/// <returns>The DMA buffer size which will automatically trigger a DMA, in bytes</returns>
		uint16_t ReadTriggerDMATransferLength();
		/// <summary>
		/// Set the minimum DMA transfer size. Absolute minimum is 64 bytes.
		/// Buffers smaller than this size will be padded to the minimum.
		/// </summary>
		/// <param name="length">Size, in bytes, of the minimum DMA transfer buffer</param>
		void SetMinDMATransferLength(uint16_t length);
		/// <summary>
		/// Read the minimum DMA transfer size.
		/// </summary>
		/// <returns>The minimum DMA size, in bytes</returns>
		uint16_t ReadMinDMATransferLength();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDMATransferLength();

		// SERDES Loopback Enable Register
		/// <summary>
		/// Set the SERDES Loopback mode for the given ring
		/// </summary>
		/// <param name="ring">Ring to set for</param>
		/// <param name="mode">CFO_SERDESLoopbackMode to set</param>
		void SetSERDESLoopbackMode(const CFO_Ring_ID& ring, const CFO_SERDESLoopbackMode& mode);
		/// <summary>
		/// Read the SERDES Loopback mode for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_SERDESLoopbackMode of the ring</returns>
		CFO_SERDESLoopbackMode ReadSERDESLoopback(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESLoopbackEnable();

		// Clock Status Register
		/// <summary>
		/// Read the SERDES Oscillator IIC Error Bit
		/// </summary>
		/// <returns>True if the SERDES Oscillator IIC Error is set</returns>
		bool ReadSERDESOscillatorIICError();
		/// <summary>
		/// Read the SERDES Oscillator Initalization Complete flag
		/// </summary>
		/// <returns>Whether the SERDES Oscillator has completed initialization</returns>
		bool ReadSERDESOscillatorInitializationComplete();
		/// <summary>
		/// Wait for the SERDES Oscillator to initialize, up to max_wait seconds
		/// </summary>
		/// <param name="max_wait">Seconds to wait</param>
		/// <returns>Whether the SERDES OScillator Completed initialization in the timeout</returns>
		bool WaitForSERDESOscillatorInitializationComplete(double max_wait = 1.0);
		/// <summary>
		/// Read the DDR Oscillator IIC Error Bit
		/// </summary>
		/// <returns>True if the DDR Oscillator IIC Error is set</returns>
		bool ReadDDROscillatorIICError();
		/// <summary>
		/// Read the DDR Oscillator Initalization Complete flag
		/// </summary>
		/// <returns>Whether the DDR Oscillator has completed initialization</returns>
		bool ReadDDROscillatorInitializationComplete();
		/// <summary>
		/// Wait for the DDR Oscillator to initialize, up to max_wait seconds
		/// </summary>
		/// <param name="max_wait">Seconds to wait</param>
		/// <returns>Whether the DDR OScillator Completed initialization in the timeout</returns>
		bool WaitForDDROscillatorInitializationComplete(double max_wait = 1.0);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatClockOscillatorStatus();

		// Ring Enable Register
		/// <summary>
		/// Enable a SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to enable</param>
		/// <param name="mode">Ring enable bits to set (Default: All)</param>
		/// <param name="lastRoc">Number of ROCs in the Ring (Default: Invalid value)</param>
		void EnableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode = CFO_RingEnableMode());
		/// <summary>
		/// Disable a SERDES Ring
		/// The given mode bits will be UNSET
		/// </summary>
		/// <param name="ring">Ring to disable</param>
		/// <param name="mode">Ring enable bits to unset (Default: All)</param>
		void DisableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode = CFO_RingEnableMode());
		/// <summary>
		/// Read the Ring Enable bits for a given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_RingEnableMode containing TX, RX, and CFO bits</returns>
		CFO_RingEnableMode ReadRingEnabled(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatRingEnable();

		// SERDES Reset Register
		/// <summary>
		/// Reset the SERDES
		/// Will poll the Reset SERDES Done flag until the SERDES reset is complete
		/// </summary>
		/// <param name="ring">Ring to reset</param>
		/// <param name="interval">Pollint interval, in microseconds</param>
		void ResetSERDES(const CFO_Ring_ID& ring, int interval = 100);
		/// <summary>
		/// Read if a SERDES reset is currently in progress
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if a SERDES reset is in progress</returns>
		bool ReadResetSERDES(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESReset();

		// SERDES RX Disparity Error Register
		/// <summary>
		/// Read the SERDES RX Dispatity Error bits
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_SERDESRXDisparityError object with error bits</returns>
		CFO_SERDESRXDisparityError ReadSERDESRXDisparityError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESRXDisparityError();

		// SERDES Character Not In Table Error Register
		/// <summary>
		/// Read the SERDES Character Not In Table Error bits
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_CharacterNotInTableError object with error bits</returns>
		CFO_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESRXCharacterNotInTableError();

		// SERDES Unlock Error Register
		/// <summary>
		/// Read whether the SERDES Unlock Error bit is set
		/// </summary>
		/// <param name="ring">Ring to check</param>
		/// <returns>True if the SERDES Unlock Error bit is set on the given ring</returns>
		bool ReadSERDESUnlockError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESUnlockError();

		// SERDES PLL Locked Register
		/// <summary>
		/// Read if the SERDES PLL is locked for the given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the PLL is locked, false otherwise</returns>
		bool ReadSERDESPLLLocked(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESPLLLocked();

		// SERDES TX Buffer Status Register
		/// <summary>
		/// Read the Overflow or Underflow error bit for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Overflow or Underflow Error bit is set</returns>
		bool ReadSERDESOverflowOrUnderflow(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the SERDES Buffer FIFO Half Full status bit for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES buffer FIFO is half-full</returns>
		bool ReadSERDESBufferFIFOHalfFull(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESTXBufferStatus();

		// SERDES RX Buffer Status Register
		/// <summary>
		/// Read the SERDES RX Buffer Status for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_RXBufferStatus object</returns>
		CFO_RXBufferStatus ReadSERDESRXBufferStatus(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESRXBufferStatus();

		// SERDES RX Status Register
		/// <summary>
		/// Read the SERDES RX Status for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_RXStatus object</returns>
		CFO_RXStatus ReadSERDESRXStatus(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESRXStatus();

		// SERDES Reset Done Register
		/// <summary>
		/// Read if the SERDES reset is complete on the given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES Reset is done, false otherwise</returns>
		bool ReadResetSERDESDone(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESResetDone();

		// Eyescan Data Error Register
		/// <summary>
		/// Read if the Eyescan Data Error flag is set on the given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Eyescan Error Bit is set, false otherwise</returns>
		bool ReadSERDESEyescanError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESEyescanData();

		// SFP / SERDES Status Register
		/// <summary>
		/// Read the SERDES SFP Present bit for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES SFP Present bit is set</returns>
		bool ReadSERDESSFPPresent(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the SERDES SFP Loss-of-Signal bit for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES SFP Loss-of-Signal bit is set</returns>
		bool ReadSERDESSFPLOS(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the SERDES SFP TX Fault bit for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES SFP TX Fault bit is set</returns>
		bool ReadSERDESSFPTXFault(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the SERDES CDR Lock bit for the given SERDES Ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the SERDES CDR Lock bit is set</returns>
		bool ReadSERDESRXCDRLock(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSFPSERDESStatus();

		// DMA Timeout Preset Regsiter
		/// <summary>
		/// Set the maximum time a DMA buffer may be active before it is sent, in 4ns ticks.
		/// The default value is 0x800
		/// </summary>
		/// <param name="preset">Maximum active time for DMA buffers</param>
		void SetDMATimeoutPreset(uint32_t preset);
		/// <summary>
		/// Read the maximum time a DMA buffer may be active before it is sent, in 4ns ticks.
		/// The default value is 0x800
		/// </summary>
		/// <returns>Maximum active time for DMA buffers</returns>
		uint32_t ReadDMATimeoutPreset();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDMATimeoutPreset();

		// ROC Timeout (Header Packet to All Packets Received) Preset Register
		/// <summary>
		/// Set the timeout between the reception of a Data Header packet from a ROC and receiving all of the associated Data Packets.
		/// If a timeout occurrs, the ROCTimeoutError flag will be set. Timeout is in SERDES clock ticks
		/// </summary>
		/// <param name="preset">Timeout value. Default: 0x200000</param>
		void SetROCTimeoutPreset(uint32_t preset);
		/// <summary>
		/// Read the timeout between the reception of a Data Header packet from a ROC and receiving all of the associated Data Packets.
		/// If a timeout occurrs, the ROCTimeoutError flag will be set. Timeout is in SERDES clock ticks
		/// </summary>
		/// <returns>Timeout value</returns>
		uint32_t ReadROCTimeoutPreset();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatROCReplyTimeout();

		// ROC Timeout Error Register
		/// <summary>
		/// Clear the ROC Data Packet timeout error flag for the given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearROCTimeoutError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the ROC Data Packet Timeout Error Flag for the given SERDES ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the error flag is set, false otherwise</returns>
		bool ReadROCTimeoutError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatROCReplyTimeoutError();

		// Ring Packet Length Register
		/// <summary>
		/// Set the size of CFO SERDES packets. Default is 16 bytes
		/// This value should most likely never be changed.
		/// </summary>
		/// <param name="packetSize">New packet size, in bytes</param>
		void SetPacketSize(uint16_t packetSize);
		/// <summary>
		/// Read the size of CFO SERDES packets. Default is 16 bytes
		/// </summary>
		/// <returns>Packet size, in bytes</returns>
		uint16_t ReadPacketSize();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatRingPacketLength();

		// EVB Network Partition ID / EVB Network Local MAC Index Register
		/// <summary>
		/// Set the EVB Mode byte
		/// </summary>
		/// <param name="mode">New Mode byte</param>
		void SetEVBMode(uint8_t mode);
		/// <summary>
		/// Read the EVB Mode byte
		/// </summary>
		/// <returns>EVB Mode byte</returns>
		uint8_t ReadEVBMode();
		/// <summary>
		/// Set the local partition ID
		/// </summary>
		/// <param name="id">Local partition ID</param>
		void SetEVBLocalParitionID(uint8_t id);
		/// <summary>
		/// Read the local partition ID
		/// </summary>
		/// <returns>Partition ID</returns>
		uint8_t ReadEVBLocalParitionID();
		/// <summary>
		/// Set the MAC address for the EVB network (lowest byte)
		/// </summary>
		/// <param name="macByte">MAC Address</param>
		void SetEVBLocalMACAddress(uint8_t macByte);
		/// <summary>
		/// Read the MAC address for the EVB network (lowest byte)
		/// </summary>
		/// <returns>MAC Address</returns>
		uint8_t ReadEVBLocalMACAddress();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatEVBLocalParitionIDMACIndex();

		// EVB Number of Destination Nodes Register
		/// <summary>
		/// Set the start node in the EVB cluster
		/// </summary>
		/// <param name="node">Node ID (MAC Address)</param>
		void SetEVBStartNode(uint8_t node);
		/// <summary>
		/// Read the start node in the EVB cluster
		/// </summary>
		/// <returns>Node ID (MAC Address)</returns>
		uint8_t ReadEVBStartNode();
		/// <summary>
		/// Set the number of destination nodes in the EVB cluster
		/// </summary>
		/// <param name="number">Number of nodes</param>
		void SetEVBNumberOfDestinationNodes(uint8_t number);
		/// <summary>
		/// Read the number of destination nodes in the EVB cluster
		/// </summary>
		/// <returns>Number of nodes</returns>
		uint8_t ReadEVBNumberOfDestinationNodes();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatEVBNumberOfDestinationNodes();

		// Heartbeat Error Register
		/// <summary>
		/// Read the Heartbeat Timeout Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Heartbeat Timeout Error Flag is set</returns>
		bool ReadHeartbeatTimeout(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the Heartbeat 2-0 Mismatch Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Heartbeat 2-0 Mismatch Error Flag is set</returns>
		bool ReadHeartbeat20Mismatch(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the Heartbeat 1-2 Mismatch Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Heartbeat 1-2 Mismatch Error Flag is set</returns>
		bool ReadHeartbeat12Mismatch(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the Heartbeat 0-1 Mismatch Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Heartbeat 0-1 Mismatch Error Flag is set</returns>
		bool ReadHeartbeat01Mismatch(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatHeartbeatError();

		// SERDES Oscillator Registers
		/// <summary>
		/// Read the current SERDES Oscillator frequency, in Hz
		/// </summary>
		/// <returns>Current SERDES Oscillator frequency, in Hz</returns>
		uint32_t ReadSERDESOscillatorFrequency();
		/// <summary>
		/// Set the SERDES Oscillator frequency
		/// </summary>
		/// <param name="freq">New frequency, in Hz</param>
		void SetSERDESOscillatorFrequency(uint32_t freq);
		/// <summary>
		/// Read the SERDES Oscillator IIC FSM bit
		/// </summary>
		/// <returns>True if the Oscillator IIC FSM bit is set</returns>
		bool ReadSERDESOscillaotrIICFSMEnable();
		/// <summary>
		/// Enable the SERDES Oscillator IIC FSM
		/// </summary>
		void EnableSERDESOscillatorIICFSM();
		/// <summary>
		/// Disable the SERDES Oscillator IIC FSM
		/// </summary>
		void DisableSERDESOscillatorIICFSM();
		/// <summary>
		/// Returns the current mode of the SERDES Oscillator Parameter register
		/// </summary>
		/// <returns>True if the Oscillator is in Write mode, false for Read mode</returns>
		bool ReadSERDESOscillatorReadWriteMode();
		/// <summary>
		/// Set the SERDES Oscillator to Write mode
		/// </summary>
		void SetSERDESOscillatorWriteMode();
		/// <summary>
		/// Set the SERDES Oscillator to Read mode
		/// </summary>
		void SetSERDESOscillatorReadMode();
		/// <summary>
		/// Read the current Oscillator program for the SERDES Oscillator
		/// </summary>
		/// <returns>SERDES Oscillator Program</returns>
		uint64_t ReadSERDESOscillatorParameters();
		/// <summary>
		/// Set the SERDES Oscillator program
		/// </summary>
		/// <param name="parameters">New program for the SERDES Oscillator</param>
		void SetSERDESOscillatorParameters(uint64_t parameters);
		/// <summary>
		/// Read the current SERDES Oscillator clock speed
		/// </summary>
		/// <returns>Current SERDES clock speed</returns>
		CFO_SerdesClockSpeed ReadSERDESOscillatorClock();
		/// <summary>
		/// Set the SERDES Oscillator clock speed for the given SERDES transfer rate
		/// </summary>
		/// <param name="speed">Clock speed to set</param>
		void SetSERDESOscillatorClock(CFO_SerdesClockSpeed speed);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESOscillatorFrequency();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESOscillatorControl();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESOscillatorParameterLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatSERDESOscillatorParameterHigh();

		// DDR Oscillator Registers
		/// <summary>
		/// Read the current DDR Oscillator frequency, in Hz
		/// </summary>
		/// <returns>Current DDR Oscillator frequency, in Hz</returns>
		uint32_t ReadDDROscillatorFrequency();
		/// <summary>
		/// Set the DDR Oscillator frequency
		/// </summary>
		/// <param name="freq">New frequency, in Hz</param>
		void SetDDROscillatorFrequency(uint32_t freq);
		/// <summary>
		/// Read the DDR Oscillator IIC FSM bit
		/// </summary>
		/// <returns>True if the Oscillator IIC FSM bit is set</returns>
		bool ReadDDROscillaotrIICFSMEnable();
		/// <summary>
		/// Enable the DDR Oscillator IIC FSM
		/// </summary>
		void EnableDDROscillatorIICFSM();
		/// <summary>
		/// Disable the DDR Oscillator IIC FSM
		/// </summary>
		void DisableDDROscillatorIICFSM();
		/// <summary>
		/// Returns the current mode of the DDR Oscillator Parameter register
		/// </summary>
		/// <returns>True if the Oscillator is in Write mode, false for Read mode</returns>
		bool ReadDDROscillatorReadWriteMode();
		/// <summary>
		/// Set the DDR Oscillator to Write mode
		/// </summary>
		void SetDDROscillatorWriteMode();
		/// <summary>
		/// Set the DDR Oscillator to Read mode
		/// </summary>
		void SetDDROscillatorReadMode();
		/// <summary>
		/// Read the current Oscillator program for the DDR Oscillator
		/// </summary>
		/// <returns>DDR Oscillator Program</returns>
		uint64_t ReadDDROscillatorParameters();
		/// <summary>
		/// Set the DDR Oscillator program
		/// </summary>
		/// <param name="parameters">New program for the DDR Oscillator</param>
		void SetDDROscillatorParameters(uint64_t parameters);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDROscillatorFrequency();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDROscillatorControl();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDROscillatorParameterLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDROscillatorParameterHigh();

		// Timestamp Preset Registers
		/// <summary>
		/// Set the Timestamp preset for Timing system emulation mode
		/// </summary>
		/// <param name="preset">Timestamp for Timing emulation</param>
		void SetTimestampPreset(const CFO_Timestamp& preset);
		/// <summary>
		/// Read the Timestamp preset for Timing system emulation mode
		/// </summary>
		/// <returns>Timestamp preset</returns>
		CFO_Timestamp ReadTimestampPreset();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTimestampPreset0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTimestampPreset1();

		// Data Pending Timer Register
		/// <summary>
		/// Set the timeout for waiting for a reply after sending a Data Request packet.
		/// Timeout is in SERDES clock ticks. Default value is 0x2000
		/// </summary>
		/// <param name="timer">New timeout</param>
		void SetDataPendingTimer(uint32_t timer);
		/// <summary>
		/// Read the timeout for waiting for a reply after sending a Data Request packet.
		/// Timeout is in SERDES clock ticks. Default value is 0x2000
		/// </summary>
		/// <returns>Current timeout</returns>
		uint32_t ReadDataPendingTimer();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDataPendingTimer();

		// NUMROCs Register
		/// <summary>
		/// Set the maximum ROC ID for the given ring
		/// </summary>
		/// <param name="ring">Ring to set</param>
		/// <param name="lastRoc">ID of the last ROC in the ring</param>
		void SetMaxROCNumber(const CFO_Ring_ID& ring, const CFO_ROC_ID& lastRoc);
		/// <summary>
		/// Read the number of ROCs configured on the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <param name="local">Whether to use the NUMROCs virtual register or perform a register access (Default: true, use virtual register)</param>
		/// <returns>ID of last ROC on ring</returns>
		CFO_ROC_ID ReadRingROCCount(const CFO_Ring_ID& ring, bool local = true);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatNUMROCs();

		// FIFO Full Error Flags Registers
		/// <summary>
		/// Clear all FIFO Full Error Flags for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearFIFOFullErrorFlags(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the FIFO Full Error/Status Flags for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>CFO_FIFOFullErrorFlags object</returns>
		CFO_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatFIFOFullErrorFlag0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatFIFOFullErrorFlag1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatFIFOFullErrorFlag2();

		// Receive Packet Error Register
		/// <summary>
		/// Clear the RX Elastic Buffer Underrun Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearRXElasticBufferUnderrun(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the RX Elastic Buffer Underrun Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the RX Elastic Buffer Underrun Error Flag is set</returns>
		bool ReadRXElasticBufferUnderrun(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the RX Elastic Buffer Overrun Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearRXElasticBufferOverrun(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the RX Elastic Buffer Overrun Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the RX Elastic Buffer Overrun Error Flag is set</returns>
		bool ReadRXElasticBufferOverrun(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the Packet Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearPacketError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the Packet Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Packet Error Flag is set</returns>
		bool ReadPacketError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the Packet CRC Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearPacketCRCError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the Packet CRC Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the Packet CRC Error Flag is set</returns>
		bool ReadPacketCRCError(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketError();

		// CFO Emulation Timestamp Registers 
		/// <summary>
		/// Set the starting CFO_Timestamp for the CFO Emulator
		/// </summary>
		/// <param name="ts">Starting Timestamp for CFO Emulation</param>
		void SetCFOEmulationTimestamp(const CFO_Timestamp& ts);
		/// <summary>
		/// Read the starting CFO_Timestamp for the CFO Emulator
		/// </summary>
		/// <returns>CFO_Timestamp object</returns>
		CFO_Timestamp ReadCFOEmulationTimestamp();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationTimestampLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationTimestampHigh();

		// CFO Emulation Request Interval Regsister
		/// <summary>
		/// Set the clock interval between CFO Emulator Readout Requests.
		/// This value is dependent upon the SERDES clock speed (2.5 Gbps = 125 MHz, 3.125 Gbps = 156.25 MHz)
		/// </summary>
		/// <param name="interval">Clock cycles between Readout Requests</param>
		void SetCFOEmulationRequestInterval(uint32_t interval);
		/// <summary>
		/// Read the clock interval between CFO Emulator Readout Requests.
		/// This value is dependent upon the SERDES clock speed (2.5 Gbps = 125 MHz, 3.125 Gbps = 156.25 MHz)
		/// </summary>
		/// <returns>Clock cycles between Readout Requests</returns>
		uint32_t ReadCFOEmulationRequestInterval();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationRequestInterval();

		// CFO Emulation Number of Requests Register
		/// <summary>
		/// Set the number of Readout Requests the CFO Emulator is configured to send.
		/// A value of 0 means that the CFO Emulator will send requests continuously.
		/// </summary>
		/// <param name="numRequests">Number of Readout Requests the CFO Emulator will send</param>
		void SetCFOEmulationNumRequests(uint32_t numRequests);
		/// <summary>
		/// Reads the number of Readout Requests the CFO Emulator is configured to send.
		/// A value of 0 means that the CFO Emulator will send requests continuously.
		/// </summary>
		/// <returns>Number of Readout Requests the CFO Emulator will send</returns>
		uint32_t ReadCFOEmulationNumRequests();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationNumRequests();

		// CFO Emulation Number of Packets Registers
		/// <summary>
		/// Set the number of packets the CFO Emulator will request from the ring
		/// </summary>
		/// <param name="ring">Ring to set</param>
		/// <param name="numPackets">Number of packets to request</param>
		void SetCFOEmulationNumPackets(const CFO_Ring_ID& ring, uint16_t numPackets);
		/// <summary>
		/// Read the requested number of packets the CFO Emulator will request from the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>Number of packets requested from the ring</returns>
		uint16_t ReadCFOEmulationNumPackets(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationNumPacketsRing01();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationNumPacketsRing23();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationNumPacketsRing45();

		// CFO Emulation Event Mode Bytes Registers
		/// <summary>
		/// Set the given CFO Emulation Mode byte to the given value
		/// </summary>
		/// <param name="byteNum">Byte to set. Valid range is 0-5</param>
		/// <param name="data">Data for byte</param>
		void SetCFOEmulationModeByte(const uint8_t& byteNum, uint8_t data);
		/// <summary>
		/// Read the given CFO Emulation Mode byte
		/// </summary>
		/// <param name="byteNum">Byte to read. Valid range is 0-5</param>
		/// <returns>Current value of the mode byte</returns>
		uint8_t ReadCFOEmulationModeByte(const uint8_t& byteNum);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationModeBytes03();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationModeBytes45();

		// CFO Emulation Debug Packet Type Register
		/// <summary>
		/// Enable putting the Debug Mode in Readout Requests
		/// </summary>
		void EnableDebugPacketMode();
		/// <summary>
		/// Disable putting the Debug Mode in Readout Requests
		/// </summary>
		void DisableDebugPacketMode();
		/// <summary>
		/// Whether Debug mode packets are enabled
		/// </summary>
		/// <returns>True if Debug Mode is enabled</returns>
		bool ReadDebugPacketMode();
		/// <summary>
		/// Set the DebugType used by the CFO Emulator
		/// </summary>
		/// <param name="type">The CFO_DebugType the CFO Emulator will fill into Readout Requests</param>
		void SetCFOEmulationDebugType(CFO_DebugType type);
		/// <summary>
		/// Read the DebugType field filled into Readout Requests generated by the CFO Emulator
		/// </summary>
		/// <returns>The CFO_DebugType used by the CFO Emulator</returns>
		CFO_DebugType ReadCFOEmulationDebugType();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatCFOEmulationDebugPacketType();

		// RX Packet Count Error Flags Register
		/// <summary>
		/// Read the RX Packet Count Error flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>Whether the RX Packet Count Error flag is set on the ring</returns>
		bool ReadRXPacketCountErrorFlags(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the RX Packet Count Error flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearRXPacketCountErrorFlags(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear all RX Packet Count Error Flags
		/// </summary>
		void ClearRXPacketCountErrorFlags();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatRXPacketCountErrorFlags();

		// Detector Emulation DMA Count Register
		/// <summary>
		/// Set the number of DMAs that the Detector Emulator will generate when enabled
		/// </summary>
		/// <param name="count">The number of DMAs that the Detector Emulator will generate when enabled</param>
		void SetDetectorEmulationDMACount(uint32_t count);
		/// <summary>
		/// Read the number of DMAs that the Detector Emulator will generate
		/// </summary>
		/// <returns>The number of DMAs that the Detector Emulator will generate</returns>
		uint32_t ReadDetectorEmulationDMACount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDetectorEmulationDMACount();

		// Detector Emulation DMA Delay Count Register
		/// <summary>
		/// Set the delay between DMAs in Detector Emulator mode
		/// </summary>
		/// <param name="count">Delay between DMAs in Detector Emulation mode, in 4ns ticks</param>
		void SetDetectorEmulationDMADelayCount(uint32_t count);
		/// <summary>
		/// Read the amount of the delay between DMAs in Detector Emulator mode
		/// </summary>
		/// <returns>The amount of the delay between DMAs in Detector Emulator mode, in 4ns ticks</returns>
		uint32_t ReadDetectorEmulationDMADelayCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDetectorEmulationDMADelayCount();

		// Detector Emulation Control Registers
		/// <summary>
		/// Enable Detector Emulator Mode. This sends all DMA writes to DMA channel 0 (DAQ) to DDR memory
		/// </summary>
		void EnableDetectorEmulatorMode();
		/// <summary>
		/// Disable sending DMA data to DDR memory
		/// </summary>
		void DisableDetectorEmulatorMode();
		/// <summary>
		/// Read whether writes to DMA Channel 0 will be loaded into DDR memory
		/// </summary>
		/// <returns>Whether the Detector Emulator Mode bit is set</returns>
		bool ReadDetectorEmulatorMode();
		/// <summary>
		/// Enable the Detector Emulator (Playback Mode)
		/// This assumes that data has been loaded into DDR memory using DMA Channel 0 before enabling.
		/// </summary>
		void EnableDetectorEmulator();
		/// <summary>
		/// Turn off the Detector Emulator (Playback Mode)
		/// </summary>
		void DisableDetectorEmulator();
		/// <summary>
		/// Read whether the Detector Emulator is enabled
		/// </summary>
		/// <returns>Whether the Detector Emulator is enabled</returns>
		bool ReadDetectorEmulatorEnable();
		/// <summary>
		/// Read whether a Detector Emulator Disable operation is in progress
		/// </summary>
		/// <returns>Whether the Detector Emulator Enable Clear bit is set</returns>
		bool ReadDetectorEmulatorEnableClear();
		/// <summary>
		/// Return the current value of the "Detector Emulator In Use" virtual register
		/// </summary>
		/// <returns>Whether the CFO Detector Emulator has been set up</returns>
		bool IsDetectorEmulatorInUse() const
		{
			return usingDetectorEmulator_;
		}
		/// <summary>
		/// Set the "Detector Emulator In Use" virtual register to true
		/// </summary>
		void SetDetectorEmulatorInUse() { usingDetectorEmulator_ = true; }
		/// <summary>
		/// Clear the "Detector Emulator In Use" virtual register
		/// </summary>
		void ClearDetectorEmulatorInUse();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDetectorEmulationControl0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDetectorEmulationControl1();

		// SERDES Counter Registers
		// DDR Event Data Local Start Address Register
		/// <summary>
		/// Set the DDR Data Start Address
		/// DDR Addresses are in bytes and must be 64-bit aligned
		/// </summary>
		/// <param name="address">Start address for the DDR data section</param>
		void SetDDRDataLocalStartAddress(uint32_t address);
		/// <summary>
		/// Read the DDR Data Start Address
		/// </summary>
		/// <returns>The current DDR data start address</returns>
		uint32_t ReadDDRDataLocalStartAddress();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRDataLocalStartAddress();

		// DDR Event Data Local End Address Register
		/// <summary>
		/// Set the end address for the DDR data section
		/// DDR Addresses are in bytes and must be 64-bit aligned
		/// </summary>
		/// <param name="address"></param>
		void SetDDRDataLocalEndAddress(uint32_t address);
		/// <summary>
		/// Read the current end address for the DDR Data section
		/// </summary>
		/// <returns>End address for the DDR data section</returns>
		uint32_t ReadDDRDataLocalEndAddress();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRDataLocalEndAddress();

		// ROC DRP Sync Error Register
		/// <summary>
		/// Read the ROC DRP Sync Error Flag for the given ring
		/// </summary>
		/// <param name="ring">Ring to read</param>
		/// <returns>True if the ROC DRP Sync Error Flag is set on the given ring, false otherwise</returns>
		bool ReadROCDRPSyncErrors(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear ROC DRP Sync Errors for the given ring
		/// </summary>
		/// <param name="ring">Ring to clear</param>
		void ClearROCDRPSyncErrors(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clears all ROC DRP Sync Errors
		/// </summary>
		void ClearROCDRPSyncErrors();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatROCDRPSyncError();

		// Ethernet Frame Payload Max Size
		/// <summary>
		/// Read the current maximum Ethernet payload size
		/// </summary>
		/// <returns>The current maximum Ethernet payload size</returns>
		uint32_t ReadEthernetPayloadSize();
		/// <summary>
		/// Set the maximum Ethernet payload size, in bytes. Maximum is 1492 bytes.
		/// </summary>
		/// <param name="size">Maximum Ethernet payload size, in bytes</param>
		void SetEthernetPayloadSize(uint32_t size);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatEthernetPayloadSize();


		/// <summary>
		/// Clear the value of the Receive byte counter
		/// </summary>
		/// <param name="ring">Ring to clear counter for</param>
		void ClearReceiveByteCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the value of the Receive byte counter
		/// </summary>
		/// <param name="ring">Ring to read counter for</param>
		/// <returns>Current value of the Receive byte counter on the given ring</returns>
		uint32_t ReadReceiveByteCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the value of the Receive Packet counter
		/// </summary>
		/// <param name="ring">Ring to clear counter for</param>
		void ClearReceivePacketCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the value of the Receive Packet counter
		/// </summary>
		/// <param name="ring">Ring to read counter for</param>
		/// <returns>Current value of the Receive Packet counter on the given ring</returns>
		uint32_t ReadReceivePacketCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the value of the Transmit byte counter
		/// </summary>
		/// <param name="ring">Ring to clear counter for</param>
		void ClearTransmitByteCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the value of the Transmit byye counter
		/// </summary>
		/// <param name="ring">Ring to read counter for</param>
		/// <returns>Current value of the Transmit byte counter on the given ring</returns>
		uint32_t ReadTransmitByteCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Clear the value of the Transmit Packet counter
		/// </summary>
		/// <param name="ring">Ring to clear counter for</param>
		void ClearTransmitPacketCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Read the value of the Transmit Packet counter
		/// </summary>
		/// <param name="ring">Ring to read counter for</param>
		/// <returns>Current value of the Transmit Packet counter on the given ring</returns>
		uint32_t ReadTransmitPacketCount(const CFO_Ring_ID& ring);
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing2();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing3();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing4();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountRing5();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountCFO();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceiveByteCountEVB();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing2();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing3();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing4();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountRing5();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountCFO();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatReceivePacketCountEVB();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing2();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing3();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing4();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountRing5();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountCFO();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTramsitByteCountEVB();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing2();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing3();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing4();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountRing5();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountCFO();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatTransmitPacketCountEVB();

		// DDR Memory Flags Registers
		/// <summary>
		/// Read the DDR Ring Buffer Full Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with full status of each of the buffers</returns>
		std::bitset<64> ReadDDRRingBufferFullFlags();
		/// <summary>
		/// Read the DDR Ring Buffer Full Error Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with full error status of each of the buffers</returns>
		std::bitset<64> ReadDDRRingBufferFullErrorFlags();
		/// <summary>
		/// Read the DDR Ring Buffer Empty Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with empty status of each of the buffers</returns>
		std::bitset<64> ReadDDRRingBufferEmptyFlags();
		/// <summary>
		/// Read the DDR Ring Buffer Half-Full Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with half-full status of each of the buffers</returns>
		std::bitset<64> ReadDDRRingBufferHalfFullFlags();
		/// <summary>
		/// Read the DDR EVB Buffer Full Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with full status of each of the buffers</returns>
		std::bitset<64> ReadDDREventBuilderBufferFullFlags();
		/// <summary>
		/// Read the DDR EVB Buffer Full Error Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with full error status of each of the buffers</returns>
		std::bitset<64> ReadDDREventBuilderBufferFullErrorFlags();
		/// <summary>
		/// Read the DDR EVB Buffer Empty Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with empty status of each of the buffers</returns>
		std::bitset<64> ReadDDREventBuilderBufferEmptyFlags();
		/// <summary>
		/// Read the DDR EVB Buffer Half-Full Flags for each of the 64 DDR buffers
		/// </summary>
		/// <returns>64-bit bitset with half-full status of each of the buffers</returns>
		std::bitset<64> ReadDDREventBuilderBufferHalfFullFlags();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferFullFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferFullErrorFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferEmptyFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferHalfFullFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferFullFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferFullErrorFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferEmptyFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlagsLow();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferFullFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferFullErrorFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferEmptyFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDRRingBufferHalfFullFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferFullFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferFullErrorFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferEmptyFlagsHigh();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatDDREventBuilderBufferHalfFullFlagsHigh();

		// EVB SERDES PRBS Control / Status Register
		/// <summary>
		/// Determine if an error was detected in the EVB SERDES PRBS module
		/// </summary>
		/// <returns>True if error condition was detected, false otherwise</returns>
		bool ReadEVBSERDESPRBSErrorFlag();
		/// <summary>
		/// Read the EVB SERDES TX PRBS Select
		/// </summary>
		/// <returns>Value of the EVB SERDES TX PRBS Select</returns>
		uint8_t ReadEVBSERDESTXPRBSSEL();
		/// <summary>
		/// Set the EVB SERDES TX PRBS Select
		/// </summary>
		/// <param name="byte">Value of the EVB SERDES TX PRBS Select</param>
		void SetEVBSERDESTXPRBSSEL(uint8_t byte);
		/// <summary>
		/// Read the EVB SERDES RX PRBS Select
		/// </summary>
		/// <returns>Value of the EVB SERDES RX PRBS Select</returns>
		uint8_t ReadEVBSERDESRXPRBSSEL();
		/// <summary>
		/// Set the EVB SERDES RX PRBS Select
		/// </summary>
		/// <param name="byte">Value of the EVB SERDES RX PRBS Select</param>
		void SetEVBSERDESRXPRBSSEL(uint8_t byte);
		/// <summary>
		/// Read the state of the EVB SERDES PRBS Force Error bit
		/// </summary>
		/// <returns>True if the EVB SERDES PRBS Force Error bit is high</returns>
		bool ReadEVBSERDESPRBSForceError();
		/// <summary>
		/// Set the EVB SERDES PRBS Force Error bit
		/// </summary>
		/// <param name="flag">New value for the EVB SERDES PRBS Reset bit</param>
		void SetEVBSERDESPRBSForceError(bool flag);
		/// <summary>
		/// Toggle the EVB SERDES PRBS Force Error bit (make it true if false, false if true)
		/// </summary>
		void ToggleEVBSERDESPRBSForceError();
		/// <summary>
		/// Read the state of the EVB SERDES PRBS Reset bit
		/// </summary>
		/// <returns>True if the EVB SERDES PRBS Reset bit is high</returns>
		bool ReadEVBSERDESPRBSReset();
		/// <summary>
		/// Set the EVB SERDES PRBS Reset bit
		/// </summary>
		/// <param name="flag">New value for the EVB SERDES PRBS Reset bit</param>
		void SetEVBSERDESPRBSReset(bool flag);
		/// <summary>
		/// Toggle the EVB SERDES PRBS Reset bit (make it true if false, false if true)
		/// </summary>
		void ToggleEVBSERDESPRBSReset();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatEVBSERDESPRBSControl();

		// Missed CFO Packet Count Registers
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 0
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 0</returns>
		uint32_t ReadMissedCFOPacketCountRing0();
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 1
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 1</returns>
		uint32_t ReadMissedCFOPacketCountRing1();
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 2
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 2</returns>
		uint32_t ReadMissedCFOPacketCountRing2();
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 3
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 3</returns>
		uint32_t ReadMissedCFOPacketCountRing3();
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 4
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 4</returns>
		uint32_t ReadMissedCFOPacketCountRing4();
		/// <summary>
		/// Reads the current value of the Missed CFO Packet Counter for Ring 5
		/// </summary>
		/// <returns>the current value of the Missed CFO Packet Counter for Ring 5</returns>
		uint32_t ReadMissedCFOPacketCountRing5();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 0
		/// </summary>
		void ClearMissedCFOPacketCountRing0();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 1
		/// </summary>
		void ClearMissedCFOPacketCountRing1();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 2
		/// </summary>
		void ClearMissedCFOPacketCountRing2();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 3
		/// </summary>
		void ClearMissedCFOPacketCountRing3();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 4
		/// </summary>
		void ClearMissedCFOPacketCountRing4();
		/// <summary>
		/// Clears the Missed CFO Packet Count for Ring 5
		/// </summary>
		void ClearMissedCFOPacketCountRing5();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing0();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing1();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing2();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing3();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing4();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatMissedCFOPacketCountRing5();

		// Local Fragment Drop Count
		/// <summary>
		/// Reads the current value of the Local Fragment Drop Counter
		/// </summary>
		/// <returns>The number of fragments dropped by the CFO</returns>
		uint32_t ReadLocalFragmentDropCount();
		/// <summary>
		/// Clears the Local Fragment Drop Counter
		/// </summary>
		void ClearLocalFragmentDropCount();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatLocalFragmentDropCount();

		// FPGA PROM Program Data Register

		// FPGA PROM Program Status Register
		/// <summary>
		/// Read the full bit on the FPGA PROM FIFO
		/// </summary>
		/// <returns>the full bit on the FPGA PROM FIFO</returns>
		bool ReadFPGAPROMProgramFIFOFull();
		/// <summary>
		/// Read whether the FPGA PROM is ready for data
		/// </summary>
		/// <returns>whether the FPGA PROM is ready for data</returns>
		bool ReadFPGAPROMReady();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatFPGAPROMProgramStatus();

		// FPGA Core Access Register
		/// <summary>
		/// Performs the chants necessary to reload the CFO firmware
		/// </summary>
		void ReloadFPGAFirmware();
		/// <summary>
		/// Read the FPGA Core Access FIFO Full bit
		/// </summary>
		/// <returns>Whether the FPGA Core Access FIFO is full</returns>
		bool ReadFPGACoreAccessFIFOFull();
		/// <summary>
		/// Read the FPGA Core Access FIFO Empty bit
		/// </summary>
		/// <returns>Whether the FPGA Core Access FIFO is empty</returns>
		bool ReadFPGACoreAccessFIFOEmpty();
		/// <summary>
		/// Formats the register's current value for register dumps
		/// </summary>
		/// <returns>CFO_RegisterFormatter object containing register information</returns>
		CFO_RegisterFormatter FormatFPGACoreAccess();

		// Event Mode Lookup Table
		/// <summary>
		/// Set all event mode words to the given value
		/// </summary>
		/// <param name="data">Value for all event mode words</param>
		void SetAllEventModeWords(uint32_t data);
		/// <summary>
		/// Set a given event mode word
		/// </summary>
		/// <param name="which">Word index to write</param>
		/// <param name="data">Data for word</param>
		void SetEventModeWord(uint8_t which, uint32_t data);
		/// <summary>
		/// Read an event mode word from the Event Mode lookup table
		/// </summary>
		/// <param name="which">Word index to read</param>
		/// <returns>Value of the given event mode word</returns>
		uint32_t ReadEventModeWord(uint8_t which);


		// Oscillator Programming (DDR and SERDES)
		/// <summary>
		/// Set the given oscillator to the given frequency, calculating a new program in the process.
		/// </summary>
		/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
		/// <param name="targetFrequency">New frequency to program, in Hz</param>
		void SetNewOscillatorFrequency(CFO_OscillatorType oscillator, double targetFrequency);
		/// <summary>
		/// Get the CFO's idea of the current frequency of the specified oscillator
		/// </summary>
		/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
		/// <returns>Current frequency of oscillator, in Hz</returns>
		double ReadCurrentFrequency(CFO_OscillatorType oscillator);
		/// <summary>
		/// Read the current RFREQ and dividers of the given oscillator clock
		/// </summary>
		/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
		/// <returns>64-bit integer contianing current oscillator program</returns>
		uint64_t ReadCurrentProgram(CFO_OscillatorType oscillator);
		/// <summary>
		/// Write the current frequency, in Hz to the frequency register
		/// </summary>
		/// <param name="freq">Frequency of oscillator</param>
		/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
		void WriteCurrentFrequency(double freq, CFO_OscillatorType oscillator);
		/// <summary>
		/// Writes a program for the given oscillator crystal. This function should be paired with a call to WriteCurrentFrequency
		/// so that subsequent programming attempts work as expected.
		/// </summary>
		/// <param name="program">64-bit integer with new RFREQ and dividers</param>
		/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
		void WriteCurrentProgram(uint64_t program, CFO_OscillatorType oscillator);

	private:
		void WriteRegister_(uint32_t data, const CFO_Register& address);
		uint32_t ReadRegister_(const CFO_Register& address);

		static int DecodeHighSpeedDivider_(int input);
		static int DecodeOutputDivider_(int input) { return input + 1; }
		static double DecodeRFREQ_(uint64_t input) { return input / 268435456.0; }
		static int EncodeHighSpeedDivider_(int input);
		static int EncodeOutputDivider_(int input);
		static uint64_t EncodeRFREQ_(double input) { return static_cast<uint64_t>(input * 268435456) & 0x3FFFFFFFFF; }
		static uint64_t CalculateFrequencyForProgramming_(double targetFrequency, double currentFrequency, uint64_t currentProgram);


	protected:
		cfodev device_; ///< Device handle
		CFO_SimMode simMode_; ///< Simulation mode
		CFO_ROC_ID maxROCs_[6]; ///< Map of active ROCs
		bool usingDetectorEmulator_; ///< Whether Detector Emulation mode is enabled
		uint16_t dmaSize_; ///< Size of DMAs, in bytes (default 32k)
		int formatterWidth_; ///< Description field width, in characters

		/// <summary>
		/// Functions needed to print regular register map
		/// </summary>
		const std::vector<std::function<CFO_RegisterFormatter()>> formattedDumpFunctions_
		{
			[this]() { return this->FormatDesignVersion(); },
			[this]() { return this->FormatDesignDate(); },
			[this]() { return this->FormatCFOControl(); },
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

		/// <summary>
		/// Dump Monitor Performance Registers
		/// </summary>
		const std::vector<std::function<CFO_RegisterFormatter()>> formattedPerfMonFunctions_
		{
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

		/// <summary>
		/// Dump Byte/Packet Counter Registers
		/// </summary>
		const std::vector<std::function<CFO_RegisterFormatter()>> formattedCounterFunctions_
		{
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

#endif //CFO_REGISTERS_H
