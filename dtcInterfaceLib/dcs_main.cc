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
		return strtoul((*argv)[*index], NULL, 0);
	}
	else
	{
		int offset = 2;
		if (arg[2] == '=')
		{
			offset = 3;
		}

		return strtoul(&(arg[offset]), NULL, 0);
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
	cout << "Usage: mu2eUtil [options] [read,read_data,toggle_serdes,loopback,buffer_test,read_release,DTC]" << endl;
	cout << "Options are:" << endl
		<< "    -h: This message." << endl
		<< "    -n: Number of times to repeat test. (Default: 1)" << endl
		<< "    -o: Starting Timestamp offest. (Default: 1)." << endl
		<< "    -i: Do not increment Timestamps." << endl
		<< "    -S: Synchronous Timestamp mode (1 RR & DR per Read operation)" << endl
		<< "    -d: Delay between tests, in us (Default: 0)." << endl
		<< "    -c: Number of Debug Packets to request (Default: 0)." << endl
		<< "    -a: Number of Readout Request/Data Requests to send before starting to read data (Default: 0)." << endl
		<< "    -q: Quiet mode (Don't print requests)" << endl
		<< "    -Q: Really Quiet mode (Try not to print anything)" << endl
		<< "    -s: Stop on SERDES Error." << endl
		<< "    -e: Use DTC CFO Emulator instead of DTCLib's SoftwareCFO" << endl
		<< "    -f: RAW Output file path" << endl;
	exit(0);
}

