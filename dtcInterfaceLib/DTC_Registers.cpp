#include "DTC_Registers.h"

#include <sstream> // Convert uint to hex string

#include <iomanip> // std::setw, std::setfill

#include <chrono>
#ifndef _WIN32
# include <unistd.h>
# include "trace.h"
#else
#endif
#define TRACE_NAME "MU2EDEV"

DTCLib::DTC_Registers::DTC_Registers(DTC_SimMode mode) : device_(), simMode_(mode), dmaSize_(16)
{
	for (int ii = 0; ii < 6; ++ii)
	{
		maxROCs_[ii] = DTC_ROC_Unused;
	}

#ifdef _WIN32
	simMode_ = DTCLib::DTC_SimMode_Tracker;
#else
	char* sim = getenv("DTCLIB_SIM_ENABLE");
	if (sim != NULL)
	{
		switch (sim[0])
		{
		case '1':
		case 't':
		case 'T':
			simMode_ = DTCLib::DTC_SimMode_Tracker;
			break;
		case '2':
		case 'c':
		case 'C':
			simMode_ = DTCLib::DTC_SimMode_Calorimeter;
			break;
		case '3':
		case 'v':
		case 'V':
			simMode_ = DTCLib::DTC_SimMode_CosmicVeto;
			break;
		case '4':
		case 'n':
		case 'N':
			simMode_ = DTCLib::DTC_SimMode_NoCFO;
			break;
		case '5':
		case 'r':
		case 'R':
			simMode_ = DTCLib::DTC_SimMode_ROCEmulator;
			break;
		case '6':
		case 'l':
		case 'L':
			simMode_ = DTCLib::DTC_SimMode_Loopback;
			break;
		case '7':
		case 'p':
		case 'P':
			simMode_ = DTCLib::DTC_SimMode_Performance;
			break;
		case '0':
		default:
			simMode_ = DTCLib::DTC_SimMode_Disabled;
			break;
		}
	}
#endif
	SetSimMode(simMode_);
}

DTCLib::DTC_SimMode DTCLib::DTC_Registers::SetSimMode(DTC_SimMode mode)
{
	simMode_ = mode;
	device_.init(simMode_);

	for (auto ring : DTC_Rings)
	{
		SetMaxROCNumber(ring, DTC_ROC_Unused);
		DisableROCEmulator(ring);
		SetSERDESLoopbackMode(DTC_Ring_0, DTC_SERDESLoopbackMode_Disabled);
	}

	if (simMode_ != DTCLib::DTC_SimMode_Disabled)
	{
		// Set up hardware simulation mode: Ring 0 Tx/Rx Enabled, Loopback Enabled, ROC Emulator Enabled. All other rings disabled.
		for (auto ring : DTC_Rings)
		{
			DisableRing(ring);
		}
		EnableRing(DTC_Ring_0, DTC_RingEnableMode(true, true, false), DTC_ROC_0);
		if (simMode_ == DTC_SimMode_Loopback)
		{
			SetSERDESLoopbackMode(DTC_Ring_0, DTC_SERDESLoopbackMode_NearPCS);
			SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
		}
		else if (simMode_ == DTC_SimMode_ROCEmulator)
		{
			EnableROCEmulator(DTC_Ring_0);
			SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
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
std::string DTCLib::DTC_Registers::RegDump()
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);
	o << "{";
	o << "\"SimMode\":" << DTC_SimModeConverter(simMode_) << ",\n";
	o << "\"Version\":\"" << ReadDesignVersion() << "\",\n";
	o << "\"ResetDTC\":" << ReadResetDTC() << ",\n";
	o << "\"CFOEmulation\":" << ReadCFOEmulation() << ",\n";
	o << "\"ResetSERDESOscillator\":" << ReadResetSERDESOscillator() << ",\n";
	o << "\"SERDESOscillatorClock\":" << ReadSERDESOscillatorClock() << ",\n";
	o << "\"SystemClock\":" << ReadSystemClock() << ",\n";
	o << "\"TimingEnable\":" << ReadTimingEnable() << ",\n";
	o << "\"TriggerDMALength\":" << ReadTriggerDMATransferLength() << ",\n";
	o << "\"MinDMALength\":" << ReadMinDMATransferLength() << ",\n";
	o << "\"SERDESOscillatorIICError\":" << ReadSERDESOscillatorIICError() << ",\n";
	o << "\"SERDESOscillatorInitComplete\":" << ReadSERDESOscillatorInitializationComplete() << ",\n";
	o << "\"DMATimeout\":" << ReadDMATimeoutPreset() << ",\n";
	o << "\"ROC DataBlock Timeout\":" << ReadROCTimeoutPreset() << ",\n";
	o << "\"Timestamp\":" << ReadTimestampPreset().GetTimestamp(true) << ",\n";
	o << "\"DataPendingTimer\":" << ReadDataPendingTimer() << ",\n";
	o << "\"PacketSize\":" << ReadPacketSize() << ",\n";
	o << "\"PROMFIFOFull\":" << ReadFPGAPROMProgramFIFOFull() << ",\n";
	o << "\"PROMReady\":" << ReadFPGAPROMReady() << ",\n";
	o << "\"FPGACoreFIFOFull\":" << ReadFPGACoreAccessFIFOFull() << ",\n";
	o << RingRegDump(DTC_Ring_0, "\"Ring0\"") << ",\n";
	o << RingRegDump(DTC_Ring_1, "\"Ring1\"") << ",\n";
	o << RingRegDump(DTC_Ring_2, "\"Ring2\"") << ",\n";
	o << RingRegDump(DTC_Ring_3, "\"Ring3\"") << ",\n";
	o << RingRegDump(DTC_Ring_4, "\"Ring4\"") << ",\n";
	o << RingRegDump(DTC_Ring_5, "\"Ring5\"") << ",\n";
	o << CFORegDump() << "\n";
	o << "}";

	return o.str();
}

std::string DTCLib::DTC_Registers::RingRegDump(const DTC_Ring_ID& ring, std::string id)
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);

	o << id << ":{\n";

	DTC_ROC_ID ringROCs = ReadRingROCCount(ring);
	switch (ringROCs)
	{
	case DTC_ROC_Unused:
	default:
		o << "\t\"ROC0Enabled\":false,\n";
		o << "\t\"ROC1Enabled\":false,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_0:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":false,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_1:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":false,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_2:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":false,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_3:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":false,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_4:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":true,\n";
		o << "\t\"ROC5Enabled\":false,\n";
		break;
	case DTC_ROC_5:
		o << "\t\"ROC0Enabled\":true,\n";
		o << "\t\"ROC1Enabled\":true,\n";
		o << "\t\"ROC2Enabled\":true,\n";
		o << "\t\"ROC3Enabled\":true,\n";
		o << "\t\"ROC4Enabled\":true,\n";
		o << "\t\"ROC5Enabled\":true,\n";
		break;
	}

	o << "\t\"Enabled\":" << ReadRingEnabled(ring) << ",\n";
	o << "\t\"ROCEmulator\":" << ReadROCEmulator(ring) << ",\n";
	o << "\t\"ResetSERDES\":" << ReadResetSERDES(ring) << ",\n";
	o << "\t\"SERDESLoopback\":" << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(ring)) << ",\n";
	o << "\t\"EyescanError\":" << ReadSERDESEyescanError(ring) << ",\n";
	o << "\t\"FIFOFullFlags\":" << ReadFIFOFullErrorFlags(ring) << ",\n";
	o << "\t\"FIFOHalfFull\":" << ReadSERDESBufferFIFOHalfFull(ring) << ",\n";
	o << "\t\"OverflowOrUnderflow\":" << ReadSERDESOverflowOrUnderflow(ring) << ",\n";
	o << "\t\"PLLLocked\":" << ReadSERDESPLLLocked(ring) << ",\n";
	o << "\t\"RXCDRLock\":" << ReadSERDESRXCDRLock(ring) << ",\n";
	o << "\t\"ResetDone\":" << ReadResetSERDESDone(ring) << ",\n";
	o << "\t\"UnlockError\":" << ReadSERDESUnlockError(ring) << ",\n";
	o << "\t\"RXBufferStatus\":" << DTC_RXBufferStatusConverter(ReadSERDESRXBufferStatus(ring)) << ",\n";
	o << "\t\"RXStatus\":" << DTC_RXStatusConverter(ReadSERDESRXStatus(ring)) << ",\n";
	o << "\t\"SERDESRXDisparity\":" << ReadSERDESRXDisparityError(ring) << ",\n";
	o << "\t\"CharacterError\":" << ReadSERDESRXCharacterNotInTableError(ring) << "\n";
	o << "\t\"TimeoutError\":" << ReadROCTimeoutError(ring) << "\n";
	o << "\t\"PacketCRCError\":" << ReadPacketCRCError(ring) << "\n";
	o << "\t\"PacketError\":" << ReadPacketError(ring) << "\n";
	o << "\t\"RXElasticBufferOverrun\":" << ReadRXElasticBufferOverrun(ring) << "\n";
	o << "\t\"RXElasticBufferUnderrun\":" << ReadRXElasticBufferUnderrun(ring) << "\n";
	o << "}";

	return o.str();
}

