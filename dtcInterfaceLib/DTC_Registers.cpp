#include "DTC_Registers.h"

#include <assert.h>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <iomanip>  // std::setw, std::setfill
#include <sstream>  // Convert uint to hex string

#include "TRACE/tracemf.h"

#define DTC_TLOG(lvl) TLOG(lvl) << "DTC " << device_.getDTCID() << ": "
#define TLVL_ResetDTC TLVL_DEBUG + 5
#define TLVL_AutogenDRP TLVL_DEBUG + 6
#define TLVL_SERDESReset TLVL_DEBUG + 7
#define TLVL_CalculateFreq TLVL_DEBUG + 8
#define TLVL_ReadRegister TLVL_DEBUG + 20

#define __SHORTFILE__ \
	(strstr(&__FILE__[0], "/srcs/") ? strstr(&__FILE__[0], "/srcs/") + 6 : __FILE__)
#define __COUT__ std::cout << __SHORTFILE__ << " [" << std::dec << __LINE__ << "]\t"
#define __E__ std::endl
#define Q(X) #X
#define QUOTE(X) Q(X)
#define __COUTV__(X) __COUT__ << QUOTE(X) << " = " << X << __E__

/// <summary>
/// Construct an instance of the DTC register map
/// </summary>
/// <param name="mode">Default: DTC_SimMode_Disabled; The simulation mode of the DTC</param>
/// <param name="dtc">DTC card index to use</param>
/// <param name="simFileName">Name to use for simulated DTC memory file, if needed</param>
/// <param name="rocMask">Nibble-mask for enabled ROCs. Ex. 0x1011 enables ROC0, 1, and 3</param>
/// <param name="expectedDesignVersion">Expected DTC Firmware Design Version. If set, will
/// throw an exception if the DTC firmware does not match (Default: "")</param>
/// <param name="skipInit">Whether to skip initialization phase for reading DTC out in current state</param>
DTCLib::DTC_Registers::DTC_Registers(DTC_SimMode mode, int dtc, std::string simFileName, unsigned rocMask, std::string expectedDesignVersion,
									 bool skipInit)
	: device_(), simMode_(mode), usingDetectorEmulator_(false), dmaSize_(64)
{
	auto sim = getenv("DTCLIB_SIM_ENABLE");
	if (sim != nullptr)
	{
		auto simstr = std::string(sim);
		simMode_ = DTC_SimModeConverter::ConvertToSimMode(simstr);
	}
	TLOG(TLVL_INFO) << "Sim Mode is " << DTC_SimModeConverter(simMode_).toString();

	if (dtc == -1)
	{
		auto dtcE = getenv("DTCLIB_DTC");
		if (dtcE != nullptr)
		{
			dtc = atoi(dtcE);
		}
		else
			dtc = 0;
	}
	TLOG(TLVL_INFO) << "DTC ID is " << dtc;

	SetSimMode(expectedDesignVersion, simMode_, dtc, simFileName, rocMask, skipInit);
}

/// <summary>
/// DTC_Registers destructor
/// </summary>
DTCLib::DTC_Registers::~DTC_Registers()
{
	DisableDetectorEmulator();
	// DisableDetectorEmulatorMode();
	// DisableCFOEmulation();
	// ResetDTC();
	device_.close();
}

/// <summary>
/// Initialize the DTC in the given SimMode.
/// </summary>
/// <param name="expectedDesignVersion">Expected DTC Firmware Design Version. If set, will throw an exception if the DTC firmware does not match</param>
/// <param name="mode">Mode to set</param>
/// <param name="dtc">DTC card index to use</param>
/// <param name="simMemoryFile">Name to use for simulated DTC memory file, if needed</param>
/// <param name="rocMask">Default 0x1; The initially-enabled links. Each digit corresponds to a link, so all links = 0x111111</param>
/// <param name="skipInit">Whether to skip initializing the DTC using the SimMode. Used to read state.</param>
/// <returns></returns>
DTCLib::DTC_SimMode DTCLib::DTC_Registers::SetSimMode(std::string expectedDesignVersion, DTC_SimMode mode, int dtc, std::string simMemoryFile,
													  unsigned rocMask, bool skipInit)
{
	simMode_ = mode;
	TLOG(TLVL_INFO) << "Initializing device, sim mode is " << DTC_SimModeConverter(simMode_).toString();
	device_.init(simMode_, dtc, simMemoryFile);
	if (expectedDesignVersion != "" && expectedDesignVersion != ReadDesignVersion())
	{
		throw new DTC_WrongVersionException(expectedDesignVersion, ReadDesignVersion());
	}

	if (skipInit) return simMode_;

	TLOG(TLVL_DEBUG) << "Initialize requested, setting device registers acccording to sim mode " << DTC_SimModeConverter(simMode_).toString();
	for (auto link : DTC_Links)
	{
		bool linkEnabled = ((rocMask >> (link * 4)) & 0x1) != 0;
		if (!linkEnabled)
		{
			DisableLink(link);
		}
		else
		{
			EnableLink(link, DTC_LinkEnableMode(true, true));
		}
		if (!linkEnabled) DisableROCEmulator(link);
		if (!linkEnabled) SetSERDESLoopbackMode(link, DTC_SERDESLoopbackMode_Disabled);
	}

	if (simMode_ != DTC_SimMode_Disabled)
	{
		// Set up hardware simulation mode: Link 0 Tx/Rx Enabled, Loopback Enabled, ROC Emulator Enabled. All other links
		// disabled. for (auto link : DTC_Links)
		// 	{
		// 	  DisableLink(link);
		// 	}
		//	EnableLink(DTC_Link_0, DTC_LinkEnableMode(true, true, false), DTC_ROC_0);
		for (auto link : DTC_Links)
		{
			if (simMode_ == DTC_SimMode_Loopback)
			{
				SetSERDESLoopbackMode(link, DTC_SERDESLoopbackMode_NearPCS);
				//			SetMaxROCNumber(DTC_Link_0, DTC_ROC_0);
			}
			else if (simMode_ == DTC_SimMode_ROCEmulator)
			{
				EnableROCEmulator(link);
				// SetMaxROCNumber(DTC_Link_0, DTC_ROC_0);
			}
			else
			{
				SetSERDESLoopbackMode(link, DTC_SERDESLoopbackMode_Disabled);
				DisableROCEmulator(link);
			}
		}
		SetCFOEmulationMode();
	}
	else
	{
		ClearCFOEmulationMode();
	}
	ReadMinDMATransferLength();
//-----------------------------------------------------------------------------
// 2023-04-22 P.Murat: move check of a lock loss here, to perform it in one place
//-----------------------------------------------------------------------------
	uint32_t loss_of_lock = ReadRegister_(DTC_Register_RXCDRUnlockCount_CFOLink); // 0x93c8 read loss-of-lock counter

	TLOG(TLVL_DEBUG) << "Done setting device registers, loss of lock=" << loss_of_lock;
	return simMode_;
}

//
// DTC Register Dumps
//
/// <summary>
/// Perform a register dump
/// </summary>
/// <param name="width">Printable width of description fields</param>
/// <returns>String containing all registers, with their human-readable representations</returns>
std::string DTCLib::DTC_Registers::FormattedRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28)
	{
		formatterWidth_ = 28;
	}
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "Memory Map: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedDumpFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}

/// <summary>
/// Dump the link byte/packet counters
/// </summary>
/// <param name="width">Printable width of description fields</param>
/// <returns>String containing the link counter registers, with their human-readable representations</returns>
std::string DTCLib::DTC_Registers::LinkCountersRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28)
	{
		formatterWidth_ = 28;
	}
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "SERDES Byte/Packet Counters: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedSERDESCounterFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}

/// <summary>
/// Dump the link byte/packet counters
/// </summary>
/// <param name="width">Printable width of description fields</param>
/// <returns>String containing the link counter registers, with their human-readable representations</returns>
std::string DTCLib::DTC_Registers::PerformanceCountersRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28)
	{
		formatterWidth_ = 28;
	}
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "DTC Performance Counters: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedPerformanceCounterFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}
/// <summary>
/// Dump the SERDES Error Counters
/// </summary>
/// <param name="width">Printable width of description fields</param>
/// <returns>String containing the link counter registers, with their human-readable representations</returns>
std::string DTCLib::DTC_Registers::SERDESErrorsRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28)
	{
		formatterWidth_ = 28;
	}
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "SERDES Error Counters: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedSERDESErrorFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}
/// <summary>
/// Dump the Mu2e Protocol Packet counters
/// </summary>
/// <param name="width">Printable width of description fields</param>
/// <returns>String containing the link counter registers, with their human-readable representations</returns>
std::string DTCLib::DTC_Registers::PacketCountersRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28)
	{
		formatterWidth_ = 28;
	}
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "Mu2e Protocol Packet Counters: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedPacketCounterFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}
//
// Register IO Functions
//

// Desgin Version/Date Registers
/// <summary>
/// Read the design version
/// </summary>
/// <returns>Design version, in VersionNumber_Date format</returns>
std::string DTCLib::DTC_Registers::ReadDesignVersion() { return ReadDesignVersionNumber() + "_" + ReadDesignDate(); }

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDesignVersion()
{
	auto form = CreateFormatter(DTC_Register_DesignVersion);
	form.description = "DTC Firmware Design Version";
	form.vals.push_back(ReadDesignVersionNumber());
	return form;
}

/// <summary>
/// Read the modification date of the DTC firmware
/// </summary>
/// <returns>Design date in 20YY-MM-DD-HH format</returns>
std::string DTCLib::DTC_Registers::ReadDesignDate()
{
	auto data = ReadRegister_(DTC_Register_DesignDate);
	std::ostringstream o;
	int yearHex = (data & 0xFF000000) >> 24;
	auto year = ((yearHex & 0xF0) >> 4) * 10 + (yearHex & 0xF);
	int monthHex = (data & 0xFF0000) >> 16;
	auto month = ((monthHex & 0xF0) >> 4) * 10 + (monthHex & 0xF);
	int dayHex = (data & 0xFF00) >> 8;
	auto day = ((dayHex & 0xF0) >> 4) * 10 + (dayHex & 0xF);
	int hour = ((data & 0xF0) >> 4) * 10 + (data & 0xF);
	o << "20" << std::setfill('0') << std::setw(2) << year << "-";
	o << std::setfill('0') << std::setw(2) << month << "-";
	o << std::setfill('0') << std::setw(2) << day << "-";
	o << std::setfill('0') << std::setw(2) << hour;
	// std::cout << o.str() << std::endl;
	return o.str();
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDesignDate()
{
	auto form = CreateFormatter(DTC_Register_DesignDate);
	form.description = "DTC Firmware Design Date";
	form.vals.push_back(ReadDesignDate());
	return form;
}

/// <summary>
/// Read the design version number
/// </summary>
/// <returns>The design version number, in vMM.mm format</returns>
std::string DTCLib::DTC_Registers::ReadDesignVersionNumber()
{
	auto data = ReadRegister_(DTC_Register_DesignVersion);
	int minor = data & 0xFF;
	int major = (data & 0xFF00) >> 8;
	return "v" + std::to_string(major) + "." + std::to_string(minor);
}

/// <summary>
/// Read the DDR interface reset bit
/// </summary>
/// <returns>The current value of the DDR interface reset bit</returns>
bool DTCLib::DTC_Registers::ReadDDRInterfaceReset()
{
	return !std::bitset<32>(ReadRegister_(DTC_Register_DesignStatus))[1];
}

/// <summary>
/// Set the Reset bit in the Design Status register
/// </summary>
/// <param name="reset"></param>
void DTCLib::DTC_Registers::SetDDRInterfaceReset(bool reset)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DesignStatus);
	data[1] = !reset;
	WriteRegister_(data.to_ulong(), DTC_Register_DesignStatus);
}

/// <summary>
/// Resets the DDR interface in the firmware
/// </summary>
void DTCLib::DTC_Registers::ResetDDRInterface()
{
	SetDDRInterfaceReset(true);
	usleep(1000);
	SetDDRInterfaceReset(false);
	while (ReadDDRInterfaceReset() && !ReadDDRAutoCalibrationDone())
	{
		usleep(1000);
	}
}

/// <summary>
/// Read the DDR Auto Calibration Done bit of the Design status register
/// </summary>
/// <returns>Whether the DDR Auto Calibration is done</returns>
bool DTCLib::DTC_Registers::ReadDDRAutoCalibrationDone()
{
	return std::bitset<32>(ReadRegister_(DTC_Register_DesignStatus))[0];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDesignStatus()
{
	auto form = CreateFormatter(DTC_Register_DesignStatus);
	form.description = "Control and Status";
	form.vals.push_back(std::string("Reset DDR Interface:       [") + (ReadDDRInterfaceReset() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DDR Auto-Calibration Done: [") + (ReadDDRAutoCalibrationDone() ? "x" : " ") + "]");
	return form;
}

/// <summary>
/// Read the Vivado Version Number
/// </summary>
/// <returns>The Vivado Version number</returns>
std::string DTCLib::DTC_Registers::ReadVivadoVersionNumber()
{
	auto data = ReadRegister_(DTC_Register_VivadoVersion);
	std::ostringstream o;
	int yearHex = (data & 0xFFFF0000) >> 16;
	auto year = ((yearHex & 0xF000) >> 12) * 1000 + ((yearHex & 0xF00) >> 8) * 100 + ((yearHex & 0xF0) >> 4) * 10 +
				(yearHex & 0xF);
	int versionHex = (data & 0xFFFF);
	auto version = ((versionHex & 0xF000) >> 12) * 1000 + ((versionHex & 0xF00) >> 8) * 100 +
				   ((versionHex & 0xF0) >> 4) * 10 + (versionHex & 0xF);
	o << std::setfill('0') << std::setw(4) << year << "-" << version;
	// std::cout << o.str() << std::endl;
	return o.str();
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatVivadoVersion()
{
	auto form = CreateFormatter(DTC_Register_VivadoVersion);
	form.description = "DTC Firmware Vivado Version";
	form.vals.push_back(ReadVivadoVersionNumber());
	return form;
}

/// <summary>
/// Read the FPGA On-die Temperature sensor
/// </summary>
/// <returns>Temperature of the FGPA (deg. C)</returns>
double DTCLib::DTC_Registers::ReadFPGATemperature()
{
	auto val = ReadRegister_(DTC_Register_FPGA_Temperature);

	return ((val * 503.975) / 4096.0) - 273.15;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGATemperature()
{
	auto form = CreateFormatter(DTC_Register_FPGA_Temperature);
	form.description = "FPGA Temperature";
	form.vals.push_back(std::to_string(ReadFPGATemperature()) + " C");
	return form;
}

/// <summary>
/// Read the FPGA VCC INT Voltage (Nominal is 1.0 V)
/// </summary>
/// <returns>FPGA VCC INT Voltage (V)</returns>
double DTCLib::DTC_Registers::ReadFPGAVCCINTVoltage()
{
	auto val = ReadRegister_(DTC_Register_FPGA_VCCINT);
	return (val / 4095.0) * 3.0;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAVCCINT()
{
	auto form = CreateFormatter(DTC_Register_FPGA_VCCINT);
	form.description = "FPGA VCC INT";
	form.vals.push_back(std::to_string(ReadFPGAVCCINTVoltage()) + " V");
	return form;
}

/// <summary>
/// Read the FPGA VCC AUX Voltage (Nominal is 1.8 V)
/// </summary>
/// <returns>FPGA VCC AUX Voltage (V)</returns>
double DTCLib::DTC_Registers::ReadFPGAVCCAUXVoltage()
{
	auto val = ReadRegister_(DTC_Register_FPGA_VCCAUX);
	return (val / 4095.0) * 3.0;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAVCCAUX()
{
	auto form = CreateFormatter(DTC_Register_FPGA_VCCAUX);
	form.description = "FPGA VCC AUX";
	form.vals.push_back(std::to_string(ReadFPGAVCCAUXVoltage()) + " V");
	return form;
}

/// <summary>
/// Read the FPGA VCC BRAM Voltage (Nominal 1.0 V)
/// </summary>
/// <returns>FPGA VCC BRAM Voltage (V)</returns>
double DTCLib::DTC_Registers::ReadFPGAVCCBRAMVoltage()
{
	auto val = ReadRegister_(DTC_Register_FPGA_VCCBRAM);
	return (val / 4095.0) * 3.0;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAVCCBRAM()
{
	auto form = CreateFormatter(DTC_Register_FPGA_VCCBRAM);
	form.description = "FPGA VCC BRAM";
	form.vals.push_back(std::to_string(ReadFPGAVCCBRAMVoltage()) + " V");
	return form;
}

/// <summary>
/// Read the value of the FPGA Die Temperature Alarm bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadFPGADieTemperatureAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[8];
}

/// <summary>
/// Reset the FPGA Die Temperature Alarm bit
/// </summary>
void DTCLib::DTC_Registers::ResetFPGADieTemperatureAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[8] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Read the FPGA Alarms bit (OR of VCC and User Temperature alarm bits)
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadFPGAAlarms()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[7];
}

/// <summary>
/// Reset the FPGA Alarms bit
/// </summary>
void DTCLib::DTC_Registers::ResetFPGAAlarms()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[7] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Read the VCC BRAM Alarm bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadVCCBRAMAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[3];
}

/// <summary>
/// Reset the VCC BRAM Alarm bit
/// </summary>
void DTCLib::DTC_Registers::ResetVCCBRAMAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[3] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Read the VCC AUX Alarm bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadVCCAUXAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[2];
}

/// <summary>
/// Reset the VCC AUX Alarm bit
/// </summary>
void DTCLib::DTC_Registers::ResetVCCAUXAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[2] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Read the VCC INT Alarm bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadVCCINTAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[1];
}

/// <summary>
/// Reset the VCC INT Alarm bit
/// </summary>
void DTCLib::DTC_Registers::ResetVCCINTAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Read the FPGA User Temperature Alarm bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadFPGAUserTemperatureAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	return data[0];
}

/// <summary>
/// Reset the FPGA User Temperature Alarm bit
/// </summary>
void DTCLib::DTC_Registers::ResetFPGAUserTemperatureAlarm()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FPGA_MonitorAlarm);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FPGA_MonitorAlarm);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAAlarms()
{
	auto form = CreateFormatter(DTC_Register_FPGA_MonitorAlarm);
	form.description = "FPGA Monitor Alarm";
	form.vals.push_back(std::string("FPGA Die Temperature Alarm:  [") + (ReadFPGADieTemperatureAlarm() ? "x" : " ") + "]");
	form.vals.push_back(std::string("FPGA Alarms OR:              [") + (ReadFPGAAlarms() ? "x" : " ") + "]");
	form.vals.push_back(std::string("VCC BRAM Alarm:              [") + (ReadVCCBRAMAlarm() ? "x" : " ") + "]");
	form.vals.push_back(std::string("VCC AUX Alarm:               [") + (ReadVCCAUXAlarm() ? "x" : " ") + "]");
	form.vals.push_back(std::string("VCC INT Alarm:               [") + (ReadVCCINTAlarm() ? "x" : " ") + "]");
	form.vals.push_back(std::string("FPGA User Temperature Alarm: [") + (ReadFPGAUserTemperatureAlarm() ? "x" : " ") + "]");

	return form;
}

// DTC Control Register
/// <summary>
/// Perform a DTC Reset
/// </summary>
void DTCLib::DTC_Registers::ResetDTC()
{
	DTC_TLOG(TLVL_ResetDTC) << "ResetDTC start";
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[31] = 1;  // DTC Reset bit
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the Reset DTC Bit
/// </summary>
/// <returns>True if the DTC is currently resetting, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadResetDTC()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_DTCControl);
	return dataSet[31];
}

/// <summary>
/// Enable the DTC CFO Emulator
/// Parameters for the CFO Emulator, such as count and starting timestamp, must be set before enabling.
/// </summary>
void DTCLib::DTC_Registers::EnableCFOEmulation()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[30] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable the DTC CFO Emulator
/// </summary>
void DTCLib::DTC_Registers::DisableCFOEmulation()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[30] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Reads the current state of the DTC CFO Emulator
/// </summary>
/// <returns>True if the emulator is enabled, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadCFOEmulation()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_DTCControl);
	return dataSet[30];
}

/// <summary>
/// Enable CFO Loopback. CFO packets will be returned to the CFO, for delay calculation.
/// </summary>
void DTCLib::DTC_Registers::EnableCFOLoopback()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[28] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable CFO Loopback. CFO packets will be transmitted instead to the next DTC (Normal operation)
/// </summary>
void DTCLib::DTC_Registers::DisableCFOLoopback()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[28] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the value of the CFO Link Loopback bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadCFOLoopback()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_DTCControl);
	return dataSet[28];
}

/// <summary>
/// Resets the DDR Write pointer to 0
/// </summary>
void DTCLib::DTC_Registers::ResetDDRWriteAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[27] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
	usleep(1000);
	data = ReadRegister_(DTC_Register_DTCControl);
	data[27] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Determine whether a DDR Write Pointer reset is in progress
/// </summary>
/// <returns>True if the DDR Write pointer is resetting, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadResetDDRWriteAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[27];
}

/// <summary>
/// Reset the DDR Read pointer to 0
/// </summary>
void DTCLib::DTC_Registers::ResetDDRReadAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[26] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
	usleep(1000);
	data = ReadRegister_(DTC_Register_DTCControl);
	data[26] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Determine whether the DDR Read pointer is currently being reset.
/// </summary>
/// <returns>True if the read pointer is currently being reset, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadResetDDRReadAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[26];
}

/// <summary>
/// Reset the DDR memory interface
/// </summary>
void DTCLib::DTC_Registers::ResetDDR()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[25] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
	usleep(1000);
	data = ReadRegister_(DTC_Register_DTCControl);
	data[25] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Determine whether the DDR memory interface is currently being reset
/// </summary>
/// <returns>True if the DDR memory interface is currently being reset, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadResetDDR()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[25];
}

/// <summary>
/// Enable sending Data Request packets from the DTC CFO Emulator with every
/// Readout Request
/// </summary>
void DTCLib::DTC_Registers::EnableCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[24] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable sending Data Request packets from the DTC CFO Emulator with every
/// Readout Request
/// </summary>
void DTCLib::DTC_Registers::DisableCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[24] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read whether the DTC CFO Emulator is sending Data Request packets with every readout request
/// </summary>
/// <returns>True if the DTC CFO Emulator is sending Data Request packets with every readout request, false
/// otherwise</returns>
bool DTCLib::DTC_Registers::ReadCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[24];
}

/// <summary>
/// Enable automatically generating Data Request packets when heartbeats are received
/// </summary>
void DTCLib::DTC_Registers::EnableAutogenDRP()
{
	DTC_TLOG(TLVL_AutogenDRP) << "EnableAutogenDRP start";
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[23] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable automatically generating Data Request packets when heartbeats are received
/// </summary>
void DTCLib::DTC_Registers::DisableAutogenDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[23] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read whether Data Request packets are generated by the DTC when heartbeats are received
/// </summary>
/// <returns>True if Data Request packets are generated by the DTC when heartbeats are received, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadAutogenDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[23];
}

/// <summary>
/// Enable receiving Data Request Packets from the DTCLib on DMA Channel 0
/// Possibly obsolete, ask Rick before using
/// </summary>
void DTCLib::DTC_Registers::EnableSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[22] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable receiving Data Request Packets from the DTCLib on DMA Channel 0
/// Possibly obsolete, ask Rick before using
/// </summary>
void DTCLib::DTC_Registers::DisableSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[22] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read whether receiving Data Request Packets from the DTCLib on DMA Channel 0 is enabled
/// </summary>
/// <returns>True if receiving Data Request Packets from the DTCLib on DMA Channel 0 is enabled, false
/// otherwise</returns>
bool DTCLib::DTC_Registers::ReadSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[22];
}

/// <summary>
/// Enable the Down LED0 Control register bit
/// </summary>
void DTCLib::DTC_Registers::EnableDownLED0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[19] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable the Down LED0 Control register bit
/// </summary>
void DTCLib::DTC_Registers::DisableDownLED0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[19] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the state of the Down LED0 Control register bit
/// </summary>
/// <returns>Whether the Down LED0 bit is set</returns>
bool DTCLib::DTC_Registers::ReadDownLED0State()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[19];
}
/// <summary>
/// Enable the Up LED1 Control register bit
/// </summary>
void DTCLib::DTC_Registers::EnableUpLED1()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[18] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable the Up LED1 Control register bit
/// </summary>
void DTCLib::DTC_Registers::DisableUpLED1()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[18] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the state of the Up LED1 Control register bit
/// </summary>
/// <returns>Whether the Up LED1 bit is set</returns>
bool DTCLib::DTC_Registers::ReadUpLED1State()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[18];
}
/// <summary>
/// Enable the Up LED0 Control register bit
/// </summary>
void DTCLib::DTC_Registers::EnableUpLED0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[17] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable the Up LED0 Control register bit
/// </summary>
void DTCLib::DTC_Registers::DisableUpLED0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[17] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the state of the Up LED0 Control register bit
/// </summary>
/// <returns>Whether the Up LED0 bit is set</returns>
bool DTCLib::DTC_Registers::ReadUpLED0State()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[17];
}

