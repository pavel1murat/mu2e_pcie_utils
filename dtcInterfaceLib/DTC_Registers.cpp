#include "DTC_Registers.h"
#include <sstream> // Convert uint to hex string
#include <iomanip> // std::setw, std::setfill

#include <chrono>
#include <assert.h>
#include <cmath>
# include <unistd.h>
# include "trace.h"

DTCLib::DTC_Registers::DTC_Registers(std::string expectedDesignVersion, DTC_SimMode mode, unsigned rocMask, bool skipInit) : device_(), simMode_(mode), dmaSize_(16)
{
	for (auto ii = 0; ii < 6; ++ii)
	{
		maxROCs_[ii] = DTC_ROC_Unused;
	}

	auto sim = getenv("DTCLIB_SIM_ENABLE");
	if (sim != nullptr)
	{
		switch (sim[0])
		{
		case '1':
		case 't':
		case 'T':
			simMode_ = DTC_SimMode_Tracker;
			break;
		case '2':
		case 'c':
		case 'C':
			simMode_ = DTC_SimMode_Calorimeter;
			break;
		case '3':
		case 'v':
		case 'V':
			simMode_ = DTC_SimMode_CosmicVeto;
			break;
		case '4':
		case 'n':
		case 'N':
			simMode_ = DTC_SimMode_NoCFO;
			break;
		case '5':
		case 'r':
		case 'R':
			simMode_ = DTC_SimMode_ROCEmulator;
			break;
		case '6':
		case 'l':
		case 'L':
			simMode_ = DTC_SimMode_Loopback;
			break;
		case '7':
		case 'p':
		case 'P':
			simMode_ = DTC_SimMode_Performance;
			break;
		case '8':
		case 'f':
		case 'F':
			simMode_ = DTC_SimMode_LargeFile;
			break;
		case '0':
		default:
			simMode_ = DTC_SimMode_Disabled;
			break;
		}
	}
	SetSimMode(expectedDesignVersion, simMode_, rocMask, skipInit);
}

DTCLib::DTC_Registers::~DTC_Registers()
{
	DisableDetectorEmulator();
	//DisableDetectorEmulatorMode();
	DisableCFOEmulation();
	//ResetDTC();
	device_.close();
}

DTCLib::DTC_SimMode DTCLib::DTC_Registers::SetSimMode(std::string expectedDesignVersion,DTC_SimMode mode, unsigned rocMask, bool skipInit)
{
	simMode_ = mode;
	device_.init(simMode_);
	if (expectedDesignVersion != "" && expectedDesignVersion != ReadDesignVersion())
	{
		throw new DTC_WrongVersionException(expectedDesignVersion, ReadDesignVersion());
	}

	if (skipInit) return simMode_;

	bool useTiming = simMode_ == DTC_SimMode_Disabled;
	for (auto ring : DTC_Rings)
	{
		bool ringEnabled = ((rocMask >> (ring * 4)) & 0x1) != 0;
		if (!ringEnabled) { DisableRing(ring); }
		else
		{
			int rocCount = (((rocMask >> (ring * 4)) & 0xF) - 1) / 2;
			EnableRing(ring, DTC_RingEnableMode(true, true, useTiming), DTC_ROCS[rocCount + 1]);
		}
		if (!ringEnabled)  DisableROCEmulator(ring);
		if (!ringEnabled)  SetSERDESLoopbackMode(ring, DTC_SERDESLoopbackMode_Disabled);
	}

	if (simMode_ != DTC_SimMode_Disabled)
	{
		// Set up hardware simulation mode: Ring 0 Tx/Rx Enabled, Loopback Enabled, ROC Emulator Enabled. All other rings disabled.
		// for (auto ring : DTC_Rings)
		// 	{
		// 	  DisableRing(ring);
		// 	}
		//	EnableRing(DTC_Ring_0, DTC_RingEnableMode(true, true, false), DTC_ROC_0);
		for (auto ring : DTC_Rings)
		{
			if (simMode_ == DTC_SimMode_Loopback)
			{
				SetSERDESLoopbackMode(ring, DTC_SERDESLoopbackMode_NearPCS);
				//			SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
			}
			else if (simMode_ == DTC_SimMode_ROCEmulator)
			{
				EnableROCEmulator(ring);
				//SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
			}
		}
		SetInternalSystemClock();
		DisableTiming();
	}
	ReadMinDMATransferLength();

	return simMode_;
}

//
// DTC Register Dumps
//
std::string DTCLib::DTC_Registers::FormattedRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28) { formatterWidth_ = 28; }
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

std::string DTCLib::DTC_Registers::PerformanceMonitorRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28) { formatterWidth_ = 28; }
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "Performance Monitor Registers: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedPerfMonFunctions_)
	{
		o << divider << std::endl;
		o << i();
	}
	return o.str();
}

std::string DTCLib::DTC_Registers::RingCountersRegDump(int width)
{
	std::string divider(width, '=');
	formatterWidth_ = width - 27 - 65;
	if (formatterWidth_ < 28) { formatterWidth_ = 28; }
	std::string spaces(formatterWidth_ - 4, ' ');
	std::ostringstream o;
	o << "SERDES Byte/Packet Counters: " << std::endl;
	o << "    Address | Value      | Name " << spaces << "| Translation" << std::endl;
	for (auto i : formattedCounterFunctions_)
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
std::string DTCLib::DTC_Registers::ReadDesignVersion()
{
	return ReadDesignVersionNumber() + "_" + ReadDesignDate();
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDesignVersion()
{
	auto form = CreateFormatter(DTC_Register_DesignVersion);
	form.description = "DTC Firmware Design Version";
	form.vals.push_back(ReadDesignVersionNumber());
	return form;
}

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
	//std::cout << o.str() << std::endl;
	return o.str();
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDesignDate()
{
	auto form = CreateFormatter(DTC_Register_DesignDate);
	form.description = "DTC Firmware Design Date";
	form.vals.push_back(ReadDesignDate());
	return form;
}

std::string DTCLib::DTC_Registers::ReadDesignVersionNumber()
{
	auto data = ReadRegister_(DTC_Register_DesignVersion);
	int minor = data & 0xFF;
	int major = (data & 0xFF00) >> 8;
	return "v" + std::to_string(major) + "." + std::to_string(minor);
}

// PCIE Performance Monitor Registers
uint32_t DTCLib::DTC_Registers::ReadPerfMonTXByteCount()
{
	return ReadRegister_(DTC_Register_PerfMonTXByteCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonTXByteCount()
{
	auto form = CreateFormatter(DTC_Register_PerfMonTXByteCount);
	form.description = "PerfMon TX Byte Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonTXByteCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadPerfMonRXByteCount()
{
	return ReadRegister_(DTC_Register_PerfMonRXByteCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonRXByteCount()
{
	auto form = CreateFormatter(DTC_Register_PerfMonRXByteCount);
	form.description = "PerfMon RX Byte Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonTXByteCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadPerfMonTXPayloadCount()
{
	return ReadRegister_(DTC_Register_PerfMonTXPayloadCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonTXPayloadCount()
{
	auto form = CreateFormatter(DTC_Register_PerfMonTXPayloadCount);
	form.description = "PerfMon TX Payload Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonTXByteCount();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadPerfMonRXPayloadCount()
{
	return ReadRegister_(DTC_Register_PerfMonRXPayloadCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonRXPayloadCount()
{
	auto form = CreateFormatter(DTC_Register_PerfMonRXPayloadCount);
	form.description = "PerfMon RX Payload Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonRXPayloadCount();
	form.vals.push_back(o.str());
	return form;
}

uint16_t DTCLib::DTC_Registers::ReadPerfMonInitCDC()
{
	return static_cast<uint16_t>(ReadRegister_(DTC_Register_PerfMonInitCDC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitCDC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitCDC);
	form.description = "PerfMon Init CDC";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonInitCDC();
	form.vals.push_back(o.str());
	return form;
}

uint8_t DTCLib::DTC_Registers::ReadPerfMonInitCHC()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_PerfMonInitCHC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitCHC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitCHC);
	form.description = "PerfMon Init CHC";
	std::stringstream o;
	o << "0x" << std::hex << static_cast<int>(ReadPerfMonInitCHC());
	form.vals.push_back(o.str());
	return form;
}

uint16_t DTCLib::DTC_Registers::ReadPerfMonInitNPDC()
{
	return static_cast<uint16_t>(ReadRegister_(DTC_Register_PerfMonInitNPDC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitNPDC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitNPDC);
	form.description = "PerfMon Init NPDC";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonInitNPDC();
	form.vals.push_back(o.str());
	return form;
}

uint8_t DTCLib::DTC_Registers::ReadPerfMonInitNPHC()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_PerfMonInitNPHC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitNPHC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitNPHC);
	form.description = "PerfMon Init NPHC";
	std::stringstream o;
	o << "0x" << std::hex << static_cast<int>(ReadPerfMonInitNPHC());
	form.vals.push_back(o.str());
	return form;
}

uint16_t DTCLib::DTC_Registers::ReadPerfMonInitPDC()
{
	return static_cast<uint16_t>(ReadRegister_(DTC_Register_PerfMonInitPDC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitPDC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitPDC);
	form.description = "PerfMon Init PDC";
	std::stringstream o;
	o << "0x" << std::hex << ReadPerfMonInitPDC();
	form.vals.push_back(o.str());
	return form;
}

uint8_t DTCLib::DTC_Registers::ReadPerfMonInitPHC()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_PerfMonInitPHC));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatPerfMonInitPHC()
{
	auto form = CreateFormatter(DTC_Register_PerfMonInitPHC);
	form.description = "PerfMon Init PHC";
	std::stringstream o;
	o << "0x" << std::hex << static_cast<int>(ReadPerfMonInitPHC());
	form.vals.push_back(o.str());
	return form;
}

// DTC Control Register
void DTCLib::DTC_Registers::ResetDTC()
{
	TRACE( 15, "ResetDTC start" );
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[31] = 1; // DTC Reset bit
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadResetDTC()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_DTCControl);
	return dataSet[31];
}

void DTCLib::DTC_Registers::EnableCFOEmulation()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[30] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableCFOEmulation()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[30] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadCFOEmulation()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_DTCControl);
	return dataSet[30];
}

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

bool DTCLib::DTC_Registers::ReadResetDDRWriteAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[27];
}

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

bool DTCLib::DTC_Registers::ReadResetDDRReadAddress()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[26];
}

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

bool DTCLib::DTC_Registers::ReadResetDDR()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[25];
}

void DTCLib::DTC_Registers::EnableCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[24] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[24] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadCFOEmulatorDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[24];
}

void DTCLib::DTC_Registers::EnableAutogenDRP()
{
	TRACE( 15, "EnableAutogenDRP start" );
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[23] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableAutogenDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[23] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadAutogenDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[23];
}

void DTCLib::DTC_Registers::EnableSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[22] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[22] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadSoftwareDRP()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[22];
}

void DTCLib::DTC_Registers::EnableDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[2] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}
void DTCLib::DTC_Registers::DisableDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[2] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}
bool DTCLib::DTC_Registers::ReadDCSReception()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[2];
}

void DTCLib::DTC_Registers::SetExternalSystemClock()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::SetInternalSystemClock()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[1] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadSystemClock()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[1];
}