std::string DTCLib::DTC_Registers::CFORegDump()
{
	std::ostringstream o;
	o.setf(std::ios_base::boolalpha);

	o << "\"CFO\":{";

	o << "\t\"Enabled\":" << ReadRingEnabled(DTC_Ring_CFO) << ",\n";
	o << "\t\"SERDESLoopback\":" << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_CFO)) << ",\n";
	o << "\t\"CharacterError\":" << ReadSERDESRXCharacterNotInTableError(DTC_Ring_CFO) << ",\n";
	o << "\t\"EyescanError\":" << ReadSERDESEyescanError(DTC_Ring_CFO) << ",\n";
	o << "\t\"FIFOFullFlags\":" << ReadFIFOFullErrorFlags(DTC_Ring_CFO) << ",\n";
	o << "\t\"FIFOHalfFull\":" << ReadSERDESBufferFIFOHalfFull(DTC_Ring_CFO) << ",\n";
	o << "\t\"OverflowOrUnderflow\":" << ReadSERDESOverflowOrUnderflow(DTC_Ring_CFO) << ",\n";
	o << "\t\"PLLLocked\":" << ReadSERDESPLLLocked(DTC_Ring_CFO) << ",\n";
	o << "\t\"RXBufferStatus\":" << DTC_RXBufferStatusConverter(ReadSERDESRXBufferStatus(DTC_Ring_CFO)) << ",\n";
	o << "\t\"RXCDRLock\":" << ReadSERDESRXCDRLock(DTC_Ring_CFO) << ",\n";
	o << "\t\"RXStatus\":" << DTC_RXStatusConverter(ReadSERDESRXStatus(DTC_Ring_CFO)) << ",\n";
	o << "\t\"ResetDone\":" << ReadResetSERDESDone(DTC_Ring_CFO) << ",\n";
	o << "\t\"ResetSERDES\":" << ReadResetSERDES(DTC_Ring_CFO) << ",\n";
	o << "\t\"SERDESRXDisparity\":" << ReadSERDESRXDisparityError(DTC_Ring_CFO) << ",\n";
	o << "\t\"UnlockError\":" << ReadSERDESUnlockError(DTC_Ring_CFO) << "\n";
	o << "\t\"PacketCRCError\":" << ReadPacketCRCError(DTC_Ring_CFO) << "\n";
	o << "\t\"PacketError\":" << ReadPacketError(DTC_Ring_CFO) << "\n";
	o << "\t\"RXElasticBufferOverrun\":" << ReadRXElasticBufferOverrun(DTC_Ring_CFO) << "\n";
	o << "\t\"RXElasticBufferUnderrun\":" << ReadRXElasticBufferUnderrun(DTC_Ring_CFO) << "\n";

	o << "}";

	return o.str();
}

std::string DTCLib::DTC_Registers::ConsoleFormatRegDump()
{
	std::ostringstream o;
	o << "Memory Map: " << std::endl;
	o << "    Address | Value      | Name                         | Translation" << std::endl;
	for (auto i : DTC_Readable_Registers)
	{
		o << "================================================================================" << std::endl;
		o << FormatRegister(i);
	}
	return o.str();
}

