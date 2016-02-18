// This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <cstdio> // printf

#include <cstdlib> // strtoul

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

std::string FormatBytes(double bytes)
{
	auto kb = bytes / 1024.0;
	auto mb = kb / 1024.0;
	auto gb = mb / 1024.0;
	auto tb = gb / 1024.0;
	auto val = bytes;
	auto unit = " bytes";

	if (tb > 1)
	{
		val = tb;
		unit = " TB";
	}
	else if (gb > 1)
	{
		val = gb;
		unit = " GB";
	}
	else if (mb > 1)
	{
		val = mb;
		unit = " MB";
	}
	else if (kb > 1)
	{
		val = kb;
		unit = " KB";
	}
	std::stringstream s;
	s << std::setprecision(5) << val << unit;
	return s.str();

}

unsigned getOptionValue(int* index, char** argv[])
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

std::string getOptionString(int* index, char** argv[])
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
	std::cout << "Usage: mu2eUtil [options] [read,read_data,toggle_serdes,loopback,buffer_test,read_release,DTC]" << std::endl;
	std::cout << "Options are:" << std::endl
		<< "    -h: This message." << std::endl
		<< "    -n: Number of times to repeat test. (Default: 1)" << std::endl
		<< "    -o: Starting Timestamp offest. (Default: 1)." << std::endl
		<< "    -i: Do not increment Timestamps." << std::endl
		<< "    -S: Synchronous Timestamp mode (1 RR & DR per Read operation)" << std::endl
		<< "    -d: Delay between tests, in us (Default: 0)." << std::endl
		<< "    -c: Number of Debug Packets to request (Default: 0)." << std::endl
		<< "    -a: Number of Readout Request/Data Requests to send before starting to read data (Default: 0)." << std::endl
		<< "    -q: Quiet mode (Don't print requests)" << std::endl
		<< "    -Q: Really Quiet mode (Try not to print anything)" << std::endl
		<< "    -s: Stop on SERDES Error." << std::endl
		<< "    -e: Use DTC CFO Emulator instead of DTCLib's SoftwareCFO" << std::endl
		<< "    -t: Use DebugType flag (1st request gets ExternalDataWithFIFOReset, the rest get ExternalData)" << std::endl
		<< "    -T: Set DebugType flag for ALL requests (0, 1, or 2)" << std::endl
		<< "    -f: RAW Output file path" << std::endl
		<< "    -g: Generate (and send) N DMA blocks for testing the Detector Emulator (Default: 0)" << std::endl;
	exit(0);
}