void DTCLib::DTC_Registers::EnableTiming()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableTiming()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	data[0] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadTimingEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DTCControl);
	return data[0];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDTCControl()
{
	auto form = CreateFormatter(DTC_Register_DTCControl);
	form.description = "DTC Control";
	form.vals.push_back(std::string("Reset:                   [") + (ReadResetDTC() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Emulation Enable:    [") + (ReadCFOEmulation() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR Write Address: [") + (ReadResetDDRWriteAddress() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR Read Address:  [") + (ReadResetDDRReadAddress() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Reset DDR:               [") + (ReadResetDDR() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Emulator DRP Enable: [") + (ReadCFOEmulatorDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("CFO Autogenerate DRP:    [") + (ReadAutogenDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Software DRP Enable:     [") + (ReadSoftwareDRP() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DCS Enable:              [") + (ReadDCSReception() ? "x" : " ") + "]");
	form.vals.push_back(std::string("System Clock Select:     [") + (ReadSystemClock() ? "Ext" : "Int") + "]");
	form.vals.push_back(std::string("Timing Enable:           [") + (ReadTimingEnable() ? "x" : " ") + "]");
	return form;
}


// DMA Transfer Length Register
void DTCLib::DTC_Registers::SetTriggerDMATransferLength(uint16_t length)
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = (data & 0x0000FFFF) + (length << 16);
	WriteRegister_(data, DTC_Register_DMATransferLength);
}

uint16_t DTCLib::DTC_Registers::ReadTriggerDMATransferLength()
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data >>= 16;
	return static_cast<uint16_t>(data);
}

void DTCLib::DTC_Registers::SetMinDMATransferLength(uint16_t length)
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = (data & 0xFFFF0000) + length;
	WriteRegister_(data, DTC_Register_DMATransferLength);
}

uint16_t DTCLib::DTC_Registers::ReadMinDMATransferLength()
{
	auto data = ReadRegister_(DTC_Register_DMATransferLength);
	data = data & 0x0000FFFF;
	dmaSize_ = static_cast<uint16_t>(data);
	return dmaSize_;
}

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
void DTCLib::DTC_Registers::SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESLoopbackEnable);
	std::bitset<3> modeSet = mode;
	data[3 * ring] = modeSet[0];
	data[3 * ring + 1] = modeSet[1];
	data[3 * ring + 2] = modeSet[2];
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESLoopbackEnable);
}

DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC_Registers::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = ReadRegister_(DTC_Register_SERDESLoopbackEnable) >> 3 * ring;
	return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESLoopbackEnable()
{
	auto form = CreateFormatter(DTC_Register_SERDESLoopbackEnable);
	form.description = "SERDES Loopback Enable";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": " + DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString());
	}
	form.vals.push_back(std::string("CFO:    ") + DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_CFO)).toString());
	form.vals.push_back(std::string("EVB:    ") + DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_EVB)).toString());
	return form;
}

// Clock Status Register
bool DTCLib::DTC_Registers::ReadSERDESOscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ClockOscillatorStatus);
	return dataSet[2];
}

bool DTCLib::DTC_Registers::ReadSERDESOscillatorInitializationComplete()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ClockOscillatorStatus);
	return dataSet[1];
}

bool DTCLib::DTC_Registers::WaitForSERDESOscillatorInitializationComplete(double max_wait)
{
	auto start_time = std::chrono::steady_clock::now();
	while (!ReadSERDESOscillatorInitializationComplete() &&
		   std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start_time).count() < max_wait)
	{
		usleep(1000);
	}
	return ReadSERDESOscillatorInitializationComplete();
}

bool DTCLib::DTC_Registers::ReadDDROscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ClockOscillatorStatus);
	return dataSet[18];
}
bool DTCLib::DTC_Registers::ReadDDROscillatorInitializationComplete()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ClockOscillatorStatus);
	return dataSet[17];
}

bool DTCLib::DTC_Registers::WaitForDDROscillatorInitializationComplete(double max_wait)
{
	auto start_time = std::chrono::steady_clock::now();
	while (!ReadDDROscillatorInitializationComplete() &&
		   std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start_time).count() < max_wait)
	{
		usleep(1000);
	}
	return ReadDDROscillatorInitializationComplete();
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatClockOscillatorStatus()
{
	auto form = CreateFormatter(DTC_Register_ClockOscillatorStatus);
	form.description = "Clock Oscillator Status";
	form.vals.push_back(std::string("SERDES IIC Error:     [") + (ReadSERDESOscillatorIICError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("SERDES Init.Complete: [") + (ReadSERDESOscillatorInitializationComplete() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DDR IIC Error:        [") + (ReadDDROscillatorIICError() ? "x" : " ") + "]");
	form.vals.push_back(std::string("DDR Init.Complete:    [") + (ReadDDROscillatorInitializationComplete() ? "x" : " ") + "]");
	return form;

}
// ROC Emulation Enable Register
void DTCLib::DTC_Registers::EnableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 1;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
}

void DTCLib::DTC_Registers::DisableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 0;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
}

bool DTCLib::DTC_Registers::ReadROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCEmulationEnable);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCEmulationEnable()
{
	auto form = CreateFormatter(DTC_Register_ROCEmulationEnable);
	form.description = "ROC Emulator Enable";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadROCEmulator(r) ? "x" : " ") + "]");
	}
	return form;
}

// Ring Enable Register
void DTCLib::DTC_Registers::EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_RingEnable);
	data[ring] = mode.TransmitEnable;
	data[ring + 8] = mode.ReceiveEnable;
	data[ring + 16] = mode.TimingEnable;
	WriteRegister_(data.to_ulong(), DTC_Register_RingEnable);
	SetMaxROCNumber(ring, lastRoc);
}

void DTCLib::DTC_Registers::DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_RingEnable);
	data[ring] = data[ring] && !mode.TransmitEnable;
	data[ring + 8] = data[ring + 8] && !mode.ReceiveEnable;
	data[ring + 16] = data[ring + 16] && !mode.TimingEnable;
	WriteRegister_(data.to_ulong(), DTC_Register_RingEnable);
}

DTCLib::DTC_RingEnableMode DTCLib::DTC_Registers::ReadRingEnabled(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_RingEnable);
	return DTC_RingEnableMode(dataSet[ring], dataSet[ring + 8], dataSet[ring + 16]);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRingEnable()
{
	auto form = CreateFormatter(DTC_Register_RingEnable);
	form.description = "Ring Enable";
	form.vals.push_back("       ([TX, RX, Timing])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadRingEnabled(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (re.TransmitEnable ? "x" : " ") + ","
							+ (re.ReceiveEnable ? "x" : " ") + ","
							+ (re.TimingEnable ? "x" : " ") + "]");
	}
	{
		auto ce = ReadRingEnabled(DTC_Ring_CFO);
		form.vals.push_back(std::string("CFO:    [") + "TX:[" + (ce.TransmitEnable ? "x" : " ") + "], " + "RX:[" + (ce.ReceiveEnable ? "x" : " ") + "]]");
	}
	{
		auto ee = ReadRingEnabled(DTC_Ring_EVB);
		form.vals.push_back(std::string("EVB:    [") + "TX:[" + (ee.TransmitEnable ? "x" : " ") + "], " + "RX:[" + (ee.ReceiveEnable ? "x" : " ") + "]]");
	}
	return form;
}

