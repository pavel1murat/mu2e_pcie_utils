// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

#include "dtcInterfaceLib/DTC_Registers.h"

void printHelpMsg()
{
	std::cout << "Usage: DTCRegDump [options]" << std::endl;
	std::cout << "Options are:" << std::endl
			  << "    -h: This message." << std::endl
			  << "    -R: DON'T Print Register Dump." << std::endl
			  << "    -s: Print SERDES Byte and Packet Counters." << std::endl
			  << "    -p: Print Performance Counters." << std::endl
			  << "    -e: Print SERDES Error Counters" << std::endl
			  << "    -c: Print Mu2e protocol packet Counters" << std::endl
			  << "    -m: Use <file> as the emulated DTC memory area" << std::endl
			  << "    -d: DTC instance to use (defaults to DTCLIB_DTC if set, 0 otherwise)" << std::endl;

	exit(0);
}

int main(int argc, char* argv[])
{
	auto printSERDESCounters = false;
	auto printRegisterDump = true;
	auto printSERDESErrors = false;
	auto printProtocolCounters = false;
	auto printPerformanceCounters = false;
	int dtc = -1;
	std::string memFileName = "mu2esim.bin";

	for (auto optind = 1; optind < argc; ++optind)
	{
		if (argv[optind][0] == '-')
		{
			switch (argv[optind][1])
			{
				case 's':
					printSERDESCounters = true;
					break;
				case 'R':
					printRegisterDump = false;
					break;
				case 'd':
					dtc = DTCLib::Utilities::getOptionValue(&optind, &argv);
					break;
				case 'm':
					memFileName = DTCLib::Utilities::getOptionString(&optind, &argv);
					break;
				case 'p':
					printPerformanceCounters = true;
					break;
				case 'e':
					printSERDESErrors = true;
					break;
				case 'c':
					printProtocolCounters = true;
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
	}

	auto thisDTC = new DTCLib::DTC_Registers(DTCLib::DTC_SimMode_Disabled, dtc,memFileName, 0x1, "", true);

	auto cols = 80;
	auto lines = 24;

#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	cols = ts.ts_cols;
	lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	cols = ts.ws_col;
	lines = ts.ws_row;
#endif /* TIOCGSIZE */

	printf("Terminal is %dx%d\n", cols, lines);

	if (cols > 400)
	{
		cols = 120;
	}

	if (printRegisterDump)
	{
		std::cout << thisDTC->FormattedRegDump(cols) << std::endl;
	}

	if (printSERDESCounters)
	{
		std::cout << std::endl
				  << std::endl;
		std::cout << thisDTC->LinkCountersRegDump(cols);
	}

	if (printPerformanceCounters)
	{
		std::cout << std::endl
				  << std::endl;
		std::cout << thisDTC->PerformanceCountersRegDump(cols);
	}

	if (printSERDESErrors)
	{
		std::cout << std::endl
				  << std::endl;
		std::cout << thisDTC->SERDESErrorsRegDump(cols);
	}

	if (printProtocolCounters)
	{
		std::cout << std::endl
				  << std::endl;
		std::cout << thisDTC->PacketCountersRegDump(cols);
	}

	delete thisDTC;
	return 0;
}
