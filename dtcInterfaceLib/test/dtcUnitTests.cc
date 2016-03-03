
#include <iostream>
#ifdef _WIN32
# include "..\DTCLibTest.h"
# include <chrono>
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#else
# include "dtcInterfaceLib/DTCLibTest.h"
#endif

void usage()
{
	std::cout << "This program runs several functionality tests of libDTCInterface." << std::endl
		<< "If run with no options, it will run all 4 tests." << std::endl
		<< "Otherwise, it accepts a space-delimited list of the tests to run," << std::endl
		<< "defined either by test number {0,1,4,5}, or test name {class, reg, dcs, daq}" << std::endl
		<< "It also accepts a -n argument indicating how many iterations of the tests it should run" << std::endl;
}

int main(int argc, char* argv[])
{
	int testCount = 1;
	bool classTest = false,
		registerTest = false,
		dcsTest = false,
		daqTest = false;
	bool testsSpecified = false;

	if (argc == 1)
	{
		std::cout << "Running all DTC Tests." << std::endl << std::endl;
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			int firstChar = 0;
			if (argv[i][0] == '-')
			{
				firstChar = 1;
				if (argv[i][1] == 'n' && argc >= i + 1)
				{
					++i;
					testCount = atoi(argv[i]);
					continue;
				}
				else if (argv[i][1] == 'h' || argc == i + 1)
				{
					usage();
					exit(0);
				}
			}
			if (isdigit((unsigned char)argv[i][firstChar]))
			{
				testsSpecified = true;
				switch (argv[i][firstChar] - '0')
				{
				case 0:
					classTest = true;
					break;
				case 1:
					registerTest = true;
					break;
				case 4:
					dcsTest = true;
					break;
				case 5:
					daqTest = true;
					break;
				}
			}
			else
			{
				std::string arg(argv[i]);
				arg = arg.substr(firstChar);
				if (arg.find("class") != std::string::npos)
				{
					testsSpecified = true;
					classTest = true;
				}
				if (arg.find("reg") != std::string::npos)
				{
					testsSpecified = true;
					registerTest = true;
				}
				else if (arg.find("dcs") != std::string::npos)
				{
					testsSpecified = true;
					dcsTest = true;
				}
				else if (arg.find("daq") != std::string::npos || arg.find("dma") != std::string::npos)
				{
					daqTest = true;
					testsSpecified = true;
				}
				else
				{
					usage();
					exit(0);
				}
			}
		}
	}
	if (!testsSpecified)
	{
		classTest = true;
		registerTest = true;
		dcsTest = true;
		daqTest = true;
	}

	std::cout << "Running tests: "
		<< (classTest ? "Class Construction/Destruction " : "")
		<< (registerTest ? "Register I/O " : "")
		<< (dcsTest ? "DCS DMA I/O " : "")
		<< (daqTest ? "DAQ DMA I/O " : "")
		<< ", " << testCount << " times." << std::endl;

	DTCLib::DTCLibTest* tester = new DTCLib::DTCLibTest();

	tester->startTest(classTest, registerTest, daqTest, dcsTest, testCount, true);

	while (tester->isRunning())
	{
		usleep(500000);
	}
}