// SERDES Reset Register
void DTCLib::DTC_Registers::ResetSERDES(const DTC_Ring_ID& ring, int interval)
{
	auto resetDone = false;
	while (!resetDone)
	{
		TRACE(0, "Entering SERDES Reset Loop for Ring %u", ring);
		std::bitset<32> data = ReadRegister_(DTC_Register_SERDESReset);
		data[ring] = 1;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		data = ReadRegister_(DTC_Register_SERDESReset);
		data[ring] = 0;
		WriteRegister_(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		resetDone = ReadResetSERDESDone(ring);
		TRACE(0, "End of SERDES Reset loop, done=%s", (resetDone ? "true" : "false"));
	}
}

bool DTCLib::DTC_Registers::ReadResetSERDES(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESReset);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESReset()
{
	auto form = CreateFormatter(DTC_Register_SERDESReset);
	form.description = "SERDES Reset";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadResetSERDES(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadResetSERDES(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadResetSERDES(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES RX Disparity Error Register
DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC_Registers::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXDisparityError(ReadRegister_(DTC_Register_SERDESRXDisparityError), ring);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXDisparityError()
{
	auto form = CreateFormatter(DTC_Register_SERDESRXDisparityError);
	form.description = "SERDES RX Disparity Error";
	form.vals.push_back("       ([H,L])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadSERDESRXDisparityError(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + to_string(re.GetData()[1]) + "," + to_string(re.GetData()[0]) + "]");
	}
	auto ce = ReadSERDESRXDisparityError(DTC_Ring_CFO);
	form.vals.push_back(std::string("CFO:    [") + to_string(ce.GetData()[1]) + "," + to_string(ce.GetData()[0]) + "]");
	auto ee = ReadSERDESRXDisparityError(DTC_Ring_EVB);
	form.vals.push_back(std::string("EVB:    [") + to_string(ee.GetData()[1]) + "," + to_string(ee.GetData()[0]) + "]");
	return form;
}

// SERDES RX Character Not In Table Error Register
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC_Registers::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	return DTC_CharacterNotInTableError(ReadRegister_(DTC_Register_SERDESRXCharacterNotInTableError), ring);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXCharacterNotInTableError()
{
	auto form = CreateFormatter(DTC_Register_SERDESRXCharacterNotInTableError);
	form.description = "SERDES RX CNIT Error";
	form.vals.push_back("       ([H,L])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadSERDESRXCharacterNotInTableError(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + to_string(re.GetData()[1]) + "," + to_string(re.GetData()[0]) + "]");
	}
	auto ce = ReadSERDESRXCharacterNotInTableError(DTC_Ring_CFO);
	form.vals.push_back(std::string("CFO:    [") + to_string(ce.GetData()[1]) + "," + to_string(ce.GetData()[0]) + "]");
	auto ee = ReadSERDESRXCharacterNotInTableError(DTC_Ring_EVB);
	form.vals.push_back(std::string("EVB:    [") + to_string(ee.GetData()[1]) + "," + to_string(ee.GetData()[0]) + "]");
	return form;
}

// SERDES Unlock Error Register
bool DTCLib::DTC_Registers::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESUnlockError);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESUnlockError()
{
	auto form = CreateFormatter(DTC_Register_SERDESUnlockError);
	form.description = "SERDES Unlock Error";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadSERDESUnlockError(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESUnlockError(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESUnlockError(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES PLL Locked Register
bool DTCLib::DTC_Registers::ReadSERDESPLLLocked(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESPLLLocked);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESPLLLocked()
{
	auto form = CreateFormatter(DTC_Register_SERDESPLLLocked);
	form.description = "SERDES PLL Locked";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadSERDESPLLLocked(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESPLLLocked(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESPLLLocked(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES TX Buffer Status Register
bool DTCLib::DTC_Registers::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2 + 1];
}

bool DTCLib::DTC_Registers::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESTXBufferStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDESTXBufferStatus);
	form.description = "SERDES TX Buffer Status";
	form.vals.push_back("       ([OF or UF, FIFO Half Full])");
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (ReadSERDESOverflowOrUnderflow(r) ? "x" : " ") + ","
							+ (ReadSERDESBufferFIFOHalfFull(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [")
						+ (ReadSERDESOverflowOrUnderflow(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadSERDESBufferFIFOHalfFull(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [")
						+ (ReadSERDESOverflowOrUnderflow(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadSERDESBufferFIFOHalfFull(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES RX Buffer Status Register
DTCLib::DTC_RXBufferStatus DTCLib::DTC_Registers::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = ReadRegister_(DTC_Register_SERDESRXBufferStatus) >> 3 * ring;
	return static_cast<DTC_RXBufferStatus>(dataSet.to_ulong());
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXBufferStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDESRXBufferStatus);
	form.description = "SERDES RX Buffer Status";
	for (auto r : DTC_Rings)
	{
		auto re = ReadSERDESRXBufferStatus(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": " + DTC_RXBufferStatusConverter(re).toString());
	}

	auto ce = ReadSERDESRXBufferStatus(DTC_Ring_CFO);
	form.vals.push_back(std::string("CFO:    ") + DTC_RXBufferStatusConverter(ce).toString());


	auto ee = ReadSERDESRXBufferStatus(DTC_Ring_EVB);
	form.vals.push_back(std::string("EVB:    ") + DTC_RXBufferStatusConverter(ee).toString());

	return form;
}

// SERDES RX Status Register
DTCLib::DTC_RXStatus DTCLib::DTC_Registers::ReadSERDESRXStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = ReadRegister_(DTC_Register_SERDESRXStatus) >> 3 * ring;
	return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESRXStatus()
{
	auto form = CreateFormatter(DTC_Register_SERDESRXStatus);
	form.description = "SERDES RX Status";
	for (auto r : DTC_Rings)
	{
		auto re = ReadSERDESRXStatus(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": " + DTC_RXStatusConverter(re).toString());
	}
	auto ce = ReadSERDESRXStatus(DTC_Ring_CFO);
	form.vals.push_back(std::string("CFO:    ") + DTC_RXStatusConverter(ce).toString());
	auto ee = ReadSERDESRXStatus(DTC_Ring_EVB);
	form.vals.push_back(std::string("EVB:    ") + DTC_RXStatusConverter(ee).toString());

	return form;
}

// SERDES Reset Done Register
bool DTCLib::DTC_Registers::ReadResetSERDESDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESResetDone);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESResetDone()
{
	auto form = CreateFormatter(DTC_Register_SERDESResetDone);
	form.description = "SERDES Reset Done";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadResetSERDESDone(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadResetSERDESDone(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadResetSERDESDone(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SERDES Eyescan Data Error Register
bool DTCLib::DTC_Registers::ReadSERDESEyescanError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SERDESEyescanData);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESEyescanData()
{
	auto form = CreateFormatter(DTC_Register_SERDESEyescanData);
	form.description = "SERDES Eyescan Data Error";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadSERDESEyescanError(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESEyescanError(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESEyescanError(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// SFP / SERDES Status Register

bool DTCLib::DTC_Registers::ReadSERDESSFPPresent(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SFPSERDESStatus);
	return dataSet[ring + 24];
}

bool DTCLib::DTC_Registers::ReadSERDESSFPLOS(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SFPSERDESStatus);
	return dataSet[ring + 16];
}

bool DTCLib::DTC_Registers::ReadSERDESSFPTXFault(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SFPSERDESStatus);
	return dataSet[ring + 8];
}


bool DTCLib::DTC_Registers::ReadSERDESRXCDRLock(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_SFPSERDESStatus);
	return dataSet[ring];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSFPSERDESStatus()
{
	auto form = CreateFormatter(DTC_Register_SFPSERDESStatus);
	form.description = "SFP / SERDES Status";
	form.vals.push_back("       ([SFP Present, LOS, TX Fault, CDR Lock])");
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadSERDESSFPPresent(r) ? "x" : " ") + ","
							+ (ReadSERDESSFPLOS(r) ? "x" : " ") + ","
							+ (ReadSERDESSFPTXFault(r) ? "x" : " ") + ","
							+ (ReadSERDESRXCDRLock(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadSERDESSFPPresent(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadSERDESSFPLOS(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadSERDESSFPTXFault(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadSERDESRXCDRLock(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadSERDESSFPPresent(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadSERDESSFPLOS(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadSERDESSFPTXFault(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadSERDESRXCDRLock(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// DMA Timeout Preset Register
void DTCLib::DTC_Registers::SetDMATimeoutPreset(uint32_t preset)
{
	WriteRegister_(preset, DTC_Register_DMATimeoutPreset);
}

uint32_t DTCLib::DTC_Registers::ReadDMATimeoutPreset()
{
	return ReadRegister_(DTC_Register_DMATimeoutPreset);
}

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
uint32_t DTCLib::DTC_Registers::ReadROCTimeoutPreset()
{
	return ReadRegister_(DTC_Register_ROCReplyTimeout);
}

void DTCLib::DTC_Registers::SetROCTimeoutPreset(uint32_t preset)
{
	WriteRegister_(preset, DTC_Register_ROCReplyTimeout);
}

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
bool DTCLib::DTC_Registers::ReadROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ROCReplyTimeoutError);
	return data[static_cast<int>(ring)];
}

void DTCLib::DTC_Registers::ClearROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = 0x0;
	data[ring] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_ROCReplyTimeoutError);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCReplyTimeoutError()
{
	auto form = CreateFormatter(DTC_Register_ROCReplyTimeoutError);
	form.description = "ROC Reply Timeout Error";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadROCTimeoutError(r) ? "x" : " ") + "]");
	}
	return form;
}

// Ring Packet Length Register
void DTCLib::DTC_Registers::SetPacketSize(uint16_t packetSize)
{
	WriteRegister_(0x00000000 + packetSize, DTC_Register_RingPacketLength);
}

uint16_t DTCLib::DTC_Registers::ReadPacketSize()
{
	return static_cast<uint16_t>(ReadRegister_(DTC_Register_RingPacketLength));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRingPacketLength()
{
	auto form = CreateFormatter(DTC_Register_RingPacketLength);
	form.description = "DMA Ring Packet Length";
	std::stringstream o;
	o << "0x" << std::hex << ReadPacketSize();
	form.vals.push_back(o.str());
	return form;
}


// EVB Network Partition ID / EVB Network Local MAC Index Register
void DTCLib::DTC_Registers::SetEVBMode(uint8_t mode)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF00FFFF;
	regVal += mode << 16;
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

uint8_t DTCLib::DTC_Registers::ReadEVBMode()
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF0000;
	return static_cast<uint8_t>(regVal >> 16);
}

void DTCLib::DTC_Registers::SetEVBLocalParitionID(uint8_t id)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFFFFFCFF;
	regVal += (id & 0x3) << 8;
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

uint8_t DTCLib::DTC_Registers::ReadEVBLocalParitionID()
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFF0000;
	return static_cast<uint8_t>((regVal >> 8) & 0x3);
}

void DTCLib::DTC_Registers::SetEVBLocalMACAddress(uint8_t macByte)
{
	auto regVal = ReadRegister_(DTC_Register_EVBPartitionID) & 0xFFFFFFC0;
	regVal += (macByte & 0x3F);
	WriteRegister_(regVal, DTC_Register_EVBPartitionID);
}

uint8_t DTCLib::DTC_Registers::ReadEVBLocalMACAddress()
{
	return ReadRegister_(DTC_Register_EVBPartitionID) & 0x3F;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBLocalParitionIDMACIndex()
{
	auto form = CreateFormatter(DTC_Register_EVBPartitionID);
	form.description = "EVB Local Partition ID / MAC Index";
	std::ostringstream o;
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

void DTCLib::DTC_Registers::SetEVBStartNode(uint8_t node)
{
	auto regVal = ReadRegister_(DTC_Register_EVBDestCount) & 0xFFFFC0FF;
	regVal += (node & 0x3F) << 8;
	WriteRegister_(regVal, DTC_Register_EVBDestCount);
}

uint8_t DTCLib::DTC_Registers::ReadEVBStartNode()
{
	return static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBDestCount) & 0x3F00) >> 8);
}

void DTCLib::DTC_Registers::SetEVBNumberOfDestinationNodes(uint8_t number)
{
	auto regVal = ReadRegister_(DTC_Register_EVBDestCount) & 0xFFFFFFC0;
	regVal += (number & 0x3F);
	WriteRegister_(regVal, DTC_Register_EVBDestCount);
}

uint8_t DTCLib::DTC_Registers::ReadEVBNumberOfDestinationNodes()
{
	return static_cast<uint8_t>(ReadRegister_(DTC_Register_EVBDestCount) & 0x3F);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEVBNumberOfDestinationNodes()
{
	auto form = CreateFormatter(DTC_Register_EVBDestCount);
	form.description = "EVB Number of Destination Nodes / Start Node";
	std::stringstream o;
	o << "EVB Start Node: " << std::dec << static_cast<int>(ReadEVBStartNode());
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "EVB Number of Destination Nodes: " << std::dec << static_cast<int>(ReadEVBNumberOfDestinationNodes());
	form.vals.push_back(o.str());
	return form;
}

// Heartbeat Error Register
bool DTCLib::DTC_Registers::ReadHeartbeatTimeout(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_HeartbeatErrorFlags);
	switch (ring)
	{
	case DTC_Ring_0:
		return data[24];
	case DTC_Ring_1:
		return data[25];
	case DTC_Ring_2:
		return data[26];
	case DTC_Ring_3:
		return data[27];
	case DTC_Ring_4:
		return data[28];
	case DTC_Ring_5:
		return data[29];
	default:
		return false;
	}
}

bool DTCLib::DTC_Registers::ReadHeartbeat20Mismatch(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_HeartbeatErrorFlags);
	switch (ring)
	{
	case DTC_Ring_0:
		return data[16];
	case DTC_Ring_1:
		return data[17];
	case DTC_Ring_2:
		return data[18];
	case DTC_Ring_3:
		return data[19];
	case DTC_Ring_4:
		return data[20];
	case DTC_Ring_5:
		return data[21];
	default:
		return false;
	}
}

bool DTCLib::DTC_Registers::ReadHeartbeat12Mismatch(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_HeartbeatErrorFlags);
	switch (ring)
	{
	case DTC_Ring_0:
		return data[8];
	case DTC_Ring_1:
		return data[9];
	case DTC_Ring_2:
		return data[10];
	case DTC_Ring_3:
		return data[11];
	case DTC_Ring_4:
		return data[12];
	case DTC_Ring_5:
		return data[13];
	default:
		return false;
	}
}

bool DTCLib::DTC_Registers::ReadHeartbeat01Mismatch(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_HeartbeatErrorFlags);
	switch (ring)
	{
	case DTC_Ring_0:
		return data[0];
	case DTC_Ring_1:
		return data[1];
	case DTC_Ring_2:
		return data[2];
	case DTC_Ring_3:
		return data[3];
	case DTC_Ring_4:
		return data[4];
	case DTC_Ring_5:
		return data[5];
	default:
		return false;
	}
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatHeartbeatError()
{
	auto form = CreateFormatter(DTC_Register_HeartbeatErrorFlags);
	form.description = "Heartbeat Error Flags";
	form.vals.push_back("       ([Timeout, 2-0 Mismatch, 1-2 Mismatch, 0-1 Mismatch])");
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (ReadHeartbeatTimeout(r) ? "x" : " ") + ","
							+ (ReadHeartbeat20Mismatch(r) ? "x" : " ") + ","
							+ (ReadHeartbeat12Mismatch(r) ? "x" : " ") + ","
							+ (ReadHeartbeat01Mismatch(r) ? "x" : " ") + "]");
	}
	return form;
}

// SEREDES Oscillator Registers
uint32_t DTCLib::DTC_Registers::ReadSERDESOscillatorFrequency()
{
	return ReadRegister_(DTC_Register_SERDESOscillatorFrequency);
}
void DTCLib::DTC_Registers::SetSERDESOscillatorFrequency(uint32_t freq)
{
	WriteRegister_(freq, DTC_Register_SERDESOscillatorFrequency);
}
bool DTCLib::DTC_Registers::ReadSERDESOscillaotrIICFSMEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	return data[31];
}
void DTCLib::DTC_Registers::EnableSERDESOscillatorIICFSM()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	data[31] = true;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESOscillatorControl);
}
void DTCLib::DTC_Registers::DisableSERDESOscillatorIICFSM()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	data[31] = false;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESOscillatorControl);
}
bool DTCLib::DTC_Registers::ReadSERDESOscillatorReadWriteMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	return data[0];
}
void DTCLib::DTC_Registers::SetSERDESOscillatorWriteMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	data[0] = true;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESOscillatorControl);
}
void DTCLib::DTC_Registers::SetSERDESOscillatorReadMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_SERDESOscillatorControl);
	data[0] = false;
	WriteRegister_(data.to_ulong(), DTC_Register_SERDESOscillatorControl);
}
uint64_t DTCLib::DTC_Registers::ReadSERDESOscillatorParameters()
{
	SetSERDESOscillatorReadMode();
	EnableSERDESOscillatorIICFSM();
	usleep(10000);
	DisableSERDESOscillatorIICFSM();
	WaitForSERDESOscillatorInitializationComplete();
	return (static_cast<uint64_t>(ReadRegister_(DTC_Register_SERDESOscillatorParameterHigh)) << 32) + ReadRegister_(DTC_Register_SERDESOscillatorParameterLow);
}
void DTCLib::DTC_Registers::SetSERDESOscillatorParameters(uint64_t parameters)
{
	SetSERDESOscillatorWriteMode();
	WriteRegister_(parameters >> 32, DTC_Register_SERDESOscillatorParameterHigh);
	WriteRegister_(static_cast<uint32_t>(parameters), DTC_Register_SERDESOscillatorParameterLow);
	EnableSERDESOscillatorIICFSM();
	usleep(10000);
	DisableSERDESOscillatorIICFSM();
	WaitForSERDESOscillatorInitializationComplete();
}

DTCLib::DTC_SerdesClockSpeed DTCLib::DTC_Registers::ReadSERDESOscillatorClock()
{
	auto freq = ReadSERDESOscillatorFrequency();

	//Clocks should be accurate to 30 ppm
	if (freq > 156250000 - 4687.5 && freq < 156250000 + 4687.5)
		return DTC_SerdesClockSpeed_3125Gbps;
	if (freq > 125000000 - 3750 && freq < 125000000 + 3750)
		return DTC_SerdesClockSpeed_25Gbps;
	return DTC_SerdesClockSpeed_Unknown;
}
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
	SetNewOscillatorFrequency(DTC_OscillatorType_SERDES, targetFreq);
	for (auto & ring : DTC_Rings)
	{
		ResetSERDES(ring, 1000);
	}
	ResetSERDES(DTC_Ring_CFO, 1000);
	ResetSERDES(DTC_Ring_EVB, 1000);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorFrequency()
{
	auto form = CreateFormatter(DTC_Register_SERDESOscillatorFrequency);
	form.description = "SERDES Oscillator Frequency";
	std::stringstream o;
	o << std::dec << ReadSERDESOscillatorFrequency();
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorControl()
{
	auto form = CreateFormatter(DTC_Register_SERDESOscillatorControl);
	form.description = "SERDES Oscillator Control";
	form.vals.push_back(std::string("IIC FSM Active:  [") + (ReadSERDESOscillaotrIICFSMEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read/Write Mode: [") + (ReadSERDESOscillatorReadWriteMode() ? "W" : "R") + "]");
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorParameterLow()
{
	ReadSERDESOscillatorParameters();
	auto form = CreateFormatter(DTC_Register_SERDESOscillatorParameterLow);
	form.description = "SERDES Oscillator RFREQ 31:0";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_SERDESOscillatorParameterLow);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatSERDESOscillatorParameterHigh()
{
	ReadSERDESOscillatorParameters();
	auto form = CreateFormatter(DTC_Register_SERDESOscillatorParameterHigh);
	form.description = "SERDES Oscillator Parameters";
	std::stringstream o1, o2, o3, o4;
	auto hsdiv = (ReadRegister_(DTC_Register_SERDESOscillatorParameterHigh) >> 16) & 0x7;
	o1 << "HSDIV:       " << std::dec << hsdiv << " (" << DecodeHighSpeedDivider_(hsdiv) << ")";
	form.vals.push_back(o1.str());
	auto n1 = (ReadRegister_(DTC_Register_SERDESOscillatorParameterHigh) >> 8) & 0x7F;
	o2 << "N1:          " << std::dec << n1 << " (" << DecodeOutputDivider_(n1) << ")";
	form.vals.push_back(o2.str());
	o3 << "RFREQ 37:32: " << std::hex << (ReadRegister_(DTC_Register_SERDESOscillatorParameterHigh) & 0xFF);
	form.vals.push_back(o3.str());
	o4 << "RFREQ: " << std::dec << DecodeRFREQ_((static_cast<uint64_t>(ReadRegister_(DTC_Register_SERDESOscillatorParameterHigh) & 0x3F) << 32) + ReadRegister_(DTC_Register_SERDESOscillatorParameterLow));
	form.vals.push_back(o4.str());
	return form;
}

// DDR Oscillator Registers
uint32_t DTCLib::DTC_Registers::ReadDDROscillatorFrequency()
{
	return ReadRegister_(DTC_Register_DDROscillatorFrequency);
}
void DTCLib::DTC_Registers::SetDDROscillatorFrequency(uint32_t freq)
{
	WriteRegister_(freq, DTC_Register_DDROscillatorFrequency);
}
bool DTCLib::DTC_Registers::ReadDDROscillaotrIICFSMEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	return data[31];
}
void DTCLib::DTC_Registers::EnableDDROscillatorIICFSM()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	data[31] = true;
	WriteRegister_(data.to_ulong(), DTC_Register_DDROscillatorControl);
}
void DTCLib::DTC_Registers::DisableDDROscillatorIICFSM()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	data[31] = false;
	WriteRegister_(data.to_ulong(), DTC_Register_DDROscillatorControl);
}
bool DTCLib::DTC_Registers::ReadDDROscillatorReadWriteMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	return data[0];
}
void DTCLib::DTC_Registers::SetDDROscillatorWriteMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	data[0] = true;
	WriteRegister_(data.to_ulong(), DTC_Register_DDROscillatorControl);
}
void DTCLib::DTC_Registers::SetDDROscillatorReadMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DDROscillatorControl);
	data[0] = false;
	WriteRegister_(data.to_ulong(), DTC_Register_DDROscillatorControl);
}
uint64_t DTCLib::DTC_Registers::ReadDDROscillatorParameters()
{
	SetDDROscillatorReadMode();
	EnableDDROscillatorIICFSM();
	usleep(10000);
	DisableDDROscillatorIICFSM();
	WaitForDDROscillatorInitializationComplete();
	return (static_cast<uint64_t>(ReadRegister_(DTC_Register_DDROscillatorParameterHigh)) << 32) + ReadRegister_(DTC_Register_DDROscillatorParameterLow);
}
void DTCLib::DTC_Registers::SetDDROscillatorParameters(uint64_t parameters)
{
	SetDDROscillatorWriteMode();
	WriteRegister_(parameters >> 32, DTC_Register_DDROscillatorParameterHigh);
	WriteRegister_(static_cast<uint32_t>(parameters), DTC_Register_DDROscillatorParameterLow);
	EnableDDROscillatorIICFSM();
	usleep(10000);
	DisableDDROscillatorIICFSM();
	WaitForDDROscillatorInitializationComplete();
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorFrequency()
{
	auto form = CreateFormatter(DTC_Register_DDROscillatorFrequency);
	form.description = "DDR Oscillator Frequency";
	std::stringstream o;
	o << std::dec << ReadDDROscillatorFrequency();
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorControl()
{
	auto form = CreateFormatter(DTC_Register_DDROscillatorControl);
	form.description = "DDR Oscillator Control";
	form.vals.push_back(std::string("IIC FSM Active:  [") + (ReadDDROscillaotrIICFSMEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Read/Write Mode: [") + (ReadDDROscillatorReadWriteMode() ? "W" : "R") + "]");
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorParameterLow()
{
	ReadDDROscillatorParameters();
	auto form = CreateFormatter(DTC_Register_DDROscillatorParameterLow);
	form.description = "DDR Oscillator RFREQ 31:0";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDROscillatorParameterLow);
	form.vals.push_back(o.str());
	return form;
}
DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDROscillatorParameterHigh()
{
	ReadDDROscillatorParameters();
	auto form = CreateFormatter(DTC_Register_DDROscillatorParameterHigh);
	form.description = "DDR Oscillator Parameters";
	std::stringstream o1, o2, o3, o4;
	auto hsdiv = (ReadRegister_(DTC_Register_DDROscillatorParameterHigh) >> 16) & 0x7;
	o1 << "HSDIV:       " << std::dec << hsdiv << " (" << DecodeHighSpeedDivider_(hsdiv) << ")";
	form.vals.push_back(o1.str());
	auto n1 = (ReadRegister_(DTC_Register_DDROscillatorParameterHigh) >> 8) & 0x7F;
	o2 << "N1:          " << std::dec << n1 << " (" << DecodeOutputDivider_(n1) << ")";
	form.vals.push_back(o2.str());
	o3 << "RFREQ 37:32: " << std::hex << (ReadRegister_(DTC_Register_DDROscillatorParameterHigh) & 0xFF);
	form.vals.push_back(o3.str());
	o4 << "RFREQ: " << std::dec << DecodeRFREQ_((static_cast<uint64_t>(ReadRegister_(DTC_Register_DDROscillatorParameterHigh) & 0x3F) << 32) + ReadRegister_(DTC_Register_DDROscillatorParameterLow));
	form.vals.push_back(o4.str());
	return form;
}

// Timestamp Preset Registers
void DTCLib::DTC_Registers::SetTimestampPreset(const DTC_Timestamp& preset)
{
	auto timestamp = preset.GetTimestamp();
	auto timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	auto timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister_(timestampLow, DTC_Register_TimestampPreset0);
	WriteRegister_(timestampHigh, DTC_Register_TimestampPreset1);
}

DTCLib::DTC_Timestamp DTCLib::DTC_Registers::ReadTimestampPreset()
{
	auto timestampLow = ReadRegister_(DTC_Register_TimestampPreset0);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister_(DTC_Register_TimestampPreset1)));
	return output;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTimestampPreset0()
{
	auto form = CreateFormatter(DTC_Register_TimestampPreset0);
	form.description = "Timestamp Preset 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_TimestampPreset0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTimestampPreset1()
{
	auto form = CreateFormatter(DTC_Register_TimestampPreset1);
	form.description = "Timestamp Preset 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_TimestampPreset1);
	form.vals.push_back(o.str());
	return form;
}

// Data Pending Timer Register
void DTCLib::DTC_Registers::SetDataPendingTimer(uint32_t timer)
{
	WriteRegister_(timer, DTC_Register_DataPendingTimer);
}

uint32_t DTCLib::DTC_Registers::ReadDataPendingTimer()
{
	return ReadRegister_(DTC_Register_DataPendingTimer);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDataPendingTimer()
{
	auto form = CreateFormatter(DTC_Register_DataPendingTimer);
	form.description = "DMA Data Pending Timer";
	std::stringstream o;
	o << "0x" << std::hex << ReadDataPendingTimer();
	form.vals.push_back(o.str());
	return form;
}

// NUMROCs Register
void DTCLib::DTC_Registers::SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> ringRocs = ReadRegister_(DTC_Register_NUMROCs);
	maxROCs_[ring] = lastRoc;
	auto numRocs = lastRoc == DTC_ROC_Unused ? 0 : lastRoc + 1;
	ringRocs[ring * 3] = numRocs & 1;
	ringRocs[ring * 3 + 1] = ((numRocs & 2) >> 1) & 1;
	ringRocs[ring * 3 + 2] = ((numRocs & 4) >> 2) & 1;
	WriteRegister_(ringRocs.to_ulong(), DTC_Register_NUMROCs);
}

DTCLib::DTC_ROC_ID DTCLib::DTC_Registers::ReadRingROCCount(const DTC_Ring_ID& ring, bool local)
{
	if (local)
	{
		return maxROCs_[ring];
	}
	std::bitset<32> ringRocs = ReadRegister_(DTC_Register_NUMROCs);
	auto number = ringRocs[ring * 3] + (ringRocs[ring * 3 + 1] << 1) + (ringRocs[ring * 3 + 2] << 2);
	return DTC_ROCS[number];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatNUMROCs()
{
	auto form = CreateFormatter(DTC_Register_NUMROCs);
	form.description = "NUMROCs";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + " NUMROCs:    " + DTC_ROCIDConverter(ReadRingROCCount(r, false)).toString());
	}
	return form;
}

// FIFO Full Error Flags Registers
void DTCLib::DTC_Registers::ClearFIFOFullErrorFlags(const DTC_Ring_ID& ring)
{
	auto flags = ReadFIFOFullErrorFlags(ring);
	std::bitset<32> data0 = 0;
	std::bitset<32> data1 = 0;
	std::bitset<32> data2 = 0;

	data0[ring] = flags.OutputData;
	data0[ring + 8] = flags.CFOLinkInput;
	data0[ring + 16] = flags.ReadoutRequestOutput;
	data0[ring + 24] = flags.DataRequestOutput;
	data1[ring] = flags.OtherOutput;
	data1[ring + 8] = flags.OutputDCS;
	data1[ring + 16] = flags.OutputDCSStage2;
	data1[ring + 24] = flags.DataInput;
	data2[ring] = flags.DCSStatusInput;

	WriteRegister_(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister_(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister_(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);
}

DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC_Registers::ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring)
{
	std::bitset<32> data0 = ReadRegister_(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister_(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister_(DTC_Register_FIFOFullErrorFlag2);
	DTC_FIFOFullErrorFlags flags;

	flags.OutputData = data0[ring];
	flags.CFOLinkInput = data0[ring + 8];
	flags.ReadoutRequestOutput = data0[ring + 16];
	flags.DataRequestOutput = data0[ring + 24];
	flags.OtherOutput = data1[ring];
	flags.OutputDCS = data1[ring + 8];
	flags.OutputDCSStage2 = data1[ring + 16];
	flags.DataInput = data1[ring + 24];
	flags.DCSStatusInput = data2[ring];

	return flags;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag0()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag0);
	form.description = "FIFO Full Error Flags 0";
	form.vals.push_back("       ([DataRequest, ReadoutRequest, CFOLink, OutputData])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (re.DataRequestOutput ? "x" : " ") + ","
							+ (re.ReadoutRequestOutput ? "x" : " ") + ","
							+ (re.CFOLinkInput ? "x" : " ") + ","
							+ (re.OutputData ? "x" : " ") + "]");
	}
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag1()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag1);
	form.description = "FIFO Full Error Flags 1";
	form.vals.push_back("       ([DataInput, OutputDCSStage2, OutputDCS, OtherOutput])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (re.DataInput ? "x" : " ") + ","
							+ (re.OutputDCSStage2 ? "x" : " ") + ","
							+ (re.OutputDCS ? "x" : " ") + ","
							+ (re.OtherOutput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
		form.vals.push_back(std::string("CFO:    [") +
							+(ce.DataInput ? "x" : " ") + ","
							+ (ce.OutputDCSStage2 ? "x" : " ") + ","
							+ (ce.OutputDCS ? "x" : " ") + ","
							+ (ce.OtherOutput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Ring_EVB);
		form.vals.push_back(std::string("EVB:    [") +
							+(ce.DataInput ? "x" : " ") + ","
							+ (ce.OutputDCSStage2 ? "x" : " ") + ","
							+ (ce.OutputDCS ? "x" : " ") + ","
							+ (ce.OtherOutput ? "x" : " ") + "]");
	}
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFIFOFullErrorFlag2()
{
	auto form = CreateFormatter(DTC_Register_FIFOFullErrorFlag2);
	form.description = "FIFO Full Error Flags 2";
	form.vals.push_back("       ([DCSStatusInput])");
	for (auto r : DTC_Rings)
	{
		auto re = ReadFIFOFullErrorFlags(r);
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (re.DCSStatusInput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
		form.vals.push_back(std::string("CFO:    [") + (ce.DCSStatusInput ? "x" : " ") + "]");
	}
	{
		auto ce = ReadFIFOFullErrorFlags(DTC_Ring_EVB);
		form.vals.push_back(std::string("EVB:    [") + (ce.DCSStatusInput ? "x" : " ") + "]");
	}
	return form;
}

// Receive Packet Error Register
void DTCLib::DTC_Registers::ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(ring) + 24] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

bool DTCLib::DTC_Registers::ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(ring) + 24];
}

void DTCLib::DTC_Registers::ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(ring) + 16] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

bool DTCLib::DTC_Registers::ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(ring) + 16];
}

void DTCLib::DTC_Registers::ClearPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(ring) + 8] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

bool DTCLib::DTC_Registers::ReadPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(ring) + 8];
}

void DTCLib::DTC_Registers::ClearPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	data[static_cast<int>(ring)] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_ReceivePacketError);
}