/// <summary>
/// Enable the LED6 Control register bit
/// </summary>
void DTCLib::DTC_Registers::EnableLED6()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[16] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable the LED6 Control register bit
/// </summary>
void DTCLib::DTC_Registers::DisableLED6()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[16] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the state of the LED6 Control register bit
/// </summary>
/// <returns>Whether the LED6 bit is set</returns>
bool DTCLib::DTC_Registers::ReadLED6State()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[16];
}

/// <summary>
/// Enable CFO Emulation mode.
/// </summary>
void DTCLib::DTC_Registers::SetCFOEmulationMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[15] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Disable CFO Emulation mode.
/// </summary>
void DTCLib::DTC_Registers::ClearCFOEmulationMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[15] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

/// <summary>
/// Read the state of the CFO Emulation Mode bit
/// </summary>
/// <returns>Whether CFO Emulation Mode is enabled</returns>
bool DTCLib::DTC_Registers::ReadCFOEmulationMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[15];
}

void DTCLib::DTC_Registers::SetDataFilterEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[13] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearDataFilterEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[13] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadDataFilterEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[13];
}

void DTCLib::DTC_Registers::SetDRPPrefetchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[12] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearDRPPrefetchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[12] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadDRPPrefetchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[12];
}

void DTCLib::DTC_Registers::ROCInterfaceSoftReset()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[12] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
	usleep(100000);
	data[12] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadROCInterfaceSoftReset()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[11];
}

void DTCLib::DTC_Registers::SetSequenceNumberDisable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[10] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearSequenceNumberDisable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[10] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadSequenceNumberDisable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[10];
}

void DTCLib::DTC_Registers::SetPunchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[9] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearPunchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[9] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadPunchEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[9];
}

/// <summary>
/// Set the SERDES Global Reset bit to true, and wait for the reset to complete
/// </summary>
void DTCLib::DTC_Registers::ResetSERDES()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[8] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
	while (ReadResetSERDES())
	{
		usleep(1000);
	}
}

/// <summary>
/// Read the SERDES Global Reset bit
/// </summary>
/// <returns>Whether a SERDES global reset is in progress</returns>
bool DTCLib::DTC_Registers::ReadResetSERDES()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[8];
}

void DTCLib::DTC_Registers::SetRxPacketErrorFeedbackEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[6] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearRxPacketErrorFeedbackEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[6] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadRxPacketErrorFeedbackEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[6];
}

void DTCLib::DTC_Registers::SetCommaToleranceEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[5] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::ClearCommaToleranceEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[5] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadCommaToleranceEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[5];
}

void DTCLib::DTC_Registers::SetExternalFanoutClockInput()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[4] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::SetInternalFanoutClockInput()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[4] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadFanoutClockInput()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[4];
}

/// <summary>
/// Enalbe receiving DCS packets.
/// </summary>
void DTCLib::DTC_Registers::EnableDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[2] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}
/// <summary>
/// Disable receiving DCS packets. Any DCS packets received will be ignored
/// </summary>
void DTCLib::DTC_Registers::DisableDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[2] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}
/// <summary>
/// Read the status of the DCS Enable bit
/// </summary>
/// <returns>Whether DCS packet reception is enabled</returns>
bool DTCLib::DTC_Registers::ReadDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[2];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDTCControl()
{
	auto form = CreateFormatter(DTC_Register_DTCControl);
	form.description = "DTC Control";
	form.vals.push_back(std::string("Reset:                           [") + (ReadResetDTC() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Emulation Enable:            [") + (ReadCFOEmulation() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Link Output Control:         [") + (ReadCFOLoopback() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR Write Address:         [") + (ReadResetDDRWriteAddress() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR Read Address:          [") + (ReadResetDDRReadAddress() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR Interface:             [") + (ReadResetDDR() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Emulator DRP Enable:         [") + (ReadCFOEmulatorDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DTC Autogenerate DRP:            [") + (ReadAutogenDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Software DRP Enable:             [") + (ReadSoftwareDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Down LED 0:                      [") + (ReadDownLED0State() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Up LED 1:                        [") + (ReadUpLED1State() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Up LED 0:                        [") + (ReadUpLED0State() ? "x" : " ") + "]");
	form.vals.push_back(std::string("LED 6:                           [") + (ReadLED6State() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Emulation Mode:              [") + (ReadCFOEmulationMode() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Filter Enable:              [") + (ReadDataFilterEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DRP Prefetch Enable:             [") + (ReadDRPPrefetchEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("ROC Interface Soft Reset:        [") + (ReadROCInterfaceSoftReset() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Sequence Number Disable:         [") + (ReadSequenceNumberDisable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Punch Enable:                    [") + (ReadPunchEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SERDES Global Reset:             [") + (ReadResetSERDES() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error Feedback Enable: [") + (ReadRxPacketErrorFeedbackEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Comma Tolerance Enable:          [") + (ReadCommaToleranceEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Fanout Input Select:             [") + (ReadFanoutClockInput() ? "E" : "I") + "]");
	form.vals.push_back(std::string("DCS Enable:                      [") + (ReadDCSReception() ? "x" : " ") + "]");
	return form;
}

// DMA Transfer Length Register
/// <summary>
/// Set the DMA buffer size which will automatically trigger a DMA
/// </summary>
/// <param name="length">Size, in bytes of buffer that will trigger a DMA</param>
void DTCLib::DTC_Registers::SetTriggerDMATransferLength(uint16_t length)
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = (data & 0x0000FFFF) + (length << 16);
	WriteRegister_(data, DTC_Register_DMATransferLength);
}

/// <summary>
/// Read the DMA buffer size which will automatically trigger a DMA
/// </summary>
/// <returns>The DMA buffer size which will automatically trigger a DMA, in bytes</returns>
uint16_t DTCLib::DTC_Registers::ReadTriggerDMATransferLength()
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data >>= 16;
	return static_cast<uint16_t>(data);
}

/// <summary>
/// Set the minimum DMA transfer size. Absolute minimum is 64 bytes.
/// Buffers smaller than this size will be padded to the minimum.
/// </summary>
/// <param name="length">Size, in bytes, of the minimum DMA transfer buffer</param>
void DTCLib::DTC_Registers::SetMinDMATransferLength(uint16_t length)
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = (data & 0xFFFF0000) + length;
	WriteRegister_(data, DTC_Register_DMATransferLength);
}

/// <summary>
/// Read the minimum DMA transfer size.
/// </summary>
/// <returns>The minimum DMA size, in bytes</returns>
uint16_t DTCLib::DTC_Registers::ReadMinDMATransferLength()
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = data & 0x0000FFFF;
	dmaSize_ = static_cast<uint16_t>(data);
	return dmaSize_;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDMATransferLength()
{
	auto form = CreateFormatter(DTC_Register_DMATransferLength);
	form.description = "DMA Transfer Length";
	std::stringstream o;
	o << "Trigger Length: 0x" << std::hex << ReadTriggerDMATransferLength();
	form.vals.push_back(o.str());
	std::stringstream p;
	p << "Minimum Length: 0x" << std::hex << ReadMinDMATransferLength();
	form.vals.push_back(p.str());
	return form;
}

// SERDES Loopback Enable Register
/// <summary>
/// Set the SERDES Loopback mode for the given link
/// </summary>
/// <param name="link">Link to set for</param>
/// <param name="mode">DTC_SERDESLoopbackMode to set</param>
void DTCLib::DTC_Registers::SetSERDESLoopbackMode(DTC_Link_ID const& link, const DTC_SERDESLoopbackMode& mode)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESLoopbackEnable);
	std::bitset<3> modeSet = mode;
	data[3 * link] = modeSet[0];
	data[3 * link + 1] = modeSet[1];
	data[3 * link + 2] = modeSet[2];
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESLoopbackEnable);
}

/// <summary>
/// Read the SERDES Loopback mode for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_SERDESLoopbackMode of the link</returns>
DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC_Registers::ReadSERDESLoopback(DTC_Link_ID const& link)
{
	std::bitset<3> dataSet = ReadRegister_(DTC_Register_SERDESLoopbackEnable) >> 3 * link;
	return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESLoopbackEnable()
{
	auto form = CreateFormatter(DTC_Register_SERDESLoopbackEnable);
	form.description = "SERDES Loopback Enable";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": " +
							DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString());
	}
	form.vals.push_back(std::string("CFO:    ") +
						DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Link_CFO)).toString());
	form.vals.push_back(std::string("EVB:    ") +
						DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Link_EVB)).toString());
	return form;
}

// Clock Status Register
/// <summary>
/// Read the SERDES Oscillator IIC Error Bit
/// </summary>
/// <returns>True if the SERDES Oscillator IIC Error is set</returns>
bool DTCLib::DTC_Registers::ReadSERDESOscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESDDRClockStatus);
	return dataSet[2];
}

/// <summary>
/// Read the DDR Oscillator IIC Error Bit
/// </summary>
/// <returns>True if the DDR Oscillator IIC Error is set</returns>
bool DTCLib::DTC_Registers::ReadDDROscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESDDRClockStatus);
	return dataSet[18];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatClockOscillatorStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDESDDRClockStatus);
	form.description = "Clock Oscillator Status";
	form.vals.push_back(std::string("SERDES IIC Error:     [") + (ReadSERDESOscillatorIICError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DDR IIC Error:        [") + (ReadDDROscillatorIICError() ? "x" : " ") + "]");
	return form;
}

// ROC Emulation Enable Register
/// <summary>
/// Enable the ROC emulator on the given link
/// Note that the ROC Emulator will use the NUMROCs register to determine how many ROCs to emulate
/// </summary>
/// <param name="link">Link to enable</param>
void DTCLib::DTC_Registers::EnableROCEmulator(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	dataSet[link] = 1;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
}

/// <summary>
/// Disable the ROC emulator on the given link
/// </summary>
/// <param name="link">Link to disable</param>
void DTCLib::DTC_Registers::DisableROCEmulator(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	dataSet[link] = 0;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
}

/// <summary>
/// Read the state of the ROC emulator on the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the ROC Emulator is enabled on the link</returns>
bool DTCLib::DTC_Registers::ReadROCEmulator(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	return dataSet[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulationEnable()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulationEnable);
	form.description = "ROC Emulator Enable";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadROCEmulator(r) ? "x" : " ") + "]");
	}
	return form;
}

// Link Enable Register
/// <summary>
/// Enable a SERDES Link
/// </summary>
/// <param name="link">Link to enable</param>
/// <param name="mode">Link enable bits to set (Default: All)</param>
void DTCLib::DTC_Registers::EnableLink(DTC_Link_ID const& link, const DTC_LinkEnableMode& mode)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_LinkEnable);
	data[link] = mode.TransmitEnable;
	data[link + 8] = mode.ReceiveEnable;
	WriteRegister_(data.to_ulong(), DTC_Register_LinkEnable);
}

/// <summary>
/// Disable a SERDES Link
/// The given mode bits will be UNSET
/// </summary>
/// <param name="link">Link to disable</param>
/// <param name="mode">Link enable bits to unset (Default: All)</param>
void DTCLib::DTC_Registers::DisableLink(DTC_Link_ID const& link, const DTC_LinkEnableMode& mode)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_LinkEnable);
	data[link] = data[link] && !mode.TransmitEnable;
	data[link + 8] = data[link + 8] && !mode.ReceiveEnable;
	WriteRegister_(data.to_ulong(), DTC_Register_LinkEnable);
}

/// <summary>
/// Read the Link Enable bits for a given SERDES link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_LinkEnableMode containing TX, RX, and CFO bits</returns>
DTCLib::DTC_LinkEnableMode DTCLib::DTC_Registers::ReadLinkEnabled(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_LinkEnable);
	return DTC_LinkEnableMode(dataSet[link], dataSet[link + 8]);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatLinkEnable()
{
	auto form = CreateFormatter(DTC_Register_LinkEnable);
	form.description = "Link Enable";
	form.vals.push_back("       ([TX, RX, Timing])");
	for (auto r : DTC_Links)
	{
		auto re = ReadLinkEnabled(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (re.TransmitEnable ? "x" : " ") + "," +
							(re.ReceiveEnable ? "x" : " ") + "]");
	}
	{
		auto ce = ReadLinkEnabled(DTC_Link_CFO);
		form.vals.push_back(std::string("CFO:    [") + "TX:[" + (ce.TransmitEnable ? "x" : " ") + "], " + "RX:[" +
							(ce.ReceiveEnable ? "x" : " ") + "]]");
	}
	{
		auto ee = ReadLinkEnabled(DTC_Link_EVB);
		form.vals.push_back(std::string("EVB:    [") + "TX:[" + (ee.TransmitEnable ? "x" : " ") + "], " + "RX:[" +
							(ee.ReceiveEnable ? "x" : " ") + "]]");
	}
	return form;
}

// SERDES Reset Register
/// <summary>
/// Reset the SERDES TX side
/// Will poll the Reset SERDES TX Done flag until the SERDES reset is complete
/// </summary>
/// <param name="link">Link to reset</param>
/// <param name="interval">Polling interval, in microseconds</param>
void DTCLib::DTC_Registers::ResetSERDESTX(DTC_Link_ID const& link, int interval)
{
	auto resetDone = false;
	while (!resetDone)
	{
		DTC_TLOG(TLVL_SERDESReset) << "Entering SERDES Reset Loop for Link " << link;
		std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link + 24] = 1;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link + 24] = 0;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		resetDone = ReadResetTXSERDESDone(link);
		DTC_TLOG(TLVL_SERDESReset) << "End of SERDES Reset loop, done=" << std::boolalpha << resetDone;
	}
}

/// <summary>
/// Read if a SERDES TX reset is currently in progress
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if a SERDES TX reset is in progress</returns>
bool DTCLib::DTC_Registers::ReadResetSERDESTX(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_Reset);
	return dataSet[link + 24];
}

/// <summary>
/// Reset the SERDES RX side
/// Will poll the Reset SERDES RX Done flag until the SERDES reset is complete
/// </summary>
/// <param name="link">Link to reset</param>
/// <param name="interval">Polling interval, in microseconds</param>
void DTCLib::DTC_Registers::ResetSERDESRX(DTC_Link_ID const& link, int interval)
{
	auto resetDone = false;
	while (!resetDone)
	{
		DTC_TLOG(TLVL_SERDESReset) << "Entering SERDES Reset Loop for Link " << link;
		std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link + 16] = 1;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link + 16] = 0;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		resetDone = ReadResetRXSERDESDone(link);
		DTC_TLOG(TLVL_SERDESReset) << "End of SERDES Reset loop, done=" << std::boolalpha << resetDone;
	}
}

/// <summary>
/// Read if a SERDES RX reset is currently in progress
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if a SERDES reset is in progress</returns>
bool DTCLib::DTC_Registers::ReadResetSERDESRX(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_Reset);
	return dataSet[link + 16];
}

/// <summary>
/// Reset the SERDES PLL
/// Will poll the Reset SERDES PLL Done flag until the SERDES reset is complete
/// </summary>
/// <param name="pll">PLL to reset</param>
/// <param name="interval">Polling interval, in microseconds</param>
void DTCLib::DTC_Registers::ResetSERDESPLL(const DTC_PLL_ID& pll, int interval)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_Reset);
	data[pll + 8] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

	usleep(interval);

	data = ReadRegister_(DTC_Register_SERDES_Reset);
	data[pll + 8] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);
}

/// <summary>
/// Read if a SERDES PLL reset is currently in progress
/// </summary>
/// <param name="pll">PLL to read</param>
/// <returns>True if a SERDES reset is in progress</returns>
bool DTCLib::DTC_Registers::ReadResetSERDESPLL(const DTC_PLL_ID& pll)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_Reset);
	return dataSet[pll + 8];
}

/// <summary>
/// Reset the SERDES
/// Will poll the Reset SERDES Done flag until the SERDES reset is complete
/// </summary>
/// <param name="link">Link to reset</param>
/// <param name="interval">Polling interval, in microseconds</param>
void DTCLib::DTC_Registers::ResetSERDES(DTC_Link_ID const& link, int interval)
{
	auto resetDone = false;
	while (!resetDone)
	{
		DTC_TLOG(TLVL_SERDESReset) << "Entering SERDES Reset Loop for Link " << link;
		std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link] = 1;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		data = ReadRegister_(DTC_Register_SERDES_Reset);
		data[link] = 0;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDES_Reset);

		usleep(interval);

		resetDone = ReadResetRXSERDESDone(link);
		resetDone = resetDone && ReadResetTXSERDESDone(link);
		DTC_TLOG(TLVL_SERDESReset) << "End of SERDES Reset loop, done=" << std::boolalpha << resetDone;
	}
}

/// <summary>
/// Read if a SERDES reset is currently in progress
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if a SERDES reset is in progress</returns>
bool DTCLib::DTC_Registers::ReadResetSERDES(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_Reset);
	return dataSet[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESReset()
{
	auto form = CreateFormatter(DTC_Register_SERDES_Reset);
	form.description = "SERDES Reset";
	form.vals.push_back("       ([TX,RX,Link])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ":     [" + (ReadResetSERDESTX(r) ? "x" : " ") + (ReadResetSERDESRX(r) ? "x" : " ") + (ReadResetSERDES(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:        [") + (ReadResetSERDESTX(DTC_Link_CFO) ? "x" : " ") + (ReadResetSERDESRX(DTC_Link_CFO) ? "x" : " ") + (ReadResetSERDES(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:        [") + (ReadResetSERDESTX(DTC_Link_EVB) ? "x" : " ") + (ReadResetSERDESRX(DTC_Link_EVB) ? "x" : " ") + (ReadResetSERDES(DTC_Link_EVB) ? "x" : " ") + "]");
	for (auto r : DTC_PLLs)
	{
		form.vals.push_back(std::string("PLL Link ") + std::to_string(r) + ": [" + (ReadResetSERDESPLL(r) ? "x" : " ") +
							"]");
	}
	form.vals.push_back(std::string("PLL CFO RX: [") + (ReadResetSERDESPLL(DTC_PLL_CFO_RX) ? "x" : " ") + "]");
	form.vals.push_back(std::string("PLL CFO TX: [") + (ReadResetSERDESPLL(DTC_PLL_CFO_TX) ? "x" : " ") + "]");

	return form;
}

// SERDES RX Disparity Error Register
/// <summary>
/// Read the SERDES RX Dispatity Error bits
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_SERDESRXDisparityError object with error bits</returns>
DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC_Registers::ReadSERDESRXDisparityError(DTC_Link_ID const& link)
{
	return DTC_SERDESRXDisparityError(ReadRegister_(DTC_Register_SERDES_RXDisparityError), link);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityError()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityError);
	form.description = "SERDES RX Disparity Error";
	form.vals.push_back("       ([H,L])");
	for (auto r : DTC_Links)
	{
		auto re = ReadSERDESRXDisparityError(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + to_string(re.GetData()[1]) + "," +
							to_string(re.GetData()[0]) + "]");
	}
	auto ce = ReadSERDESRXDisparityError(DTC_Link_CFO);
	form.vals.push_back(std::string("CFO:    [") + to_string(ce.GetData()[1]) + "," + to_string(ce.GetData()[0]) + "]");
	return form;
}

// SERDES RX Character Not In Table Error Register
/// <summary>
/// Read the SERDES Character Not In Table Error bits
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_CharacterNotInTableError object with error bits</returns>
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC_Registers::ReadSERDESRXCharacterNotInTableError(
	DTC_Link_ID const& link)
{
	return DTC_CharacterNotInTableError(ReadRegister_(DTC_Register_SERDES_RXCharacterNotInTableError), link);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCharacterNotInTableError()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCharacterNotInTableError);
	form.description = "SERDES RX CNIT Error";
	form.vals.push_back("       ([H,L])");
	for (auto r : DTC_Links)
	{
		auto re = ReadSERDESRXCharacterNotInTableError(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + to_string(re.GetData()[1]) + "," +
							to_string(re.GetData()[0]) + "]");
	}
	auto ce = ReadSERDESRXCharacterNotInTableError(DTC_Link_CFO);
	form.vals.push_back(std::string("CFO:    [") + to_string(ce.GetData()[1]) + "," + to_string(ce.GetData()[0]) + "]");
	return form;
}

// SERDES Unlock Error Register
/// <summary>
/// Read whether the SERDES CDR Unlock Error bit is set
/// </summary>
/// <param name="link">Link to check</param>
/// <returns>True if the SERDES CDR Unlock Error bit is set on the given link</returns>
bool DTCLib::DTC_Registers::ReadSERDESCDRUnlockError(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_UnlockError);
	return dataSet[16 + link];
}

/// <summary>
/// Read whether the SERDES PLL Unlock Error bit is set
/// </summary>
/// <param name="pll">PLL to check</param>
/// <returns>True if the SERDES PLL Unlock Error bit is set for the given PLL</returns>
bool DTCLib::DTC_Registers::ReadSERDESPLLUnlockError(const DTC_PLL_ID& pll)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_UnlockError);
	return dataSet[pll];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESUnlockError()
{
	auto form = CreateFormatter(DTC_Register_SERDES_UnlockError);
	form.description = "SERDES Unlock Error";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("CDR Link ") + std::to_string(r) + ": [" + (ReadSERDESCDRUnlockError(r) ? "x" : " ") +
							"]");
	}
	form.vals.push_back(std::string("CDR CFO:    [") + (ReadSERDESCDRUnlockError(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("CDR EVB:    [") + (ReadSERDESCDRUnlockError(DTC_Link_EVB) ? "x" : " ") + "]");
	for (auto r : DTC_PLLs)
	{
		form.vals.push_back(std::string("PLL Link ") + std::to_string(r) + ":    [" + (ReadSERDESPLLUnlockError(r) ? "x" : " ") +
							"]");
	}
	form.vals.push_back(std::string("PLL CFO RX:    [") + (ReadSERDESPLLUnlockError(DTC_PLL_CFO_RX) ? "x" : " ") + "]");
	form.vals.push_back(std::string("PLL CFO TX:    [") + (ReadSERDESPLLUnlockError(DTC_PLL_CFO_TX) ? "x" : " ") + "]");
	form.vals.push_back(std::string("PLL CFO TX/RX: [") + (ReadSERDESPLLUnlockError(DTC_PLL_CFO_TXRX) ? "x" : " ") + "]");
	form.vals.push_back(std::string("PLL Timing:    [") + (ReadSERDESPLLUnlockError(DTC_PLL_PunchedClock) ? "x" : " ") + "]");
	return form;
}

// SERDES PLL Locked Register
/// <summary>
/// Read if the SERDES PLL is locked for the given SERDES link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the PLL is locked, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadSERDESPLLLocked(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_PLLLocked);
	return dataSet[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESPLLLocked()
{
	auto form = CreateFormatter(DTC_Register_SERDES_PLLLocked);
	form.description = "SERDES PLL Locked";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadSERDESPLLLocked(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESPLLLocked(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESPLLLocked(DTC_Link_EVB) ? "x" : " ") + "]");
	return form;
}

/// <summary>
/// Disable the SERDES PLL Power Down bit for the given link, enabling that link
/// </summary>
/// <param name="link">Link to set</param>
void DTCLib::DTC_Registers::EnableSERDESPLL(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_PLLPowerDown);
	data[link] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDES_PLLPowerDown);
}

/// <summary>
/// Enable the SERDES PLL Power Down bit for the given link, disabling that link
/// </summary>
/// <param name="link">Link to set</param>
void DTCLib::DTC_Registers::DisableSERDESPLL(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_PLLPowerDown);
	data[link] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDES_PLLPowerDown);
}