int
main(int argc
	, char* argv[])
{
	bool incrementTimestamp = true;
	bool syncRequests = false;
	bool checkSERDES = false;
	bool quiet = false;
	bool reallyQuiet = false;
	bool rawOutput = false;
	bool useCFOEmulator = false;
	unsigned genDMABlocks = 0;
	std::string rawOutputFile = "/tmp/mu2eUtil.raw";
	unsigned delay = 0;
	unsigned number = 1;
	unsigned timestampOffset = 1;
	unsigned packetCount = 0;
	int requestsAhead = 0;
	std::string op = "";
	DTCLib::DTC_DebugType debugType = DTCLib::DTC_DebugType_SpecialSequence;
	bool stickyDebugType = true;
	int val = 0;

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
			case 'g':
				genDMABlocks = getOptionValue(&optind, &argv);
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
			case 't':
				debugType = DTCLib::DTC_DebugType_ExternalSerialWithReset;
				stickyDebugType = false;
				break;
			case 'T':
				val = getOptionValue(&optind, &argv);
				if (val < (int)DTCLib::DTC_DebugType_Invalid)
				{
					stickyDebugType = true;
					debugType = static_cast<DTCLib::DTC_DebugType>(val);
					break;
				}
				std::cout << "Invalid Debug Type passed to -T!" << std::endl;
				printHelpMsg();
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
		<< ", TS Offset: " << timestampOffset
		<< ", PacketCount: " << packetCount
		<< ", Requests Ahead of Reads: " << requestsAhead
		<< ", Synchronous Request Mode: " << syncRequests
		<< ", Use DTC CFO Emulator: " << useCFOEmulator
		<< ", Increment TS: " << incrementTimestamp
		<< ", Quiet Mode: " << quiet
		<< ", Really Quiet Mode: " << reallyQuiet
		<< ", Check SERDES Error Status: " << checkSERDES
		<< ", Generate DMA Blocks: " << genDMABlocks
		<< ", Debug Type: " << DTCLib::DTC_DebugTypeConverter(debugType).toString()
		<< std::endl;

	if (op == "read")
	{
		std::cout << "Operation \"read\"" << std::endl;
		DTC* thisDTC = new DTC(DTC_SimMode_NoCFO);
		DTC_DataHeaderPacket* packet = thisDTC->ReadNextDAQPacket();
		if (!reallyQuiet) std::cout << packet->toJSON() << '\n';
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
		std::cout << "Operation \"read_data\"" << std::endl;
		mu2edev device;
		device.init();

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) std::cout << "Buffer Read " << ii << std::endl;
			mu2e_databuff_t* buffer;
			int tmo_ms = 1500;
			int sts = device.read_data(DTC_DMA_Engine_DAQ, (void**)&buffer, tmo_ms);

			TRACE(1, "util - read for DAQ - ii=%u sts=%d %p", ii, sts, (void*)buffer);
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
						std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if ((line * 16) + (2 * byte) < (bufSize - 8u))
							{
								uint16_t thisWord = (((uint16_t*)buffer)[4 + (line * 8) + byte]);
								//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
								std::cout << std::setw(4) << (int)thisWord << " ";
							}
						}
						std::cout << std::endl;
					}
				}
			}
			if (!reallyQuiet) std::cout << std::endl << std::endl;
			device.read_release(DTC_DMA_Engine_DAQ, 1);
			if (delay > 0)	usleep(delay);
		}
	}
	else if (op == "toggle_serdes")
	{
		std::cout << "Swapping SERDES Oscillator Clock" << std::endl;
		DTC* thisDTC = new DTC(DTC_SimMode_NoCFO);
		if (thisDTC->ReadSERDESOscillatorClock())
		{
			thisDTC->SetSERDESOscillatorClock_25Gbps();
		}
		else
		{
			thisDTC->SetSERDESOscillatorClock_3125Gbps();
		}
	}
	else if (op == "loopback")
	{
		std::cout << "Operation \"loopback\" (implies -g)" << std::endl;
		double totalReadTime = 0, totalWriteTime = 0, totalSize = 0, totalRTTime = 0;
		int rtCount = 0;
		auto startTime = std::chrono::high_resolution_clock::now();
		DTC* thisDTC = new DTC(DTC_SimMode_Loopback);
		mu2edev* device = thisDTC->GetDevice();

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
			device->write_data(1, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
			auto endDTC = std::chrono::high_resolution_clock::now();
			totalWriteTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
				(endDTC - startDTC).count();
			unsigned returned = 0;
			int count = 5;
			while (returned < packetCount + 1 && count > 0)
			{
				mu2e_databuff_t* buffer;
				int tmo_ms = 0x150;
				auto startDTCRead = std::chrono::high_resolution_clock::now();
				device->read_release(DTC_DMA_Engine_DAQ, 1);
				int sts = device->read_data(DTC_DMA_Engine_DAQ, (void**)&buffer, tmo_ms);
				auto endDTCRead = std::chrono::high_resolution_clock::now();
				totalReadTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
					(endDTCRead - startDTCRead).count();
				count--;
				if (sts > 0)
				{
					totalRTTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(endDTCRead - startDTC).count();
					rtCount++;
					void* readPtr = &(buffer[0]);
					uint16_t bufSize = static_cast<uint16_t>(*((uint64_t*)readPtr));
					TRACE(19, "mu2eUtil::loopback test, bufSize is %u", bufSize);
					totalSize += bufSize;
					if (!reallyQuiet)
					{
						for (unsigned line = 0; line < (unsigned)(ceil((bufSize - 8) / 16)); ++line)
						{
							std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
							//for (unsigned byte = 0; byte < 16; ++byte)
							for (unsigned byte = 0; byte < 8; ++byte)
							{
								if ((line * 16) + (2 * byte) < (bufSize - 8u))
								{
									uint16_t thisWord = (((uint16_t*)buffer)[4 + (line * 8) + byte]);
									//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
									std::cout << std::setw(4) << (int)thisWord << " ";
								}
							}
							std::cout << std::endl;
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
				if (delay > 0)	usleep(delay);
			}
			if (returned < packetCount + 1)
			{
				break;
			}
			if (delay > 0)		usleep(delay);
		}

		double aveRate = totalSize / totalReadTime / 1024;
		double rtTime = totalRTTime / (rtCount > 0 ? rtCount : 1);

		auto totalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>
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
	else if (op == "buffer_test")
	{
		std::cout << "Operation \"buffer_test\"" << std::endl;
		auto startTime = std::chrono::high_resolution_clock::now();
		DTC* thisDTC = new DTC(DTC_SimMode_NoCFO);
		if (!thisDTC->ReadSERDESOscillatorClock())
		{
			thisDTC->SetSERDESOscillatorClock_25Gbps();
		} // We're going to 2.5Gbps for now

		mu2edev* device = thisDTC->GetDevice();

		auto initTime = device->GetDeviceTime();
		device->ResetDeviceTime();
		auto afterInit = std::chrono::high_resolution_clock::now();

		DTCSoftwareCFO* cfo = new DTCSoftwareCFO(thisDTC, useCFOEmulator, packetCount, debugType, stickyDebugType, quiet, false);

		if (genDMABlocks > 0)
		{
			std::cout << "Sending data to DTC" << std::endl;
			thisDTC->ResetDDRWriteAddress();
			thisDTC->SetDetectorEmulationDMACount(number);
			thisDTC->SetDDRLocalEndAddress(0xFFFFFFFF);
			size_t total_size = 0;
			unsigned ii = 0;
			for (; ii < genDMABlocks; ++ii)
			{
				uint64_t byteCount = (1 + packetCount) * 16 * sizeof(uint8_t) + 8;
				total_size += byteCount;
				mu2e_databuff_t* buf = (mu2e_databuff_t*)new mu2e_databuff_t();
				memcpy(buf, &byteCount, sizeof(byteCount));
				uint64_t currentOffset = 8;
				uint64_t ts = timestampOffset + (incrementTimestamp ? ii : 0);
				DTC_DataHeaderPacket header(DTC_Ring_0, (uint16_t)0, DTC_DataStatus_Valid, DTC_Timestamp(ts));
				DTC_DataPacket packet = header.ConvertToDataPacket();
				memcpy((uint8_t*)buf + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
				currentOffset += 16;
				for (unsigned jj = 0; jj < packetCount; ++jj)
				{
					if (currentOffset + 16 > sizeof(mu2e_databuff_t))
					{
						break;
					}
					packet.SetWord(14, (jj + 1) & 0xFF);
					memcpy((uint8_t*)buf + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
					currentOffset += 16;
				}

				device->write_data(0, buf, byteCount);
				delete buf;
			}

			std::cout << "Total bytes written: " << total_size << std::endl;
			thisDTC->SetDDRLocalEndAddress(total_size + 1);
			thisDTC->EnableDetectorEmulator();
		}

		if (thisDTC->ReadSimMode() != DTC_SimMode_Loopback && !syncRequests)
		{
			cfo->SendRequestsForRange(number, DTC_Timestamp(timestampOffset), incrementTimestamp, delay, requestsAhead);
		}
		else if (thisDTC->ReadSimMode() == DTC_SimMode_Loopback)
		{
			uint64_t ts = timestampOffset;
			DTC_DataHeaderPacket header(DTC_Ring_0, (uint16_t)0, DTC_DataStatus_Valid, DTC_Timestamp(ts));
			std::cout << "Request: " << header.toJSON() << std::endl;
			thisDTC->WriteDMAPacket(header);
		}

		auto readoutRequestTime = device->GetDeviceTime();
		device->ResetDeviceTime();
		auto afterRequests = std::chrono::high_resolution_clock::now();

		std::ofstream outputStream;
		if (rawOutput) outputStream.open(rawOutputFile, std::ios::out | std::ios::app | std::ios::binary);

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (syncRequests)
			{
				auto startRequest = std::chrono::high_resolution_clock::now();
				cfo->SendRequestForTimestamp(DTC_Timestamp(timestampOffset + (incrementTimestamp ? ii : 0)));
				auto endRequest = std::chrono::high_resolution_clock::now();
				readoutRequestTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(endRequest - startRequest).count();
			}
			if (!reallyQuiet) std::cout << "Buffer Read " << std::dec << ii << std::endl;
			mu2e_databuff_t* buffer;
			int tmo_ms = 1500;
			TRACE(1, "util - before read for DAQ - ii=%u", ii);
			int sts = device->read_data(DTC_DMA_Engine_DAQ, (void**)&buffer, tmo_ms);

			TRACE(1, "util - after read for DAQ - ii=%u sts=%d %p", ii, sts, (void*)buffer);
			if (sts > 0)
			{
				void* readPtr = &(buffer[0]);
				uint16_t bufSize = static_cast<uint16_t>(*((uint64_t*)readPtr));
				readPtr = (uint8_t*)readPtr + 8;
				if (!reallyQuiet) std::cout << "Buffer reports DMA size of " << std::dec << bufSize << " bytes. Device driver reports read of " << sts << " bytes," << std::endl;
				TRACE(1, "util - bufSize is %u", bufSize);
				if (rawOutput) outputStream.write((char*)readPtr, bufSize - 8);

				if (!reallyQuiet)
				{
					for (unsigned line = 0; line < (unsigned)(ceil((sts - 8) / 16)); ++line)
					{
						std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if ((line * 16) + (2 * byte) < (sts - 8u))
							{
								uint16_t thisWord = (((uint16_t*)buffer)[4 + (line * 8) + byte]);
								//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
								std::cout << std::setw(4) << (int)thisWord << " ";
							}
						}
						std::cout << std::endl;
					}
				}
			}
			if (!reallyQuiet) std::cout << std::endl << std::endl;
			device->read_release(DTC_DMA_Engine_DAQ, 1);
			if (delay > 0) usleep(delay);
		}
		if (rawOutput) outputStream.close();

		auto readDevTime = device->GetDeviceTime();
		auto doneTime = std::chrono::high_resolution_clock::now();
		auto totalBytesRead = device->GetReadSize();
		auto totalBytesWritten = device->GetWriteSize();
		auto totalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - startTime).count();
		auto totalInitTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterInit - startTime).count();
		auto totalRequestTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterRequests - afterInit).count();
		auto totalReadTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - afterRequests).count();

		std::cout << "STATS, "
			<< "Total Elapsed Time: " << totalTime << " s." << std::endl
			<< "Total Init Time: " << totalInitTime << " s." << std::endl
			<< "Total Readout Request Time: " << totalRequestTime << " s." << std::endl
			<< "Total Read Time: " << totalReadTime << " s." << std::endl
			<< "Device Init Time: " << initTime << " s." << std::endl
			<< "Device Request Time: " << readoutRequestTime << " s." << std::endl
			<< "Device Read Time: " << readDevTime << " s." << std::endl
			<< "Total Bytes Written: " << FormatBytes(totalBytesWritten) << "." << std::endl
			<< "Total Bytes Read: " << FormatBytes(totalBytesRead) << "." << std::endl
			<< "Total PCIe Rate: " << FormatBytes((totalBytesWritten + totalBytesRead) / totalTime) << "/s." << std::endl
			<< "Read Rate: " << FormatBytes(totalBytesRead / totalReadTime) << "/s." << std::endl
			<< "Device Read Rate: " << FormatBytes(totalBytesRead / readDevTime) << "/s." << std::endl;
	}
	else if (op == "read_release")
	{
		mu2edev device;
		device.init();
		for (unsigned ii = 0; ii < number; ++ii)
		{
			void* buffer;
			int tmo_ms = 0;
			int stsRD = device.read_data(DTC_DMA_Engine_DAQ, &buffer, tmo_ms);
			int stsRL = device.read_release(DTC_DMA_Engine_DAQ, 1);
			TRACE(12, "util - release/read for DAQ and DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
			if (delay > 0) usleep(delay);
		}
	}
	else if (op == "DTC")
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		auto* thisDTC = new DTC(DTC_SimMode_NoCFO);
		if (!thisDTC->ReadSERDESOscillatorClock())
		{
			thisDTC->SetSERDESOscillatorClock_25Gbps();
		} // We're going to 2.5Gbps for now

		auto initTime = thisDTC->GetDevice()->GetDeviceTime();
		thisDTC->GetDevice()->ResetDeviceTime();
		auto afterInit = std::chrono::high_resolution_clock::now();

		if (genDMABlocks > 0)
		{
			std::cout << "Sending data to DTC" << std::endl;
			thisDTC->ResetDDRWriteAddress();
			thisDTC->SetDDRLocalStartAddress(0);
			thisDTC->SetDDRLocalEndAddress(0x7000000);
			size_t total_size = 0;
			unsigned ii = 0;
			for (; ii < genDMABlocks; ++ii)
			{
				uint64_t byteCount = (1 + packetCount) * 16 * sizeof(uint8_t) + 8;
				mu2e_databuff_t* buf = (mu2e_databuff_t*)new mu2e_databuff_t();
				memcpy(buf, &byteCount, sizeof(byteCount));
				uint64_t currentOffset = 8;
				uint64_t ts = timestampOffset + (incrementTimestamp ? ii : 0);
				DTC_DataHeaderPacket header(DTC_Ring_0, (uint16_t)0, DTC_DataStatus_Valid, DTC_Timestamp(ts));
				DTC_DataPacket packet = header.ConvertToDataPacket();
				memcpy((uint8_t*)buf + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
				currentOffset += 16;
				for (unsigned jj = 0; jj < packetCount; ++jj)
				{
					if (currentOffset + 16 > sizeof(mu2e_databuff_t))
					{
						break;
					}
					packet.SetWord(14, (jj + 1) & 0xFF);
					memcpy((uint8_t*)buf + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
					currentOffset += 16;
				}

				if (total_size + byteCount > 0x7000000) break;

				total_size += byteCount;
				thisDTC->WriteDetectorEmulatorData(buf, byteCount);
				thisDTC->GetDevice()->ResetDeviceTime();
				delete buf;
			}

			std::cout << "Total bytes written: " << total_size << std::endl;
			thisDTC->SetDDRLocalEndAddress(total_size);
			thisDTC->SetDetectorEmulationDMACount(number);
			thisDTC->EnableDetectorEmulator();
		}


		DTCSoftwareCFO* theCFO = new DTCSoftwareCFO(thisDTC, useCFOEmulator, packetCount, debugType, stickyDebugType, quiet);
		if (!syncRequests)
		{
			theCFO->SendRequestsForRange(number, DTC_Timestamp(timestampOffset), incrementTimestamp, delay, requestsAhead);
		}

		std::ofstream outputStream;
		if (rawOutput) outputStream.open(rawOutputFile, std::ios::out | std::ios::app | std::ios::binary);

		uint64_t ii = 0;
		int retries = 4;
		uint64_t expectedTS = timestampOffset;
		int packetsProcessed = 0;

		auto afterRequests = std::chrono::high_resolution_clock::now();
		auto readoutRequestTime = thisDTC->GetDevice()->GetDeviceTime();
		thisDTC->GetDevice()->ResetDeviceTime();

		for (; ii < number; ++ii)
		{
			if (syncRequests)
			{
				auto startRequest = std::chrono::high_resolution_clock::now();
				uint64_t ts = incrementTimestamp ? ii + timestampOffset : timestampOffset;
				theCFO->SendRequestForTimestamp(DTC_Timestamp(ts));
				auto endRequest = std::chrono::high_resolution_clock::now();
				readoutRequestTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(endRequest - startRequest).count();
			}

			std::vector<void*> data = thisDTC->GetData(); //DTC_Timestamp(ts));

			if (data.size() > 0)
			{
				TRACE(19, "util_main %llu DataBlocks returned", (unsigned long long)data.size());
				if (!reallyQuiet) std::cout << data.size() << " DataBlocks returned\n";
				packetsProcessed += static_cast<int>(data.size());
				for (size_t i = 0; i < data.size(); ++i)
				{
					TRACE(19, "util_main constructing DataPacket:");
					DTC_DataPacket test = DTC_DataPacket(data[i]);
					//TRACE(19, test.toJSON().c_str());
					//if (!reallyQuiet) cout << test.toJSON() << '\n'; // dumps whole databuff_t
					DTC_DataHeaderPacket h2 = DTC_DataHeaderPacket(test);
					if (expectedTS != h2.GetTimestamp().GetTimestamp(true))
					{
						std::cout << std::dec << h2.GetTimestamp().GetTimestamp(true) << " does not match expected timestamp of " << expectedTS << "!!!" << std::endl;
						if (incrementTimestamp && h2.GetTimestamp().GetTimestamp(true) <= timestampOffset + number)
						{
							ii += h2.GetTimestamp().GetTimestamp(true) - expectedTS;
						}
						expectedTS = h2.GetTimestamp().GetTimestamp(true);
					}
					else if (i == data.size() - 1)
					{
						expectedTS += (incrementTimestamp ? 1 : 0);
					}
					TRACE(19, h2.toJSON().c_str());
					if (!reallyQuiet)
					{
						std::cout << h2.toJSON() << '\n';
					}
					if (rawOutput)
					{
						DTC_DataPacket rawPacket = h2.ConvertToDataPacket();
						outputStream << rawPacket;
						/*for (int ii = 0; ii < 16; ++ii)
						{
							uint8_t word = rawPacket.GetWord(ii);
							outputStream.write((char*)&word, sizeof(uint8_t));
						}*/
					}
					for (int jj = 0; jj < h2.GetPacketCount(); ++jj)
					{
						DTC_DataPacket packet = DTC_DataPacket(((uint8_t*)data[i]) + ((jj + 1) * 16));
						if (!quiet) std::cout << "\t" << packet.toJSON() << std::endl;
						if (rawOutput)
						{
							outputStream << packet;
							/*for (int ii = 0; ii < 16; ++ii)
							{
								uint8_t word = packet.GetWord(ii);
								outputStream.write((char*)&word, sizeof(uint8_t));
							}*/
						}
					}
				}
			}
			else
			{
				//TRACE_CNTL("modeM", 0L);
				if (!reallyQuiet) std::cout << "no data returned\n";
				//return (0);
				//break;
				usleep(100000);
				ii--;
				retries--;
				if (retries <= 0) break;
				continue;
			}
			retries = 4;

			if (checkSERDES)
			{
				auto disparity = thisDTC->ReadSERDESRXDisparityError(DTC_Ring_0);
				auto cnit = thisDTC->ReadSERDESRXCharacterNotInTableError(DTC_Ring_0);
				auto rxBufferStatus = thisDTC->ReadSERDESRXBufferStatus(DTC_Ring_0);
				bool eyescan = thisDTC->ReadSERDESEyescanError(DTC_Ring_0);
				if (eyescan)
				{
					//TRACE_CNTL("modeM", 0L);
					std::cout << "SERDES Eyescan Error Detected" << std::endl;
					//return 0;
					break;
				}
				if ((int)rxBufferStatus > 2)
				{
					//TRACE_CNTL("modeM", 0L);
					std::cout << "Bad Buffer status detected: " << rxBufferStatus << std::endl;
					//return 0;
					break;
				}
				if (cnit.GetData()[0] || cnit.GetData()[1])
				{
					//TRACE_CNTL("modeM", 0L);
					std::cout << "Character Not In Table Error detected" << std::endl;
					//return 0;
					break;
				}
				if (disparity.GetData()[0] || disparity.GetData()[1])
				{
					//TRACE_CNTL("modeM", 0L);
					std::cout << "Disparity Error Detected" << std::endl;
					//return 0;
					break;
				}
			}
			if (delay > 0)	usleep(delay);
		}

		if (rawOutput) outputStream.close();

		auto readDevTime = thisDTC->GetDevice()->GetDeviceTime();
		auto doneTime = std::chrono::high_resolution_clock::now();
		auto totalBytesRead = thisDTC->GetDevice()->GetReadSize();
		auto totalBytesWritten = thisDTC->GetDevice()->GetWriteSize();
		auto totalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - startTime).count();
		auto totalInitTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterInit - startTime).count();
		auto totalRequestTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterRequests - afterInit).count();
		auto totalReadTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - afterRequests).count();

		std::cout << "STATS, "
			<< "Total Elapsed Time: " << totalTime << " s." << std::endl
			<< "Total Init Time: " << totalInitTime << " s." << std::endl
			<< "Total Readout Request Time: " << totalRequestTime << " s." << std::endl
			<< "Total Read Time: " << totalReadTime << " s." << std::endl
			<< "Device Init Time: " << initTime << " s." << std::endl
			<< "Device Request Time: " << readoutRequestTime << " s." << std::endl
			<< "Device Read Time: " << readDevTime << " s." << std::endl
			<< "Total Bytes Written: " << FormatBytes(totalBytesWritten) << "." << std::endl
			<< "Total Bytes Read: " << FormatBytes(totalBytesRead) << "." << std::endl
			<< "Total PCIe Rate: " << FormatBytes((totalBytesWritten + totalBytesRead) / totalTime) << "/s." << std::endl
			<< "Read Rate: " << FormatBytes(totalBytesRead / totalReadTime) << "/s." << std::endl
			<< "Device Read Rate: " << FormatBytes(totalBytesRead / readDevTime) << "/s." << std::endl;
	}
	else
	{
		std::cout << "Unrecognized operation: " << op << std::endl;
		printHelpMsg();
	}
	return (0);
} // main