bool DTCLib::DTC_Registers::ReadPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister_(DTC_Register_ReceivePacketError);
	return data[static_cast<int>(ring)];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketError()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketError);
	form.description = "Receive Packet Error";
	form.vals.push_back("       ([CRC, PacketError, RX Overrun, RX Underrun])");
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": ["
							+ (ReadPacketCRCError(r) ? "x" : " ") + ","
							+ (ReadPacketError(r) ? "x" : " ") + ","
							+ (ReadRXElasticBufferOverrun(r) ? "x" : " ") + ","
							+ (ReadRXElasticBufferUnderrun(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [")
						+ (ReadPacketCRCError(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadPacketError(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadRXElasticBufferOverrun(DTC_Ring_CFO) ? "x" : " ") + ","
						+ (ReadRXElasticBufferUnderrun(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [")
						+ (ReadPacketCRCError(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadPacketError(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadRXElasticBufferOverrun(DTC_Ring_EVB) ? "x" : " ") + ","
						+ (ReadRXElasticBufferUnderrun(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// CFO Emulation Timestamp Registers
void DTCLib::DTC_Registers::SetCFOEmulationTimestamp(const DTC_Timestamp& ts)
{
	auto timestamp = ts.GetTimestamp();
	auto timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	auto timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister_(timestampLow, DTC_Register_CFOEmulationTimestampLow);
	WriteRegister_(timestampHigh, DTC_Register_CFOEmulationTimestampHigh);
}

DTCLib::DTC_Timestamp DTCLib::DTC_Registers::ReadCFOEmulationTimestamp()
{
	auto timestampLow = ReadRegister_(DTC_Register_CFOEmulationTimestampLow);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister_(DTC_Register_CFOEmulationTimestampHigh)));
	return output;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationTimestampLow()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationTimestampLow);
	form.description = "CFO Emulation Timestamp Low";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_CFOEmulationTimestampLow);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationTimestampHigh()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationTimestampHigh);
	form.description = "CFO Emulation Timestamp High";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_CFOEmulationTimestampHigh);
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Request Interval Register
void DTCLib::DTC_Registers::SetCFOEmulationRequestInterval(uint32_t interval)
{
	WriteRegister_(interval, DTC_Register_CFOEmulationRequestInterval);
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulationRequestInterval()
{
	return ReadRegister_(DTC_Register_CFOEmulationRequestInterval);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationRequestInterval()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationRequestInterval);
	form.description = "CFO Emu. Request Interval";
	std::stringstream o;
	o << "0x" << std::hex << ReadCFOEmulationRequestInterval();
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Number of Requests Register
void DTCLib::DTC_Registers::SetCFOEmulationNumRequests(uint32_t numRequests)
{
	WriteRegister_(numRequests, DTC_Register_CFOEmulationNumRequests);
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulationNumRequests()
{
	return ReadRegister_(DTC_Register_CFOEmulationNumRequests);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumRequests()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationNumRequests);
	form.description = "CFO Emulator Number Requests";
	std::stringstream o;
	o << "0x" << std::hex << ReadCFOEmulationNumRequests();
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Number of Packets Registers
void DTCLib::DTC_Registers::SetCFOEmulationNumPackets(const DTC_Ring_ID& ring, uint16_t numPackets)
{
	uint16_t data = numPackets & 0x7FF;
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
	case DTC_Ring_1:
		reg = DTC_Register_CFOEmulationNumPacketsRings10;
		break;
	case DTC_Ring_2:
	case DTC_Ring_3:
		reg = DTC_Register_CFOEmulationNumPacketsRings32;
		break;
	case DTC_Ring_4:
	case DTC_Ring_5:
		reg = DTC_Register_CFOEmulationNumPacketsRings54;
		break;
	default:
		return;
	}

	auto regval = ReadRegister_(reg);
	auto upper = (regval & 0xFFFF0000) >> 16;
	auto lower = regval & 0x0000FFFF;
	if (ring == DTC_Ring_0 || ring == DTC_Ring_2 || ring == DTC_Ring_4)
	{
		lower = data;
	}
	else
	{
		upper = data;
	}
	WriteRegister_((upper << 16) + lower, reg);
}

uint16_t DTCLib::DTC_Registers::ReadCFOEmulationNumPackets(const DTC_Ring_ID& ring)
{
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
	case DTC_Ring_1:
		reg = DTC_Register_CFOEmulationNumPacketsRings10;
		break;
	case DTC_Ring_2:
	case DTC_Ring_3:
		reg = DTC_Register_CFOEmulationNumPacketsRings32;
		break;
	case DTC_Ring_4:
	case DTC_Ring_5:
		reg = DTC_Register_CFOEmulationNumPacketsRings54;
		break;
	default:
		return 0;
	}

	auto regval = ReadRegister_(reg);
	auto upper = (regval & 0xFFFF0000) >> 16;
	auto lower = regval & 0x0000FFFF;
	if (ring == DTC_Ring_0 || ring == DTC_Ring_2 || ring == DTC_Ring_4)
	{
		return static_cast<uint16_t>(lower);
	}
	return static_cast<uint16_t>(upper);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsRing01()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationNumPacketsRings10);
	form.description = "CFO Emulator Num Packets R0,1";
	std::stringstream o;
	o << "Ring 0: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_0);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Ring 1: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsRing23()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationNumPacketsRings32);
	form.description = "CFO Emulator Num Packets R2,3";
	std::stringstream o;
	o << "Ring 2: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_2);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Ring 3: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationNumPacketsRing45()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationNumPacketsRings54);
	form.description = "CFO Emulator Num Packets R4,5";
	std::stringstream o;
	o << "Ring 4: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_4);
	form.vals.push_back(o.str());
	o.str("");
	o.clear();
	o << "Ring 5: 0x" << std::hex << ReadCFOEmulationNumPackets(DTC_Ring_5);
	form.vals.push_back(o.str());
	return form;
}

// CFO Emulation Event Mode Bytes Registers
void DTCLib::DTC_Registers::SetCFOEmulationModeByte(const uint8_t& byteNum, uint8_t data)
{
	DTC_Register reg;
	if (byteNum == 0 || byteNum == 1 || byteNum == 2 || byteNum == 3)
	{
		reg = DTC_Register_CFOEmulationEventMode1;
	}
	else if (byteNum == 4 || byteNum == 5)
	{
		reg = DTC_Register_CFOEmulationEventMode2;
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

uint8_t DTCLib::DTC_Registers::ReadCFOEmulationModeByte(const uint8_t& byteNum)
{
	DTC_Register reg;
	if (byteNum == 0 || byteNum == 1 || byteNum == 2 || byteNum == 3)
	{
		reg = DTC_Register_CFOEmulationEventMode1;
	}
	else if (byteNum == 4 || byteNum == 5)
	{
		reg = DTC_Register_CFOEmulationEventMode2;
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

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationModeBytes03()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationEventMode1);
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

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationModeBytes45()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationEventMode2);
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

void DTCLib::DTC_Registers::EnableDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_CFOEmulationDebugPacketType);
	data[16] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_CFOEmulationDebugPacketType);
}

void DTCLib::DTC_Registers::DisableDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_CFOEmulationDebugPacketType);
	data[16] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_CFOEmulationDebugPacketType);
}

bool DTCLib::DTC_Registers::ReadDebugPacketMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_CFOEmulationDebugPacketType);
	return data[16];
}