/// <summary>
/// Read the SERDES PLL Power Down bit for the given link
/// </summary>
/// <param name="link">link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESPLLPowerDown(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDES_PLLPowerDown);
	return data[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESPLLPowerDown()
{
	auto form = CreateFormatter(DTC_Register_SERDES_PLLPowerDown);
	form.description = "SERDES PLL Power Down";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadSERDESPLLPowerDown(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESPLLPowerDown(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESPLLPowerDown(DTC_Link_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES RX Status Register
/// <summary>
/// Read the SERDES RX Status for the given SERDES Link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_RXStatus object</returns>
DTCLib::DTC_RXStatus DTCLib::DTC_Registers::ReadSERDESRXStatus(DTC_Link_ID const& link)
{
	std::bitset<3> dataSet = ReadRegister_(DTC_Register_SERDES_RXStatus) >> 3 * link;
	return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXStatus);
	form.description = "SERDES RX Status";
	for (auto r : DTC_Links)
	{
		auto re = ReadSERDESRXStatus(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": " + DTC_RXStatusConverter(re).toString());
	}
	auto ce = ReadSERDESRXStatus(DTC_Link_CFO);
	form.vals.push_back(std::string("CFO:    ") + DTC_RXStatusConverter(ce).toString());

	return form;
}

// SERDES Reset Done Register
/// <summary>
/// Read the SERDES Reset RX FSM Done bit
/// </summary>
/// <param name="link">link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetRXFSMSERDESDone(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_ResetDone);
	return dataSet[link + 24];
}
/// <summary>
/// Read the SERDES Reset RX Done bit
/// </summary>
/// <param name="link">link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetRXSERDESDone(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_ResetDone);
	return dataSet[link + 16];
}
/// <summary>
/// Read the SERDES Reset TX FSM Done bit
/// </summary>
/// <param name="link">link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetTXFSMSERDESDone(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_ResetDone);
	return dataSet[link + 8];
}
/// <summary>
/// Read the SERDES Reset TX Done bit
/// </summary>
/// <param name="link">link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetTXSERDESDone(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_ResetDone);
	return dataSet[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESResetDone()
{
	auto form = CreateFormatter(DTC_Register_SERDES_ResetDone);
	form.description = "SERDES Reset Done";
	form.vals.push_back("       ([RX FSM, RX, TX FSM, TX])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadResetRXFSMSERDESDone(r) ? "x" : " ") + (ReadResetRXSERDESDone(r) ? "x" : " ") + (ReadResetTXFSMSERDESDone(r) ? "x" : " ") + (ReadResetTXSERDESDone(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadResetRXFSMSERDESDone(DTC_Link_CFO) ? "x" : " ") + (ReadResetRXSERDESDone(DTC_Link_CFO) ? "x" : " ") + (ReadResetTXFSMSERDESDone(DTC_Link_CFO) ? "x" : " ") + (ReadResetTXSERDESDone(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadResetRXFSMSERDESDone(DTC_Link_EVB) ? "x" : " ") + (ReadResetRXSERDESDone(DTC_Link_EVB) ? "x" : " ") + (ReadResetTXFSMSERDESDone(DTC_Link_EVB) ? "x" : " ") + (ReadResetTXSERDESDone(DTC_Link_EVB) ? "x" : " ") + "]");
	return form;
}

// SFP / SERDES Status Register

/// <summary>
/// Read the SERDES CDR Lock bit for the given SERDES Link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the SERDES CDR Lock bit is set</returns>
bool DTCLib::DTC_Registers::ReadSERDESRXCDRLock(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDES_RXCDRLockStatus);
	return dataSet[link];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRLockStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCDRLockStatus);
	form.description = "RX CDR Lock Status";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) +
							" CDR Lock: " + (ReadSERDESRXCDRLock(r) ? "x" : " "));
	}
	form.vals.push_back(std::string("CFO CDR Lock: ") + +(ReadSERDESRXCDRLock(DTC_Link_CFO) ? "x" : " "));
	form.vals.push_back(std::string("EVB CDR Lock: ") + (ReadSERDESRXCDRLock(DTC_Link_EVB) ? "x" : " "));
	return form;
}

// DMA Timeout Preset Register
/// <summary>
/// Set the maximum time a DMA buffer may be active before it is sent, in 4ns ticks.
/// The default value is 0x800
/// </summary>
/// <param name="preset">Maximum active time for DMA buffers</param>
void DTCLib::DTC_Registers::SetDMATimeoutPreset(uint32_t preset)
{
	WriteRegister_(preset, DTC_Register_DMATimeoutPreset);
}

/// <summary>
/// Read the maximum time a DMA buffer may be active before it is sent, in 4ns ticks.
/// The default value is 0x800
/// </summary>
/// <returns>Maximum active time for DMA buffers</returns>
uint32_t DTCLib::DTC_Registers::ReadDMATimeoutPreset() { return ReadRegister_(DTC_Register_DMATimeoutPreset); }

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDMATimeoutPreset()
{
	auto form = CreateFormatter(DTC_Register_DMATimeoutPreset);
	form.description = "DMA Timeout";
	std::stringstream o;
	o << "0x" << std::hex << ReadDMATimeoutPreset();
	form.vals.push_back(o.str());
	return form;
}

// ROC Timeout Preset Register
/// <summary>
/// Read the timeout between the reception of a Data Header packet from a ROC and receiving all of the associated Data
/// Packets. If a timeout occurrs, the ROCTimeoutError flag will be set. Timeout is in SERDES clock ticks
/// </summary>
/// <returns>Timeout value</returns>
uint32_t DTCLib::DTC_Registers::ReadROCTimeoutPreset() { return ReadRegister_(DTC_Register_ROCReplyTimeout); }

/// <summary>
/// Set the timeout between the reception of a Data Header packet from a ROC and receiving all of the associated Data
/// Packets. If a timeout occurrs, the ROCTimeoutError flag will be set. Timeout is in SERDES clock ticks
/// </summary>
/// <param name="preset">Timeout value. Default: 0x200000</param>
void DTCLib::DTC_Registers::SetROCTimeoutPreset(uint32_t preset)
{
	WriteRegister_(preset, DTC_Register_ROCReplyTimeout);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCReplyTimeout()
{
	auto form = CreateFormatter(DTC_Register_ROCReplyTimeout);
	form.description = "ROC Reply Timeout";
	std::stringstream o;
	o << "0x" << std::hex << ReadROCTimeoutPreset();
	form.vals.push_back(o.str());
	return form;
}

// ROC Timeout Error Register
/// <summary>
/// Clear the ROC Data Packet timeout error flag for the given SERDES link
/// </summary>
/// <param name="link">Link to clear</param>
bool DTCLib::DTC_Registers::ReadROCTimeoutError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ROCReplyTimeoutError);
	return data[static_cast<int>(link)];
}

/// <summary>
/// Read the ROC Data Packet Timeout Error Flag for the given SERDES link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the error flag is set, false otherwise</returns>
void DTCLib::DTC_Registers::ClearROCTimeoutError(DTC_Link_ID const& link)
{
	std::bitset<32> data = 0x0;
	data[link] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_ROCReplyTimeoutError);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCReplyTimeoutError()
{
	auto form = CreateFormatter(DTC_Register_ROCReplyTimeoutError);
	form.description = "ROC Reply Timeout Error";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadROCTimeoutError(r) ? "x" : " ") + "]");
	}
	return form;
}

// EVB Network Partition ID / EVB Network Local MAC Index Register
uint8_t DTCLib::DTC_Registers::ReadDTCID()
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF000000;
	return static_cast<uint8_t>(regVal >> 24);
}

/// <summary>
/// Set the EVB Mode byte
/// </summary>
/// <param name="mode">New Mode byte</param>
void DTCLib::DTC_Registers::SetEVBMode(uint8_t mode)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF00FFFF;
	regVal += mode << 16;
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

/// <summary>
/// Read the EVB Mode byte
/// </summary>
/// <returns>EVB Mode byte</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBMode()
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF0000;
	return static_cast<uint8_t>(regVal >> 16);
}

/// <summary>
/// Set the local partition ID
/// </summary>
/// <param name="id">Local partition ID</param>
void DTCLib::DTC_Registers::SetEVBLocalParitionID(uint8_t id)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFFFFFCFF;
	regVal += (id & 0x3) << 8;
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

/// <summary>
/// Read the local partition ID
/// </summary>
/// <returns>Partition ID</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBLocalParitionID()
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF0000;
	return static_cast<uint8_t>((regVal >> 8) & 0x3);
}

/// <summary>
/// Set the MAC address for the EVB network (lowest byte)
/// </summary>
/// <param name="macByte">MAC Address</param>
void DTCLib::DTC_Registers::SetEVBLocalMACAddress(uint8_t macByte)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFFFFFFC0;
	regVal += (macByte & 0x3F);
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

/// <summary>
/// Read the MAC address for the EVB network (lowest byte)
/// </summary>
/// <returns>MAC Address</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBLocalMACAddress() { return ReadRegister_(DTC_Register_EVBPartitionID) & 0x3F; }

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBLocalParitionIDMACIndex()
{
	auto form = CreateFormatter(DTC_Register_EVBPartitionID);
	form.description = "EVB Local Partition ID / MAC Index";
	std::ostringstream o;
	o << "DTC ID: 0x" << std::hex << static_cast<int>(ReadDTCID());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB Mode: 0x" << std::hex << static_cast<int>(ReadEVBMode());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB Local Parition ID: 0x" << std::hex << static_cast<int>(ReadEVBLocalParitionID());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB MAC Index:         0x" << std::hex << static_cast<int>(ReadEVBLocalMACAddress());
	form.vals.push_back(o.str());
	return form;
}

// EVB Number of Destination Nodes Register

void DTCLib::DTC_Registers::SetEVBNumberInputBuffers(uint8_t count)
{
	auto regVal = ReadRegister_(DTC_Register_EVBConfiguration) & 0xFF00FFFF;
	regVal += (count & 0xFF) << 16;
	WriteRegister_(regVal, DTC_Register_EVBConfiguration);
}
uint8_t DTCLib::DTC_Registers::ReadEVBNumberInputBuffers()
{
	return static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBConfiguration) & 0xFF0000) >> 16);
}

/// <summary>
/// Set the start node in the EVB cluster
/// </summary>
/// <param name="node">Node ID (MAC Address)</param>
void DTCLib::DTC_Registers::SetEVBStartNode(uint8_t node)
{
	auto regVal = ReadRegister_(DTC_Register_EVBConfiguration) & 0xFFFFC0FF;
	regVal += (node & 0x3F) << 8;
	WriteRegister_(regVal, DTC_Register_EVBConfiguration);
}

/// <summary>
/// Read the start node in the EVB cluster
/// </summary>
/// <returns>Node ID (MAC Address)</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBStartNode()
{
	return static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBConfiguration) & 0x3F00) >> 8);
}

/// <summary>
/// Set the number of destination nodes in the EVB cluster
/// </summary>
/// <param name="number">Number of nodes</param>
void DTCLib::DTC_Registers::SetEVBNumberOfDestinationNodes(uint8_t number)
{
	auto regVal = ReadRegister_(DTC_Register_EVBConfiguration) & 0xFFFFFFC0;
	regVal += (number & 0x3F);
	WriteRegister_(regVal, DTC_Register_EVBConfiguration);
}

/// <summary>
/// Read the number of destination nodes in the EVB cluster
/// </summary>
/// <returns>Number of nodes</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBNumberOfDestinationNodes()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_EVBConfiguration) & 0x3F);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBNumberOfDestinationNodes()
{
	auto form = CreateFormatter(DTC_Register_EVBConfiguration);
	form.description = "EVB Buffer Configuration";
	std::stringstream o;
	o << "Input Buffer Count: " << std::dec << static_cast<int>(ReadEVBNumberInputBuffers());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB Start Node: " << std::dec << static_cast<int>(ReadEVBStartNode());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB Number of Destination Nodes: " << std::dec << static_cast<int>(ReadEVBNumberOfDestinationNodes());
	form.vals.push_back(o.str());
	return form;
}

// SEREDES Oscillator Registers
/// <summary>
/// Read the current SERDES Oscillator reference frequency, in Hz
/// </summary>
/// <param name="device">Device to set oscillator for</param>
/// <returns>Current SERDES Oscillator reference frequency, in Hz</returns>
uint32_t DTCLib::DTC_Registers::ReadSERDESOscillatorReferenceFrequency(DTCLib::DTC_IICSERDESBusAddress device)
{
	switch (device)
	{
		case DTC_IICSERDESBusAddress_CFO:
			return ReadRegister_(DTC_Register_SERDESTimingCardOscillatorFrequency);
		case DTC_IICSERDESBusAddress_EVB:
			return ReadRegister_(DTC_Register_SERDESReferenceClockFrequency);
		default:
			return 0;
	}
	return 0;
}
/// <summary>
/// Set the SERDES Oscillator reference frequency
/// </summary>
/// <param name="device">Device to set oscillator for</param>
/// <param name="freq">New reference frequency, in Hz</param>
void DTCLib::DTC_Registers::SetSERDESOscillatorReferenceFrequency(DTCLib::DTC_IICSERDESBusAddress device,
																  uint32_t freq)
{
	switch (device)
	{
		case DTC_IICSERDESBusAddress_CFO:
			return WriteRegister_(freq, DTC_Register_SERDESTimingCardOscillatorFrequency);
		case DTC_IICSERDESBusAddress_EVB:
			return WriteRegister_(freq, DTC_Register_SERDESReferenceClockFrequency);
		default:
			return;
	}
	return;
}

/// <summary>
/// Read the Reset bit of the SERDES IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadSERDESOscillatorIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_SERDESClock_IICBusControl));
	return dataSet[31];
}

/// <summary>
/// Reset the SERDES IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetSERDESOscillatorIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_SERDESClock_IICBusControl);
	while (ReadSERDESOscillatorIICInterfaceReset())
	{
		usleep(1000);
	}
}

/// <summary>
/// Write a value to the SERDES IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteSERDESIICInterface(DTC_IICSERDESBusAddress device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_SERDESClock_IICBusLow);
	WriteRegister_(0x1, DTC_Register_SERDESClock_IICBusHigh);
	while (ReadRegister_(DTC_Register_SERDESClock_IICBusHigh) == 0x1)
	{
		usleep(1000);
	}
}

/// <summary>
/// Read a value from the SERDES IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadSERDESIICInterface(DTC_IICSERDESBusAddress device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_SERDESClock_IICBusLow);
	WriteRegister_(0x2, DTC_Register_SERDESClock_IICBusHigh);
	while (ReadRegister_(DTC_Register_SERDESClock_IICBusHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_SERDESClock_IICBusLow);
	return static_cast<uint8_t>(data);
}

/// <summary>
/// Read the current SERDES Oscillator clock speed
/// </summary>
/// <returns>Current SERDES clock speed</returns>
DTCLib::DTC_SerdesClockSpeed DTCLib::DTC_Registers::ReadSERDESOscillatorClock()
{
	auto freq = ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_EVB);

	// Clocks should be accurate to 30 ppm
	if (freq > 156250000 - 4687.5 && freq < 156250000 + 4687.5) return DTC_SerdesClockSpeed_3125Gbps;
	if (freq > 125000000 - 3750 && freq < 125000000 + 3750) return DTC_SerdesClockSpeed_25Gbps;
	return DTC_SerdesClockSpeed_Unknown;
}
/// <summary>
/// Set the SERDES Oscillator clock speed for the given SERDES transfer rate
/// </summary>
/// <param name="speed">Clock speed to set</param>
void DTCLib::DTC_Registers::SetSERDESOscillatorClock(DTC_SerdesClockSpeed speed)
{
	double targetFreq;
	switch (speed)
	{
		case DTC_SerdesClockSpeed_25Gbps:
			targetFreq = 125000000.0;
			break;
		case DTC_SerdesClockSpeed_3125Gbps:
			targetFreq = 156250000.0;
			break;
		default:
			targetFreq = 0.0;
			break;
	}
	if (SetNewOscillatorFrequency(DTC_OscillatorType_SERDES, targetFreq))
	{
		for (auto& link : DTC_Links)
		{
			ResetSERDES(link, 1000);
		}
		ResetSERDES(DTC_Link_CFO, 1000);
		// ResetSERDES(DTC_Link_EVB, 1000);
	}
}

/// <summary>
/// Set the Timing Oscillator clock to a given frequency
/// </summary>
/// <param name="freq">Frequency to set the Timing card Oscillator clock</param>
void DTCLib::DTC_Registers::SetTimingOscillatorClock(uint32_t freq)
{
	double targetFreq = freq;
	if (SetNewOscillatorFrequency(DTC_OscillatorType_Timing, targetFreq))
	{
		ResetSERDES(DTC_Link_CFO, 1000);
		// ResetSERDES(DTC_Link_EVB, 1000);
	}
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTimingSERDESOscillatorFrequency()
{
	auto form = CreateFormatter(DTC_Register_SERDESTimingCardOscillatorFrequency);
	form.description = "SERDES Timing Card Oscillator Reference Frequency";
	std::stringstream o;
	o << std::dec << ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_CFO);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMainBoardSERDESOscillatorFrequency()
{
	auto form = CreateFormatter(DTC_Register_SERDESReferenceClockFrequency);
	form.description = "SERDES Main Board Oscillator Reference Frequency";
	std::stringstream o;
	o << std::dec << ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_EVB);
	form.vals.push_back(o.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorControl()
{
	auto form = CreateFormatter(DTC_Register_SERDESClock_IICBusControl);
	form.description = "SERDES Oscillator IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadSERDESOscillatorIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorParameterLow()
{
	auto form = CreateFormatter(DTC_Register_SERDESClock_IICBusLow);
	form.description = "SERDES Oscillator IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_SERDESClock_IICBusLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "Address:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_SERDESClock_IICBusHigh);
	form.description = "SERDES Oscillator IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_SERDESClock_IICBusHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_SERDESClock_IICBusHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

// DDR Oscillator Registers
/// <summary>
/// Read the current DDR Oscillator frequency, in Hz
/// </summary>
/// <returns>Current DDR Oscillator frequency, in Hz</returns>
uint32_t DTCLib::DTC_Registers::ReadDDROscillatorReferenceFrequency()
{
	return ReadRegister_(DTC_Register_DDRReferenceClockFrequency);
}
/// <summary>
/// Set the DDR Oscillator frequency
/// </summary>
/// <param name="freq">New frequency, in Hz</param>
void DTCLib::DTC_Registers::SetDDROscillatorReferenceFrequency(uint32_t freq)
{
	WriteRegister_(freq, DTC_Register_DDRReferenceClockFrequency);
}
/// <summary>
/// Read the Reset bit of the DDR IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadDDROscillatorIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_DDRClock_IICBusControl));
	return dataSet[31];
}
/// <summary>
/// Reset the DDR IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetDDROscillatorIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_DDRClock_IICBusControl);
	while (ReadDDROscillatorIICInterfaceReset())
	{
		usleep(1000);
	}
}
/// <summary>
/// Write a value to the DDR IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteDDRIICInterface(DTC_IICDDRBusAddress device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_DDRClock_IICBusLow);
	WriteRegister_(0x1, DTC_Register_DDRClock_IICBusHigh);
	while (ReadRegister_(DTC_Register_DDRClock_IICBusHigh) == 0x1)
	{
		usleep(1000);
	}
}
/// <summary>
/// Read a value from the DDR IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadDDRIICInterface(DTC_IICDDRBusAddress device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_DDRClock_IICBusLow);
	WriteRegister_(0x2, DTC_Register_DDRClock_IICBusHigh);
	while (ReadRegister_(DTC_Register_DDRClock_IICBusHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_DDRClock_IICBusLow);
	return static_cast<uint8_t>(data);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorFrequency()
{
	auto form = CreateFormatter(DTC_Register_DDRReferenceClockFrequency);
	form.description = "DDR Oscillator Reference Frequency";
	std::stringstream o;
	o << std::dec << ReadDDROscillatorReferenceFrequency();
	form.vals.push_back(o.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorControl()
{
	auto form = CreateFormatter(DTC_Register_DDRClock_IICBusControl);
	form.description = "DDR Oscillator IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadDDROscillatorIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorParameterLow()
{
	auto form = CreateFormatter(DTC_Register_DDRClock_IICBusLow);
	form.description = "DDR Oscillator IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_DDRClock_IICBusLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "Address:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_DDRClock_IICBusHigh);
	form.description = "DDR Oscillator IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_DDRClock_IICBusHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_DDRClock_IICBusHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

// Data Pending Timer Register
/// <summary>
/// Set the timeout for waiting for a reply after sending a Data Request packet.
/// Timeout is in SERDES clock ticks. Default value is 0x2000
/// </summary>
/// <param name="timer">New timeout</param>
void DTCLib::DTC_Registers::SetDataPendingTimer(uint32_t timer)
{
	WriteRegister_(timer, DTC_Register_DataPendingTimer);
}

/// <summary>
/// Read the timeout for waiting for a reply after sending a Data Request packet.
/// Timeout is in SERDES clock ticks. Default value is 0x2000
/// </summary>
/// <returns>Current timeout</returns>
uint32_t DTCLib::DTC_Registers::ReadDataPendingTimer() { return ReadRegister_(DTC_Register_DataPendingTimer); }

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingTimer()
{
	auto form = CreateFormatter(DTC_Register_DataPendingTimer);
	form.description = "DMA Data Pending Timer";
	std::stringstream o;
	o << "0x" << std::hex << ReadDataPendingTimer();
	form.vals.push_back(o.str());
	return form;
}

// FIFO Full Error Flags Registers
/// <summary>
/// Clear all FIFO Full Error Flags for the given link
/// </summary>
/// <param name="link">Link to clear</param>
void DTCLib::DTC_Registers::ClearFIFOFullErrorFlags(DTC_Link_ID const& link)
{
	auto flags = ReadFIFOFullErrorFlags(link);
	std::bitset<32> data0 = 0;
	std::bitset<32> data1 = 0;
	std::bitset<32> data2 = 0;

	data0[link] = flags.OutputData;
	data0[link + 8] = flags.CFOLinkInput;
	data0[link + 16] = flags.ReadoutRequestOutput;
	data0[link + 24] = flags.DataRequestOutput;
	data1[link] = flags.OtherOutput;
	data1[link + 8] = flags.OutputDCS;
	data1[link + 16] = flags.OutputDCSStage2;
	data1[link + 24] = flags.DataInput;
	data2[link] = flags.DCSStatusInput;

	WriteRegister_(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister_(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister_(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);
}

/// <summary>
/// Read the FIFO Full Error/Status Flags for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>DTC_FIFOFullErrorFlags object</returns>
DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC_Registers::ReadFIFOFullErrorFlags(DTC_Link_ID const& link)
{
	std::bitset<32> data0 = ReadRegister_(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister_(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister_(DTC_Register_FIFOFullErrorFlag2);
	DTC_FIFOFullErrorFlags flags;

	flags.OutputData = data0[link];
	flags.CFOLinkInput = data0[link + 8];
	flags.ReadoutRequestOutput = data0[link + 16];
	flags.DataRequestOutput = data0[link + 24];
	flags.OtherOutput = data1[link];
	flags.OutputDCS = data1[link + 8];
	flags.OutputDCSStage2 = data1[link + 16];
	flags.DataInput = data1[link + 24];
	flags.DCSStatusInput = data2[link];

	return flags;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag0()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag0);
	form.description = "FIFO Full Error Flags 0";
	form.vals.push_back("       ([DataRequest, ReadoutRequest, CFOLink, OutputData])");
	for (auto r : DTC_Links)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (re.DataRequestOutput ? "x" : " ") + "," +
							(re.ReadoutRequestOutput ? "x" : " ") + "," + (re.CFOLinkInput ? "x" : " ") + "," +
							(re.OutputData ? "x" : " ") + "]");
	}
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag1()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag1);
	form.description = "FIFO Full Error Flags 1";
	form.vals.push_back("       ([DataInput, OutputDCSStage2, OutputDCS, OtherOutput])");
	for (auto r : DTC_Links)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (re.DataInput ? "x" : " ") + "," +
							(re.OutputDCSStage2 ? "x" : " ") + "," + (re.OutputDCS ? "x" : " ") + "," +
							(re.OtherOutput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Link_CFO);
		form.vals.push_back(std::string("CFO:    [") + +(ce.DataInput ? "x" : " ") + "," +
							(ce.OutputDCSStage2 ? "x" : " ") + "," + (ce.OutputDCS ? "x" : " ") + "," +
							(ce.OtherOutput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Link_EVB);
		form.vals.push_back(std::string("EVB:    [") + +(ce.DataInput ? "x" : " ") + "," +
							(ce.OutputDCSStage2 ? "x" : " ") + "," + (ce.OutputDCS ? "x" : " ") + "," +
							(ce.OtherOutput ? "x" : " ") + "]");
	}
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag2()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag2);
	form.description = "FIFO Full Error Flags 2";
	form.vals.push_back("       ([DCSStatusInput])");
	for (auto r : DTC_Links)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (re.DCSStatusInput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Link_CFO);
		form.vals.push_back(std::string("CFO:    [") + (ce.DCSStatusInput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Link_EVB);
		form.vals.push_back(std::string("EVB:    [") + (ce.DCSStatusInput ? "x" : " ") + "]");
	}
	return form;
}

// Receive Packet Error Register
/// <summary>
/// Clear the Packet Error Flag for the given link
/// </summary>
/// <param name="link">Link to clear</param>
void DTCLib::DTC_Registers::ClearPacketError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(link) + 8] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

/// <summary>
/// Read the Packet Error Flag for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the Packet Error Flag is set</returns>
bool DTCLib::DTC_Registers::ReadPacketError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(link) + 8];
}

/// <summary>
/// Clear the Packet CRC Error Flag for the given link
/// </summary>
/// <param name="link">Link to clear</param>
void DTCLib::DTC_Registers::ClearPacketCRCError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(link)] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