std::string DTCLib::DTC_Registers::FormatRegister(const DTC_Register& address)
{
	std::ostringstream o;
	o << std::hex << std::setfill('0');
	o << "    0x" << (int)address << "  | 0x" << std::setw(8) << (int)ReadRegister(address) << " ";

	switch (address)
	{
	case DTC_Register_DesignVersion:
		o << "| DTC Firmware Design Version  | " << ReadDesignVersionNumber();
		break;
	case DTC_Register_DesignDate:
		o << "| DTC Firmware Design Date     | " << ReadDesignDate();
		break;
	case DTC_Register_DTCControl:
		o << "| DTC Control                  | ";
		o << "Reset: [" << (ReadResetDTC() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "CFO Emulation Enable: [" << (ReadCFOEmulation() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "SERDES Oscillator Reset: [" << (ReadResetSERDESOscillator() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "SERDES Oscillator Clock Select : [" << (ReadSERDESOscillatorClock() ? " 2.5Gbs" : "3.125Gbs") << "], " << std::endl;
		o << "                                                        | ";
		o << "System Clock Select : [" << (ReadSystemClock() ? "Ext" : "Int") << "], " << std::endl;
		o << "                                                        | ";
		o << "Timing Enable : [" << (ReadTimingEnable() ? "x" : " ") << "]";
		break;
	case DTC_Register_DMATransferLength:
		o << "| DMA Transfer Length         | ";
		o << "Trigger Length: 0x" << ReadTriggerDMATransferLength() << "," << std::endl;
		o << "                                                       | ";
		o << "Minimum Length : 0x" << ReadMinDMATransferLength();
		break;
	case DTC_Register_SERDESLoopbackEnable:
		o << "| SERDES Loopback Enable       | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": " << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString() << "," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    " << DTC_SERDESLoopbackModeConverter(ReadSERDESLoopback(DTC_Ring_CFO)).toString();
		break;
	case DTC_Register_SERDESOscillatorStatus:
		o << "| SERDES Oscillator Status     | ";
		o << "IIC Error: [" << (ReadSERDESOscillatorIICError() ? "x" : " ") << "]," << std::endl;
		o << "                                                        | ";
		o << "Init.Complete: [" << (ReadSERDESOscillatorInitializationComplete() ? "x" : " ") << "]";
		break;
	case DTC_Register_ROCEmulationEnable:
		o << "| ROC Emulator Enable          | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "," << std::endl << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadROCEmulator(r) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_RingEnable:
		o << "| Ring Enable                  | ([TX,RX,Timing])" << std::endl;
		for (auto r : DTC_Rings)
		{
			DTC_RingEnableMode re = ReadRingEnabled(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (re.TransmitEnable ? "x" : " ") << ",";
			o << (re.ReceiveEnable ? "x" : " ") << ",";
			o << (re.TimingEnable ? "x" : " ") << "]," << std::endl;
		}
		{
			DTC_RingEnableMode ce = ReadRingEnabled(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << "TX:[" << (ce.TransmitEnable ? "x" : " ") << "], ";
			o << "RX:[" << (ce.ReceiveEnable ? "x" : " ") << "]]";
		}
		break;
	case DTC_Register_SERDESReset:
		o << "| SERDES Reset                 | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadResetSERDES(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadResetSERDES(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXDisparityError:
		o << "| SERDES RX Disparity Error    | ([H,L])" << std::endl;
		for (auto r : DTC_Rings)
		{
			o << "                                                        | ";
			DTC_SERDESRXDisparityError re = ReadSERDESRXDisparityError(r);
			o << "Ring " << (int)r << ": [";
			o << re.GetData()[1] << ",";
			o << re.GetData()[0] << "]," << std::endl;
		}
		{
			DTC_SERDESRXDisparityError ce = ReadSERDESRXDisparityError(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << ce.GetData()[1] << ",";
			o << ce.GetData()[0] << "]";
		}
		break;
	case DTC_Register_SERDESRXCharacterNotInTableError:
		o << "| SERDES RX CNIT Error         | ([H,L])" << std::endl;
		for (auto r : DTC_Rings)
		{
			auto re = ReadSERDESRXCharacterNotInTableError(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << re.GetData()[1] << ",";
			o << re.GetData()[0] << "]," << std::endl;
		}
		{
			auto ce = ReadSERDESRXCharacterNotInTableError(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << ce.GetData()[1] << ",";
			o << ce.GetData()[0] << "]";
		}
		break;
	case DTC_Register_SERDESUnlockError:
		o << "| SERDES Unlock Error          | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadSERDESUnlockError(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESUnlockError(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESPLLLocked:
		o << "| SERDES PLL Locked            | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadSERDESPLLLocked(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESPLLLocked(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESTXBufferStatus:
		o << "| SERDES TX Buffer Status      | ([OF or UF, FIFO Half Full])" << std::endl;
		for (auto r : DTC_Rings)
		{
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (ReadSERDESOverflowOrUnderflow(r) ? "x" : " ") << ",";
			o << (ReadSERDESBufferFIFOHalfFull(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [";
		o << (ReadSERDESOverflowOrUnderflow(DTC_Ring_CFO) ? "x" : " ") << ",";
		o << (ReadSERDESBufferFIFOHalfFull(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXBufferStatus:
		o << "| SERDES RX Buffer Status      | ";
		for (auto r : DTC_Rings)
		{
			auto re = ReadSERDESRXBufferStatus(r);
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": " << DTC_RXBufferStatusConverter(re).toString() << "," << std::endl;
		}
		{
			auto ce = ReadSERDESRXBufferStatus(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    " << DTC_RXBufferStatusConverter(ce).toString();
		}
		break;
	case DTC_Register_SERDESRXStatus:
		o << "| SERDES RX Status             | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			auto re = ReadSERDESRXStatus(r);
			o << "Ring " << (int)r << ": " << DTC_RXStatusConverter(re).toString() << "," << std::endl;
		}
		{
			auto ce = ReadSERDESRXStatus(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    " << DTC_RXStatusConverter(ce).toString();
		}
		break;
	case DTC_Register_SERDESResetDone:
		o << "| SERDES Reset Done            | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadResetSERDESDone(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadResetSERDESDone(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESEyescanData:
		o << "| SERDES Eyescan Data Error    | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadSERDESEyescanError(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESEyescanError(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_SERDESRXCDRLock:
		o << "| SERDES RX CDR Lock           | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadSERDESRXCDRLock(r) ? "x" : " ") << "]," << std::endl;
		}
		o << "                                                        | ";
		o << "CFO:    [" << (ReadSERDESRXCDRLock(DTC_Ring_CFO) ? "x" : " ") << "]";
		break;
	case DTC_Register_DMATimeoutPreset:
		o << "| DMA Timeout                  | ";
		o << "0x" << ReadDMATimeoutPreset();
		break;
	case DTC_Register_ROCReplyTimeout:
		o << "| ROC Reply Timeout            | ";
		o << "0x" << ReadROCTimeoutPreset();
		break;
	case DTC_Register_ROCReplyTimeoutError:
		o << "| ROC Reply Timeout Error      | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << ", " << std::endl;
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": [" << (ReadROCTimeoutError(r) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_ReceivePacketError:
		o << "| Receive Packet Error         | ([CRC, PacketError, RX Overrun, RX Underrun])" << std::endl;
		for (auto r : DTC_Rings)
		{
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (ReadPacketCRCError(r) ? "x" : " ") << ",";
			o << (ReadPacketError(r) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferOverrun(r) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferUnderrun(r) ? "x" : " ") << "]," << std::endl;
		}
		{
			o << "                                                        | ";
			o << "CFO:    [";
			o << (ReadPacketCRCError(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadPacketError(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferOverrun(DTC_Ring_CFO) ? "x" : " ") << ",";
			o << (ReadRXElasticBufferUnderrun(DTC_Ring_CFO) ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_TimestampPreset0:
		o << "| Timestamp Preset 0           | ";
		o << "0x" << ReadRegister(DTC_Register_TimestampPreset0);
		break;
	case DTC_Register_TimestampPreset1:
		o << "| Timestamp Preset 1           | ";
		o << "0x" << ReadRegister(DTC_Register_TimestampPreset1);
		break;
	case DTC_Register_DataPendingTimer:
		o << "| DMA Data Pending Timer       | ";
		o << "0x" << ReadDataPendingTimer();
		break;
	case DTC_Register_NUMROCs:
		o << "| NUMROCs                      | ";
		for (auto r : DTC_Rings)
		{
			if ((int)r > 0)
			{
				o << ", " << std::endl;
				o << "                                                        | ";
			}
			o << "Ring " << (int)r << ": " << DTC_ROCIDConverter(ReadRingROCCount(r, false));
		}
		break;
	case DTC_Register_FIFOFullErrorFlag0:
		o << "| FIFO Full Error Flags 0      | ([DataRequest, ReadoutRequest, CFOLink, OutputData])";
		for (auto r : DTC_Rings)
		{
			o << "," << std::endl;
			o << "                                                        | ";
			auto re = ReadFIFOFullErrorFlags(r);
			o << "Ring " << (int)r << ": [";
			o << (re.DataRequestOutput ? "x" : " ") << ",";
			o << (re.ReadoutRequestOutput ? "x" : " ") << ",";
			o << (re.CFOLinkInput ? "x" : " ") << ",";
			o << (re.OutputData ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_FIFOFullErrorFlag1:
		o << "| FIFO Full Error Flags 1      | ([DataInput, OutputDCSStage2, OutputDCS, OtherOutput]) " << std::endl;
		for (auto r : DTC_Rings)
		{
			auto re = ReadFIFOFullErrorFlags(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [";
			o << (re.DataInput ? "x" : " ") << ",";
			o << (re.OutputDCSStage2 ? "x" : " ") << ",";
			o << (re.OutputDCS ? "x" : " ") << ",";
			o << (re.OtherOutput ? "x" : " ") << "]," << std::endl;
		}
		{
			auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [";
			o << (ce.DataInput ? "x" : " ") << ",";
			o << (ce.OutputDCSStage2 ? "x" : " ") << ",";
			o << (ce.OutputDCS ? "x" : " ") << ",";
			o << (ce.OtherOutput ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_FIFOFullErrorFlag2:
		o << "| FIFO Full Error Flags 2      | ([DCSStatusInput])" << std::endl;
		for (auto r : DTC_Rings)
		{
			auto re = ReadFIFOFullErrorFlags(r);
			o << "                                                        | ";
			o << "Ring " << (int)r << ": [" << (re.DCSStatusInput ? "x" : " ") << "]," << std::endl;
		}
		{
			auto ce = ReadFIFOFullErrorFlags(DTC_Ring_CFO);
			o << "                                                        | ";
			o << "CFO:    [" << (ce.DCSStatusInput ? "x" : " ") << "]";
		}
		break;
	case DTC_Register_CFOEmulationTimestampLow:
		o << "| CFO Emulation Timestamp Low  | ";
		o << "0x" << ReadRegister(DTC_Register_CFOEmulationTimestampLow);
		break;
	case DTC_Register_CFOEmulationTimestampHigh:
		o << "| CFO Emulation Timestamp High | ";
		o << "0x" << ReadRegister(DTC_Register_CFOEmulationTimestampHigh);
		break;
	case DTC_Register_CFOEmulationRequestInterval:
		o << "| CFO Emu. Request Interval    | ";
		o << "0x" << ReadCFOEmulationRequestInterval();
		break;
	case DTC_Register_CFOEmulationNumRequests:
		o << "| CFO Emulator Number Requests | ";
		o << "0x" << ReadCFOEmulationNumRequests();
		break;
	case DTC_Register_CFOEmulationNumPacketsRing0:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_0);
		break;
	case DTC_Register_CFOEmulationNumPacketsRing1:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_1);
		break;
	case DTC_Register_CFOEmulationNumPacketsRing2:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_2);
		break;
	case DTC_Register_CFOEmulationNumPacketsRing3:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_3);
		break;
	case DTC_Register_CFOEmulationNumPacketsRing4:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_4);
		break;
	case DTC_Register_CFOEmulationNumPacketsRing5:
		o << "| CFO Emulator Num Packets R0  | ";
		o << "0x" << ReadCFOEmulationNumPackets(DTC_Ring_5);
		break;
	case DTC_Register_RingPacketLength:
		o << "| DMA Ring Packet Length       | ";
		o << "0x" << ReadPacketSize();
		break;
	case DTC_Register_FPGAPROMProgramStatus:
		o << "| FPGA PROM Program Status     | ";
		o << "FPGA PROM Program FIFO Full: [" << (ReadFPGAPROMProgramFIFOFull() ? "x" : " ") << "]" << std::endl;
		o << "                                                        | ";
		o << "FPGA PROM Ready: [" << (ReadFPGAPROMReady() ? "x" : " ") << "]";
		break;
	case DTC_Register_FPGACoreAccess:
		o << "| FPGA Core Access             | ";
		o << "FPGA Core Access FIFO Full: [" << (ReadFPGACoreAccessFIFOFull() ? "x" : " ") << "]" << std::endl;
		o << "                                                        | ";
		o << "FPGA Core Access FIFO Empty: [" << (ReadFPGACoreAccessFIFOEmpty() ? "x" : " ") << "]";
		break;
	case DTC_Register_Invalid:
	default:
		o << "| Invalid Register             | !!!";
		break;
	}
	o << std::endl;
	return o.str();
}

std::string DTCLib::DTC_Registers::RegisterRead(const DTC_Register& address)
{
	uint32_t data = ReadRegister(address);
	std::stringstream stream;
	stream << std::hex << data;
	return std::string(stream.str());
}

//
// Register IO Functions
//
// Desgin Version/Date Registers
std::string DTCLib::DTC_Registers::ReadDesignVersion()
{
	return ReadDesignVersionNumber() + "_" + ReadDesignDate();
}

std::string DTCLib::DTC_Registers::ReadDesignDate()
{
	uint32_t data = ReadRegister(DTC_Register_DesignDate);
	std::ostringstream o;
	int yearHex = (data & 0xFF000000) >> 24;
	int year = ((yearHex & 0xF0) >> 4) * 10 + (yearHex & 0xF);
	int monthHex = (data & 0xFF0000) >> 16;
	int month = ((monthHex & 0xF0) >> 4) * 10 + (monthHex & 0xF);
	int dayHex = (data & 0xFF00) >> 8;
	int day = ((dayHex & 0xF0) >> 4) * 10 + (dayHex & 0xF);
	int hour = ((data & 0xF0) >> 4) * 10 + (data & 0xF);
	o << "20" << std::setfill('0') << std::setw(2) << year << "-";
	o << std::setfill('0') << std::setw(2) << month << "-";
	o << std::setfill('0') << std::setw(2) << day << "-";
	o << std::setfill('0') << std::setw(2) << hour;
	//std::cout << o.str() << std::endl;
	return o.str();
}

std::string DTCLib::DTC_Registers::ReadDesignVersionNumber()
{
	uint32_t data = ReadRegister(DTC_Register_DesignVersion);
	int minor = data & 0xFF;
	int major = (data & 0xFF00) >> 8;
	return "v" + std::to_string(major) + "." + std::to_string(minor);
}


// DTC Control Register
void DTCLib::DTC_Registers::ResetDTC()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[31] = 1; // DTC Reset bit
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadResetDTC()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_DTCControl);
	return dataSet[31];
}

void DTCLib::DTC_Registers::EnableCFOEmulation()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[30] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}

void DTCLib::DTC_Registers::DisableCFOEmulation()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[30] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadCFOEmulation()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_DTCControl);
	return dataSet[30];
}

void DTCLib::DTC_Registers::ResetSERDESOscillator()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[29] = 1; //SERDES Oscillator Reset bit
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	usleep(1000);
	data[29] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	while (!ReadSERDESOscillatorInitializationComplete())
	{
		usleep(1000);
	}
	for (auto ring : DTC_Rings)
	{
		ResetSERDES(ring);
	}
}

bool DTCLib::DTC_Registers::ReadResetSERDESOscillator()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[29];
}

void DTCLib::DTC_Registers::ToggleSERDESOscillatorClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(28);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);

	ResetSERDESOscillator();
}

bool DTCLib::DTC_Registers::ReadSERDESOscillatorClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[28];
}

void DTCLib::DTC_Registers::ResetDDRWriteAddress()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[27] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	data[27] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
}

bool DTCLib::DTC_Registers::ReadResetDDRWriteAddress()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[27];
}

bool DTCLib::DTC_Registers::EnableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[26] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadDetectorEmulatorEnable();
}

bool DTCLib::DTC_Registers::DisableDetectorEmulator()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[26] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadDetectorEmulatorEnable();
}

bool DTCLib::DTC_Registers::ReadDetectorEmulatorEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[26];
}

bool DTCLib::DTC_Registers::SetExternalSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[1] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}

bool DTCLib::DTC_Registers::SetInternalSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[1] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}

bool DTCLib::DTC_Registers::ToggleSystemClockEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(1);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadSystemClock();
}

bool DTCLib::DTC_Registers::ReadSystemClock()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[1];
}

bool DTCLib::DTC_Registers::EnableTiming()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[0] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}

