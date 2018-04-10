// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <stdio.h>
#include <iostream>
#include "dtcInterfaceLib/DTC_Registers.h"
#include <sys/ioctl.h>
#include <unistd.h>

void printHelpMsg()
{
	std::cout << "Usage: DTCRegDump [options]" << std::endl;
	std::cout << "Options are:" << std::endl
		<< "    -h: This message." << std::endl
		<< "    -R: DON'T Print Register Dump." << std::endl
		<< "    -s: Print SERDES Byte and Packet Counters." << std::endl
		<< "    -p: Print Performance Counters." << std::endl;

	exit(0);
}

int main(int argc
		 , char* argv[])
{
	auto printPerformanceCounters = false;
	auto printSERDESCounters = false;
	auto printRegisterDump = true;

	for (auto optind = 1; optind < argc; ++optind)
	{
		if (argv[optind][0] == '-')
		{
			auto index = 1;
			while (argv[optind][index] != '\0')
			{
				switch (argv[optind][index])
				{
				case 's':
					printSERDESCounters = true;
					break;
				case 'p':
					printPerformanceCounters = true;
					break;
				case 'R':
					printRegisterDump = false;
					break;
				default:
					std::cout << "Unknown option: " << argv[optind][index] << std::endl;
					printHelpMsg();
					break;
				case 'h':
					printHelpMsg();
					break;
				}
				index++;
			}
		}
	}

	auto thisDTC = new DTCLib::DTC_Registers(DTCLib::DTC_SimMode_Disabled, 0, 0x1, "", true);

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


	if (cols > 400) { cols = 120; }

	if (printRegisterDump)
	{
		std::cout << thisDTC->FormattedRegDump(cols) << std::endl;
	}

	if (printSERDESCounters)
	{
		std::cout << std::endl << std::endl;
		std::cout << thisDTC->RingCountersRegDump(cols);
	}

	if (printPerformanceCounters)
	{
		std::cout << std::endl << std::endl;
		std::cout << thisDTC->PerformanceMonitorRegDump(cols);
	}

	delete thisDTC;
	return 0;
}