/// <summary>
/// Read the Packet CRC Error Flag for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>True if the Packet CRC Error Flag is set</returns>
bool DTCLib::DTC_Registers::ReadPacketCRCError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(link)];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketError()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketError);
	form.description = "Receive Packet Error";
	form.vals.push_back("       ([CRC, PacketError])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadPacketCRCError(r) ? "x" : " ") + "," +
							(ReadPacketError(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadPacketCRCError(DTC_Link_CFO) ? "x" : " ") + "," +
						(ReadPacketError(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadPacketCRCError(DTC_Link_EVB) ? "x" : " ") + "," +
						(ReadPacketError(DTC_Link_EVB) ? "x" : " ") + "]");
	return form;
}

// CFO Emulation Timestamp Registers
/// <summary>
/// Set the starting DTC_EventWindowTag for the CFO Emulator
/// </summary>
/// <param name="ts">Starting Timestamp for CFO Emulation</param>
void DTCLib::DTC_Registers::SetCFOEmulationTimestamp(const DTC_EventWindowTag& ts)
{
	auto timestamp = ts.GetEventWindowTag();
	auto timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	auto timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister_(timestampLow, DTC_Register_CFOEmulation_TimestampLow);
	WriteRegister_(timestampHigh, DTC_Register_CFOEmulation_TimestampHigh);
}

/// <summary>
/// Read the starting DTC_EventWindowTag for the CFO Emulator
/// </summary>
/// <returns>DTC_EventWindowTag object</returns>
DTCLib::DTC_EventWindowTag DTCLib::DTC_Registers::ReadCFOEmulationTimestamp()
{
	auto timestampLow = ReadRegister_(DTC_Register_CFOEmulation_TimestampLow);
	DTC_EventWindowTag output;
	output.SetEventWindowTag(timestampLow, static_cast<uint16_t>(ReadRegister_(DTC_Register_CFOEmulation_TimestampHigh)));
	return output;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationTimestampLow()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_TimestampLow);
	form.description = "CFO Emulation Timestamp Low";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_CFOEmulation_TimestampLow);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationTimestampHigh()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_TimestampHigh);
	form.description = "CFO Emulation Timestamp High";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_CFOEmulation_TimestampHigh);
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Request Interval Register
/// <summary>
/// Set the clock interval between CFO Emulator Readout Requests.
/// This value is dependent upon the SERDES clock speed (2.5 Gbps = 125 MHz, 3.125 Gbps = 156.25 MHz)
/// </summary>
/// <param name="interval">Clock cycles between Readout Requests</param>
void DTCLib::DTC_Registers::SetCFOEmulationHeartbeatInterval(uint32_t interval)
{
	WriteRegister_(interval, DTC_Register_CFOEmulation_HeartbeatInterval);
}

/// <summary>
/// Read the clock interval between CFO Emulator Readout Requests.
/// This value is dependent upon the SERDES clock speed (2.5 Gbps = 125 MHz, 3.125 Gbps = 156.25 MHz)
/// </summary>
/// <returns>Clock cycles between Readout Requests</returns>
uint32_t DTCLib::DTC_Registers::ReadCFOEmulationHeartbeatInterval()
{
	return ReadRegister_(DTC_Register_CFOEmulation_HeartbeatInterval);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationHeartbeatInterval()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_HeartbeatInterval);
	form.description = "CFO Emu. Request Interval";
	std::stringstream o;
	o << "0x" << std::hex << ReadCFOEmulationHeartbeatInterval();
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Number of Requests Register
/// <summary>
/// Set the number of Readout Requests the CFO Emulator is configured to send.
/// A value of 0 means that the CFO Emulator will send requests continuously.
/// </summary>
/// <param name="NumHeartbeats">Number of Readout Requests the CFO Emulator will send</param>
void DTCLib::DTC_Registers::SetCFOEmulationNumHeartbeats(uint32_t NumHeartbeats)
{
	WriteRegister_(NumHeartbeats, DTC_Register_CFOEmulation_NumHeartbeats);
}

/// <summary>
/// Reads the number of Readout Requests the CFO Emulator is configured to send.
/// A value of 0 means that the CFO Emulator will send requests continuously.
/// </summary>
/// <returns>Number of Readout Requests the CFO Emulator will send</returns>
uint32_t DTCLib::DTC_Registers::ReadCFOEmulationNumHeartbeats()
{
	return ReadRegister_(DTC_Register_CFOEmulation_NumHeartbeats);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumHeartbeats()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_NumHeartbeats);
	form.description = "CFO Emulator Number Requests";
	std::stringstream o;
	o << "0x" << std::hex << ReadCFOEmulationNumHeartbeats();
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Number of Packets Registers
/// <summary>
/// Set the number of packets the CFO Emulator will request from the link
/// </summary>
/// <param name="link">Link to set</param>
/// <param name="numPackets">Number of packets to request</param>
void DTCLib::DTC_Registers::SetCFOEmulationNumPackets(DTC_Link_ID const& link, uint16_t numPackets)
{
	uint16_t data = numPackets & 0x7FF;
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
		case DTC_Link_1:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks10;
			break;
		case DTC_Link_2:
		case DTC_Link_3:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks32;
			break;
		case DTC_Link_4:
		case DTC_Link_5:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks54;
			break;
		default:
			return;
	}

	auto regval = ReadRegister_(reg);
	auto upper = (regval & 0xFFFF0000) >> 16;
	auto lower = regval & 0x0000FFFF;
	if (link == DTC_Link_0 || link == DTC_Link_2 || link == DTC_Link_4)
	{
		lower = data;
	}
	else
	{
		upper = data;
	}
	WriteRegister_((upper << 16) + lower, reg);
}

/// <summary>
/// Read the requested number of packets the CFO Emulator will request from the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Number of packets requested from the link</returns>
uint16_t DTCLib::DTC_Registers::ReadCFOEmulationNumPackets(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
		case DTC_Link_1:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks10;
			break;
		case DTC_Link_2:
		case DTC_Link_3:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks32;
			break;
		case DTC_Link_4:
		case DTC_Link_5:
			reg = DTC_Register_CFOEmulation_NumPacketsLinks54;
			break;
		default:
			return 0;
	}

	auto regval = ReadRegister_(reg);
	auto upper = (regval & 0xFFFF0000) >> 16;
	auto lower = regval & 0x0000FFFF;
	if (link == DTC_Link_0 || link == DTC_Link_2 || link == DTC_Link_4)
	{
		return static_cast<uint16_t>(lower);
	}
	return static_cast<uint16_t>(upper);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsLink01()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_NumPacketsLinks10);
	form.description = "CFO Emulator Num Packets R0,1";
	std::stringstream o;
	o << "Link 0: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_0);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Link 1: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsLink23()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_NumPacketsLinks32);
	form.description = "CFO Emulator Num Packets R2,3";
	std::stringstream o;
	o << "Link 2: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_2);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Link 3: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsLink45()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_NumPacketsLinks54);
	form.description = "CFO Emulator Num Packets R4,5";
	std::stringstream o;
	o << "Link 4: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_4);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Link 5: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Number of Null Heartbeats Register
/// <summary>
/// Set the number of null heartbeats the CFO Emulator will generate following the requested ones
/// </summary>
/// <param name="count">Number of null heartbeats to generate</param>
void DTCLib::DTC_Registers::SetCFOEmulationNumNullHeartbeats(const uint32_t& count)
{
	WriteRegister_(count, DTC_Register_CFOEmulation_NumNullHeartbeats);
}

/// <summary>
/// Read the requested number of null heartbeats that will follow the configured heartbeats from the CFO Emulator
/// </summary>
/// <returns>Number of null heartbeats to follow "live" heartbeats</returns>
uint32_t DTCLib::DTC_Registers::ReadCFOEmulationNumNullHeartbeats()
{
	return ReadRegister_(DTC_Register_CFOEmulation_NumNullHeartbeats);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumNullHeartbeats()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_NumNullHeartbeats);
	form.description = "CFO Emulator Num Null Heartbeats";
	form.vals.push_back(std::to_string(ReadCFOEmulationNumNullHeartbeats()));
	return form;
}

// CFO Emulation Event Mode Bytes Registers
/// <summary>
/// Set the given CFO Emulation Mode byte to the given value
/// </summary>
/// <param name="byteNum">Byte to set. Valid range is 0-5</param>
/// <param name="data">Data for byte</param>
void DTCLib::DTC_Registers::SetCFOEmulationModeByte(const uint8_t& byteNum, uint8_t data)
{
	DTC_Register reg;
	if (byteNum == 0 || byteNum == 1 || byteNum == 2 || byteNum == 3)
	{
		reg = DTC_Register_CFOEmulation_EventMode1;
	}
	else if (byteNum == 4 || byteNum == 5)
	{
		reg = DTC_Register_CFOEmulation_EventMode2;
	}
	else
	{
		return;
	}
	auto regVal = ReadRegister_(reg);

	switch (byteNum)
	{
		case 0:
			regVal = (regVal & 0xFFFFFF00) + data;
			break;
		case 1:
			regVal = (regVal & 0xFFFF00FF) + (data << 8);
			break;
		case 2:
			regVal = (regVal & 0xFF00FFFF) + (data << 16);
			break;
		case 3:
			regVal = (regVal & 0x00FFFFFF) + (data << 24);
			break;
		case 4:
			regVal = (regVal & 0xFF00) + data;
			break;
		case 5:
			regVal = (regVal & 0x00FF) + (data << 8);
			break;
		default:
			return;
	}

	WriteRegister_(regVal, reg);
}

/// <summary>
/// Read the given CFO Emulation Mode byte
/// </summary>
/// <param name="byteNum">Byte to read. Valid range is 0-5</param>
/// <returns>Current value of the mode byte</returns>
uint8_t DTCLib::DTC_Registers::ReadCFOEmulationModeByte(const uint8_t& byteNum)
{
	DTC_Register reg;
	if (byteNum == 0 || byteNum == 1 || byteNum == 2 || byteNum == 3)
	{
		reg = DTC_Register_CFOEmulation_EventMode1;
	}
	else if (byteNum == 4 || byteNum == 5)
	{
		reg = DTC_Register_CFOEmulation_EventMode2;
	}
	else
	{
		return 0;
	}
	auto regVal = ReadRegister_(reg);

	switch (byteNum)
	{
		case 0:
			return static_cast<uint8_t>(regVal & 0xFF);
		case 1:
			return static_cast<uint8_t>((regVal & 0xFF00) >> 8);
		case 2:
			return static_cast<uint8_t>((regVal & 0xFF0000) >> 16);
		case 3:
			return static_cast<uint8_t>((regVal & 0xFF000000) >> 24);
		case 4:
			return static_cast<uint8_t>(regVal & 0xFF);
		case 5:
			return static_cast<uint8_t>((regVal & 0xFF00) >> 8);
		default:
			return 0;
	}
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationModeBytes03()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_EventMode1);
	form.description = "CFO Emulation Event Mode Bytes 0-3";
	std::ostringstream o;
	o << "Byte 0: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(0));
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Byte 1: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(1));
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Byte 2: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(2));
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Byte 3: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(3));
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationModeBytes45()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_EventMode2);
	form.description = "CFO Emulation Event Mode Bytes 4-5";
	std::ostringstream o;
	o << "Byte 4: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(4));
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Byte 5: 0x" << std::hex << static_cast<int>(ReadCFOEmulationModeByte(5));
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	return form;
}

// CFO Emulation Debug Packet Type Register

/// <summary>
/// Enable putting the Debug Mode in Readout Requests
/// </summary>
void DTCLib::DTC_Registers::EnableDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DebugPacketType);
	data[16] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DebugPacketType);
}

/// <summary>
/// Disable putting the Debug Mode in Readout Requests
/// </summary>
void DTCLib::DTC_Registers::DisableDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DebugPacketType);
	data[16] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DebugPacketType);
}

/// <summary>
/// Whether Debug mode packets are enabled
/// </summary>
/// <returns>True if Debug Mode is enabled</returns>
bool DTCLib::DTC_Registers::ReadDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DebugPacketType);
	return data[16];
}

/// <summary>
/// Set the DebugType used by the CFO Emulator
/// </summary>
/// <param name="type">The DTC_DebugType the CFO Emulator will fill into Readout Requests</param>
void DTCLib::DTC_Registers::SetCFOEmulationDebugType(DTC_DebugType type)
{
	std::bitset<32> data = type & 0xF;
	data[16] = ReadDebugPacketMode();
	WriteRegister_(data.to_ulong(), DTC_Register_DebugPacketType);
}

/// <summary>
/// Read the DebugType field filled into Readout Requests generated by the CFO Emulator
/// </summary>
/// <returns>The DTC_DebugType used by the CFO Emulator</returns>
DTCLib::DTC_DebugType DTCLib::DTC_Registers::ReadCFOEmulationDebugType()
{
	return static_cast<DTC_DebugType>(0xFFFF & ReadRegister_(DTC_Register_DebugPacketType));
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationDebugPacketType()
{
	auto form = CreateFormatter(DTC_Register_DebugPacketType);
	form.description = "CFO Emulation Debug Packet Type";
	form.vals.push_back(std::string("Debug Mode: [") + (ReadDebugPacketMode() ? "x" : " ") + "]");
	std::stringstream o;
	o << "Debug Packet Type: 0x" << std::hex << ReadCFOEmulationDebugType();
	form.vals.push_back(o.str());
	return form;
}

// RX Packet Count Error Flags Register
/// <summary>
/// Read the RX Packet Count Error flag for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Whether the RX Packet Count Error flag is set on the link</returns>
bool DTCLib::DTC_Registers::ReadRXPacketCountErrorFlags(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_RXPacketCountErrorFlags);
	return dataSet[link];
}

/// <summary>
/// Clear the RX Packet Count Error flag for the given link
/// </summary>
/// <param name="link">Link to clear</param>
void DTCLib::DTC_Registers::ClearRXPacketCountErrorFlags(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet;
	dataSet[link] = true;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_RXPacketCountErrorFlags);
}

/// <summary>
/// Clear all RX Packet Count Error Flags
/// </summary>
void DTCLib::DTC_Registers::ClearRXPacketCountErrorFlags()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_RXPacketCountErrorFlags);
	WriteRegister_(dataSet.to_ulong(), DTC_Register_RXPacketCountErrorFlags);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXPacketCountErrorFlags()
{
	auto form = CreateFormatter(DTC_Register_RXPacketCountErrorFlags);
	form.description = "RX Packet Count Error Flags";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" +
							(ReadRXPacketCountErrorFlags(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadRXPacketCountErrorFlags(DTC_Link_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadRXPacketCountErrorFlags(DTC_Link_EVB) ? "x" : " ") + "]");
	return form;
}

// Detector Emulator DMA Count Register
/// <summary>
/// Set the number of DMAs that the Detector Emulator will generate when enabled
/// </summary>
/// <param name="count">The number of DMAs that the Detector Emulator will generate when enabled</param>
void DTCLib::DTC_Registers::SetDetectorEmulationDMACount(uint32_t count)
{
	WriteRegister_(count, DTC_Register_DetEmulation_DMACount);
}

/// <summary>
/// Read the number of DMAs that the Detector Emulator will generate
/// </summary>
/// <returns>The number of DMAs that the Detector Emulator will generate</returns>
uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMACount()
{
	return ReadRegister_(DTC_Register_DetEmulation_DMACount);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationDMACount()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_DMACount);
	form.description = "DetEmu DMA Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadDetectorEmulationDMACount();
	form.vals.push_back(o.str());
	return form;
}

// Detector Emulator DMA Delay Counter Register
/// <summary>
/// Set the delay between DMAs in Detector Emulator mode
/// </summary>
/// <param name="count">Delay between DMAs in Detector Emulation mode, in 4ns ticks</param>
void DTCLib::DTC_Registers::SetDetectorEmulationDMADelayCount(uint32_t count)
{
	WriteRegister_(count, DTC_Register_DetEmulation_DelayCount);
}

/// <summary>
/// Read the amount of the delay between DMAs in Detector Emulator mode
/// </summary>
/// <returns>The amount of the delay between DMAs in Detector Emulator mode, in 4ns ticks</returns>
uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMADelayCount()
{
	return ReadRegister_(DTC_Register_DetEmulation_DelayCount);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationDMADelayCount()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_DelayCount);
	form.description = "DetEmu DMA Delay Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadDetectorEmulationDMADelayCount();
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Enable Detector Emulator Mode. This sends all DMA writes to DMA channel 0 (DAQ) to DDR memory
/// </summary>
void DTCLib::DTC_Registers::EnableDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control0);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulation_Control0);
}

/// <summary>
/// Disable sending DMA data to DDR memory
/// </summary>
void DTCLib::DTC_Registers::DisableDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control0);
	data[0] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulation_Control0);
}

/// <summary>
/// Read whether writes to DMA Channel 0 will be loaded into DDR memory
/// </summary>
/// <returns>Whether the Detector Emulator Mode bit is set</returns>
bool DTCLib::DTC_Registers::ReadDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control0);
	return data[0];
}

/// <summary>
/// Enable the Detector Emulator (Playback Mode)
/// This assumes that data has been loaded into DDR memory using DMA Channel 0 before enabling.
/// </summary>
void DTCLib::DTC_Registers::EnableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control0);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulation_Control0);
}

/// <summary>
/// Turn off the Detector Emulator (Playback Mode)
/// </summary>
void DTCLib::DTC_Registers::DisableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control1);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulation_Control1);
}

/// <summary>
/// Read whether the Detector Emulator is enabled
/// </summary>
/// <returns>Whether the Detector Emulator is enabled</returns>
bool DTCLib::DTC_Registers::ReadDetectorEmulatorEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control0);
	return data[1];
}

/// <summary>
/// Read whether a Detector Emulator Disable operation is in progress
/// </summary>
/// <returns>Whether the Detector Emulator Enable Clear bit is set</returns>
bool DTCLib::DTC_Registers::ReadDetectorEmulatorEnableClear()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulation_Control1);
	return data[1];
}

/// <summary>
/// Clear the "Detector Emulator In Use" virtual register
/// </summary>
void DTCLib::DTC_Registers::ClearDetectorEmulatorInUse()
{
	DisableDetectorEmulator();
	DisableDetectorEmulatorMode();
	ResetDDRWriteAddress();
	ResetDDRReadAddress();
	SetDDRDataLocalStartAddress(0);
	SetDDRDataLocalEndAddress(0x7000000);
	usingDetectorEmulator_ = false;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationControl0()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_Control0);
	form.description = "Detector Emulation Control 0";
	form.vals.push_back(std::string("Detector Emulation Enable: [") + (ReadDetectorEmulatorEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Detector Emulation Mode:   [") + (ReadDetectorEmulatorMode() ? "x" : " ") + "]");
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationControl1()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_Control1);
	form.description = "Detector Emulation Control 1";
	form.vals.push_back(std::string("Detector Emulation Enable Clear: [") +
						(ReadDetectorEmulatorEnableClear() ? "x" : " ") + "]");
	return form;
}

// DDR Event Data Local Start Address Register
/// <summary>
/// Set the DDR Data Start Address
/// DDR Addresses are in bytes and must be 64-bit aligned
/// </summary>
/// <param name="address">Start address for the DDR data section</param>
void DTCLib::DTC_Registers::SetDDRDataLocalStartAddress(uint32_t address)
{
	WriteRegister_(address, DTC_Register_DetEmulation_DataStartAddress);
}

/// <summary>
/// Read the DDR Data Start Address
/// </summary>
/// <returns>The current DDR data start address</returns>
uint32_t DTCLib::DTC_Registers::ReadDDRDataLocalStartAddress()
{
	return ReadRegister_(DTC_Register_DetEmulation_DataStartAddress);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRDataLocalStartAddress()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_DataStartAddress);
	form.description = "DDR Event Data Local Start Address";
	std::stringstream o;
	o << "0x" << std::hex << ReadDDRDataLocalStartAddress();
	form.vals.push_back(o.str());
	return form;
}

// DDR Event Data Local End Address Register
/// <summary>
/// Set the end address for the DDR data section
/// DDR Addresses are in bytes and must be 64-bit aligned
/// </summary>
/// <param name="address"></param>
void DTCLib::DTC_Registers::SetDDRDataLocalEndAddress(uint32_t address)
{
	WriteRegister_(address, DTC_Register_DetEmulation_DataEndAddress);
}

/// <summary>
/// Read the current end address for the DDR Data section
/// </summary>
/// <returns>End address for the DDR data section</returns>
uint32_t DTCLib::DTC_Registers::ReadDDRDataLocalEndAddress()
{
	return ReadRegister_(DTC_Register_DetEmulation_DataEndAddress);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRDataLocalEndAddress()
{
	auto form = CreateFormatter(DTC_Register_DetEmulation_DataEndAddress);
	form.description = "DDR Event Data Local End Address";
	std::stringstream o;
	o << "0x" << std::hex << ReadDDRDataLocalEndAddress();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulatorInterpacketDelay()
{
	return ReadRegister_(DTC_Register_CFOEmulation_DataRequestDelay);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulatorInterpacketDelay()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_DataRequestDelay);
	form.description = "CFO Emulator Data Request Interpacket Delay";
	std::stringstream o;
	o << std::dec << ReadCFOEmulatorInterpacketDelay() << " * 5 ns";
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Read the current maximum Ethernet payload size
/// </summary>
/// <returns>The current maximum Ethernet payload size</returns>
uint32_t DTCLib::DTC_Registers::ReadEthernetPayloadSize()
{
	return ReadRegister_(DTC_Register_EthernetFramePayloadSize);
}

/// <summary>
/// Set the maximum Ethernet payload size, in bytes. Maximum is 1492 bytes.
/// </summary>
/// <param name="size">Maximum Ethernet payload size, in bytes</param>
void DTCLib::DTC_Registers::SetEthernetPayloadSize(uint32_t size)
{
	if (size > 1492)
	{
		size = 1492;
	}
	WriteRegister_(size, DTC_Register_EthernetFramePayloadSize);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEthernetPayloadSize()
{
	auto form = CreateFormatter(DTC_Register_EthernetFramePayloadSize);
	form.description = "Ethernet Frame Payload Max Size";
	std::stringstream o;
	o << std::dec << ReadEthernetPayloadSize() << " bytes";
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulation40MHzMarkerInterval()
{
	return ReadRegister_(DTC_Register_CFOEmulation_40MHzClockMarkerInterval);
}

void DTCLib::DTC_Registers::SetCFOEmulation40MHzMarkerInterval(uint32_t interval)
{
	WriteRegister_(interval, DTC_Register_CFOEmulation_40MHzClockMarkerInterval);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulation40MHzMarkerInterval()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulation_40MHzClockMarkerInterval);
	form.description = "CFO Emulation 40MHz Marker Interval";
	std::stringstream o;
	o << "0x" << std::hex << ReadCFOEmulation40MHzMarkerInterval();
	form.vals.push_back(o.str());
	return form;
}

bool DTCLib::DTC_Registers::ReadCFOEmulationEventStartMarkerEnable(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_CFOMarkerEnables);
	return dataSet[link + 8];
}

void DTCLib::DTC_Registers::SetCFOEmulationEventStartMarkerEnable(DTC_Link_ID const& link, bool enable)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_CFOMarkerEnables);
	data[link + 8] = enable;
	WriteRegister_(data.to_ulong(), DTC_Register_CFOMarkerEnables);
}

bool DTCLib::DTC_Registers::ReadCFOEmulation40MHzClockMarkerEnable(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_CFOMarkerEnables);
	return dataSet[link];
}

void DTCLib::DTC_Registers::SetCFOEmulation40MHzClockMarkerEnable(DTC_Link_ID const& link, bool enable)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_CFOMarkerEnables);
	data[link] = enable;
	WriteRegister_(data.to_ulong(), DTC_Register_CFOMarkerEnables);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationMarkerEnables()
{
	auto form = CreateFormatter(DTC_Register_SERDESLoopbackEnable);
	form.description = "CFO Emulation Marker Enables";
	form.vals.push_back("        [Event Start, 40 MHz]");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadCFOEmulationEventStartMarkerEnable(r) ? "x" : " ") + "," + (ReadCFOEmulation40MHzClockMarkerEnable(r) ? "x" : " ") + "]");
	}
	return form;
}

uint8_t DTCLib::DTC_Registers::ReadROCCommaLimit()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_ROCFinishThreshold));
}

void DTCLib::DTC_Registers::SetROCCommaLimit(uint8_t limit)
{
	uint32_t data = ReadRegister_(DTC_Register_ROCFinishThreshold) & 0xFFFFFF00;
	data += limit;
	WriteRegister_(data, DTC_Register_ROCFinishThreshold);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCFinishThreshold()
{
	auto form = CreateFormatter(DTC_Register_ROCFinishThreshold);
	form.description = "ROC Finish Threshold";
	std::stringstream o;
	o << "ROC Comma Limit: 0x" << std::hex << ReadROCCommaLimit();
	form.vals.push_back(o.str());
	return form;
}

// SERDES Counter Registers
/// <summary>
/// Clear the value of the Receive byte counter
/// </summary>
/// <param name="link">Link to clear counter for</param>
void DTCLib::DTC_Registers::ClearReceiveByteCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ReceiveByteCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ReceiveByteCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ReceiveByteCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ReceiveByteCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ReceiveByteCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ReceiveByteCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_ReceiveByteCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}

/// <summary>
/// Read the value of the Receive byte counter
/// </summary>
/// <param name="link">Link to read counter for</param>
/// <returns>Current value of the Receive byte counter on the given link</returns>
uint32_t DTCLib::DTC_Registers::ReadReceiveByteCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_ReceiveByteCount_Link5);
		case DTC_Link_CFO:
			return ReadRegister_(DTC_Register_ReceiveByteCount_CFOLink);
		default:
			return 0;
	}
}