bool DTCLib::DTC_Registers::DisableTiming()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data[0] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}

bool DTCLib::DTC_Registers::ToggleTimingEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	data.flip(0);
	WriteRegister(data.to_ulong(), DTC_Register_DTCControl);
	return ReadTimingEnable();
}

bool DTCLib::DTC_Registers::ReadTimingEnable()
{
	std::bitset<32> data = ReadRegister(DTC_Register_DTCControl);
	return data[0];
}


// DMA Transfer Length Register
int DTCLib::DTC_Registers::SetTriggerDMATransferLength(uint16_t length)
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = (data & 0x0000FFFF) + (length << 16);
	WriteRegister(data, DTC_Register_DMATransferLength);
	return ReadTriggerDMATransferLength();
}

uint16_t DTCLib::DTC_Registers::ReadTriggerDMATransferLength()
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data >>= 16;
	return static_cast<uint16_t>(data);
}

int DTCLib::DTC_Registers::SetMinDMATransferLength(uint16_t length)
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = (data & 0xFFFF0000) + length;
	WriteRegister(data, DTC_Register_DMATransferLength);
	return ReadMinDMATransferLength();
}

uint16_t DTCLib::DTC_Registers::ReadMinDMATransferLength()
{
	uint32_t data = ReadRegister(DTC_Register_DMATransferLength);
	data = data & 0x0000FFFF;
	dmaSize_ = static_cast<uint16_t>(data);
	return dmaSize_;
}


