// This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <cstdio>		// printf
#include <cstdlib>		// strtoul
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cmath>
#include "DTC.h"
#include "DTCSoftwareCFO.h"
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
# endif
# define TRACE_CNTL(...)
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif
#define TRACE_NAME "MU2EDEV"

using namespace DTCLib;
using namespace std;

unsigned getOptionValue(int *index, char **argv[])
{
	char* arg = (*argv)[*index];
	if (arg[2] == '\0')
	{
		(*index)++;
		return strtoul((*argv)[*index], nullptr, 0);
	}
	else
	{
		int offset = 2;
		if (arg[2] == '=')
		{
			offset = 3;
		}

		return strtoul(&(arg[offset]), nullptr, 0);
	}
}

std::string getOptionString(int *index, char **argv[])
{
	char* arg = (*argv)[*index];
	if (arg[2] == '\0')
	{
		(*index)++;
		return std::string((*argv)[*index]);
	}
	else
	{
		int offset = 2;
		if (arg[2] == '=')
		{
			offset = 3;
		}

		return std::string(&(arg[offset]));
	}
}

void printHelpMsg()
{
	cout << "Usage: rocUtil [options] [read_register,reset_roc,write_register,write_extregister,test_read,read_release,toggle_serdes]" << endl;
	cout << "Options are:" << endl
		<< "    -h: This message." << endl
		<< "    -n: Number of times to repeat test. (Default: 1)" << endl
		<< "    -d: Delay between tests, in us (Default: 0)." << endl
		<< "    -w: Data to write to address" << endl
		<< "    -a: Address to write" << endl
		<< "    -b: Block address (for write_rocext)" << endl
		<< "    -q: Quiet mode (Don't print requests)" << endl
		<< "    -Q: Really Quiet mode (Try not to print anything)" << endl;
	exit(0);
}

int
main(int	argc
	 , char	*argv[])
{
	bool quiet = false;
	bool reallyQuiet = false;
	unsigned delay = 0;
	unsigned number = 1;
	unsigned address = 0;
	unsigned data = 0;
	unsigned block = 0;
	string op = "";

	for (int optind = 1; optind < argc; ++optind)
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
			default:
				cout << "Unknown option: " << argv[optind] << endl;
				printHelpMsg();
				break;
			case 'h':
				printHelpMsg();
				break;
			}
		}
		else
		{
			op = string(argv[optind]);
		}
	}

	cout.setf(std::ios_base::boolalpha);
	cout << "Options are: "
		<< "Operation: " << string(op)
		<< ", Num: " << number
		<< ", Delay: " << delay
		<< ", Address: " << address
		<< ", Data: " << data
		<< ", Block: " << block
		<< ", Quiet Mode: " << quiet
		<< ", Really Quiet Mode: " << reallyQuiet
		<< endl;

	if (op == "read_register")
	{
		cout << "Operation \"read_register\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		auto rocdata = thisDTC->ReadROCRegister(DTC_Ring_0, DTC_ROC_0, address);
		if (!reallyQuiet) cout << rocdata << '\n';
	}
	else if (op == "reset_roc")
	{
		cout << "Operation \"reset_roc\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 12, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 11, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 10, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 9, 1, 0x11);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, 8, 1, 0x11);
	}
	else if (op == "write_register")
	{
		cout << "Operation \"write_register\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		thisDTC->WriteROCRegister(DTC_Ring_0, DTC_ROC_0, address, data);
	}
	else if (op == "write_extregister")
	{
		cout << "Operation \"write_extregister\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		thisDTC->WriteExtROCRegister(DTC_Ring_0, DTC_ROC_0, block, address, data);
	}
	else if (op == "test_read")
	{
		cout << "Operation \"test_read\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		if (!thisDTC->ReadSERDESOscillatorClock()) { thisDTC->ToggleSERDESOscillatorClock(); } // We're going to 2.5Gbps for now

		mu2edev* device = thisDTC->GetDevice();
		thisDTC->SendDCSRequestPacket(DTC_Ring_0, DTC_ROC_0, DTC_DCSOperationType_Read,address, quiet);

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) cout << "Buffer Read " << ii << endl;
			mu2e_databuff_t* buffer;
			int tmo_ms = 1500;
			int sts = device->read_data(DTC_DMA_Engine_DCS, (void**)&buffer, tmo_ms);

			TRACE(1, "util - read for DCS - ii=%u sts=%d %p", ii, sts, (void*)buffer);
			if (sts > 0)
			{
				void* readPtr = &(buffer[0]);
				uint16_t bufSize = static_cast<uint16_t>(*((uint64_t*)readPtr));
				readPtr = (uint8_t*)readPtr + 8;
				TRACE(1, "util - bufSize is %u", bufSize);

				if (!reallyQuiet)
				{
					for (unsigned line = 0; line < (unsigned)(ceil((bufSize - 8) / 16)); ++line)
					{
						cout << "0x" << hex << setw(5) << setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if ((line * 16) + (2 * byte) < (bufSize - 8u))
							{
								uint16_t thisWord = (((uint16_t*)buffer)[4 + (line * 8) + byte]);
								//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
								cout << setw(4) << (int)thisWord << " ";
							}
						}
						cout << endl;
					}
				}
			}
			if (!reallyQuiet) cout << endl << endl;
			device->read_release(DTC_DMA_Engine_DCS, 1);
			if (delay > 0) usleep(delay);
		}
	}
	else if (op == "toggle_serdes")
	{
		cout << "Setting SERDES Oscillator Clock to 2.5 Gbps" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
                if(!thisDTC->ReadSERDESOscillatorClock()) 
		{
		  thisDTC->ToggleSERDESOscillatorClock();
                }
	}
	else if (op == "read_release")
	{
		mu2edev device;
		device.init();
		for (unsigned ii = 0; ii < number; ++ii)
		{
			void *buffer;
			int tmo_ms = 0;
			int stsRD = device.read_data(DTC_DMA_Engine_DCS, &buffer, tmo_ms);
			int stsRL = device.read_release(DTC_DMA_Engine_DCS, 1);
			TRACE(12, "dcs - release/read for DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
			if (delay > 0) usleep(delay);
		}
	}
	else
	{
		std::cout << "Unrecognized operation: " << op << std::endl;
		printHelpMsg();
	}
	return (0);
}   // main