/// <summary>
/// Clear the value of the Receive Packet counter
/// </summary>
/// <param name="link">Link to clear counter for</param>
void DTCLib::DTC_Registers::ClearReceivePacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ReceivePacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ReceivePacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ReceivePacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ReceivePacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ReceivePacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ReceivePacketCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_ReceivePacketCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}

/// <summary>
/// Read the value of the Receive Packet counter
/// </summary>
/// <param name="link">Link to read counter for</param>
/// <returns>Current value of the Receive Packet counter on the given link</returns>
uint32_t DTCLib::DTC_Registers::ReadReceivePacketCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_ReceivePacketCount_Link5);
		case DTC_Link_CFO:
			return ReadRegister_(DTC_Register_ReceivePacketCount_CFOLink);
		default:
			return 0;
	}
}

/// <summary>
/// Clear the value of the Transmit byte counter
/// </summary>
/// <param name="link">Link to clear counter for</param>
void DTCLib::DTC_Registers::ClearTransmitByteCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TransmitByteCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TransmitByteCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TransmitByteCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TransmitByteCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TransmitByteCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TransmitByteCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_TransmitByteCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}

/// <summary>
/// Read the value of the Transmit byye counter
/// </summary>
/// <param name="link">Link to read counter for</param>
/// <returns>Current value of the Transmit byte counter on the given link</returns>
uint32_t DTCLib::DTC_Registers::ReadTransmitByteCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_TransmitByteCount_Link5);
		case DTC_Link_CFO:
			return ReadRegister_(DTC_Register_TransmitByteCount_CFOLink);
		default:
			return 0;
	}
}

/// <summary>
/// Clear the value of the Transmit Packet counter
/// </summary>
/// <param name="link">Link to clear counter for</param>
void DTCLib::DTC_Registers::ClearTransmitPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TransmitPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TransmitPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TransmitPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TransmitPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TransmitPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TransmitPacketCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_TransmitPacketCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}

/// <summary>
/// Read the value of the Transmit Packet counter
/// </summary>
/// <param name="link">Link to read counter for</param>
/// <returns>Current value of the Transmit Packet counter on the given link</returns>
uint32_t DTCLib::DTC_Registers::ReadTransmitPacketCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_TransmitPacketCount_Link5);
		case DTC_Link_CFO:
			return ReadRegister_(DTC_Register_TransmitPacketCount_CFOLink);
		default:
			return 0;
	}
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink0()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link0);
	form.description = "Receive Byte Count: Link 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink1()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link1);
	form.description = "Receive Byte Count: Link 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink2()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link2);
	form.description = "Receive Byte Count: Link 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink3()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link3);
	form.description = "Receive Byte Count: Link 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink4()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link4);
	form.description = "Receive Byte Count: Link 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountLink5()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_Link5);
	form.description = "Receive Byte Count: Link 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountCFO()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCount_CFOLink);
	form.description = "Receive Byte Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link0);
	form.description = "Receive Packet Count: Link 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link1);
	form.description = "Receive Packet Count: Link 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link2);
	form.description = "Receive Packet Count: Link 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link3);
	form.description = "Receive Packet Count: Link 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link4);
	form.description = "Receive Packet Count: Link 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_Link5);
	form.description = "Receive Packet Count: Link 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountCFO()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCount_CFOLink);
	form.description = "Receive Packet Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink0()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link0);
	form.description = "Transmit Byte Count: Link 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink1()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link1);
	form.description = "Transmit Byte Count: Link 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink2()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link2);
	form.description = "Transmit Byte Count: Link 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink3()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link3);
	form.description = "Transmit Byte Count: Link 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink4()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link4);
	form.description = "Transmit Byte Count: Link 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountLink5()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_Link5);
	form.description = "Transmit Byte Count: Link 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountCFO()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCount_CFOLink);
	form.description = "Transmit Byte Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link0);
	form.description = "Transmit Packet Count: Link 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link1);
	form.description = "Transmit Packet Count: Link 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link2);
	form.description = "Transmit Packet Count: Link 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link3);
	form.description = "Transmit Packet Count: Link 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link4);
	form.description = "Transmit Packet Count: Link 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_Link5);
	form.description = "Transmit Packet Count: Link 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountCFO()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCount_CFOLink);
	form.description = "Transmit Packet Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

// Firefly TX IIC Registers
/// <summary>
/// Read the Reset bit of the Firefly TX IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadFireflyTXIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_FireflyTX_IICBusControl));
	return dataSet[31];
}
/// <summary>
/// Reset the Firefly TX IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetFireflyTXIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_FireflyTX_IICBusControl);
	while (ReadFireflyTXIICInterfaceReset())
	{
		usleep(1000);
	}
}
/// <summary>
/// Write a value to the Firefly TX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteFireflyTXIICInterface(uint8_t device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_FireflyTX_IICBusConfigLow);
	WriteRegister_(0x1, DTC_Register_FireflyTX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyTX_IICBusConfigHigh) == 0x1)
	{
		usleep(1000);
	}
}
/// <summary>
/// Read a value from the Firefly TX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadFireflyTXIICInterface(uint8_t device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_FireflyTX_IICBusConfigLow);
	WriteRegister_(0x2, DTC_Register_FireflyTX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyTX_IICBusConfigHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_FireflyTX_IICBusConfigLow);
	return static_cast<uint8_t>(data);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXIICControl()
{
	auto form = CreateFormatter(DTC_Register_FireflyTX_IICBusControl);
	form.description = "TX Firefly IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadFireflyTXIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXIICParameterLow()
{
	auto form = CreateFormatter(DTC_Register_FireflyTX_IICBusConfigLow);
	form.description = "TX Firefly IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_FireflyTX_IICBusConfigLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "Address:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXIICParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_FireflyTX_IICBusConfigHigh);
	form.description = "TX Firefly IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_FireflyTX_IICBusConfigHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_FireflyTX_IICBusConfigHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

// Firefly RX IIC Registers
/// <summary>
/// Read the Reset bit of the Firefly RX IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadFireflyRXIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_FireflyRX_IICBusControl));
	return dataSet[31];
}
/// <summary>
/// Reset the Firefly RX IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetFireflyRXIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_FireflyRX_IICBusControl);
	while (ReadFireflyRXIICInterfaceReset())
	{
		usleep(1000);
	}
}
/// <summary>
/// Write a value to the Firefly RX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteFireflyRXIICInterface(uint8_t device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_FireflyRX_IICBusConfigLow);
	WriteRegister_(0x1, DTC_Register_FireflyRX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyRX_IICBusConfigHigh) == 0x1)
	{
		usleep(1000);
	}
}
/// <summary>
/// Read a value from the Firefly RX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadFireflyRXIICInterface(uint8_t device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_FireflyRX_IICBusConfigLow);
	WriteRegister_(0x2, DTC_Register_FireflyRX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyRX_IICBusConfigHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_FireflyRX_IICBusConfigLow);
	return static_cast<uint8_t>(data);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyRXIICControl()
{
	auto form = CreateFormatter(DTC_Register_FireflyRX_IICBusControl);
	form.description = "RX Firefly IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadFireflyRXIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyRXIICParameterLow()
{
	auto form = CreateFormatter(DTC_Register_FireflyRX_IICBusConfigLow);
	form.description = "RX Firefly IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_FireflyRX_IICBusConfigLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "Address:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyRXIICParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_FireflyRX_IICBusConfigHigh);
	form.description = "RX Firefly IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_FireflyRX_IICBusConfigHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_FireflyRX_IICBusConfigHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

// Firefly TXRX IIC Registers
/// <summary>
/// Read the Reset bit of the Firefly TXRX IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadFireflyTXRXIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_FireflyTXRX_IICBusControl));
	return dataSet[31];
}
/// <summary>
/// Reset the Firefly TXRX IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetFireflyTXRXIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_FireflyTXRX_IICBusControl);
	while (ReadFireflyTXRXIICInterfaceReset())
	{
		usleep(1000);
	}
}
/// <summary>
/// Write a value to the Firefly TXRX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteFireflyTXRXIICInterface(uint8_t device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_FireflyTXRX_IICBusConfigLow);
	WriteRegister_(0x1, DTC_Register_FireflyTXRX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigHigh) == 0x1)
	{
		usleep(1000);
	}
}
/// <summary>
/// Read a value from the Firefly TXRX IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadFireflyTXRXIICInterface(uint8_t device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_FireflyTXRX_IICBusConfigLow);
	WriteRegister_(0x2, DTC_Register_FireflyTXRX_IICBusConfigHigh);
	while (ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigLow);
	return static_cast<uint8_t>(data);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXRXIICControl()
{
	auto form = CreateFormatter(DTC_Register_FireflyTXRX_IICBusControl);
	form.description = "TXRX Firefly IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadFireflyTXRXIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXRXIICParameterLow()
{
	auto form = CreateFormatter(DTC_Register_FireflyTXRX_IICBusConfigLow);
	form.description = "TXRX Firefly IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "Address:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyTXRXIICParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_FireflyTXRX_IICBusConfigHigh);
	form.description = "TXRX Firefly IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_FireflyTXRX_IICBusConfigHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

bool DTCLib::DTC_Registers::ReadTXPRBSForceError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_TXPRBSControl));
	return dataSet[link + 24];
}

void DTCLib::DTC_Registers::SetTXPRBSForceError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_TXPRBSControl));
	dataSet[link + 24] = 1;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_TXPRBSControl);
}

void DTCLib::DTC_Registers::ClearTXPRBSForceError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_TXPRBSControl));
	dataSet[link + 24] = 0;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_TXPRBSControl);
}

DTCLib::DTC_PRBSMode DTCLib::DTC_Registers::ReadTXPRBSMode(DTC_Link_ID const& link)
{
	auto data = ReadRegister_(DTC_Register_TXPRBSControl);
	auto masked = (data >> link) & 0x7;
	return static_cast<DTC_PRBSMode>(masked);
}

void DTCLib::DTC_Registers::SetTXPRBSMode(DTC_Link_ID const& link, DTC_PRBSMode mode)
{
	auto data = ReadRegister_(DTC_Register_TXPRBSControl);
	auto masked = data & ~(0x7 << link);
	auto newMode = static_cast<uint32_t>(mode);
	WriteRegister_(masked + (newMode << link), DTC_Register_TXPRBSControl);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESTXPRBSControl()
{
	auto form = CreateFormatter(DTC_Register_TXPRBSControl);
	form.description = "SERDES TX PRBS Control";
	form.vals.push_back("       ([Force Error, Mode])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadTXPRBSForceError(r) ? "x" : " ") + "," +
							(DTC_PRBSModeConverter(ReadTXPRBSMode(r)).toString()) + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadTXPRBSForceError(DTC_Link_CFO) ? "x" : " ") + "," +
						(DTC_PRBSModeConverter(ReadTXPRBSMode(DTC_Link_CFO)).toString()) + "]");
	return form;
}

bool DTCLib::DTC_Registers::ReadRXPRBSError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_RXPRBSControl));
	return dataSet[link + 24];
}

DTCLib::DTC_PRBSMode DTCLib::DTC_Registers::ReadRXPRBSMode(DTC_Link_ID const& link)
{
	auto data = ReadRegister_(DTC_Register_RXPRBSControl);
	auto masked = (data >> link) & 0x7;
	return static_cast<DTC_PRBSMode>(masked);
}

void DTCLib::DTC_Registers::SetRXPRBSMode(DTC_Link_ID const& link, DTC_PRBSMode mode)
{
	auto data = ReadRegister_(DTC_Register_RXPRBSControl);
	auto masked = data & ~(0x7 << link);
	auto newMode = static_cast<uint32_t>(mode);
	WriteRegister_(masked + (newMode << link), DTC_Register_RXPRBSControl);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSControl()
{
	auto form = CreateFormatter(DTC_Register_RXPRBSControl);
	form.description = "SERDES RX PRBS Control";
	form.vals.push_back("       ([Error, Mode])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadRXPRBSError(r) ? "x" : " ") + "," +
							(DTC_PRBSModeConverter(ReadRXPRBSMode(r)).toString()) + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadRXPRBSError(DTC_Link_CFO) ? "x" : " ") + "," +
						(DTC_PRBSModeConverter(ReadRXPRBSMode(DTC_Link_CFO)).toString()) + "]");
	return form;
}

bool DTCLib::DTC_Registers::ReadEventModeTableEnable()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_EventModeLookupTableControl));
	return dataSet[16];
}

void DTCLib::DTC_Registers::SetEventModeTableEnable()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_EventModeLookupTableControl));
	dataSet[16] = 1;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_EventModeLookupTableControl);
}

void DTCLib::DTC_Registers::ClearEventModeTableEnable()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_EventModeLookupTableControl));
	dataSet[16] = 0;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_EventModeLookupTableControl);
}

uint8_t DTCLib::DTC_Registers::ReadEventModeLookupByteSelect()
{
	auto data = ReadRegister_(DTC_Register_EventModeLookupTableControl);
	auto masked = (data)&0x7;
	return static_cast<uint8_t>(masked);
}

void DTCLib::DTC_Registers::SetEventModeLookupByteSelect(uint8_t byte)
{
	auto data = ReadRegister_(DTC_Register_EventModeLookupTableControl);
	auto masked = data & 0xFFF8;
	WriteRegister_(masked + (byte & 0x7), DTC_Register_EventModeLookupTableControl);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEventModeLookupTableControl()
{
	auto form = CreateFormatter(DTC_Register_EventModeLookupTableControl);
	form.description = "Event Mode Lookup Table Control";
	form.vals.push_back(std::string("Enabled:  [") +
						(ReadEventModeTableEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Byte:          [") +
						std::to_string(ReadEventModeLookupByteSelect()) + "]");
	return form;
}

bool DTCLib::DTC_Registers::ReadDDRMemoryTestComplete()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_DD3TestRegister));
	return dataSet[8];
}

bool DTCLib::DTC_Registers::ReadDDRMemoryTestError()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_DD3TestRegister));
	return dataSet[0];
}

void DTCLib::DTC_Registers::ClearDDRMemoryTestError()
{
	WriteRegister_(1, DTC_Register_DD3TestRegister);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRMemoryTestRegister()
{
	auto form = CreateFormatter(DTC_Register_DD3TestRegister);
	form.description = "DDR3 Memory Test";
	form.vals.push_back(std::string("Test Complete:  [") +
						(ReadDDRMemoryTestComplete() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Error:          [") +
						(ReadDDRMemoryTestError() ? "x" : " ") + "]");
	return form;
}

// SERDES Serial Inversion Enable Register
/// <summary>
/// Read the Invert SERDES RX Input bit
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Whether the Invert SERDES RX Input bit is set</returns>
bool DTCLib::DTC_Registers::ReadInvertSERDESRXInput(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESTXRXInvertEnable);
	return data[link + 8];
}
/// <summary>
/// Set the Invert SERDES RX Input bit
/// </summary>
/// <param name="link">Link to set</param>
/// <param name="invert">Whether to invert</param>
void DTCLib::DTC_Registers::SetInvertSERDESRXInput(DTC_Link_ID const& link, bool invert)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESTXRXInvertEnable);
	data[link + 8] = invert;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESTXRXInvertEnable);
}
/// <summary>
/// Read the Invert SERDES TX Output bit
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Whether the Invert SERDES TX Output bit is set</returns>
bool DTCLib::DTC_Registers::ReadInvertSERDESTXOutput(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESTXRXInvertEnable);
	return data[link];
}
/// <summary>
/// Set the Invert SERDES TX Output bit
/// </summary>
/// <param name="link">Link to set</param>
/// <param name="invert">Whether to invert</param>
void DTCLib::DTC_Registers::SetInvertSERDESTXOutput(DTC_Link_ID const& link, bool invert)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESTXRXInvertEnable);
	data[link] = invert;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESTXRXInvertEnable);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESSerialInversionEnable()
{
	auto form = CreateFormatter(DTC_Register_SERDESTXRXInvertEnable);
	form.description = "SERDES Serial Inversion Enable";
	form.vals.push_back("       ([Input, Output])");
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadInvertSERDESRXInput(r) ? "x" : " ") +
							"," + (ReadInvertSERDESTXOutput(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadInvertSERDESRXInput(DTC_Link_CFO) ? "x" : " ") + "," +
						(ReadInvertSERDESTXOutput(DTC_Link_CFO) ? "x" : " ") + "]");

	return form;
}

// Jitter Attenuator CSR Register
/// <summary>
/// Read the value of the Jitter Attenuator Select
/// </summary>
/// <returns>Jitter Attenuator Select value</returns>
std::bitset<2> DTCLib::DTC_Registers::ReadJitterAttenuatorSelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_JitterAttenuatorCSR);
	std::bitset<2> output;
	output[0] = data[4];
	output[1] = data[5];
	return output;
}
/// <summary>
/// Set the Jitter Attenuator Select bits
/// </summary>
/// <param name="data">Value to set</param>
void DTCLib::DTC_Registers::SetJitterAttenuatorSelect(std::bitset<2> data)
{
	std::bitset<32> regdata = ReadRegister_(DTC_Register_JitterAttenuatorCSR);
	regdata[4] = data[0];
	regdata[5] = data[1];
	WriteRegister_(regdata.to_ulong(), DTC_Register_JitterAttenuatorCSR);
}

/// <summary>
/// Read the Jitter Attenuator Reset bit
/// </summary>
/// <returns>Value of the Jitter Attenuator Reset bit</returns>
bool DTCLib::DTC_Registers::ReadJitterAttenuatorReset()
{
	std::bitset<32> regdata = ReadRegister_(DTC_Register_JitterAttenuatorCSR);
	return regdata[0];
}
/// <summary>
/// Reset the Jitter Attenuator
/// </summary>
void DTCLib::DTC_Registers::ResetJitterAttenuator()
{
	std::bitset<32> regdata = ReadRegister_(DTC_Register_JitterAttenuatorCSR);
	regdata[0] = 1;
	WriteRegister_(regdata.to_ulong(), DTC_Register_JitterAttenuatorCSR);
	usleep(1000);
	regdata[0] = 0;
	WriteRegister_(regdata.to_ulong(), DTC_Register_JitterAttenuatorCSR);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatJitterAttenuatorCSR()
{
	auto form = CreateFormatter(DTC_Register_JitterAttenuatorCSR);
	form.description = "Jitter Attenuator CSR";
	form.vals.push_back(std::string("Select Low: [") + (ReadJitterAttenuatorSelect()[0] ? "x" : " ") + "]");
	form.vals.push_back(std::string("Select High: [") + (ReadJitterAttenuatorSelect()[1] ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset:   [") + (ReadJitterAttenuatorReset() ? "x" : " ") + "]");
	return form;
}

// SFP IIC Registers

/// <summary>
/// Read the Reset bit of the SFP IIC Bus
/// </summary>
/// <returns>Reset bit value</returns>
bool DTCLib::DTC_Registers::ReadSFPIICInterfaceReset()
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_SFP_IICBusControl));
	return dataSet[31];
}
/// <summary>
/// Reset the SFP IIC Bus
/// </summary>
void DTCLib::DTC_Registers::ResetSFPIICInterface()
{
	auto bs = std::bitset<32>();
	bs[31] = 1;
	WriteRegister_(bs.to_ulong(), DTC_Register_SFP_IICBusControl);
	while (ReadSFPIICInterfaceReset())
	{
		usleep(1000);
	}
}
/// <summary>
/// Write a value to the SFP IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <param name="data">Data to write</param>
void DTCLib::DTC_Registers::WriteSFPIICInterface(uint8_t device, uint8_t address, uint8_t data)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16) + (data << 8);
	WriteRegister_(reg_data, DTC_Register_SFP_IICBusLow);
	WriteRegister_(0x1, DTC_Register_SFP_IICBusHigh);
	while (ReadRegister_(DTC_Register_SFP_IICBusHigh) == 0x1)
	{
		usleep(1000);
	}
}
/// <summary>
/// Read a value from the SFP IIC Bus
/// </summary>
/// <param name="device">Device address</param>
/// <param name="address">Register address</param>
/// <returns>Value of register</returns>
uint8_t DTCLib::DTC_Registers::ReadSFPIICInterface(uint8_t device, uint8_t address)
{
	uint32_t reg_data = (static_cast<uint8_t>(device) << 24) + (address << 16);
	WriteRegister_(reg_data, DTC_Register_SFP_IICBusLow);
	WriteRegister_(0x2, DTC_Register_SFP_IICBusHigh);
	while (ReadRegister_(DTC_Register_SFP_IICBusHigh) == 0x2)
	{
		usleep(1000);
	}
	auto data = ReadRegister_(DTC_Register_SFP_IICBusLow);
	return static_cast<uint8_t>(data);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSFPIICControl()
{
	auto form = CreateFormatter(DTC_Register_SFP_IICBusControl);
	form.description = "SFP Oscillator IIC Bus Control";
	form.vals.push_back(std::string("Reset:  [") + (ReadSFPIICInterfaceReset() ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSFPIICParameterLow()
{
	auto form = CreateFormatter(DTC_Register_SFP_IICBusLow);
	form.description = "SFP Oscillator IIC Bus Low";
	auto data = ReadRegister_(DTC_Register_SFP_IICBusLow);
	std::ostringstream s1, s2, s3, s4;
	s1 << "Device:     " << std::showbase << std::hex << ((data & 0xFF000000) >> 24);
	form.vals.push_back(s1.str());
	s2 << "ASFPess:    " << std::showbase << std::hex << ((data & 0xFF0000) >> 16);
	form.vals.push_back(s2.str());
	s3 << "Write Data: " << std::showbase << std::hex << ((data & 0xFF00) >> 8);
	form.vals.push_back(s3.str());
	s4 << "Read Data:  " << std::showbase << std::hex << (data & 0xFF);
	form.vals.push_back(s4.str());
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSFPIICParameterHigh()
{
	auto form = CreateFormatter(DTC_Register_SFP_IICBusHigh);
	form.description = "SFP Oscillator IIC Bus High";
	form.vals.push_back(std::string("Write:  [") +
						(ReadRegister_(DTC_Register_SFP_IICBusHigh) & 0x1 ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read:   [") +
						(ReadRegister_(DTC_Register_SFP_IICBusHigh) & 0x2 ? "x" : " ") + "]");
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadRetransmitRequestCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_RetransmitRequestCount_Link5);
		default:
			return 0;
	}
}
void DTCLib::DTC_Registers::ClearRetransmitRequestCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link0);
			break;
		case DTC_Link_1:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link1);
			break;
		case DTC_Link_2:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link2);
			break;
		case DTC_Link_3:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link3);
			break;
		case DTC_Link_4:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link4);
			break;
		case DTC_Link_5:
			WriteRegister_(1, DTC_Register_RetransmitRequestCount_Link5);
			break;
		default:
			break;
	}
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink0()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link0);
	form.description = "ROC Retransmit Request Count Link 0";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink1()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link1);
	form.description = "ROC Retransmit Request Count Link 1";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink2()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link2);
	form.description = "ROC Retransmit Request Count Link 2";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink3()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link3);
	form.description = "ROC Retransmit Request Count Link 3";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink4()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link4);
	form.description = "ROC Retransmit Request Count Link 4";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRetransmitRequestCountLink5()
{
	auto form = CreateFormatter(DTC_Register_RetransmitRequestCount_Link5);
	form.description = "ROC Retransmit Request Count Link 5";
	std::stringstream o;
	o << std::dec << ReadRetransmitRequestCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// Missed CFO Packet Count Registers
/**
 * @brief Read the missed CFO Packet Count for the given link
 * @param link Link to read
 * @returns Number of missed CFO packets for the given link, or 0 if an invalid link is given
 */
uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_MissedCFOPacketCount_Link5);
		default:
			return 0;
	}
}

/// <summary>
/// Clears the Missed CFO Packet Count for the given link
/// </summary>
/// <param name="link">Link to clear</param>
void DTCLib::DTC_Registers::ClearMissedCFOPacketCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link0);
			break;
		case DTC_Link_1:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link1);
			break;
		case DTC_Link_2:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link2);
			break;
		case DTC_Link_3:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link3);
			break;
		case DTC_Link_4:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link4);
			break;
		case DTC_Link_5:
			WriteRegister_(1, DTC_Register_MissedCFOPacketCount_Link5);
			break;
		default:
			break;
	}
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link0);
	form.description = "Missed CFO Packet Count Link 0";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link1);
	form.description = "Missed CFO Packet Count Link 1";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link2);
	form.description = "Missed CFO Packet Count Link 2";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link3);
	form.description = "Missed CFO Packet Count Link 3";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link4);
	form.description = "Missed CFO Packet Count Link 4";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCount_Link5);
	form.description = "Missed CFO Packet Count Link 5";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// Local Fragment Drop Count Register
/// <summary>
/// Reads the current value of the Local Fragment Drop Counter
/// </summary>
/// <returns>The number of fragments dropped by the DTC</returns>
uint32_t DTCLib::DTC_Registers::ReadLocalFragmentDropCount()
{
	return ReadRegister_(DTC_Register_LocalFragmentDropCount);
}