void DTCLib::DTC_Registers::SetCFOEmulationDebugType(DTC_DebugType type)
{
	std::bitset<32> data = type & 0xF;
	data[16] = ReadDebugPacketMode();
	WriteRegister_(data.to_ulong(), DTC_Register_CFOEmulationDebugPacketType);
}

DTCLib::DTC_DebugType DTCLib::DTC_Registers::ReadCFOEmulationDebugType()
{
	return static_cast<DTC_DebugType>(0xFFFF & ReadRegister_(DTC_Register_CFOEmulationDebugPacketType));
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatCFOEmulationDebugPacketType()
{
	auto form = CreateFormatter(DTC_Register_CFOEmulationDebugPacketType);
	form.description = "CFO Emulation Debug Packet Type";
	form.vals.push_back(std::string("Debug Mode: [") + (ReadDebugPacketMode() ? "x" : " ") + "]");
	std::stringstream o;
	o << "Debug Packet Type: 0x" << std::hex << ReadCFOEmulationDebugType();
	form.vals.push_back(o.str());
	return form;
}

// RX Packet Count Error Flags Register
bool DTCLib::DTC_Registers::ReadRXPacketCountErrorFlags(const DTC_Ring_ID & ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_RXPacketCountErrorFlags);
	return dataSet[ring];
}