// SERDES Loopback Enable Register
DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC_Registers::SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_SERDESLoopbackEnable);
	std::bitset<3> modeSet = mode;
	data[3 * ring] = modeSet[0];
	data[3 * ring + 1] = modeSet[1];
	data[3 * ring + 2] = modeSet[2];
	WriteRegister(data.to_ulong(), DTC_Register_SERDESLoopbackEnable);
	return ReadSERDESLoopback(ring);
}

DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC_Registers::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESLoopbackEnable) >> (3 * ring));
	return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}


// SERDES Status Register
bool DTCLib::DTC_Registers::ReadSERDESOscillatorIICError()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESOscillatorStatus);
	return dataSet[2];
}

bool DTCLib::DTC_Registers::ReadSERDESOscillatorInitializationComplete()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESOscillatorStatus);
	return dataSet[1];
}


// ROC Emulation Enable Register
bool DTCLib::DTC_Registers::EnableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 1;
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}

bool DTCLib::DTC_Registers::DisableROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = 0;
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}

bool DTCLib::DTC_Registers::ToggleROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	dataSet[ring] = !dataSet[ring];
	WriteRegister(dataSet.to_ulong(), DTC_Register_ROCEmulationEnable);
	return ReadROCEmulator(ring);
}

bool DTCLib::DTC_Registers::ReadROCEmulator(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_ROCEmulationEnable);
	return dataSet[ring];
}