/// <summary>
/// Clears the Local Fragment Drop Counter
/// </summary>
void DTCLib::DTC_Registers::ClearLocalFragmentDropCount() { WriteRegister_(1, DTC_Register_LocalFragmentDropCount); }

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatLocalFragmentDropCount()
{
	auto form = CreateFormatter(DTC_Register_LocalFragmentDropCount);
	form.description = "Local Data Fragment Drop Count";
	std::stringstream o;
	o << std::dec << ReadLocalFragmentDropCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadEVBSubEventReceiveTimer()
{
	return ReadRegister_(DTC_Register_EVBSubEventReceiveTimerPreset);
}

void DTCLib::DTC_Registers::SetEVBSubEventReceiveTimer(uint32_t timer)
{
	WriteRegister_(timer, DTC_Register_EVBSubEventReceiveTimerPreset);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBSubEventReceiveTimer()
{
	auto form = CreateFormatter(DTC_Register_EVBSubEventReceiveTimerPreset);
	form.description = "EVB SubEvent Receive Timer (*4ns)";
	std::stringstream o;
	o << std::dec << ReadEVBSubEventReceiveTimer();
	form.vals.push_back(o.str());
	return form;
}

// EVB SERDES PRBS Register
/// <summary>
/// Determine if an error was detected in the EVB SERDES PRBS module
/// </summary>
/// <returns>True if error condition was detected, false otherwise</returns>
bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSErrorFlag()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[31];
}

/// <summary>
/// Read the EVB SERDES TX PRBS Select
/// </summary>
/// <returns>Value of the EVB SERDES TX PRBS Select</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBSERDESTXPRBSSEL()
{
	uint8_t regVal = static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0x7000) >> 12);
	return regVal;
}

/// <summary>
/// Set the EVB SERDES TX PRBS Select
/// </summary>
/// <param name="byte">Value of the EVB SERDES TX PRBS Select</param>
void DTCLib::DTC_Registers::SetEVBSERDESTXPRBSSEL(uint8_t byte)
{
	uint8_t regVal = (ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0xFFFF8FFF);
	regVal += ((byte & 0x7) << 12);
	WriteRegister_(regVal, DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Read the EVB SERDES RX PRBS Select
/// </summary>
/// <returns>Value of the EVB SERDES RX PRBS Select</returns>
uint8_t DTCLib::DTC_Registers::ReadEVBSERDESRXPRBSSEL()
{
	uint8_t regVal = static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0x700) >> 8);
	return regVal;
}

/// <summary>
/// Set the EVB SERDES RX PRBS Select
/// </summary>
/// <param name="byte">Value of the EVB SERDES RX PRBS Select</param>
void DTCLib::DTC_Registers::SetEVBSERDESRXPRBSSEL(uint8_t byte)
{
	uint8_t regVal = (ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0xFFFFF8FF);
	regVal += ((byte & 0x7) << 8);
	WriteRegister_(regVal, DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Read the state of the EVB SERDES PRBS Force Error bit
/// </summary>
/// <returns>True if the EVB SERDES PRBS Force Error bit is high</returns>
bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSForceError()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[1];
}

/// <summary>
/// Set the EVB SERDES PRBS Force Error bit
/// </summary>
/// <param name="flag">New value for the EVB SERDES PRBS Reset bit</param>
void DTCLib::DTC_Registers::SetEVBSERDESPRBSForceError(bool flag)
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal[1] = flag;
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Toggle the EVB SERDES PRBS Force Error bit (make it true if false, false if true)
/// </summary>
void DTCLib::DTC_Registers::ToggleEVBSERDESPRBSForceError()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal.flip(0);
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Read the state of the EVB SERDES PRBS Reset bit
/// </summary>
/// <returns>True if the EVB SERDES PRBS Reset bit is high</returns>
bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSReset()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[0];
}

/// <summary>
/// Set the EVB SERDES PRBS Reset bit
/// </summary>
/// <param name="flag">New value for the EVB SERDES PRBS Reset bit</param>
void DTCLib::DTC_Registers::SetEVBSERDESPRBSReset(bool flag)
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal[0] = flag;
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Toggle the EVB SERDES PRBS Reset bit (make it true if false, false if true)
/// </summary>
void DTCLib::DTC_Registers::ToggleEVBSERDESPRBSReset()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal.flip(0);
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBSERDESPRBSControl()
{
	auto form = CreateFormatter(DTC_Register_EVBSERDESPRBSControlStatus);
	form.description = "EVB SERDES PRBS Control / Status";
	form.vals.push_back(std::string("PRBS Error: [") + (ReadEVBSERDESPRBSErrorFlag() ? "x" : " ") + "]");
	std::stringstream o;
	o << "EVB SERDES TX PRBS Select: 0x" << std::hex << static_cast<int>(ReadEVBSERDESTXPRBSSEL());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB SERDES RX PRBS Select: 0x" << std::hex << static_cast<int>(ReadEVBSERDESRXPRBSSEL());
	form.vals.push_back(std::string("PRBS Force Error: [") + (ReadEVBSERDESPRBSForceError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("PRBS Reset: [") + (ReadEVBSERDESPRBSReset() ? "x" : " ") + "]");

	return form;
}

// Event Builder Error Register
/// <summary>
/// Read the Event Builder SubEvent Receiver Flags Buffer Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_SubEventReceiverFlagsBufferError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[24];
}
/// <summary>
/// Read the Event Builder Ethernet Input FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_EthernetInputFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[16];
}
/// <summary>
/// Read the Event Builder Link Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_LinkError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[9];
}
/// <summary>
/// Read the Event Builder TX Packet Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_TXPacketError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[8];
}
/// <summary>
/// Read the Event Builder Local Data Pointer FIFO Queue Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_LocalDataPointerFIFOQueueError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[1];
}
/// <summary>
/// Read the Event Builder Transmit DMA Byte Count FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadEventBuilder_TransmitDMAByteCountFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_EventBuilderErrorFlags);
	return data[0];
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEventBuilderErrorRegister()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderErrorFlags);
	form.description = "Event Builder Error Flags";
	form.vals.push_back(std::string("Sub-Event Received Flags Buffer Error: [") +
						(ReadEventBuilder_SubEventReceiverFlagsBufferError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Input FIFO Full:                       [") +
						(ReadEventBuilder_EthernetInputFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Link Error:                            [") +
						(ReadEventBuilder_LinkError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TX Packet Error:                       [") +
						(ReadEventBuilder_TXPacketError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Local Data Pointer FIFO Queue Error:   [") +
						(ReadEventBuilder_LocalDataPointerFIFOQueueError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Transmit DMA Byte Count FIFO Full:     [") +
						(ReadEventBuilder_TransmitDMAByteCountFIFOFull() ? "x" : " ") + "]");
	return form;
}

// SERDES VFIFO Error Register
/// <summary>
/// Read the SERDES VFIFO Egress FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_EgressFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[10];
}
/// <summary>
/// Read the SERDES VFIFO Ingress FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_IngressFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[9];
}
/// <summary>
/// Read the SERDES VFIFO Event Byte Count Total Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_EventByteCountTotalError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[8];
}
/// <summary>
/// Read the SERDES VFIFO Last Word Written Timeout Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_LastWordWrittenTimeoutError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[2];
}
/// <summary>
/// Read the SERDES VFIFO Fragment Count Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_FragmentCountError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[1];
}
/// <summary>
/// Read the SERDES VFIFO DDR Full Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSERDESVFIFO_DDRFullError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_InputBufferErrorFlags);
	return data[0];
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESVFIFOError()
{
	auto form = CreateFormatter(DTC_Register_InputBufferErrorFlags);
	form.description = "SERDES VFIFO Error Flags";
	form.vals.push_back(std::string("Egress FIFO Full:             [") + (ReadSERDESVFIFO_EgressFIFOFull() ? "x" : " ") +
						"]");
	form.vals.push_back(std::string("Ingress FIFO Full:            [") + (ReadSERDESVFIFO_IngressFIFOFull() ? "x" : " ") +
						"]");
	form.vals.push_back(std::string("Event Byte Count Total Error: [") +
						(ReadSERDESVFIFO_EventByteCountTotalError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Last Word Written Timeout:    [") +
						(ReadSERDESVFIFO_LastWordWrittenTimeoutError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Fragment count Error:         [") +
						(ReadSERDESVFIFO_FragmentCountError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DDR Full Error:               [") + (ReadSERDESVFIFO_DDRFullError() ? "x" : " ") +
						"]");
	return form;
}

// PCI VFIFO Error Register
/// <summary>
/// Read the PCI VIFO DDR Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_DDRFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[12];
}
/// <summary>
/// Read the PCI VIFO Memmory Mapped Write Complete FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_MemoryMappedWriteCompleteFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[11];
}
/// <summary>
/// Read the PCI VIFO PCI Write Event FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_PCIWriteEventFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[10];
}
/// <summary>
/// Read the PCI VIFO Local Data Pointer FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_LocalDataPointerFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[9];
}
/// <summary>
/// Read the PCI VIFO Egress FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_EgressFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[8];
}
/// <summary>
/// Read the PCI VIFO RX Buffer Select FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_RXBufferSelectFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[2];
}
/// <summary>
/// Read the PCI VIFO Ingress FIFO Full bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_IngressFIFOFull()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[1];
}
/// <summary>
/// Read the PCI VIFO Event Byte Count Total Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadPCIVFIFO_EventByteCountTotalError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_OutputBufferErrorFlags);
	return data[0];
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPCIVFIFOError()
{
	auto form = CreateFormatter(DTC_Register_OutputBufferErrorFlags);
	form.description = "PCI VFIFO Error Flags";
	form.vals.push_back(std::string("DDR Full Error:               [") + (ReadPCIVFIFO_DDRFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Memmap Write Cmplt FIFO Full: [") +
						(ReadPCIVFIFO_MemoryMappedWriteCompleteFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("PCI Write Event FIFO Full:    [") +
						(ReadPCIVFIFO_PCIWriteEventFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Local Data Pointer FIFO Full: [") +
						(ReadPCIVFIFO_LocalDataPointerFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Egress FIFO Full:             [") + (ReadPCIVFIFO_EgressFIFOFull() ? "x" : " ") +
						"]");
	form.vals.push_back(std::string("RX Buffer Select FIFO Full:   [") +
						(ReadPCIVFIFO_RXBufferSelectFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Ingress FIFO Ful:             [") + (ReadPCIVFIFO_IngressFIFOFull() ? "x" : " ") +
						"]");
	form.vals.push_back(std::string("Event Byte Count Total Error: [") +
						(ReadPCIVFIFO_EventByteCountTotalError() ? "x" : " ") + "]");
	return form;
}

// ROC Link Error Registers
/// <summary>
/// Read the Receive Data Request Sync Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_ROCDataRequestSyncError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[5];
}
/// <summary>
/// Read the Receive RX Packet Count Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_RXPacketCountError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[4];
}
/// <summary>
/// Read the Receive RX Packet Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_RXPacketError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[3];
}
/// <summary>
/// Read the Receive RX Packet CRC Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_RXPacketCRCError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[2];
}
/// <summary>
/// Read the Receive Data Pending Timeout Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_DataPendingTimeoutError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[1];
}
/// <summary>
/// Read the Receive Data Packet Count Error bit for the given link
/// </summary>
/// <param name="link">Link to read</param>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadROCLink_ReceiveDataPacketCountError(DTC_Link_ID const& link)
{
	std::bitset<32> data;
	switch (link)
	{
		case DTC_Link_0:
			data = ReadRegister_(DTC_Register_Link0ErrorFlags);
			break;
		case DTC_Link_1:
			data = ReadRegister_(DTC_Register_Link1ErrorFlags);
			break;
		case DTC_Link_2:
			data = ReadRegister_(DTC_Register_Link2ErrorFlags);
			break;
		case DTC_Link_3:
			data = ReadRegister_(DTC_Register_Link3ErrorFlags);
			break;
		case DTC_Link_4:
			data = ReadRegister_(DTC_Register_Link4ErrorFlags);
			break;
		case DTC_Link_5:
			data = ReadRegister_(DTC_Register_Link5ErrorFlags);
			break;
		default:
			return false;
	}
	return data[0];
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink0Error()
{
	auto form = CreateFormatter(DTC_Register_Link0ErrorFlags);
	form.description = "ROC Link 0 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_0) ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink1Error()
{
	auto form = CreateFormatter(DTC_Register_Link1ErrorFlags);
	form.description = "ROC Link 1 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_1) ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink2Error()
{
	auto form = CreateFormatter(DTC_Register_Link2ErrorFlags);
	form.description = "ROC Link 2 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_2) ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink3Error()
{
	auto form = CreateFormatter(DTC_Register_Link3ErrorFlags);
	form.description = "ROC Link 3 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_3) ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink4Error()
{
	auto form = CreateFormatter(DTC_Register_Link4ErrorFlags);
	form.description = "ROC Link 4 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_4) ? "x" : " ") + "]");
	return form;
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRocLink5Error()
{
	auto form = CreateFormatter(DTC_Register_Link5ErrorFlags);
	form.description = "ROC Link 5 Error";
	form.vals.push_back(std::string("ROC Data Request Sync Error:     [") +
						(ReadROCLink_ROCDataRequestSyncError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Count Error:           [") +
						(ReadROCLink_RXPacketCountError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet Error:                 [") +
						(ReadROCLink_RXPacketError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX Packet CRC Error:             [") +
						(ReadROCLink_RXPacketCRCError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Pending Timeout Error:      [") +
						(ReadROCLink_DataPendingTimeoutError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Receive Data Packet Count Error: [") +
						(ReadROCLink_ReceiveDataPacketCountError(DTC_Link_5) ? "x" : " ") + "]");
	return form;
}

// CFO Link Error Register
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOLinkError()
{
	auto form = CreateFormatter(DTC_Register_CFOLinkErrorFlags);
	form.description = "CFO Link Error Flags";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_CFOLinkErrorFlags);
	form.vals.push_back(o.str());
	return form;
}

// Link Mux Error Register
/// <summary>
/// Read the DCS Mux Decode Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadDCSMuxDecodeError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_LinkMuxErrorFlags);
	return data[1];
}
/// <summary>
/// Read the Data Mux Decode Error bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadDataMuxDecodeError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_LinkMuxErrorFlags);
	return data[0];
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatLinkMuxError()
{
	auto form = CreateFormatter(DTC_Register_LinkMuxErrorFlags);
	form.description = "Link Mux Error Flags";
	form.vals.push_back(std::string("DCS Mux Decode Error:  [") + (ReadDCSMuxDecodeError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Data Mux Decode Error: [") + (ReadDataMuxDecodeError() ? "x" : " ") + "]");
	return form;
}

// Firefly CSR Register
/// <summary>
/// Read the TXRX Firefly Present bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXRXFireflyPresent()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[26];
}
/// <summary>
/// Read the RX Firefly Present bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadRXFireflyPresent()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[25];
}
/// <summary>
/// Read the TX Firefly Present bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXFireflyPresent()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[24];
}
/// <summary>
/// Read the TXRX Firefly Interrupt bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXRXFireflyInterrupt()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[18];
}
/// <summary>
/// Read the RX Firefly Interrupt bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadRXFireflyInterrupt()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[17];
}
/// <summary>
/// Read the TX Firefly Interrupt bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXFireflyInterrupt()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[16];
}
/// <summary>
/// Read the TXRX Firefly Select bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXRXFireflySelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[10];
}
/// <summary>
/// Set the TXRX Firefly Select bit
/// </summary>
/// <param name="select">Value to write</param>
void DTCLib::DTC_Registers::SetTXRXFireflySelect(bool select)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[10] = select;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Read the TX Firefly Select bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadTXFireflySelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[9];
}
/// <summary>
/// Set the TX Firefly Select bit
/// </summary>
/// <param name="select">Value to write</param>
void DTCLib::DTC_Registers::SetTXFireflySelect(bool select)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[9] = select;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Read the RX Firefly Select bit
/// </summary>
/// <returns>Value of bit</returns>
bool DTCLib::DTC_Registers::ReadRXFireflySelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[8];
}
/// <summary>
/// Set the RX Firefly Select bit
/// </summary>
/// <param name="select">Value to write</param>
void DTCLib::DTC_Registers::SetRXFireflySelect(bool select)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[8] = select;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Read the TXRX Firefly Reset bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetTXRXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[2];
}
/// Reset the TXRX Firefly
void DTCLib::DTC_Registers::ResetTXRXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[2] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
	usleep(1000);
	data[2] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Read the TX Firefly Reset bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetTXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[1];
}
/// Reset the TX Firefly
void DTCLib::DTC_Registers::ResetTXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
	usleep(1000);
	data[1] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Read the RX Firefly Reset bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadResetRXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	return data[0];
}
/// Reset the RX Firefly
void DTCLib::DTC_Registers::ResetRXFirefly()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_FireFlyControlStatus);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
	usleep(1000);
	data[0] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_FireFlyControlStatus);
}
/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFireflyCSR()
{
	auto form = CreateFormatter(DTC_Register_FireFlyControlStatus);
	form.description = "FireFly Control and Status";
	form.vals.push_back(std::string("TXRX FireFly Present:   [") + (ReadTXRXFireflyPresent() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TX FireFly Present:     [") + (ReadRXFireflyPresent() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX FireFly Present:     [") + (ReadTXFireflyPresent() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TXRX FireFly Interrupt: [") + (ReadTXRXFireflyInterrupt() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TX FireFly Interrupt:   [") + (ReadRXFireflyInterrupt() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX FireFly Interrupt:   [") + (ReadTXFireflyInterrupt() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TXRX FireFly Select:    [") + (ReadTXRXFireflySelect() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TX FireFly Select:      [") + (ReadTXFireflySelect() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX FireFly Select:      [") + (ReadRXFireflySelect() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TXRX FireFly Reset:     [") + (ReadResetTXRXFirefly() ? "x" : " ") + "]");
	form.vals.push_back(std::string("TX FireFly Reset:       [") + (ReadResetTXFirefly() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX FireFly Reset:       [") + (ReadResetRXFirefly() ? "x" : " ") + "]");
	return form;
}

// SFP Control/Status Register
/// <summary>
/// Read the SFP Present bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSFPPresent()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	return data[31];
}

/// <summary>
/// Read the SFP LOS bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSFPLOS()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	return data[17];
}

/// <summary>
/// Read the SFP TX Fault bit
/// </summary>
/// <returns>Value of the bit</returns>
bool DTCLib::DTC_Registers::ReadSFPTXFault()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	return data[16];
}

/// <summary>
/// Set the SFP Rate Select bit high
/// </summary>
void DTCLib::DTC_Registers::EnableSFPRateSelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SFPControlStatus);
}

/// <summary>
/// Set the SFP Rate Select bit low
/// </summary>
void DTCLib::DTC_Registers::DisableSFPRateSelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	data[1] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_SFPControlStatus);
}

/// <summary>
/// Read the value of the SFP Rate Select bit
/// </summary>
/// <returns>The value of the SFP Rate Select bit</returns>
bool DTCLib::DTC_Registers::ReadSFPRateSelect()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	return data[1];
}

/// <summary>
/// Disable SFP TX
/// </summary>
void DTCLib::DTC_Registers::DisableSFPTX()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SFPControlStatus);
}

/// <summary>
/// Enable SFP TX
/// </summary>
void DTCLib::DTC_Registers::EnableSFPTX()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	data[0] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_SFPControlStatus);
}

/// <summary>
/// Read the SFP TX Disable bit
/// </summary>
/// <returns>Value of the SFP TX Disable bit</returns>
bool DTCLib::DTC_Registers::ReadSFPTXDisable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SFPControlStatus);
	return data[0];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSFPControlStatus()
{
	auto form = CreateFormatter(DTC_Register_SFPControlStatus);
	form.description = "SFP Control Status";
	form.vals.push_back(std::string("SFP Present:     [") + (ReadSFPPresent() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SFP LOS:         [") + (ReadSFPLOS() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SFP TX Fault:    [") + (ReadSFPTXFault() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SFP Rate Select: [") + (ReadSFPRateSelect() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SFP TX Disable:  [") + (ReadSFPTXDisable() ? "x" : " ") + "]");
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadRXCDRUnlockCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_Link5);
		case DTC_Link_CFO:
			return ReadRegister_(DTC_Register_RXCDRUnlockCount_CFOLink);
		default:
			return 0;
	}
}

void DTCLib::DTC_Registers::ClearRXCDRUnlockCount(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link0);
			break;
		case DTC_Link_1:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link1);
			break;
		case DTC_Link_2:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link2);
			break;
		case DTC_Link_3:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link3);
			break;
		case DTC_Link_4:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link4);
			break;
		case DTC_Link_5:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_Link5);
			break;
		case DTC_Link_CFO:
			WriteRegister_(0, DTC_Register_RXCDRUnlockCount_CFOLink);
			break;
		default:
			break;
	}
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink0()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link0);
	form.description = "RX CDR Unlock Count Link 0";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink1()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link1);
	form.description = "RX CDR Unlock Count Link 1";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink2()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link2);
	form.description = "RX CDR Unlock Count Link 2";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink3()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link3);
	form.description = "RX CDR Unlock Count Link 3";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink4()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link4);
	form.description = "RX CDR Unlock Count Link 4";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountLink5()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_Link5);
	form.description = "RX CDR Unlock Count Link 5";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCDRUnlockCountCFOLink()
{
	auto form = CreateFormatter(DTC_Register_RXCDRUnlockCount_CFOLink);
	form.description = "RX CDR Unlock Count CFO Link";
	std::stringstream o;
	o << std::dec << ReadRXCDRUnlockCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadJitterAttenuatorUnlockCuont()
{
	return ReadRegister_(DTC_Register_JitterAttenuatorLossOfLockCount);
}

void DTCLib::DTC_Registers::ClearJitterAttenuatorUnlockCount()
{
	WriteRegister_(1, DTC_Register_JitterAttenuatorLossOfLockCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatJitterAttenuatorUnlockCount()
{
	auto form = CreateFormatter(DTC_Register_JitterAttenuatorLossOfLockCount);
	form.description = "RX Jitter Attenuator Unlock Count";
	std::stringstream o;
	o << std::dec << ReadJitterAttenuatorUnlockCuont();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadRXCFOLinkEventStartCharacterErrorCount()
{
	return ReadRegister_(DTC_Register_CFOLinkEventStartErrorCount);
}

void DTCLib::DTC_Registers::ClearRXCFOLinkEventStartCharacterErrorCount()
{
	WriteRegister_(0, DTC_Register_CFOLinkEventStartErrorCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCFOLinkEventStartCharacterErrorCount()
{
	auto form = CreateFormatter(DTC_Register_CFOLinkEventStartErrorCount);
	form.description = "CFO Link Event Start Error Count";
	std::stringstream o;
	o << std::dec << ReadRXCFOLinkEventStartCharacterErrorCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadRXCFOLink40MHzCharacterErrorCount()
{
	return ReadRegister_(DTC_Register_CFOLink40MHzErrorCount);
}

void DTCLib::DTC_Registers::ClearRXCFOLink40MHzCharacterErrorCount()
{
	WriteRegister_(0, DTC_Register_CFOLink40MHzErrorCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXCFOLink40MHzCharacterErrorCount()
{
	auto form = CreateFormatter(DTC_Register_CFOLink40MHzErrorCount);
	form.description = "CFO Link 40 MHz Error Count";
	std::stringstream o;
	o << std::dec << ReadRXCFOLink40MHzCharacterErrorCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadInputBufferFragmentDumpCount()
{
	return ReadRegister_(DTC_Register_InputBufferDropCount);
}

void DTCLib::DTC_Registers::ClearInputBufferFragmentDumpCount()
{
	WriteRegister_(0, DTC_Register_InputBufferDropCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatInputBufferFragmentDumpCount()
{
	auto form = CreateFormatter(DTC_Register_InputBufferDropCount);
	form.description = "Input Buffer Fragment Drop Count";
	std::stringstream o;
	o << std::dec << ReadInputBufferFragmentDumpCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadOutputBufferFragmentDumpCount()
{
	return ReadRegister_(DTC_Register_OutputBufferDropCount);
}

void DTCLib::DTC_Registers::ClearOutputBufferFragmentDumpCount()
{
	WriteRegister_(0, DTC_Register_OutputBufferDropCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatOutputBufferFragmentDumpCount()
{
	auto form = CreateFormatter(DTC_Register_OutputBufferDropCount);
	form.description = "Output Buffer Fragment Drop Count";
	std::stringstream o;
	o << std::dec << ReadOutputBufferFragmentDumpCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadROCDCSResponseTimer()
{
	return ReadRegister_(DTC_Register_ROCDCSTimerPreset);
}

void DTCLib::DTC_Registers::SetROCDCSResponseTimer(uint32_t timer)
{
	WriteRegister_(timer, DTC_Register_ROCDCSTimerPreset);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCDCSResponseTimerPreset()
{
	auto form = CreateFormatter(DTC_Register_ROCDCSTimerPreset);
	form.description = "ROC DCS Response Timer Preset (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCDCSResponseTimer();
	form.vals.push_back(o.str());
	return form;
}

// FPGA PROM Program Status Register
/// <summary>
/// Read the full bit on the FPGA PROM FIFO
/// </summary>
/// <returns>the full bit on the FPGA PROM FIFO</returns>
bool DTCLib::DTC_Registers::ReadFPGAPROMProgramFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[1];
}

/// <summary>
/// Read whether the FPGA PROM is ready for data
/// </summary>
/// <returns>whether the FPGA PROM is ready for data</returns>
bool DTCLib::DTC_Registers::ReadFPGAPROMReady()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[0];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAPROMProgramStatus()
{
	auto form = CreateFormatter(DTC_Register_FPGAPROMProgramStatus);
	form.description = "FPGA PROM Program Status";
	form.vals.push_back(std::string("FPGA PROM Program FIFO Full: [") + (ReadFPGAPROMProgramFIFOFull() ? "x" : " ") +
						"]");
	form.vals.push_back(std::string("FPGA PROM Ready:             [") + (ReadFPGAPROMReady() ? "x" : " ") + "]");
	return form;
}

// FPGA Core Access Register
/// <summary>
/// Performs the chants necessary to reload the DTC firmware
/// </summary>
void DTCLib::DTC_Registers::ReloadFPGAFirmware()
{
	WriteRegister_(0xFFFFFFFF, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0xAA995566, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x20000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x30020001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x00000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x30008001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x0000000F, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister_(0x20000000, DTC_Register_FPGACoreAccess);
}

/// <summary>
/// Read the FPGA Core Access FIFO Full bit
/// </summary>
/// <returns>Whether the FPGA Core Access FIFO is full</returns>
bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGACoreAccess);
	return dataSet[1];
}

/// <summary>
/// Read the FPGA Core Access FIFO Empty bit
/// </summary>
/// <returns>Whether the FPGA Core Access FIFO is empty</returns>
bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOEmpty()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGACoreAccess);
	return dataSet[0];
}

/// <summary>
/// Formats the register's current value for register dumps
/// </summary>
/// <returns>DTC_RegisterFormatter object containing register information</returns>
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGACoreAccess()
{
	auto form = CreateFormatter(DTC_Register_FPGACoreAccess);
	form.description = "FPGA Core Access";
	form.vals.push_back(std::string("FPGA Core Access FIFO Full:  [") + (ReadFPGACoreAccessFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("FPGA Core Access FIFO Empty: [") + (ReadFPGACoreAccessFIFOEmpty() ? "x" : " ") +
						"]");

	return form;
}

bool DTCLib::DTC_Registers::ReadRXOKErrorSlowOpticalLink3()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[9];
}

bool DTCLib::DTC_Registers::ReadRXOKErrorSlowOpticalLink2()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[8];
}

bool DTCLib::DTC_Registers::ReadRXOKErrorSlowOpticalLink1()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[7];
}

bool DTCLib::DTC_Registers::ReadRXOKErrorSlowOpticalLink0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[6];
}

bool DTCLib::DTC_Registers::ReadLatchedSpareSMAInputOKError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[5];
}

void DTCLib::DTC_Registers::ClearLatchedSpareSMAInputOKError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[5] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

bool DTCLib::DTC_Registers::ReadLatchedEventMarkerSMAInputOKError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[4];
}

void DTCLib::DTC_Registers::ClearLatchedEventMarkerSMASMAInputOKError()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[4] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

bool DTCLib::DTC_Registers::ReadLatchedRXOKErrorSlowOpticalLink3()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[3];
}

void DTCLib::DTC_Registers::ClearLatchedRXOKErrorSlowOpticalLink3()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[3] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

bool DTCLib::DTC_Registers::ReadLatchedRXOKErrorSlowOpticalLink2()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[2];
}

void DTCLib::DTC_Registers::ClearLatchedRXOKErrorSlowOpticalLink2()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[2] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

bool DTCLib::DTC_Registers::ReadLatchedRXOKErrorSlowOpticalLink1()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[1];
}

void DTCLib::DTC_Registers::ClearLatchedRXOKErrorSlowOpticalLink1()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

bool DTCLib::DTC_Registers::ReadLatchedRXOKErrorSlowOpticalLink0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	return data[0];
}

void DTCLib::DTC_Registers::ClearLatchedRXOKErrorSlowOpticalLink0()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SlowOpticalLinksDiag);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_SlowOpticalLinksDiag);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSlowOpticalLinkControlStatus()
{
	auto form = CreateFormatter(DTC_Register_SFPControlStatus);
	form.description = "Slow Optical Link Control Status";
	form.vals.push_back(std::string("RX OK SOL Link 3:           [") + (ReadRXOKErrorSlowOpticalLink3() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX OK SOL Link 2:           [") + (ReadRXOKErrorSlowOpticalLink2() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX OK SOL Link 1:           [") + (ReadRXOKErrorSlowOpticalLink1() ? "x" : " ") + "]");
	form.vals.push_back(std::string("RX OK SOL Link 0:           [") + (ReadRXOKErrorSlowOpticalLink0() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched Spare SMA Input:    [") + (ReadLatchedSpareSMAInputOKError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched Event Marker Input: [") + (ReadLatchedEventMarkerSMAInputOKError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched RX OK SOL Link 3:   [") + (ReadLatchedRXOKErrorSlowOpticalLink3() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched RX OK SOL Link 2:   [") + (ReadLatchedRXOKErrorSlowOpticalLink2() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched RX OK SOL Link 1:   [") + (ReadLatchedRXOKErrorSlowOpticalLink1() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Latched RX OK SOL Link 0:   [") + (ReadLatchedRXOKErrorSlowOpticalLink0() ? "x" : " ") + "]");
	return form;
}

bool DTCLib::DTC_Registers::ReadSERDESInduceErrorEnable(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DiagSERDESErrorEnable);
	return data[link];
}

void DTCLib::DTC_Registers::EnableSERDESInduceError(DTC_Link_ID const& link)
{
	SetBit_(DTC_Register_DiagSERDESErrorEnable, link, true);
}

void DTCLib::DTC_Registers::DisableSERDESInduceError(DTC_Link_ID const& link)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DiagSERDESErrorEnable);
	data[link] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DiagSERDESErrorEnable);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorEnable()
{
	auto form = CreateFormatter(DTC_Register_SERDESTXRXInvertEnable);
	form.description = "SERDES Induce Error Enable";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Link ") + std::to_string(r) + ": [" + (ReadSERDESInduceErrorEnable(r) ? "x" : " ") + "]");
	}

	return form;
}

uint32_t DTCLib::DTC_Registers::ReadSERDESInduceErrorSequenceNumber(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_DiagSERDESPacket0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_DiagSERDESPacket1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_DiagSERDESPacket2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_DiagSERDESPacket3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_DiagSERDESPacket4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_DiagSERDESPacket5);
		default:
			return 0;
	}
}

