// This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <cstdio> // printf

#include <cstdlib> // strtoul

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cmath>
#include "DTC.h"
#ifdef _WIN32
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# ifndef TRACE
#  include <stdio.h>
#  ifdef _DEBUG
#   define TRACE(lvl,...) printf(__VA_ARGS__); printf("\n")
#  else
#   define TRACE(...)
#  endif
# define TRACE_CNTL(...)
# endif
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif
#define TRACE_NAME "MU2EDEV"

using namespace DTCLib;

unsigned getOptionValue(int* index, char** argv[])
{
	auto arg = (*argv)[*index];
	if (arg[2] == '\0')
	{
		(*index)++;
		return strtoul((*argv)[*index], nullptr, 0);
	}
	auto offset = 2;
	if (arg[2] == '=')
	{
		offset = 3;
	}

	return strtoul(&arg[offset], nullptr, 0);
}

std::string getOptionString(int* index, char** argv[])
{
	auto arg = (*argv)[*index];
	if (arg[2] == '\0')
	{
		(*index)++;
		return std::string((*argv)[*index]);
	}
	auto offset = 2;
	if (arg[2] == '=')
	{
		offset = 3;
	}

	return std::string(&arg[offset]);
}

void printHelpMsg()
{
	std::cout << "Usage: rocUtil [options] [read_register,reset_roc,write_register,write_extregister,test_read,read_release,toggle_serdes]" << std::endl;
	std::cout << "Options are:" << std::endl
		<< "    -h: This message." << std::endl
		<< "    -n: Number of times to repeat test. (Default: 1)" << std::endl
		<< "    -d: Delay between tests, in us (Default: 0)." << std::endl
		<< "    -w: Data to write to address" << std::endl
		<< "    -a: Address to write" << std::endl
		<< "    -b: Block address (for write_rocext)" << std::endl
		<< "    -q: Quiet mode (Don't print requests)" << std::endl
		<< "    -Q: Really Quiet mode (Try not to print anything)" << std::endl
		<< "    -v: Expected DTC Design version string (Default: \"\")" << std::endl
		;
	exit(0);
}

int main(int argc, char* argv[])
{
	auto quiet = false;
	auto reallyQuiet = false;
	unsigned delay = 0;
	unsigned number = 1;
	unsigned address = 0;
	unsigned data = 0;
	unsigned block = 0;
	std::string expectedDesignVersion = "";
	std::string op = "";

	for (auto optind = 1; optind < argc; ++optind)
	{
		if (argv[optind][0] == '-')
		{
			switch (argv[optind][1])
			{
			case 'd':
				delay = getOptionValue(&optind, &argv);
				break;
			case 'n':
				number = getOptionValue(&optind, &argv);
				break;
			case 'w':
				data = getOptionValue(&optind, &argv);
				break;
			case 'a':
				address = getOptionValue(&optind, &argv);
				break;
			case 'b':
				block = getOptionValue(&optind, &argv);
				break;
			case 'q':
				quiet = true;
				break;
			case 'Q':
				quiet = true;
				reallyQuiet = true;
				break;
			case 'v':
				expectedDesignVersion = getOptionString(&optind, &argv);
				break;
			default:
				std::cout << "Unknown option: " << argv[optind] << std::endl;
				printHelpMsg();
				break;
			case 'h':
				printHelpMsg();
				break;
			}
		}
		else
		{
			op = std::string(argv[optind]);
		}
	}

	std::cout.setf(std::ios_base::boolalpha);
	std::cout << "Options are: "
		<< "Operation: " << std::string(op)
		<< ", Num: " << number
		<< ", Delay: " << delay
		<< ", Address: " << address
		<< ", Data: " << data
		<< ", Block: " << block
		<< ", Quiet Mode: " << quiet
		<< ", Really Quiet Mode: " << reallyQuiet
		<< std::endl;


	auto thisDTC = new DTC(expectedDesignVersion, DTC_SimMode_NoCFO);
	auto device = thisDTC->GetDevice();

	if (op == "read_register")
	{
		std::cout << "Operation \"read_register\"" << std::endl;
		auto rocdata = thisDTC->ReadROCRegister(DTC_Ring_0, DTC_ROC_0, address);
		if (!reallyQuiet) std::cout << rocdata << '\n';
	}
	else if (op == "reset_roc")
	{
		std::cout << "Operation \"reset_roc\"" << std::endl;
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 12, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 11, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 10, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 9, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 8, 1, 0x11);
	}
	else if (op == "write_register")
	{
		std::cout << "Operation \"write_register\"" << std::endl;
		thisDTC->WriteROCRegister(DTC_Ring_0, DTC_ROC_0, address, data);
	}
	else if (op == "write_extregister")
	{
		std::cout << "Operation \"write_extregister\"" << std::endl;
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, block, address, data);
	}
	else if (op == "test_read")
	{
		std::cout << "Operation \"test_read\"" << std::endl;

		thisDTC->SendDCSRequestPacket(DTC_Ring_0, DTC_ROC_0, DTC_DCSOperationType_Read, address, quiet);

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) std::cout << "Buffer Read " << ii << std::endl;
			mu2e_databuff_t* buffer;
			auto tmo_ms = 1500;
			auto sts = device->read_data(DTC_DMA_Engine_DCS, reinterpret_cast<void**>(&buffer), tmo_ms);

			TRACE(1, "util - read for DCS - ii=%u sts=%d %p", ii, sts, static_cast<void*>(buffer));
			if (sts > 0)
			{
				auto bufSize = *reinterpret_cast<uint64_t*>(&buffer[0]);
				TRACE(1, "util - bufSize is %llu", static_cast<unsigned long long>(bufSize));

				if (!reallyQuiet)
				{
					for (unsigned line = 0; line < static_cast<unsigned>(ceil((bufSize - 8) / 16)); ++line)
					{
						std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if (line * 16 + 2 * byte < bufSize - 8u)
							{
								auto thisWord = reinterpret_cast<uint16_t*>(buffer)[4 + line * 8 + byte];
								//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
								std::cout << std::setw(4) << static_cast<int>(thisWord) << " ";
							}
						}
						std::cout << std::endl;
					}
				}
			}
			if (!reallyQuiet) std::cout << std::endl << std::endl;
			device->read_release(DTC_DMA_Engine_DCS, 1);
			if (delay > 0)
			usleep(delay);
		}
	}
	else if (op == "toggle_serdes")
	{
		if (!thisDTC->ReadSERDESOscillatorClock())
		{
			std::cout << "Setting SERDES Oscillator Clock to 2.5 Gbps" << std::endl;
			thisDTC->SetSERDESOscillatorClock(DTC_SerdesClockSpeed_25Gbps);
		}
		else
		{
			std::cout << "Setting SERDES Oscillator Clock to 3.125 Gbps" << std::endl;
			thisDTC->SetSERDESOscillatorClock(DTC_SerdesClockSpeed_3125Gbps);
		}
	}
	else if (op == "read_release")
	{
		for (unsigned ii = 0; ii < number; ++ii)
		{
			void* buffer;
			auto tmo_ms = 0;
			auto stsRD = device->read_data(DTC_DMA_Engine_DCS, &buffer, tmo_ms);
			auto stsRL = device->read_release(DTC_DMA_Engine_DCS, 1);
			TRACE(12, "dcs - release/read for DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
			if (delay > 0)
			usleep(delay);
		}
	}
	else
	{
		std::cout << "Unrecognized operation: " << op << std::endl;
		printHelpMsg();
	}

	delete thisDTC;
	return 0;
} // main