// Ring Enable Register
DTCLib::DTC_RingEnableMode DTCLib::DTC_Registers::EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	data[ring] = mode.TransmitEnable;
	data[ring + 8] = mode.ReceiveEnable;
	data[ring + 16] = mode.TimingEnable;
	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	SetMaxROCNumber(ring, lastRoc);
	return ReadRingEnabled(ring);
}

DTCLib::DTC_RingEnableMode DTCLib::DTC_Registers::DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	data[ring] = data[ring] && !mode.TransmitEnable;
	data[ring + 8] = data[ring + 8] && !mode.ReceiveEnable;
	data[ring + 16] = data[ring + 16] && !mode.TimingEnable;
	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	return ReadRingEnabled(ring);
}

DTCLib::DTC_RingEnableMode DTCLib::DTC_Registers::ToggleRingEnabled(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode)
{
	std::bitset<32> data = ReadRegister(DTC_Register_RingEnable);
	if (mode.TransmitEnable)
	{
		data.flip((uint8_t)ring);
	}
	if (mode.ReceiveEnable)
	{
		data.flip((uint8_t)ring + 8);
	}
	if (mode.TimingEnable)
	{
		data.flip((uint8_t)ring + 16);
	}

	WriteRegister(data.to_ulong(), DTC_Register_RingEnable);
	return ReadRingEnabled(ring);
}

DTCLib::DTC_RingEnableMode DTCLib::DTC_Registers::ReadRingEnabled(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_RingEnable);
	return DTC_RingEnableMode(dataSet[ring], dataSet[ring + 8], dataSet[ring + 16]);
}

//
// SERDES Registers
//
// SERDES Reset Register
bool DTCLib::DTC_Registers::ResetSERDES(const DTC_Ring_ID& ring, int interval)
{
	bool resetDone = false;
	while (!resetDone)
	{
		TRACE(0, "Entering SERDES Reset Loop for Ring %u", ring);
		std::bitset<32> data = ReadRegister(DTC_Register_SERDESReset);
		data[ring] = 1;
		WriteRegister(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		data = ReadRegister(DTC_Register_SERDESReset);
		data[ring] = 0;
		WriteRegister(data.to_ulong(), DTC_Register_SERDESReset);

		usleep(interval);

		resetDone = ReadResetSERDESDone(ring);
		TRACE(0, "End of SERDES Reset loop, done=%s", (resetDone ? "true" : "false"));
	}
	return resetDone;
}

bool DTCLib::DTC_Registers::ReadResetSERDES(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESReset);
	return dataSet[ring];
}


// SERDES RX Disparity Error Register
DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC_Registers::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
	return DTC_SERDESRXDisparityError(ReadRegister(DTC_Register_SERDESRXDisparityError), ring);
}

// SERDES RX Character Not In Table Error Register
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC_Registers::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
	return DTC_CharacterNotInTableError(ReadRegister(DTC_Register_SERDESRXCharacterNotInTableError), ring);
}

// SERDES Unlock Error Register
bool DTCLib::DTC_Registers::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESUnlockError);
	return dataSet[ring];
}

// SERDES PLL Locked Register
bool DTCLib::DTC_Registers::ReadSERDESPLLLocked(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESPLLLocked);
	return dataSet[ring];
}

// SERDES TX Buffer Status Register
bool DTCLib::DTC_Registers::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2 + 1];
}

bool DTCLib::DTC_Registers::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESTXBufferStatus);
	return dataSet[ring * 2];
}


// SERDES RX Buffer Status Register
DTCLib::DTC_RXBufferStatus DTCLib::DTC_Registers::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESRXBufferStatus) >> (3 * ring));
	return static_cast<DTC_RXBufferStatus>(dataSet.to_ulong());
}

// SERDES RX Status Register
DTCLib::DTC_RXStatus DTCLib::DTC_Registers::ReadSERDESRXStatus(const DTC_Ring_ID& ring)
{
	std::bitset<3> dataSet = (ReadRegister(DTC_Register_SERDESRXStatus) >> (3 * ring));
	return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

// SERDES Reset Done Register
bool DTCLib::DTC_Registers::ReadResetSERDESDone(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESResetDone);
	return dataSet[ring];
}

// SERDES Eyescan Data Error Register
bool DTCLib::DTC_Registers::ReadSERDESEyescanError(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESEyescanData);
	return dataSet[ring];
}

// SERDES RX CDR Lock Register
bool DTCLib::DTC_Registers::ReadSERDESRXCDRLock(const DTC_Ring_ID& ring)
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_SERDESRXCDRLock);
	return dataSet[ring];
}


// DMA Timeout Preset Register
int DTCLib::DTC_Registers::WriteDMATimeoutPreset(uint32_t preset)
{
	WriteRegister(preset, DTC_Register_DMATimeoutPreset);
	return ReadDMATimeoutPreset();
}