int
main(int	argc
	 , char	*argv[])
{
	bool incrementTimestamp = true;
	bool syncRequests = false;
	bool checkSERDES = false;
	bool quiet = false;
	bool reallyQuiet = false;
	bool rawOutput = false;
	bool useCFOEmulator = false;
	std::string rawOutputFile = "/tmp/mu2eUtil.raw";
	unsigned delay = 0;
	unsigned number = 1;
	unsigned timestampOffset = 1;
	unsigned packetCount = 0;
	int requestsAhead = 0;
	string op = "";

	for (int optind = 1; optind < argc; ++optind)
	{
		if (argv[optind][0] == '-')
		{
			switch (argv[optind][1])
			{
			case 'i':
				incrementTimestamp = false;
				break;
			case 'd':
				delay = getOptionValue(&optind, &argv);
				break;
			case 'S':
				syncRequests = true;
				break;
			case 'n':
				number = getOptionValue(&optind, &argv);
				break;
			case 'o':
				timestampOffset = getOptionValue(&optind, &argv);
				break;
			case 'c':
				packetCount = getOptionValue(&optind, &argv);
				break;
			case 'a':
				requestsAhead = getOptionValue(&optind, &argv);
				break;
			case 'q':
				quiet = true;
				break;
			case 'Q':
				quiet = true;
				reallyQuiet = true;
				break;
			case 's':
				checkSERDES = true;
				break;
			case 'e':
				useCFOEmulator = true;
				break;
			case 'f':
				rawOutput = true;
				rawOutputFile = getOptionString(&optind, &argv);
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

	string incrementStr = incrementTimestamp ? "true" : "false";
	string quietStr = quiet ? "true" : "false";
	string reallyQuietStr = reallyQuiet ? "true" : "false";
	string syncStr = syncRequests ? "true" : "false";
	string cfoStr = useCFOEmulator ? "true" : "false";
	string serdesStr = checkSERDES ? "true" : "false";
	cout << "Options are: "
		<< "Operation: " << string(op)
		<< ", Num: " << number
		<< ", Delay: " << delay
		<< ", TS Offset: " << timestampOffset
		<< ", PacketCount: " << packetCount
		<< ", Requests Ahead of Reads: " << requestsAhead
		<< ", Synchronous Request Mode: " << syncStr
		<< ", Use DTC CFO Emulator: " << cfoStr
		<< ", Increment TS: " << incrementStr
		<< ", Quiet Mode: " << quietStr
		<< ", Really Quiet Mode: " << reallyQuietStr
		<< ", Check SERDES Error Status: " << serdesStr
		<< endl;

	if (op == "read")
	{
		cout << "Operation \"read\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		DTC_DataHeaderPacket* packet = thisDTC->ReadNextDAQPacket();
		if (!reallyQuiet) cout << packet->toJSON() << '\n';
		if (rawOutput)
		{
			std::ofstream outputStream;
			outputStream.open(rawOutputFile, std::ios::out | std::ios::app | std::ios::binary);
			DTC_DataPacket rawPacket = packet->ConvertToDataPacket();
			for (int ii = 0; ii < 16; ++ii)
			{
				uint8_t word = rawPacket.GetWord(ii);
				outputStream.write((char*)&word, sizeof(uint8_t));
			}
			outputStream.close();
		}
	}
	else if (op == "read_data")
	{
		cout << "Operation \"read_data\"" << endl;
		mu2edev device;
		device.init();

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) cout << "Buffer Read " << ii << endl;
			mu2e_databuff_t* buffer;
			int tmo_ms = 1500;
			int sts = device.read_data(DTC_DMA_Engine_DAQ, (void**)&buffer, tmo_ms);

			TRACE(1, "util - read for DAQ - ii=%u sts=%d %p", ii, sts, (void*)buffer);
			if (sts > 0) {
				void* readPtr = &(buffer[0]);
				uint16_t bufSize = static_cast<uint16_t>(*((uint64_t*)readPtr));
				readPtr = (uint8_t*)readPtr + 8;
				TRACE(1, "util - bufSize is %u", bufSize);

				if (!reallyQuiet) {
					for (unsigned line = 0; line < (unsigned)(ceil((bufSize - 8) / 16)); ++line)
					{
						cout << "0x" << hex << setw(5) << setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if ((line * 16) + (2 * byte) < (bufSize - 8u)) {
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
			device.read_release(DTC_DMA_Engine_DAQ, 1);
			if (delay > 0) usleep(delay);
		}
	}
	else if (op == "read_dcs")
	{
		cout << "Operation \"read_dcs\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		auto data = thisDTC->ReadROCRegister(DTC_Ring_0, DTC_ROC_0, 2);
		if (!reallyQuiet) cout << data << '\n';
	}
	else if (op == "test_dcs")
	{
		cout << "Operation \"test_dcs\"" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		if (!thisDTC->ReadSERDESOscillatorClock()) { thisDTC->ToggleSERDESOscillatorClock(); } // We're going to 2.5Gbps for now

		mu2edev device = thisDTC->GetDevice();
		thisDTC->SendDCSRequestPacket(DTC_Ring_0, DTC_ROC_0, DTC_DCSOperationType_Read, 0x2);

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) cout << "Buffer Read " << ii << endl;
			mu2e_databuff_t* buffer;
			int tmo_ms = 1500;
			int sts = device.read_data(DTC_DMA_Engine_DCS, (void**)&buffer, tmo_ms);

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
			device.read_release(DTC_DMA_Engine_DCS, 1);
			if (delay > 0) usleep(delay);
		}
	}
	else if (op == "toggle_serdes")
	{
		cout << "Swapping SERDES Oscillator Clock" << endl;
		DTC *thisDTC = new DTC(DTC_SimMode_NoCFO);
		thisDTC->ToggleSERDESOscillatorClock();
	}
	else if (op == "loopback")
	{
		cout << "Operation \"loopback\"" << endl;
		double totalReadTime = 0, totalWriteTime = 0, totalSize = 0, totalRTTime = 0;
		int rtCount = 0;
		auto startTime = std::chrono::high_resolution_clock::now();
		DTC *thisDTC = new DTC(DTC_SimMode_Loopback);
		mu2edev device = thisDTC->GetDevice();

		thisDTC->SetSERDESLoopbackMode(DTC_Ring_0, DTC_SERDESLoopbackMode_NearPCS);

		unsigned ii = 0;
		for (; ii < number; ++ii)
		{
			uint64_t ts = timestampOffset + (incrementTimestamp ? ii : 0);
			DTC_DataHeaderPacket header(DTC_Ring_0, (uint16_t)0, DTC_DataStatus_Valid, DTC_Timestamp(ts));
			DTC_DataPacket packet = header.ConvertToDataPacket();
			DTC_DataPacket thisPacket = header.ConvertToDataPacket();
			thisPacket.Resize(16 * (packetCount + 1));
			for (unsigned jj = 0; jj < packetCount; ++jj)
			{
				thisPacket.CramIn(packet, 16 * (jj + 1));
			}
			auto startDTC = std::chrono::high_resolution_clock::now();
			device.write_data(0, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
			auto endDTC = std::chrono::high_resolution_clock::now();
			totalWriteTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1> >>
				(endDTC - startDTC).count();
			unsigned returned = 0;
			int count = 5;
			while (returned < packetCount + 1 && count > 0)
			{
				mu2e_databuff_t* buffer;
				int tmo_ms = 0x150;
				auto startDTCRead = std::chrono::high_resolution_clock::now();
				device.read_release(DTC_DMA_Engine_DAQ, 1);
				int sts = device.read_data(DTC_DMA_Engine_DAQ, (void**)&buffer, tmo_ms);
				auto endDTCRead = std::chrono::high_resolution_clock::now();
				totalReadTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1> >>
					(endDTCRead - startDTCRead).count();
				count--;
				if (sts > 0)
				{
					totalRTTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1> >>(endDTCRead - startDTC).count();
					rtCount++;
					void* readPtr = &(buffer[0]);
					uint16_t bufSize = static_cast<uint16_t>(*((uint64_t*)readPtr));
					TRACE(19, "mu2eUtil::loopback test, bufSize is %u", bufSize);
					totalSize += bufSize;
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

					for (int offset = 0; offset < (bufSize - 8) / 16; ++offset)
					{
						DTC_DataPacket test = DTC_DataPacket(&((uint8_t*)buffer)[8 + offset * 16]);
						std::string output = "returned=" + std::to_string(returned) + ", test=" + test.toJSON();
						TRACE(19, output.c_str());

						if (test == packet)
						{
							returned++;
						}
					}
				}
				if (delay > 0) usleep(delay);
			}
			if (returned < packetCount + 1) { break; }
			if (delay > 0) usleep(delay);
		}

		double aveRate = totalSize / totalReadTime / 1024;
		double rtTime = totalRTTime / (rtCount > 0 ? rtCount : 1);

		auto totalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1> >>
			(std::chrono::high_resolution_clock::now() - startTime).count();
		std::cout << "STATS, " << std::dec << ii << " DataHeaders looped back "
			<< "(Out of " << number << " requested):" << std::endl
			<< "Total Elapsed Time: " << totalTime << " s." << std::endl
			<< "Device Read Time: " << totalReadTime << " s." << std::endl
			<< "Device Write Time: " << totalWriteTime << " s." << std::endl
			<< "Total Data Size: " << totalSize / 1024 << " KB." << std::endl
			<< "Average Data Rate: " << aveRate << " KB/s." << std::endl
			<< "Average Round-Trip time: " << rtTime << " s." << std::endl;
	}
	else if (op == "read_release")
	{
		mu2edev device;
		device.init();
		for (unsigned ii = 0; ii < number; ++ii)
		{
			void *buffer;
			int tmo_ms = 0;
			int stsRD = device.read_data(DTC_DMA_Engine_DAQ, &buffer, tmo_ms);
			int stsRL = device.read_release(DTC_DMA_Engine_DAQ, 1);
			TRACE(12, "util - release/read for DAQ and DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
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