void DTCLib::DTC_Registers::ClearRXPacketCountErrorFlags(const DTC_Ring_ID& ring)
{

	std::bitset<32> dataSet;
	dataSet[ring] = true;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_RXPacketCountErrorFlags);
}

void DTCLib::DTC_Registers::ClearRXPacketCountErrorFlags()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_RXPacketCountErrorFlags);
	WriteRegister_(dataSet.to_ulong(), DTC_Register_RXPacketCountErrorFlags);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatRXPacketCountErrorFlags()
{
	auto form = CreateFormatter(DTC_Register_RXPacketCountErrorFlags);
	form.description = "RX Packet Count Error Flags";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadRXPacketCountErrorFlags(r) ? "x" : " ") + "]");
	}
	form.vals.push_back(std::string("CFO:    [") + (ReadRXPacketCountErrorFlags(DTC_Ring_CFO) ? "x" : " ") + "]");
	form.vals.push_back(std::string("EVB:    [") + (ReadRXPacketCountErrorFlags(DTC_Ring_EVB) ? "x" : " ") + "]");
	return form;
}

// Detector Emulator DMA Count Register
void DTCLib::DTC_Registers::SetDetectorEmulationDMACount(uint32_t count)
{
	WriteRegister_(count, DTC_Register_DetEmulationDMACount);
}

uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMACount()
{
	return ReadRegister_(DTC_Register_DetEmulationDMACount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationDMACount()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationDMACount);
	form.description = "DetEmu DMA Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadDetectorEmulationDMACount();
	form.vals.push_back(o.str());
	return form;
}

// Detector Emulator DMA Delay Counter Register
void DTCLib::DTC_Registers::SetDetectorEmulationDMADelayCount(uint32_t count)
{
	WriteRegister_(count, DTC_Register_DetEmulationDelayCount);
}

uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMADelayCount()
{
	return ReadRegister_(DTC_Register_DetEmulationDelayCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationDMADelayCount()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationDelayCount);
	form.description = "DetEmu DMA Delay Count";
	std::stringstream o;
	o << "0x" << std::hex << ReadDetectorEmulationDMADelayCount();
	form.vals.push_back(o.str());
	return form;
}

void DTCLib::DTC_Registers::EnableDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl0);
	data[0] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulationControl0);
}

void DTCLib::DTC_Registers::DisableDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl0);
	data[0] = 0;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulationControl0);
}

bool DTCLib::DTC_Registers::ReadDetectorEmulatorMode()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl0);
	return data[0];
}

void DTCLib::DTC_Registers::EnableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl0);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulationControl0);
}

void DTCLib::DTC_Registers::DisableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl1);
	data[1] = 1;
	WriteRegister_(data.to_ulong(), DTC_Register_DetEmulationControl1);
}

bool DTCLib::DTC_Registers::ReadDetectorEmulatorEnable()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl0);
	return data[1];
}

bool DTCLib::DTC_Registers::ReadDetectorEmulatorEnableClear()
{
	std::bitset<32> data = ReadRegister_(DTC_Register_DetEmulationControl1);
	return data[1];
}

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

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationControl0()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationControl0);
	form.description = "Detector Emulation Control 0";
	form.vals.push_back(std::string("Detector Emulation Enable: [") + (ReadDetectorEmulatorEnable() ? "x" : " ") + "]");
	form.vals.push_back(std::string("Detector Emulation Mode:   [") + (ReadDetectorEmulatorMode() ? "x" : " ") + "]");
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDetectorEmulationControl1()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationControl1);
	form.description = "Detector Emulation Control 1";
	form.vals.push_back(std::string("Detector Emulation Enable Clear: [") + (ReadDetectorEmulatorEnableClear() ? "x" : " ") + "]");
	return form;
}

// DDR Event Data Local Start Address Register
void DTCLib::DTC_Registers::SetDDRDataLocalStartAddress(uint32_t address)
{
	WriteRegister_(address, DTC_Register_DetEmulationDataStartAddress);
}

uint32_t DTCLib::DTC_Registers::ReadDDRDataLocalStartAddress()
{
	return ReadRegister_(DTC_Register_DetEmulationDataStartAddress);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRDataLocalStartAddress()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationDataStartAddress);
	form.description = "DDR Event Data Local Start Address";
	std::stringstream o;
	o << "0x" << std::hex << ReadDDRDataLocalStartAddress();
	form.vals.push_back(o.str());
	return form;
}

// DDR Event Data Local End Address Register
void DTCLib::DTC_Registers::SetDDRDataLocalEndAddress(uint32_t address)
{
	WriteRegister_(address, DTC_Register_DetEmulationDataEndAddress);
}

uint32_t DTCLib::DTC_Registers::ReadDDRDataLocalEndAddress()
{
	return ReadRegister_(DTC_Register_DetEmulationDataEndAddress);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRDataLocalEndAddress()
{
	auto form = CreateFormatter(DTC_Register_DetEmulationDataEndAddress);
	form.description = "DDR Event Data Local End Address";
	std::stringstream o;
	o << "0x" << std::hex << ReadDDRDataLocalEndAddress();
	form.vals.push_back(o.str());
	return form;
}

// ROC DRP Sync Error Register
bool DTCLib::DTC_Registers::ReadROCDRPSyncErrors(const DTC_Ring_ID & ring)
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCDRPDataSyncError);
	return dataSet[ring];
}

void DTCLib::DTC_Registers::ClearROCDRPSyncErrors(const DTC_Ring_ID& ring)
{

	std::bitset<32> dataSet;
	dataSet[ring] = true;
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCDRPDataSyncError);
}

void DTCLib::DTC_Registers::ClearROCDRPSyncErrors()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_ROCDRPDataSyncError);
	WriteRegister_(dataSet.to_ulong(), DTC_Register_ROCDRPDataSyncError);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatROCDRPSyncError()
{
	auto form = CreateFormatter(DTC_Register_ROCDRPDataSyncError);
	form.description = "RX Packet Count Error Flags";
	for (auto r : DTC_Rings)
	{
		form.vals.push_back(std::string("Ring ") + std::to_string(r) + ": [" + (ReadRXPacketCountErrorFlags(r) ? "x" : " ") + "]");
	}
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadEthernetPayloadSize()
{
	return ReadRegister_(DTC_Register_EthernetFramePayloadSize);
}

void DTCLib::DTC_Registers::SetEthernetPayloadSize(uint32_t size)
{
	if (size > 1492) { size = 1492; }
	WriteRegister_(size, DTC_Register_EthernetFramePayloadSize);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatEthernetPayloadSize()
{
	auto form = CreateFormatter(DTC_Register_EthernetFramePayloadSize);
	form.description = "Ethernet Frame Payload Max Size";
	std::stringstream o;
	o << std::dec << ReadEthernetPayloadSize() << " bytes";
	form.vals.push_back(o.str());
	return form;
}

// SERDES Counter Registers
void DTCLib::DTC_Registers::ClearReceiveByteCount(const DTC_Ring_ID& ring)
{
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
		reg = DTC_Register_ReceiveByteCountDataRing0;
		break;
	case DTC_Ring_1:
		reg = DTC_Register_ReceiveByteCountDataRing1;
		break;
	case DTC_Ring_2:
		reg = DTC_Register_ReceiveByteCountDataRing2;
		break;
	case DTC_Ring_3:
		reg = DTC_Register_ReceiveByteCountDataRing3;
		break;
	case DTC_Ring_4:
		reg = DTC_Register_ReceiveByteCountDataRing4;
		break;
	case DTC_Ring_5:
		reg = DTC_Register_ReceiveByteCountDataRing5;
		break;
	case DTC_Ring_CFO:
		reg = DTC_Register_ReceiveByteCountDataCFO;
		break;
	case DTC_Ring_EVB:
		reg = DTC_Register_ReceiveByteCountDataEVB;
		break;
	default:
		return;
	}
	WriteRegister_(0, reg);
}

uint32_t DTCLib::DTC_Registers::ReadReceiveByteCount(const DTC_Ring_ID& ring)
{
	switch (ring)
	{
	case DTC_Ring_0:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing0);
	case DTC_Ring_1:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing1);
	case DTC_Ring_2:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing2);
	case DTC_Ring_3:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing3);
	case DTC_Ring_4:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing4);
	case DTC_Ring_5:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataRing5);
	case DTC_Ring_CFO:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataCFO);
	case DTC_Ring_EVB:
		return ReadRegister_(DTC_Register_ReceiveByteCountDataEVB);
	default:
		return 0;
	}
}

void DTCLib::DTC_Registers::ClearReceivePacketCount(const DTC_Ring_ID& ring)
{
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
		reg = DTC_Register_ReceivePacketCountDataRing0;
		break;
	case DTC_Ring_1:
		reg = DTC_Register_ReceivePacketCountDataRing1;
		break;
	case DTC_Ring_2:
		reg = DTC_Register_ReceivePacketCountDataRing2;
		break;
	case DTC_Ring_3:
		reg = DTC_Register_ReceivePacketCountDataRing3;
		break;
	case DTC_Ring_4:
		reg = DTC_Register_ReceivePacketCountDataRing4;
		break;
	case DTC_Ring_5:
		reg = DTC_Register_ReceivePacketCountDataRing5;
		break;
	case DTC_Ring_CFO:
		reg = DTC_Register_ReceivePacketCountDataCFO;
		break;
	case DTC_Ring_EVB:
		reg = DTC_Register_ReceivePacketCountDataEVB;
		break;
	default:
		return;
	}
	WriteRegister_(0, reg);
}