uint32_t DTCLib::DTC_Registers::ReadDMATimeoutPreset()
{
	return ReadRegister(DTC_Register_DMATimeoutPreset);
}


// ROC Timeout Preset Register
uint32_t DTCLib::DTC_Registers::ReadROCTimeoutPreset()
{
	return ReadRegister(DTC_Register_ROCReplyTimeout);
}

int DTCLib::DTC_Registers::WriteROCTimeoutPreset(uint32_t preset)
{
	WriteRegister(preset, DTC_Register_ROCReplyTimeout);
	return ReadROCTimeoutPreset();
}


// ROC Timeout Error Register
bool DTCLib::DTC_Registers::ReadROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ROCReplyTimeoutError);
	return data[(int)ring];
}

bool DTCLib::DTC_Registers::ClearROCTimeoutError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = 0x0;
	data[ring] = 1;
	WriteRegister(data.to_ulong(), DTC_Register_ROCReplyTimeoutError);
	return ReadROCTimeoutError(ring);
}


// Ring Packet Length Register
int DTCLib::DTC_Registers::SetPacketSize(uint16_t packetSize)
{
	WriteRegister(0x00000000 + packetSize, DTC_Register_RingPacketLength);
	return ReadPacketSize();
}

uint16_t DTCLib::DTC_Registers::ReadPacketSize()
{
	return static_cast<uint16_t>(ReadRegister(DTC_Register_RingPacketLength));
}


// Timestamp Preset Registers
DTCLib::DTC_Timestamp DTCLib::DTC_Registers::WriteTimestampPreset(const DTC_Timestamp& preset)
{
	std::bitset<48> timestamp = preset.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister(timestampLow, DTC_Register_TimestampPreset0);
	WriteRegister(timestampHigh, DTC_Register_TimestampPreset1);
	return ReadTimestampPreset();
}

DTCLib::DTC_Timestamp DTCLib::DTC_Registers::ReadTimestampPreset()
{
	uint32_t timestampLow = ReadRegister(DTC_Register_TimestampPreset0);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister(DTC_Register_TimestampPreset1)));
	return output;
}


// Data Pending Timer Register
int DTCLib::DTC_Registers::WriteDataPendingTimer(uint32_t timer)
{
	WriteRegister(timer, DTC_Register_DataPendingTimer);
	return ReadDataPendingTimer();
}

uint32_t DTCLib::DTC_Registers::ReadDataPendingTimer()
{
	return ReadRegister(DTC_Register_DataPendingTimer);
}


// NUMROCs Register
DTCLib::DTC_ROC_ID DTCLib::DTC_Registers::SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
	std::bitset<32> ringRocs = ReadRegister(DTC_Register_NUMROCs);
	maxROCs_[ring] = lastRoc;
	int numRocs = (lastRoc == DTC_ROC_Unused) ? 0 : lastRoc + 1;
	ringRocs[ring * 3] = numRocs & 1;
	ringRocs[ring * 3 + 1] = ((numRocs & 2) >> 1) & 1;
	ringRocs[ring * 3 + 2] = ((numRocs & 4) >> 2) & 1;
	WriteRegister(ringRocs.to_ulong(), DTC_Register_NUMROCs);
	return ReadRingROCCount(ring);
}

DTCLib::DTC_ROC_ID DTCLib::DTC_Registers::ReadRingROCCount(const DTC_Ring_ID& ring, bool local)
{
	if (local)
	{
		return maxROCs_[ring];
	}
	std::bitset<32> ringRocs = ReadRegister(DTC_Register_NUMROCs);
	int number = ringRocs[ring * 3] + (ringRocs[ring * 3 + 1] << 1) + (ringRocs[ring * 3 + 2] << 2);
	return DTC_ROCS[number];
}


// FIFO Full Error Flags Registers
DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC_Registers::WriteFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);

	data0[ring] = flags.OutputData;
	data0[ring + 8] = flags.CFOLinkInput;
	data0[ring + 16] = flags.ReadoutRequestOutput;
	data0[ring + 24] = flags.DataRequestOutput;
	data1[ring] = flags.OtherOutput;
	data1[ring + 8] = flags.OutputDCS;
	data1[ring + 16] = flags.OutputDCSStage2;
	data1[ring + 24] = flags.DataInput;
	data2[ring] = flags.DCSStatusInput;

	WriteRegister(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);

	return ReadFIFOFullErrorFlags(ring);
}

DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC_Registers::ToggleFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);

	data0[ring] = flags.OutputData ? !data0[ring] : data0[ring];
	data0[ring + 8] = flags.CFOLinkInput ? !data0[ring + 8] : data0[ring + 8];
	data0[ring + 16] = flags.ReadoutRequestOutput ? !data0[ring + 16] : data0[ring + 16];
	data0[ring + 24] = flags.DataRequestOutput ? !data0[ring + 24] : data0[ring + 24];
	data1[ring] = flags.OtherOutput ? !data1[ring] : data1[ring];
	data1[ring + 8] = flags.OutputDCS ? !data1[ring + 8] : data1[ring + 8];
	data1[ring + 16] = flags.OutputDCSStage2 ? !data1[ring + 16] : data1[ring + 16];
	data1[ring + 24] = flags.DataInput ? !data1[ring + 24] : data1[ring + 24];
	data2[ring] = flags.DCSStatusInput ? !data2[ring] : data2[ring];

	WriteRegister(data0.to_ulong(), DTC_Register_FIFOFullErrorFlag0);
	WriteRegister(data1.to_ulong(), DTC_Register_FIFOFullErrorFlag1);
	WriteRegister(data2.to_ulong(), DTC_Register_FIFOFullErrorFlag2);

	return ReadFIFOFullErrorFlags(ring);
}

DTCLib::DTC_FIFOFullErrorFlags DTCLib::DTC_Registers::ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring)
{
	std::bitset<32> data0 = ReadRegister(DTC_Register_FIFOFullErrorFlag0);
	std::bitset<32> data1 = ReadRegister(DTC_Register_FIFOFullErrorFlag1);
	std::bitset<32> data2 = ReadRegister(DTC_Register_FIFOFullErrorFlag2);
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


// Receive Packet Error Register
bool DTCLib::DTC_Registers::ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 24];
}

bool DTCLib::DTC_Registers::ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 24] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC_Registers::ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 16];
}