void DTCLib::DTC_Registers::SetSERDESInduceErrorSequenceNumber(DTC_Link_ID const& link, uint32_t sequence)
{
	switch (link)
	{
		case DTC_Link_0:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket0);
			break;
		case DTC_Link_1:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket1);
			break;
		case DTC_Link_2:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket2);
			break;
		case DTC_Link_3:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket3);
			break;
		case DTC_Link_4:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket4);
			break;
		case DTC_Link_5:
			WriteRegister_(sequence, DTC_Register_DiagSERDESPacket5);
			break;
		default:
			break;
	}
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink0()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket0);
	form.description = "Link 0 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink1()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket1);
	form.description = "Link 1 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink2()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket2);
	form.description = "Link 2 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink3()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket3);
	form.description = "Link 3 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink4()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket4);
	form.description = "Link 4 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESInduceErrorSequenceNumberLink5()
{
	auto form = CreateFormatter(DTC_Register_DiagSERDESPacket5);
	form.description = "Link 5 Induced Error Sequence Number";
	std::stringstream o;
	o << std::dec << ReadSERDESInduceErrorSequenceNumber(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_DDRFlags DTCLib::DTC_Registers::ReadDDRFlags(uint8_t buffer_id)
{
	return DTC_DDRFlags(ReadDDRLinkBufferFullFlags()[buffer_id],
						ReadDDRLinkBufferEmptyFlags()[buffer_id],
						ReadDDRLinkBufferHalfFullFlags()[buffer_id],
						ReadDDREventBuilderBufferFullFlags()[buffer_id],
						ReadDDREventBuilderBufferEmptyFlags()[buffer_id],
						ReadDDREventBuilderBufferHalfFullFlags()[buffer_id]);
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDRLinkBufferFullFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3LinkBufferFullFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3LinkBufferFullFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3LinkBufferFullFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3LinkBufferFullFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDRLinkBufferEmptyFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3LinkBufferEmptyFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3LinkBufferEmptyFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3LinkBufferEmptyFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3LinkBufferEmptyFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDRLinkBufferHalfFullFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3LinkBufferHalfFullFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3LinkBufferHalfFullFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3LinkBufferHalfFullFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3LinkBufferHalfFullFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDREventBuilderBufferFullFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3EVBBufferFullFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3EVBBufferFullFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3EVBBufferFullFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3EVBBufferFullFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDREventBuilderBufferEmptyFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3EVBBufferEmptyFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3EVBBufferEmptyFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3EVBBufferEmptyFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3EVBBufferEmptyFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

std::bitset<128> DTCLib::DTC_Registers::ReadDDREventBuilderBufferHalfFullFlags()
{
	uint32_t word0 = ReadRegister_(DTC_Register_DDR3EVBBufferHalfFullFlags0);
	uint32_t word1 = ReadRegister_(DTC_Register_DDR3EVBBufferHalfFullFlags1);
	uint32_t word2 = ReadRegister_(DTC_Register_DDR3EVBBufferHalfFullFlags2);
	uint32_t word3 = ReadRegister_(DTC_Register_DDR3EVBBufferHalfFullFlags3);
	std::bitset<128> out;
	for (size_t ii = 0; ii < 32; ++ii)
	{
		out[ii] = (word0 >> ii) & 0x1;
		out[ii + 32] = (word1 >> ii) & 0x1;
		out[ii + 64] = (word2 >> ii) & 0x1;
		out[ii + 96] = (word3 >> ii) & 0x1;
	}
	return out;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferEmptyFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags0);
	form.description = "DDR Link Buffer Empty Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferEmptyFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags1);
	form.description = "DDR Link Buffer Empty Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferEmptyFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags2);
	form.description = "DDR Link Buffer Empty Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferEmptyFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags3);
	form.description = "DDR Link Buffer Empty Flags 127-96";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferHalfFullFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags0);
	form.description = "DDR Link Buffer Half-Full Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferHalfFullFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags1);
	form.description = "DDR Link Buffer Half-Full Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferHalfFullFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags2);
	form.description = "DDR Link Buffer Half-Full Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferHalfFullFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags3);
	form.description = "DDR Link Buffer Half-Full Flags 127-96";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferFullFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags0);
	form.description = "DDR Link Buffer Full Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferFullFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags1);
	form.description = "DDR Link Buffer Full Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferFullFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags2);
	form.description = "DDR Link Buffer Full Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRLinkBufferFullFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferEmptyFlags3);
	form.description = "DDR Link Buffer Full Flags 127-96";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3EVBBufferEmptyFlags0);
	form.description = "DDR EVB Buffer Empty Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3EVBBufferEmptyFlags1);
	form.description = "DDR EVB Buffer Empty Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3EVBBufferEmptyFlags2);
	form.description = "DDR EVB Buffer Empty Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3EVBBufferEmptyFlags3);
	form.description = "DDR EVB Buffer Empty Flags 127-96";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags0);
	form.description = "DDR Link Buffer Half-Full Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags1);
	form.description = "DDR Link Buffer Half-Full Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags2);
	form.description = "DDR Link Buffer Half-Full Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferHalfFullFlags3);
	form.description = "DDR Link Buffer Half-Full Flags 127-96";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlags0()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferFullFlags0);
	form.description = "DDR Link Buffer Full Flags 0-31";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlags1()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferFullFlags1);
	form.description = "DDR Link Buffer Full Flags 63-32";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlags2()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferFullFlags2);
	form.description = "DDR Link Buffer Full Flags 95-64";
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlags3()
{
	auto form = CreateFormatter(DTC_Register_DDR3LinkBufferFullFlags3);
	form.description = "DDR Link Buffer Full Flags 127-96";
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadDataPendingDiagnosticTimer(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link0);
		case DTC_Link_1:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link1);
		case DTC_Link_2:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link2);
		case DTC_Link_3:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link3);
		case DTC_Link_4:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link4);
		case DTC_Link_5:
			return ReadRegister_(DTC_Register_DataPendingDiagTimer_Link5);
		default:
			return 0;
	}
}

void DTCLib::DTC_Registers::ResetDataPendingDiagnosticTimerFIFO(DTC_Link_ID const& link)
{
	switch (link)
	{
		case DTC_Link_0:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link0);
			break;
		case DTC_Link_1:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link1);
			break;
		case DTC_Link_2:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link2);
			break;
		case DTC_Link_3:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link3);
			break;
		case DTC_Link_4:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link4);
			break;
		case DTC_Link_5:
			WriteRegister_(0, DTC_Register_DataPendingDiagTimer_Link5);
			break;
		default:
			break;
	}
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink0()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link0);
	form.description = "Data Pending Diagnostic Timer Link 0";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink1()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link1);
	form.description = "Data Pending Diagnostic Timer Link 1";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink2()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link2);
	form.description = "Data Pending Diagnostic Timer Link 2";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink3()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link3);
	form.description = "Data Pending Diagnostic Timer Link 3";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink4()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link4);
	form.description = "Data Pending Diagnostic Timer Link 4";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingDiagnosticTimerLink5()
{
	auto form = CreateFormatter(DTC_Register_DataPendingDiagTimer_Link5);
	form.description = "Data Pending Diagnostic Timer Link 5";
	std::stringstream o;
	o << std::dec << ReadDataPendingDiagnosticTimer(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

bool DTCLib::DTC_Registers::ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet;
	switch (link)
	{
		case DTC_Link_0:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link0));
			break;
		case DTC_Link_1:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link1));
			break;
		case DTC_Link_2:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link2));
			break;
		case DTC_Link_3:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link3));
			break;
		case DTC_Link_4:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link4));
			break;
		case DTC_Link_5:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceTimeoutError_Link5));
			break;
		default:
			return false;
	}
	return dataSet[31];
}

void DTCLib::DTC_Registers::EnableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[31] = 1;
	WriteRegister_(dataSet.to_ulong(), reg);
}
void DTCLib::DTC_Registers::DisableROCEmulatorPeriodicTimeoutError(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[31] = 0;
	WriteRegister_(dataSet.to_ulong(), reg);
}
bool DTCLib::DTC_Registers::ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return false;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	return dataSet[30];
}
void DTCLib::DTC_Registers::EnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[30] = 1;
	WriteRegister_(dataSet.to_ulong(), reg);
}
void DTCLib::DTC_Registers::DisableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[30] = 0;
	WriteRegister_(dataSet.to_ulong(), reg);
}

uint32_t DTCLib::DTC_Registers::ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg) & 0x3FFFFFFF;
}
void DTCLib::DTC_Registers::SetROCEmulatorTimeoutErrorTimestamp(DTC_Link_ID const& link, uint32_t timestamp)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceTimeoutError_Link5;
			break;
		default:
			return;
	}
	timestamp &= 0x3FFFFFFF;
	WriteRegister_(timestamp, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink0()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link0);
	form.description = "ROC Emulator Induce Timeout Error Link 0";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_0) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_0) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink1()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link1);
	form.description = "ROC Emulator Induce Timeout Error Link 1";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_1) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_1) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink2()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link2);
	form.description = "ROC Emulator Induce Timeout Error Link 2";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_2) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_2) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink3()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link3);
	form.description = "ROC Emulator Induce Timeout Error Link 3";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_3) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_3) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink4()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link4);
	form.description = "ROC Emulator Induce Timeout Error Link 4";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_4) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_4) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInduceTimeoutErrorLink5()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceTimeoutError_Link5);
	form.description = "ROC Emulator Induce Timeout Error Link 5";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorPeriodicTimeoutError(DTC_Link_5) ? "x" : " ") + "]");
	form.vals.push_back(std::string("Enable Output Partial Data: [") + (ReadEnableROCEmulatorTimeoutErrorOutputPartialData(DTC_Link_5) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorTimeoutErrorTimestamp(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

bool DTCLib::DTC_Registers::ReadEnableROCEmulatorExtraWordError(DTC_Link_ID const& link)
{
	std::bitset<32> dataSet;
	switch (link)
	{
		case DTC_Link_0:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link0));
			break;
		case DTC_Link_1:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link1));
			break;
		case DTC_Link_2:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link2));
			break;
		case DTC_Link_3:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link3));
			break;
		case DTC_Link_4:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link4));
			break;
		case DTC_Link_5:
			dataSet = std::bitset<32>(ReadRegister_(DTC_Register_ROCEmulator_InduceExtraWordError_Link5));
			break;
		default:
			return false;
	}
	return dataSet[31];
}

void DTCLib::DTC_Registers::EnableROCEmulatorExtraWordError(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[31] = 1;
	WriteRegister_(dataSet.to_ulong(), reg);
}
void DTCLib::DTC_Registers::DisableROCEmulatorExtraWordError(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link5;
			break;
		default:
			return;
	}
	auto dataSet = std::bitset<32>(ReadRegister_(reg));
	dataSet[31] = 0;
	WriteRegister_(dataSet.to_ulong(), reg);
}

uint32_t DTCLib::DTC_Registers::ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg) & 0x7FFFFFFF;
}
void DTCLib::DTC_Registers::SetROCEmulatorExtraWordErrorTimestamp(DTC_Link_ID const& link, uint32_t timestamp)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InduceExtraWordError_Link5;
			break;
		default:
			return;
	}
	timestamp &= 0x7FFFFFFF;
	WriteRegister_(timestamp, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink0()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link0);
	form.description = "ROC Emulator Induce Extra Word Error Link 0";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_0) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink1()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link1);
	form.description = "ROC Emulator Induce Extra Word Error Link 1";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_1) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink2()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link2);
	form.description = "ROC Emulator Induce Extra Word Error Link 2";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_2) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink3()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link3);
	form.description = "ROC Emulator Induce Extra Word Error Link 3";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_3) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink4()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link4);
	form.description = "ROC Emulator Induce Extra Word Error Link 4";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_4) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorExtraWordErrorLink5()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InduceExtraWordError_Link5);
	form.description = "ROC Emulator Induce Extra Word Error Link 5";
	form.vals.push_back(std::string("Enable Periodic Error:      [") + (ReadEnableROCEmulatorExtraWordError(DTC_Link_5) ? "x" : " ") + "]");
	std::stringstream o;
	o << std::dec << ReadROCEmulatorExtraWordErrorTimestamp(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadSERDESCharacterNotInTableErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_CFOLink;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearSERDESCharacterNotInTableErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_CharacterNotInTableErrorCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(1, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink0()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link0);
	form.description = "SERDES Character Not In Table Error Count Link 0";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink1()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link1);
	form.description = "SERDES Character Not In Table Error Count Link 1";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink2()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link2);
	form.description = "SERDES Character Not In Table Error Count Link 2";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink3()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link3);
	form.description = "SERDES Character Not In Table Error Count Link 3";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink4()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link4);
	form.description = "SERDES Character Not In Table Error Count Link 4";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountLink5()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_Link5);
	form.description = "SERDES Character Not In Table Error Count Link 5";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESCharacterNotInTableErrorCountCFOLink()
{
	auto form = CreateFormatter(DTC_Register_SERDES_CharacterNotInTableErrorCount_CFOLink);
	form.description = "SERDES Character Not In Table Error Count CFO Link";
	std::stringstream o;
	o << std::dec << ReadSERDESCharacterNotInTableErrorCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadSERDESRXDisparityErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_CFOLink;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearSERDESRXDisparityErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXDisparityErrorCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(1, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink0()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link0);
	form.description = "SERDES RX Disparity Error Count Link 0";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink1()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link1);
	form.description = "SERDES RX Disparity Error Count Link 1";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink2()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link2);
	form.description = "SERDES RX Disparity Error Count Link 2";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink3()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link3);
	form.description = "SERDES RX Disparity Error Count Link 3";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink4()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link4);
	form.description = "SERDES RX Disparity Error Count Link 4";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountLink5()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_Link5);
	form.description = "SERDES RX Disparity Error Count Link 5";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityErrorCountCFOLink()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXDisparityErrorCount_CFOLink);
	form.description = "SERDES RX Disparity Error Count CFO Link";
	std::stringstream o;
	o << std::dec << ReadSERDESRXDisparityErrorCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadSERDESRXPRBSErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_CFOLink;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearSERDESRXPRBSErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXPRBSErrorCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(1, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink0()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link0);
	form.description = "SERDES RX PRBS Error Count Link 0";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink1()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link1);
	form.description = "SERDES RX PRBS Error Count Link 1";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink2()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link2);
	form.description = "SERDES RX PRBS Error Count Link 2";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink3()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link3);
	form.description = "SERDES RX PRBS Error Count Link 3";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink4()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link4);
	form.description = "SERDES RX PRBS Error Count Link 4";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountLink5()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_Link5);
	form.description = "SERDES RX PRBS Error Count Link 5";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXPRBSErrorCountCFOLink()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXPRBSErrorCount_CFOLink);
	form.description = "SERDES RX PRBS Error Count CFO Link";
	std::stringstream o;
	o << std::dec << ReadSERDESRXPRBSErrorCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadSERDESRXCRCErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXCRCErrorCount_CFOLink;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearSERDESRXCRCErrorCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_SERDES_RXCRCErrorCount_Link5;
			break;
		case DTC_Link_CFO:
			reg = DTC_Register_SERDES_RXCRCErrorCount_CFOLink;
			break;
		default:
			return;
	}
	WriteRegister_(1, reg);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink0()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link0);
	form.description = "SERDES RX CRC Error Count Link 0";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink1()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link1);
	form.description = "SERDES RX CRC Error Count Link 1";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink2()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link2);
	form.description = "SERDES RX CRC Error Count Link 2";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink3()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link3);
	form.description = "SERDES RX CRC Error Count Link 3";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink4()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link4);
	form.description = "SERDES RX CRC Error Count Link 4";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountLink5()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_Link5);
	form.description = "SERDES RX CRC Error Count Link 5";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorCountCFOLink()
{
	auto form = CreateFormatter(DTC_Register_SERDES_RXCRCErrorCount_CFOLink);
	form.description = "SERDES RX CRC Error Count CFO Link";
	std::stringstream o;
	o << std::dec << ReadSERDESRXCRCErrorCount(DTC_Link_CFO);
	form.vals.push_back(o.str());
	return form;
}