uint32_t DTCLib::DTC_Registers::ReadReceivePacketCount(const DTC_Ring_ID& ring)
{
	switch (ring)
	{
	case DTC_Ring_0:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing0);
	case DTC_Ring_1:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing1);
	case DTC_Ring_2:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing2);
	case DTC_Ring_3:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing3);
	case DTC_Ring_4:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing4);
	case DTC_Ring_5:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataRing5);
	case DTC_Ring_CFO:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataCFO);
	case DTC_Ring_EVB:
		return ReadRegister_(DTC_Register_ReceivePacketCountDataEVB);
	default:
		return 0;
	}
}

void DTCLib::DTC_Registers::ClearTransmitByteCount(const DTC_Ring_ID& ring)
{
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
		reg = DTC_Register_TransmitByteCountDataRing0;
		break;
	case DTC_Ring_1:
		reg = DTC_Register_TransmitByteCountDataRing1;
		break;
	case DTC_Ring_2:
		reg = DTC_Register_TransmitByteCountDataRing2;
		break;
	case DTC_Ring_3:
		reg = DTC_Register_TransmitByteCountDataRing3;
		break;
	case DTC_Ring_4:
		reg = DTC_Register_TransmitByteCountDataRing4;
		break;
	case DTC_Ring_5:
		reg = DTC_Register_TransmitByteCountDataRing5;
		break;
	case DTC_Ring_CFO:
		reg = DTC_Register_TransmitByteCountDataCFO;
		break;
	case DTC_Ring_EVB:
		reg = DTC_Register_TransmitByteCountDataEVB;
		break;
	default:
		return;
	}
	WriteRegister_(0, reg);
}

uint32_t DTCLib::DTC_Registers::ReadTransmitByteCount(const DTC_Ring_ID& ring)
{
	switch (ring)
	{
	case DTC_Ring_0:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing0);
	case DTC_Ring_1:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing1);
	case DTC_Ring_2:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing2);
	case DTC_Ring_3:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing3);
	case DTC_Ring_4:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing4);
	case DTC_Ring_5:
		return ReadRegister_(DTC_Register_TransmitByteCountDataRing5);
	case DTC_Ring_CFO:
		return ReadRegister_(DTC_Register_TransmitByteCountDataCFO);
	case DTC_Ring_EVB:
		return ReadRegister_(DTC_Register_TransmitByteCountDataEVB);
	default:
		return 0;
	}
}

void DTCLib::DTC_Registers::ClearTransmitPacketCount(const DTC_Ring_ID& ring)
{
	DTC_Register reg;
	switch (ring)
	{
	case DTC_Ring_0:
		reg = DTC_Register_TransmitPacketCountDataRing0;
		break;
	case DTC_Ring_1:
		reg = DTC_Register_TransmitPacketCountDataRing1;
		break;
	case DTC_Ring_2:
		reg = DTC_Register_TransmitPacketCountDataRing2;
		break;
	case DTC_Ring_3:
		reg = DTC_Register_TransmitPacketCountDataRing3;
		break;
	case DTC_Ring_4:
		reg = DTC_Register_TransmitPacketCountDataRing4;
		break;
	case DTC_Ring_5:
		reg = DTC_Register_TransmitPacketCountDataRing5;
		break;
	case DTC_Ring_CFO:
		reg = DTC_Register_TransmitPacketCountDataCFO;
		break;
	case DTC_Ring_EVB:
		reg = DTC_Register_TransmitPacketCountDataEVB;
		break;
	default:
		return;
	}
	WriteRegister_(0, reg);
}

uint32_t DTCLib::DTC_Registers::ReadTransmitPacketCount(const DTC_Ring_ID& ring)
{
	switch (ring)
	{
	case DTC_Ring_0:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing0);
	case DTC_Ring_1:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing1);
	case DTC_Ring_2:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing2);
	case DTC_Ring_3:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing3);
	case DTC_Ring_4:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing4);
	case DTC_Ring_5:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataRing5);
	case DTC_Ring_CFO:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataCFO);
	case DTC_Ring_EVB:
		return ReadRegister_(DTC_Register_TransmitPacketCountDataEVB);
	default:
		return 0;
	}
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing0()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing0);
	form.description = "Receive Byte Count: Ring 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing1()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing1);
	form.description = "Receive Byte Count: Ring 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing2()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing2);
	form.description = "Receive Byte Count: Ring 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing3()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing3);
	form.description = "Receive Byte Count: Ring 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing4()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing4);
	form.description = "Receive Byte Count: Ring 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountRing5()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataRing5);
	form.description = "Receive Byte Count: Ring 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountCFO()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataCFO);
	form.description = "Receive Byte Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_CFO);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceiveByteCountEVB()
{
	auto form = CreateFormatter(DTC_Register_ReceiveByteCountDataEVB);
	form.description = "Receive Byte Count: EVB";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceiveByteCount(DTC_Ring_EVB);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing0()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing0);
	form.description = "Receive Packet Count: Ring 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing1()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing1);
	form.description = "Receive Packet Count: Ring 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing2()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing2);
	form.description = "Receive Packet Count: Ring 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing3()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing3);
	form.description = "Receive Packet Count: Ring 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing4()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing4);
	form.description = "Receive Packet Count: Ring 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountRing5()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataRing5);
	form.description = "Receive Packet Count: Ring 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountCFO()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataCFO);
	form.description = "Receive Packet Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_CFO);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatReceivePacketCountEVB()
{
	auto form = CreateFormatter(DTC_Register_ReceivePacketCountDataEVB);
	form.description = "Receive Packet Count: EVB";
	std::stringstream o;
	o << "0x" << std::hex << ReadReceivePacketCount(DTC_Ring_EVB);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing0()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing0);
	form.description = "Transmit Byte Count: Ring 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing1()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing1);
	form.description = "Transmit Byte Count: Ring 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing2()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing2);
	form.description = "Transmit Byte Count: Ring 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing3()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing3);
	form.description = "Transmit Byte Count: Ring 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing4()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing4);
	form.description = "Transmit Byte Count: Ring 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountRing5()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataRing5);
	form.description = "Transmit Byte Count: Ring 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountCFO()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataCFO);
	form.description = "Transmit Byte Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_CFO);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTramsitByteCountEVB()
{
	auto form = CreateFormatter(DTC_Register_TransmitByteCountDataEVB);
	form.description = "Transmit Byte Count: EVB";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitByteCount(DTC_Ring_EVB);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing0()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing0);
	form.description = "Transmit Packet Count: Ring 0";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_0);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing1()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing1);
	form.description = "Transmit Packet Count: Ring 1";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing2()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing2);
	form.description = "Transmit Packet Count: Ring 2";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing3()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing3);
	form.description = "Transmit Packet Count: Ring 3";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_3);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing4()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing4);
	form.description = "Transmit Packet Count: Ring 4";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_4);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountRing5()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataRing5);
	form.description = "Transmit Packet Count: Ring 5";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_5);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountCFO()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataCFO);
	form.description = "Transmit Packet Count: CFO";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_CFO);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatTransmitPacketCountEVB()
{
	auto form = CreateFormatter(DTC_Register_TransmitPacketCountDataEVB);
	form.description = "Transmit Packet Count: EVB";
	std::stringstream o;
	o << "0x" << std::hex << ReadTransmitPacketCount(DTC_Ring_EVB);
	form.vals.push_back(o.str());
	return form;
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDRRingBufferFullFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_DDRRingBufferFullFlags1) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_DDRRingBufferFullFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDRRingBufferFullErrorFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_DDRRingBufferFullErrorFlags1) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_DDRRingBufferFullErrorFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDRRingBufferEmptyFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_DDRRingBufferEmptyFlags1) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_DDRRingBufferEmptyFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDRRingBufferHalfFullFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_DDRRingBufferHalfFullFlags1) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_DDRRingBufferHalfFullFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDREventBuilderBufferFullFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_EventBuilderBufferFullFlags2) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_EventBuilderBufferFullFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDREventBuilderBufferFullErrorFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_EventBuilderBufferFullErrorFlags2) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_EventBuilderBufferFullErrorFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDREventBuilderBufferEmptyFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_EventBuilderBufferEmptyFlags2) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_EventBuilderBufferEmptyFlags2)) << 32);
	return std::bitset<64>(flags);
}

std::bitset<64> DTCLib::DTC_Registers::ReadDDREventBuilderBufferHalfFullFlags()
{
	uint64_t flags = ReadRegister_(DTC_Register_EventBuilderBufferHalfFullFlags2) + (static_cast<uint64_t>(ReadRegister_(DTC_Register_EventBuilderBufferHalfFullFlags2)) << 32);
	return std::bitset<64>(flags);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferFullFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferFullFlags1);
	form.description = "DDR Ring Buffer Full Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferFullFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferFullErrorFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferFullErrorFlags1);
	form.description = "DDR Ring Buffer Full Error Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferFullErrorFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferEmptyFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferEmptyFlags1);
	form.description = "DDR Ring Buffer Empty Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferEmptyFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferHalfFullFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferHalfFullFlags1);
	form.description = "DDR Ring Buffer Half Full Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferHalfFullFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferFullFlags1);
	form.description = "DDR Event Builder Buffer Full Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferFullFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullErrorFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferFullErrorFlags1);
	form.description = "DDR Event Builder Buffer Full Error Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferFullErrorFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferEmptyFlags1);
	form.description = "DDR Event Builder Buffer Empty Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferEmptyFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlagsLow()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferHalfFullFlags1);
	form.description = "DDR Event Builder Buffer Half Full Flags (0-31)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferHalfFullFlags1);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferFullFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferFullFlags2);
	form.description = "DDR Ring Buffer Full Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferFullFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferFullErrorFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferFullErrorFlags2);
	form.description = "DDR Ring Buffer Full Error Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferFullErrorFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferEmptyFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferEmptyFlags2);
	form.description = "DDR Ring Buffer Empty Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferEmptyFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDRRingBufferHalfFullFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_DDRRingBufferHalfFullFlags2);
	form.description = "DDR Ring Buffer Half Full Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_DDRRingBufferHalfFullFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferFullFlags2);
	form.description = "DDR Event Builder Buffer Full Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferFullFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferFullErrorFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferFullErrorFlags2);
	form.description = "DDR Event Builder Buffer Full Error Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferFullErrorFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferEmptyFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferEmptyFlags2);
	form.description = "DDR Event Builder Buffer Empty Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferEmptyFlags2);
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatDDREventBuilderBufferHalfFullFlagsHigh()
{
	auto form = CreateFormatter(DTC_Register_EventBuilderBufferHalfFullFlags2);
	form.description = "DDR Event Builder Buffer Half Full Flags (32-63)";
	std::stringstream o;
	o << "0x" << std::hex << ReadRegister_(DTC_Register_EventBuilderBufferHalfFullFlags2);
	form.vals.push_back(o.str());
	return form;
}

bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSErrorFlag()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[30];
}

uint8_t DTCLib::DTC_Registers::ReadEVBSERDESTXPRBSSEL()
{
	uint8_t regVal = static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0x7000) >> 12);
	return regVal;
}

void DTCLib::DTC_Registers::SetEVBSERDESTXPRBSSEL(uint8_t byte)
{
	uint8_t regVal = (ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0xFFFF8FFF);
	regVal += ((byte & 0x7) << 12);
	WriteRegister_(regVal, DTC_Register_EVBSERDESPRBSControlStatus);
}

uint8_t DTCLib::DTC_Registers::ReadEVBSERDESRXPRBSSEL()
{
	uint8_t regVal = static_cast<uint8_t>((ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0x700) >> 8);
	return regVal;
}