bool DTCLib::DTC_Registers::ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 16] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC_Registers::ReadPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring + 8];
}

bool DTCLib::DTC_Registers::ClearPacketError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring + 8] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

bool DTCLib::DTC_Registers::ReadPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	return data[(int)ring];
}

bool DTCLib::DTC_Registers::ClearPacketCRCError(const DTC_Ring_ID& ring)
{
	std::bitset<32> data = ReadRegister(DTC_Register_ReceivePacketError);
	data[(int)ring] = 0;
	WriteRegister(data.to_ulong(), DTC_Register_ReceivePacketError);
	return ReadRXElasticBufferUnderrun(ring);
}

//
// CFO Emulator Registers
//
// CFO Emulation Timestamp Registers
void DTCLib::DTC_Registers::SetCFOEmulationTimestamp(const DTC_Timestamp& ts)
{
	std::bitset<48> timestamp = ts.GetTimestamp();
	uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
	timestamp >>= 32;
	uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

	WriteRegister(timestampLow, DTC_Register_CFOEmulationTimestampLow);
	WriteRegister(timestampHigh, DTC_Register_CFOEmulationTimestampHigh);
}

DTCLib::DTC_Timestamp DTCLib::DTC_Registers::ReadCFOEmulationTimestamp()
{
	uint32_t timestampLow = ReadRegister(DTC_Register_CFOEmulationTimestampLow);
	DTC_Timestamp output;
	output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationTimestampHigh)));
	return output;
}


// CFO Emulation Request Interval Register
void DTCLib::DTC_Registers::SetCFOEmulationRequestInterval(uint32_t interval)
{
	WriteRegister(interval, DTC_Register_CFOEmulationRequestInterval);
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulationRequestInterval()
{
	return ReadRegister(DTC_Register_CFOEmulationRequestInterval);
}


// CFO Emulation Number of Requests Register
void DTCLib::DTC_Registers::SetCFOEmulationNumRequests(uint32_t numRequests)
{
	WriteRegister(numRequests, DTC_Register_CFOEmulationNumRequests);
}

uint32_t DTCLib::DTC_Registers::ReadCFOEmulationNumRequests()
{
	return ReadRegister(DTC_Register_CFOEmulationNumRequests);
}


// CFO Emulation Number of Packets Registers
void DTCLib::DTC_Registers::SetCFOEmulationNumPackets(const DTC_Ring_ID& ring, uint16_t numPackets)
{
	uint32_t data = numPackets & 0x7FF;
	switch (ring)
	{
	case DTC_Ring_0:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing0);
	case DTC_Ring_1:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing1);
	case DTC_Ring_2:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing2);
	case DTC_Ring_3:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing3);
	case DTC_Ring_4:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing4);
	case DTC_Ring_5:
		return WriteRegister(data, DTC_Register_CFOEmulationNumPacketsRing5);
	default:
		break;
	}
}

uint16_t DTCLib::DTC_Registers::ReadCFOEmulationNumPackets(const DTC_Ring_ID& ring)
{
	switch (ring)
	{
	case DTC_Ring_0:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing0));
	case DTC_Ring_1:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing1));
	case DTC_Ring_2:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing2));
	case DTC_Ring_3:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing3));
	case DTC_Ring_4:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing4));
	case DTC_Ring_5:
		return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPacketsRing5));
	default:
		return 0;
	}
}


// CFO Emulation Debug Packet Type Register
void DTCLib::DTC_Registers::SetCFOEmulationDebugType(DTC_DebugType type)
{
	std::bitset<32> data = type & 0xF;
	WriteRegister(data.to_ulong(), DTC_Register_CFOEmulationDebugPacketType);
}

DTCLib::DTC_DebugType DTCLib::DTC_Registers::ReadCFOEmulationDebugType()
{
	return static_cast<DTC_DebugType>(ReadRegister(DTC_Register_CFOEmulationDebugPacketType));
}


//
// Detector Emulator Registers
//
// Detector Emulator DMA Count Register
void DTCLib::DTC_Registers::SetDetectorEmulationDMACount(uint32_t count)
{
	WriteRegister(count, DTC_Register_DetEmulationDMACount);
}

uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMACount()
{
	return ReadRegister(DTC_Register_DetEmulationDMACount);
}


// Detector Emulator DMA Delay Counter Register
void DTCLib::DTC_Registers::SetDetectorEmulationDMADelayCount(uint32_t count)
{
	WriteRegister(count, DTC_Register_DetEmulationDelayCount);
}

uint32_t DTCLib::DTC_Registers::ReadDetectorEmulationDMADelayCount()
{
	return ReadRegister(DTC_Register_DetEmulationDelayCount);
}


//
// FPGA Registers
//
// FPGA PROM Program Status Register
bool DTCLib::DTC_Registers::ReadFPGAPROMProgramFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[1];
}

bool DTCLib::DTC_Registers::ReadFPGAPROMReady()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGAPROMProgramStatus);
	return dataSet[0];
}


// FPGA Core Access Register
void DTCLib::DTC_Registers::ReloadFPGAFirmware()
{
	WriteRegister(0xFFFFFFFF, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0xAA995566, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x20000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x30020001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x00000000, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x30008001, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x0000000F, DTC_Register_FPGACoreAccess);
	while (ReadFPGACoreAccessFIFOFull())
	{
		usleep(10);
	}
	WriteRegister(0x20000000, DTC_Register_FPGACoreAccess);
}

bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOFull()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGACoreAccess);
	return dataSet[1];
}

bool DTCLib::DTC_Registers::ReadFPGACoreAccessFIFOEmpty()
{
	std::bitset<32> dataSet = ReadRegister(DTC_Register_FPGACoreAccess);
	return dataSet[0];
}



// Private Functions
void DTCLib::DTC_Registers::WriteRegister(uint32_t data, const DTC_Register& address)
{
	int retry = 3;
	int errorCode = 0;
	do
	{
		errorCode = device_.write_register(address, 100, data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}
}

uint32_t DTCLib::DTC_Registers::ReadRegister(const DTC_Register& address)
{
	int retry = 3;
	int errorCode;
	uint32_t data;
	do
	{
		errorCode = device_.read_register(address, 100, &data);
		--retry;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw new DTC_IOErrorException();
	}

	return data;
}