// SERDES RX CRC Error Control
bool DTCLib::DTC_Registers::ReadEnableInduceSERDESRXCRCError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_SERDES_RXCRCErrorControl));
	return dataSet[link];
}
void DTCLib::DTC_Registers::EnableInduceSERDESRXCRCError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_SERDES_RXCRCErrorControl));
	dataSet[link] = 1;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_SERDES_RXCRCErrorControl);
}
void DTCLib::DTC_Registers::DisableInduceSERDESRXCRCError(DTC_Link_ID const& link)
{
	auto dataSet = std::bitset<32>(ReadRegister_(DTC_Register_SERDES_RXCRCErrorControl));
	dataSet[link] = 0;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_SERDES_RXCRCErrorControl);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCRCErrorControl()
{
	auto form = CreateFormatter(DTC_Register_SERDESTXRXInvertEnable);
	form.description = "SERDES RX CRC Error Control";
	for (auto r : DTC_Links)
	{
		form.vals.push_back(std::string("Induce Error Link ") + std::to_string(r) + ":  [" + (ReadEnableInduceSERDESRXCRCError(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("Incude Error CFO Link: [") + (ReadEnableInduceSERDESRXCRCError(DTC_Link_CFO) ? "x" : " ") + "]");

	return form;
}

uint32_t DTCLib::DTC_Registers::ReadEVBSERDESRXPacketErrorCounter()
{
	return ReadRegister_(DTC_Register_EBVSERDES_RXPacketErrorCount);
}
void DTCLib::DTC_Registers::ClearEVBSERDESRXPacketErrorCounter()
{
	WriteRegister_(1, DTC_Register_EBVSERDES_RXPacketErrorCount);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBSERDESRXPacketErrorCounter()
{
	auto form = CreateFormatter(DTC_Register_EBVSERDES_RXPacketErrorCount);
	form.description = "EVB SERDES RX Packet Error Counter";
	std::stringstream o;
	o << std::dec << ReadEVBSERDESRXPacketErrorCounter();
	form.vals.push_back(o.str());
	return form;
}

// Jitter Attenuator SERDES RX Recovered Clock LOS Counter
uint32_t DTCLib::DTC_Registers::ReadJitterAttenuatorRecoveredClockLOSCount()
{
	return ReadRegister_(DTC_Register_JitterAttenuator_SERDES_RXRecoveredClockLOSCount);
}
void DTCLib::DTC_Registers::ClearJitterAttenuatorRecoeveredClockLOSCount()
{
	WriteRegister_(1, DTC_Register_JitterAttenuator_SERDES_RXRecoveredClockLOSCount);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatJitterAttenuatorRecoveredClockLOSCount()
{
	auto form = CreateFormatter(DTC_Register_JitterAttenuator_SERDES_RXRecoveredClockLOSCount);
	form.description = "Jitter Attenuator SERDES RX Recovered Clock LOS Counter";
	std::stringstream o;
	o << std::dec << ReadJitterAttenuatorRecoveredClockLOSCount();
	form.vals.push_back(o.str());
	return form;
}

// Jitter Attenuator SERDES RX External Clock LOS Counter
uint32_t DTCLib::DTC_Registers::ReadJitterAttenuatorExternalClockLOSCount()
{
	return ReadRegister_(DTC_Register_JitterAttenuator_SERDES_RXExternalClockLOSCount);
}
void DTCLib::DTC_Registers::ClearJitterAttenuatorExternalClockLOSCount()
{
	WriteRegister_(1, DTC_Register_JitterAttenuator_SERDES_RXExternalClockLOSCount);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatJitterAttenuatorExternalClockLOSCount()
{
	auto form = CreateFormatter(DTC_Register_JitterAttenuator_SERDES_RXExternalClockLOSCount);
	form.description = "Jitter Attenuator SERDES RX External Clock LOS Counter";
	std::stringstream o;
	o << std::dec << ReadJitterAttenuatorExternalClockLOSCount();
	form.vals.push_back(o.str());
	return form;
}

// ROC Emulator Interpacket Delay
uint32_t DTCLib::DTC_Registers::ReadROCEmulatorInterpacketDelay(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::SetROCEmulatorInterpacketDelay(DTC_Link_ID const& link, uint32_t delay)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_ROCEmulator_InterpacketDelay_Link5;
			break;
		default:
			return;
	}
	WriteRegister_(delay,reg);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink0()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link0);
	form.description = "ROC Emulator Interpacket Delay Link 0 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink1()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link1);
	form.description = "ROC Emulator Interpacket Delay Link 1 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink2()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link2);
	form.description = "ROC Emulator Interpacket Delay Link 2 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink3()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link3);
	form.description = "ROC Emulator Interpacket Delay Link 3 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink4()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link4);
	form.description = "ROC Emulator Interpacket Delay Link 4 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulatorInterpacketDelayLink5()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulator_InterpacketDelay_Link5);
	form.description = "ROC Emulator Interpacket Delay Link 5 (*5ns)";
	std::stringstream o;
	o << std::dec << ReadROCEmulatorInterpacketDelay(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// TX Data Request Packet Count
uint32_t DTCLib::DTC_Registers::ReadTXDataRequestPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TXDataRequestPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TXDataRequestPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TXDataRequestPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TXDataRequestPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TXDataRequestPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TXDataRequestPacketCount_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearTXDataRequetsPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TXDataRequestPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TXDataRequestPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TXDataRequestPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TXDataRequestPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TXDataRequestPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TXDataRequestPacketCount_Link5;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link0);
	form.description = "Data Request Packet TX Counter Link 0";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link1);
	form.description = "Data Request Packet TX Counter Link 1";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link2);
	form.description = "Data Request Packet TX Counter Link 2";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link3);
	form.description = "Data Request Packet TX Counter Link 3";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link4);
	form.description = "Data Request Packet TX Counter Link 4";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXDataRequestPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_TXDataRequestPacketCount_Link5);
	form.description = "Data Request Packet TX Counter Link 5";
	std::stringstream o;
	o << std::dec << ReadTXDataRequestPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// TX Heartbeat Packet Count
uint32_t DTCLib::DTC_Registers::ReadTXHeartbeatPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TXHeartbeatPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TXHeartbeatPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TXHeartbeatPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TXHeartbeatPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TXHeartbeatPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TXHeartbeatPacketCount_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearTXHeartbeatPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_TXHeartbeatPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_TXHeartbeatPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_TXHeartbeatPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_TXHeartbeatPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_TXHeartbeatPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_TXHeartbeatPacketCount_Link5;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link0);
	form.description = "Heartbeat Packet TX Counter Link 0";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link1);
	form.description = "Heartbeat Packet TX Counter Link 1";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link2);
	form.description = "Heartbeat Packet TX Counter Link 2";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link3);
	form.description = "Heartbeat Packet TX Counter Link 3";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link4);
	form.description = "Heartbeat Packet TX Counter Link 4";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTXHeartbeatPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_TXHeartbeatPacketCount_Link5);
	form.description = "Heartbeat Packet TX Counter Link 5";
	std::stringstream o;
	o << std::dec << ReadTXHeartbeatPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// RX Data Header Packet Count
uint32_t DTCLib::DTC_Registers::ReadRXDataHeaderPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_RXDataHeaderPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_RXDataHeaderPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_RXDataHeaderPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_RXDataHeaderPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_RXDataHeaderPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_RXDataHeaderPacketCount_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearRXDataHeaderPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_RXDataHeaderPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_RXDataHeaderPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_RXDataHeaderPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_RXDataHeaderPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_RXDataHeaderPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_RXDataHeaderPacketCount_Link5;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link0);
	form.description = "Data Header Packet RX Counter Link 0";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link1);
	form.description = "Data Header Packet RX Counter Link 1";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link2);
	form.description = "Data Header Packet RX Counter Link 2";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link3);
	form.description = "Data Header Packet RX Counter Link 3";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link4);
	form.description = "Data Header Packet RX Counter Link 4";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataHeaderPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_RXDataHeaderPacketCount_Link5);
	form.description = "Data Header Packet RX Counter Link 5";
	std::stringstream o;
	o << std::dec << ReadRXDataHeaderPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// RX Data Packet Count
uint32_t DTCLib::DTC_Registers::ReadRXDataPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_RXDataPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_RXDataPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_RXDataPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_RXDataPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_RXDataPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_RXDataPacketCount_Link5;
			break;
		default:
			return 0;
	}
	return ReadRegister_(reg);
}
void DTCLib::DTC_Registers::ClearRXDataPacketCount(DTC_Link_ID const& link)
{
	DTC_Register reg;
	switch (link)
	{
		case DTC_Link_0:
			reg = DTC_Register_RXDataPacketCount_Link0;
			break;
		case DTC_Link_1:
			reg = DTC_Register_RXDataPacketCount_Link1;
			break;
		case DTC_Link_2:
			reg = DTC_Register_RXDataPacketCount_Link2;
			break;
		case DTC_Link_3:
			reg = DTC_Register_RXDataPacketCount_Link3;
			break;
		case DTC_Link_4:
			reg = DTC_Register_RXDataPacketCount_Link4;
			break;
		case DTC_Link_5:
			reg = DTC_Register_RXDataPacketCount_Link5;
			break;
		default:
			return;
	}
	WriteRegister_(0, reg);
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink0()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link0);
	form.description = "Data Packet RX Counter Link 0";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_0);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink1()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link1);
	form.description = "Data Packet RX Counter Link 1";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_1);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink2()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link2);
	form.description = "Data Packet RX Counter Link 2";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_2);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink3()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link3);
	form.description = "Data Packet RX Counter Link 3";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_3);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink4()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link4);
	form.description = "Data Packet RX Counter Link 4";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_4);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXDataPacketCountLink5()
{
	auto form = CreateFormatter(DTC_Register_RXDataPacketCount_Link5);
	form.description = "Data Packet RX Counter Link 5";
	std::stringstream o;
	o << std::dec << ReadRXDataPacketCount(DTC_Link_5);
	form.vals.push_back(o.str());
	return form;
}

// EVB Diagnostic RX Packet FIFO
uint64_t DTCLib::DTC_Registers::ReadEVBDiagnosticFIFO() {
	uint64_t ret = ReadRegister_(DTC_Register_EVBDiagnosticRXPacket_High);
	ret = (ret << 32) + ReadRegister_(DTC_Register_EVBDiagnosticRXPacket_Low);
	return ret;
}
void DTCLib::DTC_Registers::ClearEVBDiagnosticFIFO() {
	WriteRegister_(0, DTC_Register_EVBDiagnosticRXPacket_Low);
}

// Event Mode Lookup Table
/// <summary>
/// Set all event mode words to the given value
/// </summary>
/// <param name="data">Value for all event mode words</param>
void DTCLib::DTC_Registers::SetAllEventModeWords(uint32_t data)
{
	for (uint16_t address = DTC_Register_EventModeLookupTableStart; address <= DTC_Register_EventModeLookupTableEnd;
		 address += 4)
	{
		auto retry = 3;
		int errorCode;
		do
		{
			errorCode = device_.write_register(address, 100, data);
			--retry;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			TLOG(TLVL_ERROR) << "Error writing register " << address;
			throw DTC_IOErrorException(errorCode);
		}
	}
}

/// <summary>
/// Set a given event mode word
/// </summary>
/// <param name="which">Word index to write</param>
/// <param name="data">Data for word</param>
void DTCLib::DTC_Registers::SetEventModeWord(uint8_t which, uint32_t data)
{
	uint16_t address = DTC_Register_EventModeLookupTableStart + (which * 4);
	if (address <= DTC_Register_EventModeLookupTableEnd)
	{
		auto retry = 3;
		int errorCode;
		do
		{
			errorCode = device_.write_register(address, 100, data);
			--retry;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			TLOG(TLVL_ERROR) << "Error writing register " << address;
			throw DTC_IOErrorException(errorCode);
		}
	}
}

/// <summary>
/// Read an event mode word from the Event Mode lookup table
/// </summary>
/// <param name="which">Word index to read</param>
/// <returns>Value of the given event mode word</returns>
uint32_t DTCLib::DTC_Registers::ReadEventModeWord(uint8_t which)
{
	uint16_t address = DTC_Register_EventModeLookupTableStart + (which * 4);
	if (address <= DTC_Register_EventModeLookupTableEnd)
	{
		auto retry = 3;
		int errorCode;
		uint32_t data;
		do
		{
			errorCode = device_.read_register(address, 100, &data);
			--retry;
		} while (retry > 0 && errorCode != 0);
		if (errorCode != 0)
		{
			TLOG(TLVL_ERROR) << "Error writing register " << address;
			throw DTC_IOErrorException(errorCode);
		}

		return data;
	}
	return 0;
}

// Oscillator Programming (DDR and SERDES)
/// <summary>
/// Set the given oscillator to the given frequency, calculating a new program in the process.
/// </summary>
/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
/// <param name="targetFrequency">New frequency to program, in Hz</param>
/// <returns>Whether the oscillator was changed (Will not reset if already set to desired frequency)</returns>
bool DTCLib::DTC_Registers::SetNewOscillatorFrequency(DTC_OscillatorType oscillator, double targetFrequency)
{
	auto currentFrequency = ReadCurrentFrequency(oscillator);
	auto currentProgram = ReadCurrentProgram(oscillator);
	DTC_TLOG(TLVL_DEBUG) << "Target Frequency: " << targetFrequency << ", Current Frequency: " << currentFrequency
						 << ", Current Program: " << std::showbase << std::hex << currentProgram;

	// Check if targetFrequency is essentially the same as the current frequency...
	if (fabs(currentFrequency - targetFrequency) < targetFrequency * 30 / 1000000)
	{
		DTC_TLOG(TLVL_INFO) << "New frequency and old frequency are within 30 ppm of each other, not reprogramming!";
		return false;
	}

	auto newParameters = CalculateFrequencyForProgramming_(targetFrequency, currentFrequency, currentProgram);
	if (newParameters == 0)
	{
		DTC_TLOG(TLVL_WARNING) << "New program calculated as 0! Check parameters!";
		return false;
	}
	WriteCurrentProgram(newParameters, oscillator);
	WriteCurrentFrequency(targetFrequency, oscillator);
	return true;
}

/// <summary>
/// Get the DTC's idea of the current frequency of the specified oscillator
/// </summary>
/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
/// <returns>Current frequency of oscillator, in Hz</returns>
double DTCLib::DTC_Registers::ReadCurrentFrequency(DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
		case DTC_OscillatorType_SERDES:
			return ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_EVB);
		case DTC_OscillatorType_Timing:
			return ReadSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_CFO);
		case DTC_OscillatorType_DDR:
			return ReadDDROscillatorReferenceFrequency();
	}
	return 0;
}

/// <summary>
/// Read the current RFREQ and dividers of the given oscillator clock
/// </summary>
/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
/// <returns>64-bit integer contianing current oscillator program</returns>
uint64_t DTCLib::DTC_Registers::ReadCurrentProgram(DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
		case DTC_OscillatorType_SERDES:
			return ReadSERDESOscillatorParameters_();
		case DTC_OscillatorType_Timing:
			return ReadTimingOscillatorParameters_();
		case DTC_OscillatorType_DDR:
			return ReadDDROscillatorParameters_();
	}
	return 0;
}
/// <summary>
/// Write the current frequency, in Hz to the frequency register
/// </summary>
/// <param name="freq">Frequency of oscillator</param>
/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
void DTCLib::DTC_Registers::WriteCurrentFrequency(double freq, DTC_OscillatorType oscillator)
{
	auto newFreq = static_cast<uint32_t>(freq);
	switch (oscillator)
	{
		case DTC_OscillatorType_SERDES:
			SetSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_EVB, newFreq);
			break;
		case DTC_OscillatorType_Timing:
			SetSERDESOscillatorReferenceFrequency(DTC_IICSERDESBusAddress_CFO, newFreq);
			break;
		case DTC_OscillatorType_DDR:
			SetDDROscillatorReferenceFrequency(newFreq);
			break;
	}
}
/// <summary>
/// Writes a program for the given oscillator crystal. This function should be paired with a call to
/// WriteCurrentFrequency so that subsequent programming attempts work as expected.
/// </summary>
/// <param name="program">64-bit integer with new RFREQ and dividers</param>
/// <param name="oscillator">Oscillator to program, either DDR or SERDES</param>
void DTCLib::DTC_Registers::WriteCurrentProgram(uint64_t program, DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
		case DTC_OscillatorType_SERDES:
			SetSERDESOscillatorParameters_(program);
			break;
		case DTC_OscillatorType_Timing:
			SetTimingOscillatorParameters_(program);
			break;
		case DTC_OscillatorType_DDR:
			SetDDROscillatorParameters_(program);
			break;
	}
}

// Private Functions
void DTCLib::DTC_Registers::WriteRegister_(uint32_t data, const DTC_Register& address)
{
	auto retry = 3;
	int errorCode;
	do
	{
		errorCode = device_.write_register(address, 100, data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		TLOG(TLVL_ERROR) << "Error writing register 0x" << std::hex << static_cast<uint32_t>(address) << " " << errorCode;
		throw DTC_IOErrorException(errorCode);
	}
}

uint32_t DTCLib::DTC_Registers::ReadRegister_(const DTC_Register& address)
{
	auto retry = 3;
	int errorCode;
	uint32_t data;
	do
	{
		errorCode = device_.read_register(address, 100, &data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		TLOG(TLVL_ERROR) << "Error reading register 0x" << std::hex << static_cast<uint32_t>(address) << " " << errorCode;
		throw DTC_IOErrorException(errorCode);
	}

	DTC_TLOG(TLVL_ReadRegister) << "ReadRegister_ returning " << std::hex << std::showbase << data << " for address " << static_cast<uint32_t>(address);
	return data;
}

bool DTCLib::DTC_Registers::GetBit_(const DTC_Register& address, size_t bit)
{
	if (bit > 31)
	{
		TLOG(TLVL_ERROR) << "Cannot read bit " << bit << ", as it is out of range";
		throw std::out_of_range("Cannot read bit " + std::to_string(bit) + ", as it is out of range");
	}
	return std::bitset<32>(ReadRegister_(address))[bit];
}

void DTCLib::DTC_Registers::SetBit_(const DTC_Register& address, size_t bit, bool value)
{
	if (bit > 31)
	{
		TLOG(TLVL_ERROR) << "Cannot set bit " << bit << ", as it is out of range";
		throw std::out_of_range("Cannot set bit " + std::to_string(bit) + ", as it is out of range");
	}
	auto regVal = std::bitset<32>(ReadRegister_(address));
	regVal[bit] = value;
	WriteRegister_(regVal.to_ulong(), address);
}

int DTCLib::DTC_Registers::DecodeHighSpeedDivider_(int input)
{
	switch (input)
	{
		case 0:
			return 4;
		case 1:
			return 5;
		case 2:
			return 6;
		case 3:
			return 7;
		case 5:
			return 9;
		case 7:
			return 11;
		default:
			return -1;
	}
}

int DTCLib::DTC_Registers::EncodeHighSpeedDivider_(int input)
{
	switch (input)
	{
		case 4:
			return 0;
		case 5:
			return 1;
		case 6:
			return 2;
		case 7:
			return 3;
		case 9:
			return 5;
		case 11:
			return 7;
		default:
			return -1;
	}
}

int DTCLib::DTC_Registers::EncodeOutputDivider_(int input)
{
	if (input == 1) return 0;
	int temp = input / 2;
	return (temp * 2) - 1;
}

uint64_t DTCLib::DTC_Registers::CalculateFrequencyForProgramming_(double targetFrequency, double currentFrequency,
																  uint64_t currentProgram)
{
	DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: targetFrequency=" << targetFrequency << ", currentFrequency=" << currentFrequency
								 << ", currentProgram=" << std::showbase << std::hex << static_cast<unsigned long long>(currentProgram);
	auto currentHighSpeedDivider = DecodeHighSpeedDivider_((currentProgram >> 45) & 0x7);
	auto currentOutputDivider = DecodeOutputDivider_((currentProgram >> 38) & 0x7F);
	auto currentRFREQ = DecodeRFREQ_(currentProgram & 0x3FFFFFFFFF);
	DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: Current HSDIV=" << currentHighSpeedDivider << ", N1=" << currentOutputDivider << ", RFREQ=" << currentRFREQ;
	const auto minFreq = 4850000000;  // Hz
	const auto maxFreq = 5670000000;  // Hz

	auto fXTAL = currentFrequency * currentHighSpeedDivider * currentOutputDivider / currentRFREQ;
	DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: fXTAL=" << fXTAL;

	std::vector<int> hsdiv_values = {11, 9, 7, 6, 5, 4};
	std::vector<std::pair<int, double>> parameter_values;
	for (auto hsdiv : hsdiv_values)
	{
		auto minN = minFreq / (targetFrequency * hsdiv);
		if (minN > 128) break;

		auto thisN = 2;
		if (minN < 1) thisN = 1;
		while (thisN < minN)
		{
			thisN += 2;
		}
		auto fdco_new = hsdiv * thisN * targetFrequency;
		DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: Adding solution: HSDIV=" << hsdiv << ", N1=" << thisN << ", fdco_new=" << fdco_new;
		parameter_values.push_back(std::make_pair(thisN, fdco_new));
	}

	auto counter = -1;
	auto newHighSpeedDivider = 0;
	auto newOutputDivider = 0;
	auto newRFREQ = 0.0;

	for (auto values : parameter_values)
	{
		++counter;
		if (values.second > maxFreq) continue;

		newHighSpeedDivider = hsdiv_values[counter];
		newOutputDivider = values.first;
		newRFREQ = values.second / fXTAL;
		break;
	}
	DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: New Program: HSDIV=" << newHighSpeedDivider << ", N1=" << newOutputDivider << ", RFREQ=" << newRFREQ;

	if (EncodeHighSpeedDivider_(newHighSpeedDivider) == -1)
	{
		DTC_TLOG(TLVL_ERROR) << "ERROR: CalculateFrequencyForProgramming: Invalid HSDIV " << newHighSpeedDivider << "!";
		return 0;
	}
	if (newOutputDivider > 128 || newOutputDivider < 0)
	{
		DTC_TLOG(TLVL_ERROR) << "ERROR: CalculateFrequencyForProgramming: Invalid N1 " << newOutputDivider << "!";
		return 0;
	}
	if (newRFREQ <= 0)
	{
		DTC_TLOG(TLVL_ERROR) << "ERROR: CalculateFrequencyForProgramming: Invalid RFREQ " << newRFREQ << "!";
		return 0;
	}

	auto output = (static_cast<uint64_t>(EncodeHighSpeedDivider_(newHighSpeedDivider)) << 45) +
				  (static_cast<uint64_t>(EncodeOutputDivider_(newOutputDivider)) << 38) + EncodeRFREQ_(newRFREQ);
	DTC_TLOG(TLVL_CalculateFreq) << "CalculateFrequencyForProgramming: New Program: " << std::showbase << std::hex << static_cast<unsigned long long>(output);
	return output;
}

uint64_t DTCLib::DTC_Registers::ReadSERDESOscillatorParameters_()
{
	uint64_t data = (static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 7)) << 40) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 8)) << 32) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 9)) << 24) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 10)) << 16) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 11)) << 8) +
					static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 12));
	return data;
}

uint64_t DTCLib::DTC_Registers::ReadTimingOscillatorParameters_()
{
	uint64_t data = (static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 7)) << 40) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 8)) << 32) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 9)) << 24) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 10)) << 16) +
					(static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 11)) << 8) +
					static_cast<uint64_t>(ReadSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 12));
	return data;
}

uint64_t DTCLib::DTC_Registers::ReadDDROscillatorParameters_()
{
	uint64_t data = (static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 7)) << 40) +
					(static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 8)) << 32) +
					(static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 9)) << 24) +
					(static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 10)) << 16) +
					(static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 11)) << 8) +
					static_cast<uint64_t>(ReadDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 12));
	return data;
}

void DTCLib::DTC_Registers::SetSERDESOscillatorParameters_(uint64_t program)
{
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 0x89, 0x10);

	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 7, static_cast<uint8_t>(program >> 40));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 8, static_cast<uint8_t>(program >> 32));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 9, static_cast<uint8_t>(program >> 24));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 10, static_cast<uint8_t>(program >> 16));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 11, static_cast<uint8_t>(program >> 8));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 12, static_cast<uint8_t>(program));

	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 0x89, 0);
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_EVB, 0x87, 0x40);
}

void DTCLib::DTC_Registers::SetTimingOscillatorParameters_(uint64_t program)
{
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 0x89, 0x10);

	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 7, static_cast<uint8_t>(program >> 40));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 8, static_cast<uint8_t>(program >> 32));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 9, static_cast<uint8_t>(program >> 24));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 10, static_cast<uint8_t>(program >> 16));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 11, static_cast<uint8_t>(program >> 8));
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 12, static_cast<uint8_t>(program));

	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 0x89, 0);
	WriteSERDESIICInterface(DTC_IICSERDESBusAddress_CFO, 0x87, 0x40);
}

void DTCLib::DTC_Registers::SetDDROscillatorParameters_(uint64_t program)
{
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 0x89, 0x10);

	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 7, static_cast<uint8_t>(program >> 40));
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 8, static_cast<uint8_t>(program >> 32));
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 9, static_cast<uint8_t>(program >> 24));
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 10, static_cast<uint8_t>(program >> 16));
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 11, static_cast<uint8_t>(program >> 8));
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 12, static_cast<uint8_t>(program));

	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 0x89, 0);
	WriteDDRIICInterface(DTC_IICDDRBusAddress_DDROscillator, 0x87, 0x40);
}

bool DTCLib::DTC_Registers::WaitForLinkReady_(DTC_Link_ID const& link, size_t interval, double timeout /*seconds*/)
{
	auto start = std::chrono::steady_clock::now();
	auto last_print = start;
	bool ready = ReadSERDESPLLLocked(link) && ReadResetRXSERDESDone(link) && ReadResetTXSERDESDone(link) && ReadSERDESRXCDRLock(link);

	while (!ready)
	{
		usleep(interval);
		ready = ReadSERDESPLLLocked(link) && ReadResetRXSERDESDone(link) && ReadResetTXSERDESDone(link) && ReadSERDESRXCDRLock(link);
		if (std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(std::chrono::steady_clock::now() - last_print).count() > 5.0)
		{
			DTC_TLOG(TLVL_DEBUG) << "DTC_Registers: WaitForLinkReady_: ROC Link " << link << ": PLL Locked: " << std::boolalpha << ReadSERDESPLLLocked(link) << ", RX Reset Done: " << ReadResetRXSERDESDone(link) << ", TX Reset Done: " << ReadResetTXSERDESDone(link) << ", CDR Lock: " << ReadSERDESRXCDRLock(link);
			last_print = std::chrono::steady_clock::now();
		}
		if (std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(std::chrono::steady_clock::now() - start).count() > timeout)
		{
			DTC_TLOG(TLVL_ERROR) << "DTC_Registers: WaitForLinkReady_ ABORTING: ROC Link " << link << ": PLL Locked: " << std::boolalpha << ReadSERDESPLLLocked(link) << ", RX Reset Done: " << ReadResetRXSERDESDone(link) << ", TX Reset Done: " << ReadResetTXSERDESDone(link) << ", CDR Lock: " << ReadSERDESRXCDRLock(link);
			return false;
		}
	}
	return true;
}