void DTCLib::DTC_Registers::SetEVBSERDESRXPRBSSEL(uint8_t byte)
{
	uint8_t regVal = (ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus) & 0xFFFFF8FF);
	regVal += ((byte & 0x7) << 8);
	WriteRegister_(regVal, DTC_Register_EVBSERDESPRBSControlStatus);
}

bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSForceError()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[1];
}

void DTCLib::DTC_Registers::SetEVBSERDESPRBSForceError(bool flag)
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal[1] = flag;
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

void DTCLib::DTC_Registers::ToggleEVBSERDESPRBSForceError()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal.flip(0);
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

bool DTCLib::DTC_Registers::ReadEVBSERDESPRBSReset()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	return regVal[0];
}

void DTCLib::DTC_Registers::SetEVBSERDESPRBSReset(bool flag)
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal[0] = flag;
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

void DTCLib::DTC_Registers::ToggleEVBSERDESPRBSReset()
{
	std::bitset<32> regVal(ReadRegister_(DTC_Register_EVBSERDESPRBSControlStatus));
	regVal.flip(0);
	WriteRegister_(regVal.to_ulong(), DTC_Register_EVBSERDESPRBSControlStatus);
}

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

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing0()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing0);
}

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing1()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing1);
}

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing2()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing2);
}

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing3()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing3);
}

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing4()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing4);
}

uint32_t DTCLib::DTC_Registers::ReadMissedCFOPacketCountRing5()
{
	return ReadRegister_(DTC_Register_MissedCFOPacketCountRing5);
}

void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing0()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing0);
}
void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing1()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing1);
}
void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing2()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing2);
}
void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing3()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing3);
}
void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing4()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing4);
}
void DTCLib::DTC_Registers::ClearMissedCFOPacketCountRing5()
{
	WriteRegister_(1, DTC_Register_MissedCFOPacketCountRing5);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing0()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing0);
	form.description = "Missed CFO Packet Count Ring 0";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing0();
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing1()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing1);
	form.description = "Missed CFO Packet Count Ring 1";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing1();
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing2()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing2);
	form.description = "Missed CFO Packet Count Ring 2";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing2();
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing3()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing3);
	form.description = "Missed CFO Packet Count Ring 3";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing3();
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing4()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing4);
	form.description = "Missed CFO Packet Count Ring 4";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing4();
	form.vals.push_back(o.str());
	return form;
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatMissedCFOPacketCountRing5()
{
	auto form = CreateFormatter(DTC_Register_MissedCFOPacketCountRing5);
	form.description = "Missed CFO Packet Count Ring 5";
	std::stringstream o;
	o << std::dec << ReadMissedCFOPacketCountRing5();
	form.vals.push_back(o.str());
	return form;
}

uint32_t DTCLib::DTC_Registers::ReadLocalFragmentDropCount()
{
	return ReadRegister_(DTC_Register_LocalFragmentDropCount);
}

void DTCLib::DTC_Registers::ClearLocalFragmentDropCount()
{
	WriteRegister_(1, DTC_Register_LocalFragmentDropCount);
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatLocalFragmentDropCount()
{
	auto form = CreateFormatter(DTC_Register_LocalFragmentDropCount);
	form.description = "Local Data Fragment Drop Count";
	std::stringstream o;
	o << std::dec << ReadLocalFragmentDropCount();
	form.vals.push_back(o.str());
	return form;
}

// FPGA PROM Program Status Register
bool DTCLib::DTC_Registers::ReadFPGAPROMProgramFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[1];
}

bool DTCLib::DTC_Registers::ReadFPGAPROMReady()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[0];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGAPROMProgramStatus()
{
	auto form = CreateFormatter(DTC_Register_FPGAPROMProgramStatus);
	form.description = "FPGA PROM Program Status";
	form.vals.push_back(std::string("FPGA PROM Program FIFO Full: [") + (ReadFPGAPROMProgramFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("FPGA PROM Ready:             [") + (ReadFPGAPROMReady() ? "x" : " ") + "]");
	return form;
}

// FPGA Core Access Register
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

bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGACoreAccess);
	return dataSet[1];
}

bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOEmpty()
{
	std::bitset<32> dataSet = ReadRegister_(DTC_Register_FPGACoreAccess);
	return dataSet[0];
}

DTCLib::DTC_RegisterFormatter DTCLib::DTC_Registers::FormatFPGACoreAccess()
{
	auto form = CreateFormatter(DTC_Register_FPGACoreAccess);
	form.description = "FPGA Core Access";
	form.vals.push_back(std::string("FPGA Core Access FIFO Full:  [") + (ReadFPGACoreAccessFIFOFull() ? "x" : " ") + "]");
	form.vals.push_back(std::string("FPGA Core Access FIFO Empty: [") + (ReadFPGACoreAccessFIFOEmpty() ? "x" : " ") + "]");

	return form;
}

// Event Mode Lookup Table
void DTCLib::DTC_Registers::SetAllEventModeWords(uint32_t data)
{
	for (uint16_t address = DTC_Register_EventModeLookupTableStart; address <= DTC_Register_EventModeLookupTableEnd; address += 4)
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
			throw DTC_IOErrorException();
		}
	}
}

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
			throw DTC_IOErrorException();
		}
	}
}

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
			throw DTC_IOErrorException();
		}

		return data;
	}
	return 0;
}

// Oscillator Programming (DDR and SERDES)
void DTCLib::DTC_Registers::SetNewOscillatorFrequency(DTC_OscillatorType oscillator, double targetFrequency)
{
	auto currentFrequency = ReadCurrentFrequency(oscillator);
	auto currentProgram = ReadCurrentProgram(oscillator);

	// Check if targetFrequency is essentially the same as the current frequency...
	if (fabs(currentFrequency - targetFrequency) < targetFrequency * 30 / 1000000) return;

	auto newParameters = CalculateFrequencyForProgramming_(targetFrequency, currentFrequency, currentProgram);
	if (newParameters == 0) return;
	WriteCurrentProgram(newParameters, oscillator);
	WriteCurrentFrequency(targetFrequency, oscillator);
}

double DTCLib::DTC_Registers::ReadCurrentFrequency(DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
	case DTC_OscillatorType_SERDES:
		return ReadSERDESOscillatorFrequency();
	case DTC_OscillatorType_DDR:
		return ReadDDROscillatorFrequency();
	}
	return 0;
}

uint64_t DTCLib::DTC_Registers::ReadCurrentProgram(DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
	case DTC_OscillatorType_SERDES:
		return ReadSERDESOscillatorParameters();
	case DTC_OscillatorType_DDR:
		return ReadDDROscillatorParameters();
	}
	return 0;
}
void DTCLib::DTC_Registers::WriteCurrentFrequency(double freq, DTC_OscillatorType oscillator)
{
	auto newFreq = static_cast<uint32_t>(freq);
	switch (oscillator)
	{
	case DTC_OscillatorType_SERDES:
		SetSERDESOscillatorFrequency(newFreq);
		break;
	case DTC_OscillatorType_DDR:
		SetDDROscillatorFrequency(newFreq);
		break;
	}
}
void DTCLib::DTC_Registers::WriteCurrentProgram(uint64_t program, DTC_OscillatorType oscillator)
{
	switch (oscillator)
	{
	case DTC_OscillatorType_SERDES:
		SetSERDESOscillatorParameters(program);
		break;
	case DTC_OscillatorType_DDR:
		SetDDROscillatorParameters(program);
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
		throw DTC_IOErrorException();
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
		throw DTC_IOErrorException();
	}

	return data;
}

int DTCLib::DTC_Registers::DecodeHighSpeedDivider_(int input)
{
	switch (input)
	{
	case 0: return 4;
	case 1: return 5;
	case 2: return 6;
	case 3: return 7;
	case 5: return 9;
	case 7: return 11;
	default: return -1;
	}
}

int DTCLib::DTC_Registers::EncodeHighSpeedDivider_(int input)
{
	switch (input)
	{
	case 4: return 0;
	case 5: return 1;
	case 6: return 2;
	case 7: return 3;
	case 9: return 5;
	case 11: return 7;
	default: return -1;
	}
}

int DTCLib::DTC_Registers::EncodeOutputDivider_(int input)
{
	if (input == 1) return 0;
	int temp = input / 2;
	return (temp * 2) - 1;
}

uint64_t DTCLib::DTC_Registers::CalculateFrequencyForProgramming_(double targetFrequency, double currentFrequency, uint64_t currentProgram)
{
	TRACE(4, "CalculateFrequencyForProgramming: targetFrequency=%lf, currentFrequency=%lf, currentProgram=0x%llx", targetFrequency, currentFrequency, static_cast<unsigned long long>(currentProgram));
	auto currentHighSpeedDivider = DecodeHighSpeedDivider_((currentProgram >> 48) & 0x7);
	auto currentOutputDivider = DecodeOutputDivider_((currentProgram >> 40) & 0x7F);
	auto currentRFREQ = DecodeRFREQ_(currentProgram & 0x3FFFFFFFFF);
	TRACE(4, "CalculateFrequencyForProgramming: Current HSDIV=%d, N1=%d, RFREQ=%lf", currentHighSpeedDivider, currentOutputDivider, currentRFREQ);
	const auto minFreq = 4850000000; // Hz
	const auto maxFreq = 5670000000; // Hz

	auto fXTAL = currentFrequency * currentHighSpeedDivider * currentOutputDivider / currentRFREQ;
	TRACE(4, "CalculateFrequencyForProgramming: fXTAL=%lf", fXTAL);

	std::vector<int> hsdiv_values = { 11, 9, 7, 6, 5, 4 };
	std::vector < std::pair<int, double>> parameter_values;
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
		TRACE(4, "CalculateFrequencyForProgramming: Adding solution: HSDIV=%d, N1=%d, fdco_new=%lf", hsdiv, thisN, fdco_new);
		parameter_values.push_back(std::make_pair(thisN, fdco_new));
	}

	auto counter = -1;
	auto newHighSpeedDivider = 0;
	auto newOutputDivider = 0;
	auto newRFREQ = 0.0;

	for (auto values : parameter_values)
	{
		++counter;
		if (values.second > maxFreq)	continue;


		newHighSpeedDivider = hsdiv_values[counter];
		newOutputDivider = values.first;
		newRFREQ = values.second / fXTAL;
		break;
	}
	TRACE(4, "CalculateFrequencyForProgramming: New Program: HSDIV=%d, N1=%d, RFREQ=%lf", newHighSpeedDivider, newOutputDivider, newRFREQ);

	if (EncodeHighSpeedDivider_(newHighSpeedDivider) == -1)
	{
		TRACE(0, "ERROR: CalculateFrequencyForProgramming: Invalid HSDIV %d!", newHighSpeedDivider);
		return 0;
	}
	if (newOutputDivider > 128 || newOutputDivider < 0)
	{
		TRACE(0, "ERROR: CalculateFrequencyForProgramming: Invalid N1 %d!", newOutputDivider);
		return 0;
	}
	if (newRFREQ <= 0)
	{
		TRACE(0, "ERROR: CalculateFrequencyForProgramming: Invalid RFREQ %lf!", newRFREQ);
		return 0;
	}

	auto output = (static_cast<uint64_t>(EncodeHighSpeedDivider_(newHighSpeedDivider)) << 48) + (static_cast<uint64_t>(EncodeOutputDivider_(newOutputDivider)) << 40) + EncodeRFREQ_(newRFREQ);
	TRACE(4, "CalculateFrequencyForProgramming: New Program: 0x%llx", static_cast<unsigned long long>(output));
	return output;
}

