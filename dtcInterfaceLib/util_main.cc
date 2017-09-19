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
# define TRACE_CNTL(...)
# endif
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif
#define TRACE_NAME "MU2EDEV"

using namespace DTCLib;

bool incrementTimestamp = true;
bool syncRequests = false;
bool checkSERDES = false;
bool quiet = false;
unsigned quietCount = 1;
bool reallyQuiet = false;
bool rawOutput = false;
bool useCFOEmulator = true;
unsigned genDMABlocks = 0;
std::string rawOutputFile = "/tmp/mu2eUtil.raw";
std::string simFile = "";
bool useSimFile = false;
unsigned delay = 0;
unsigned cfodelay = 1000;
unsigned number = 1;
unsigned timestampOffset = 1;
unsigned eventCount = 1;
unsigned blockCount = 1;
unsigned packetCount = 0;
int requestsAhead = 0;
std::string op = "";
DTC_DebugType debugType = DTC_DebugType_SpecialSequence;
bool stickyDebugType = true;
int val = 0;
bool readGenerated = false;
std::ofstream outputStream;
unsigned rocMask = 0x1;
unsigned targetFrequency = 166666667;
int clockToProgram = 0;


unsigned getOptionValue(int* index, char** argv[])
{
	auto arg = (*argv)[*index];
	if (arg[2] == '\0')
	{
		(*index)++;
		unsigned ret = strtoul((*argv)[*index], nullptr, 0);
		if (ret == 0 && (*argv)[*index][0] != '0') // No option given 
		{
			(*index)--;
		}
		return ret;
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

void WriteGeneratedData(DTC* thisDTC)
{
	std::cout << "Sending data to DTC" << std::endl;
	thisDTC->DisableDetectorEmulator();
	thisDTC->DisableDetectorEmulatorMode();
	thisDTC->SetDDRDataLocalStartAddress(0);
	thisDTC->ResetDDRReadAddress();
	thisDTC->ResetDDRWriteAddress();
	thisDTC->EnableDetectorEmulatorMode();
	thisDTC->SetDDRDataLocalEndAddress(0xFFFFFFFF);
	uint64_t total_size_written = 0;
	uint32_t end_address = 0;
	unsigned ii = 0;
	uint64_t ts = timestampOffset;
	for (; ii < genDMABlocks; ++ii)
	{
		auto blockByteCount = static_cast<uint16_t>((1 + packetCount) * 16 * sizeof(uint8_t));
		auto eventByteCount = static_cast<uint64_t>(blockCount * blockByteCount); // Exclusive byte count
		auto eventWriteByteCount = static_cast<uint64_t>(eventByteCount + sizeof(uint64_t)); // Inclusive byte count
		auto dmaByteCount = static_cast<uint64_t>(eventWriteByteCount * eventCount); // Exclusive byte count
		auto dmaWriteByteCount = dmaByteCount + sizeof(uint64_t); // Inclusive byte count

		if (dmaWriteByteCount > 0x10000)
		{
			std::cerr << "Requested DMA write is larger than the allowed size! Reduce event/block/packet counts!" << std::endl;
			exit(1);
		}

		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto buf = reinterpret_cast<mu2e_databuff_t*>(new char[0x10000]);
		memcpy(buf, &dmaWriteByteCount, sizeof(uint64_t));
		auto currentOffset = sizeof(uint64_t);

		for (unsigned ll = 0; ll < eventCount; ++ll) {

			memcpy(reinterpret_cast<uint8_t*>(buf) + currentOffset, &eventByteCount, sizeof(uint64_t));
			currentOffset += sizeof(uint64_t);

			if (incrementTimestamp) ++ts;

			std::vector<std::pair<DTC_Ring_ID, DTC_ROC_ID>> ids;
			for (auto ring : DTC_Rings)
			{
				for (auto roc : DTC_ROCS)
				{
					if (roc == DTC_ROC_Unused) continue;
					ids.emplace_back(std::make_pair(ring, roc));
				}
			}

			for (unsigned kk = 0; kk < blockCount; ++kk)
			{
				auto index = kk % ids.size();
				DTC_DataHeaderPacket header(ids[index].first, ids[index].second, static_cast<uint16_t>(packetCount), DTC_DataStatus_Valid, static_cast<uint8_t>(kk / ids.size()), 0, DTC_Timestamp(ts));
				auto packet = header.ConvertToDataPacket();
				memcpy(reinterpret_cast<uint8_t*>(buf) + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
				if (rawOutput) outputStream << packet;
				currentOffset += 16;
				for (unsigned jj = 0; jj < packetCount; ++jj)
				{
					if (currentOffset + 16 > sizeof(mu2e_databuff_t))
					{
						break;
					}
					// ReSharper disable CppRedundantParentheses
					packet.SetWord(14, (jj + 1) & 0xFF);
					// ReSharper restore CppRedundantParentheses
					memcpy(reinterpret_cast<uint8_t*>(buf) + currentOffset, packet.GetData(), sizeof(uint8_t) * 16);
					if (rawOutput) outputStream << packet;
					currentOffset += 16;
				}
			}
		}

		total_size_written += dmaWriteByteCount;
		end_address += static_cast<uint32_t>(dmaByteCount);

		if (!reallyQuiet)
		{
			std::cout << "Buffer " << ii << ":" << std::endl;
			for (unsigned line = 0; line < static_cast<unsigned>(ceil(dmaWriteByteCount / 16.0)); ++line)
			{
				std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
				//for (unsigned byte = 0; byte < 16; ++byte)
				for (unsigned byte = 0; byte < 8; ++byte)
				{
					if (line * 16 + 2 * byte < dmaWriteByteCount)
					{
						auto thisWord = reinterpret_cast<uint16_t*>(buf)[line * 8 + byte];
						std::cout << std::setw(4) << static_cast<int>(thisWord) << " ";
					}
				}
				std::cout << std::endl;
			}
		}

		thisDTC->GetDevice()->write_data(0, buf, static_cast<size_t>(dmaWriteByteCount));
		delete[] buf;
	}

	std::cout << "Total bytes written: " << std::dec << total_size_written << std::hex << "( 0x" << total_size_written << " )" << std::endl;
	thisDTC->SetDDRDataLocalEndAddress(end_address - 1);
	if (readGenerated)
	{
		if (rawOutput) outputStream.close();
		exit(0);
	}
	thisDTC->SetDetectorEmulationDMACount(number);
	thisDTC->EnableDetectorEmulator();
}

void printHelpMsg()
{
	std::cout << "Usage: mu2eUtil [options] [read,read_data,reset_detemu,toggle_serdes,loopback,buffer_test,read_release,DTC,program_clock]" << std::endl;
	std::cout << "Options are:" << std::endl
		<< "    -h: This message." << std::endl
		<< "    -n: Number of times to repeat test. (Default: 1)" << std::endl
		<< "    -o: Starting Timestamp offest. (Default: 1)." << std::endl
		<< "    -i: Do not increment Timestamps." << std::endl
		<< "    -S: Synchronous Timestamp mode (1 RR & DR per Read operation)" << std::endl
		<< "    -d: Delay between tests, in us (Default: 0)." << std::endl
		<< "    -D: CFO Request delay interval (Default: 1000 (minimum)." << std::endl
		<< "    -c: Number of Debug Packets to request (Default: 0)." << std::endl
		<< "    -b: Number of Data Blocks to generate per Event (Default: 1)." << std::endl
		<< "    -E: Number of Events to generate per DMA block (Default: 1)." << std::endl
		<< "    -a: Number of Readout Request/Data Requests to send before starting to read data (Default: 0)." << std::endl
		<< "    -q: Quiet mode (Don't print requests) Additionally, for buffer_test mode, limits to N (Default 1) packets at the beginning and end of the buffer." << std::endl
		<< "    -Q: Really Quiet mode (Try not to print anything)" << std::endl
		<< "    -s: Stop on SERDES Error." << std::endl
		<< "    -e: Use DTCLib's SoftwareCFO instead of the DTC CFO Emulator" << std::endl
		<< "    -t: Use DebugType flag (1st request gets ExternalDataWithFIFOReset, the rest get ExternalData)" << std::endl
		<< "    -T: Set DebugType flag for ALL requests (0, 1, or 2)" << std::endl
		<< "    -f: RAW Output file path" << std::endl
		<< "    -p: Send DTCLIB_SIM_FILE to DTC and enable Detector Emulator mode" << std::endl
		<< "    -P: Send <file> to DTC and enable Detector Emulator mode (Default: \"\")" << std::endl
		<< "    -g: Generate (and send) N DMA blocks for testing the Detector Emulator (Default: 0)" << std::endl
		<< "    -G: Read out generated data, but don't write new. With -g, will exit after writing data" << std::endl
		<< "    -r: # of rocs to enable. Hexadecimal, each digit corresponds to a ring. ROC_0: 1, ROC_1: 3, ROC_2: 5, ROC_3: 7, ROC_4: 9, ROC_5: B (Default 0x1, All possible: 0xBBBBBB)" << std::endl
		<< "    -F: Frequency to program (in Hz, sorry...Default 166666667 Hz)" << std::endl
		<< "    -C: Clock to program (0: SERDES, 1: DDR, Default 0)" << std::endl
		;
	exit(0);
}

int
main(int argc
	 , char* argv[])
{
	for (auto optind = 1; optind < argc; ++optind)
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
			case 'D':
				cfodelay = getOptionValue(&optind, &argv);
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
			case 'b':
				blockCount = getOptionValue(&optind, &argv);
				break;
			case 'E':
				eventCount = getOptionValue(&optind, &argv);
				break;
			case 'a':
				requestsAhead = getOptionValue(&optind, &argv);
				break;
			case 'q':
				quiet = true;
				quietCount = getOptionValue(&optind, &argv);
				if (quietCount == 0) quietCount = 1;
				break;
			case 'p':
				useSimFile = true;
				break;
			case 'P':
				useSimFile = true;
				simFile = getOptionValue(&optind, &argv);
				break;
			case 'g':
				genDMABlocks = getOptionValue(&optind, &argv);
				break;
			case 'G':
				readGenerated = true;
				break;
			case 'Q':
				quiet = true;
				reallyQuiet = true;
				break;
			case 's':
				checkSERDES = true;
				break;
			case 'e':
				useCFOEmulator = false;
				break;
			case 'f':
				rawOutput = true;
				rawOutputFile = getOptionString(&optind, &argv);
				break;
			case 't':
				debugType = DTC_DebugType_ExternalSerialWithReset;
				stickyDebugType = false;
				break;
			case 'T':
				val = getOptionValue(&optind, &argv);
				if (val < static_cast<int>(DTC_DebugType_Invalid))
				{
					stickyDebugType = true;
					debugType = static_cast<DTC_DebugType>(val);
					break;
				}
				std::cout << "Invalid Debug Type passed to -T!" << std::endl;
				printHelpMsg();
				break;
			case 'r':
				rocMask = getOptionValue(&optind, &argv);
				break;
			case 'C':
				clockToProgram = getOptionValue(&optind, &argv) % 2;
				break;
			case 'F':
				targetFrequency = getOptionValue(&optind, &argv);
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
		<< ", CFO Delay: " << cfodelay
		<< ", TS Offset: " << timestampOffset
		<< ", PacketCount: " << packetCount
		<< ", DataBlock Count: " << blockCount
		<< ", Event Count: " << eventCount
		<< ", Requests Ahead of Reads: " << requestsAhead
		<< ", Synchronous Request Mode: " << syncRequests
		<< ", Use DTC CFO Emulator: " << useCFOEmulator
		<< ", Increment TS: " << incrementTimestamp
		<< ", Quiet Mode: " << quiet << " (" << quietCount << ")"
		<< ", Really Quiet Mode: " << reallyQuiet
		<< ", Check SERDES Error Status: " << checkSERDES
		<< ", Generate DMA Blocks: " << genDMABlocks
		<< ", Read Data from DDR: " << readGenerated
		<< ", Use Sim File: " << useSimFile
		<< ", ROC Mask: " << std::hex << rocMask
		<< ", Debug Type: " << DTC_DebugTypeConverter(debugType).toString()
		<< ", Target Frequency: " << std::dec << targetFrequency
		<< ", Clock To Program: " << (clockToProgram == 0 ? "SERDES" : "DDR");
	if (rawOutput)
	{
		std::cout << ", Raw output file: " << rawOutputFile;
	}
	if (simFile.size() > 0)
	{
		std::cout << ", Sim file: " << simFile;
	}
	std::cout << std::endl;
	if (rawOutput) outputStream.open(rawOutputFile, std::ios::out | std::ios::app | std::ios::binary);

	if (op == "read")
	{
		std::cout << "Operation \"read\"" << std::endl;
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);
		auto packet = thisDTC->ReadNextDAQPacket();
		if (!reallyQuiet) std::cout << packet->toJSON() << '\n';
		if (rawOutput)
		{
			auto rawPacket = packet->ConvertToDataPacket();
			for (auto ii = 0; ii < 16; ++ii)
			{
				auto word = rawPacket.GetWord(ii);
				outputStream.write(reinterpret_cast<char*>(&word), sizeof(uint8_t));
			}
		}
		delete thisDTC;
	}
	else if (op == "read_data")
	{
		std::cout << "Operation \"read_data\"" << std::endl;
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);

		auto device = thisDTC->GetDevice();
		if (readGenerated)
		{
			thisDTC->EnableDetectorEmulatorMode();
			thisDTC->SetDetectorEmulationDMACount(number);
			thisDTC->EnableDetectorEmulator();
		}
		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (!reallyQuiet) std::cout << "Buffer Read " << ii << std::endl;
			mu2e_databuff_t* buffer;
			auto tmo_ms = 1500;
			auto sts = device->read_data(DTC_DMA_Engine_DAQ, reinterpret_cast<void**>(&buffer), tmo_ms);

			TRACE(1, "util - read for DAQ - ii=%u sts=%d %p", ii, sts, (void*)buffer);
			if (sts > 0)
			{
				auto bufSize = static_cast<uint16_t>(*reinterpret_cast<uint64_t*>(&buffer[0]));
				TRACE(1, "util - bufSize is %u", bufSize);

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
			device->read_release(DTC_DMA_Engine_DAQ, 1);
			if (delay > 0)
				usleep(delay);
		}
		delete thisDTC;
	}
	else if (op == "toggle_serdes")
	{
		std::cout << "Swapping SERDES Oscillator Clock" << std::endl;
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);
		auto clock = thisDTC->ReadSERDESOscillatorClock();
		if (clock == DTC_SerdesClockSpeed_3125Gbps)
		{
			std::cout << "Setting SERDES Oscillator Clock to 2.5 Gbps" << std::endl;
			thisDTC->SetSERDESOscillatorClock(DTC_SerdesClockSpeed_25Gbps);
		}
		else if (clock == DTC_SerdesClockSpeed_25Gbps)
		{
			std::cout << "Setting SERDES Oscillator Clock to 3.125 Gbps" << std::endl;
			thisDTC->SetSERDESOscillatorClock(DTC_SerdesClockSpeed_3125Gbps);
		}
		else
		{
			std::cerr << "Error: SERDES clock not recognized value!";
		}
		delete thisDTC;
	}
	else if (op == "reset_detemu")
	{
		std::cout << "Resetting Detector Emulator" << std::endl;
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);
		thisDTC->ClearDetectorEmulatorInUse();
		thisDTC->ResetDDR();
		thisDTC->ResetDTC();
		delete thisDTC;
	}
	else if (op == "buffer_test")
	{
		std::cout << "Operation \"buffer_test\"" << std::endl;
		auto startTime = std::chrono::steady_clock::now();
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);
		auto device = thisDTC->GetDevice();

		auto initTime = device->GetDeviceTime();
		device->ResetDeviceTime();
		auto afterInit = std::chrono::steady_clock::now();

		DTCSoftwareCFO cfo(thisDTC, useCFOEmulator, packetCount, debugType, stickyDebugType, quiet, false);

		if (genDMABlocks > 0)
		{
			WriteGeneratedData(thisDTC);
		}
		else if (useSimFile)
		{
			auto overwrite = false;
			if (simFile.size() > 0) overwrite = true;
			thisDTC->WriteSimFileToDTC(simFile, false, overwrite);
		}
		else if (readGenerated)
		{
			thisDTC->DisableDetectorEmulator();
			thisDTC->EnableDetectorEmulatorMode();
			thisDTC->SetDetectorEmulationDMACount(number);
			thisDTC->EnableDetectorEmulator();
		}

		if (thisDTC->ReadSimMode() != DTC_SimMode_Loopback && !syncRequests)
		{
			cfo.SendRequestsForRange(number, DTC_Timestamp(timestampOffset), incrementTimestamp, cfodelay, requestsAhead);
		}
		else if (thisDTC->ReadSimMode() == DTC_SimMode_Loopback)
		{
			uint64_t ts = timestampOffset;
			DTC_DataHeaderPacket header(DTC_Ring_0, DTC_ROC_0, static_cast<uint16_t>(0), DTC_DataStatus_Valid, 0, 0, DTC_Timestamp(ts));
			std::cout << "Request: " << header.toJSON() << std::endl;
			thisDTC->WriteDMAPacket(header);
		}

		auto readoutRequestTime = device->GetDeviceTime();
		device->ResetDeviceTime();
		auto afterRequests = std::chrono::steady_clock::now();

		for (unsigned ii = 0; ii < number; ++ii)
		{
			if (syncRequests)
			{
				auto startRequest = std::chrono::steady_clock::now();
				cfo.SendRequestForTimestamp(DTC_Timestamp(timestampOffset + (incrementTimestamp ? ii : 0)));
				auto endRequest = std::chrono::steady_clock::now();
				readoutRequestTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(endRequest - startRequest).count();
			}
			if (!reallyQuiet) std::cout << "Buffer Read " << std::dec << ii << std::endl;
			mu2e_databuff_t* buffer;
			auto tmo_ms = 1500;
			TRACE(1, "util - before read for DAQ - ii=%u", ii);
			auto sts = device->read_data(DTC_DMA_Engine_DAQ, reinterpret_cast<void**>(&buffer), tmo_ms);

			TRACE(1, "util - after read for DAQ - ii=%u sts=%d %p", ii, sts, (void*)buffer);
			if (sts > 0)
			{
				void* readPtr = &buffer[0];
				auto bufSize = static_cast<uint16_t>(*static_cast<uint64_t*>(readPtr));
				readPtr = static_cast<uint8_t*>(readPtr) + 8;
				if (!reallyQuiet) std::cout << "Buffer reports DMA size of " << std::dec << bufSize << " bytes. Device driver reports read of " << sts << " bytes," << std::endl;

				TRACE(1, "util - bufSize is %u", bufSize);
				if (rawOutput) outputStream.write(static_cast<char*>(readPtr), bufSize - 8);

				if (!reallyQuiet)
				{
					auto maxLine = static_cast<unsigned>(ceil((sts - 8) / 16.0));
					for (unsigned line = 0; line < maxLine; ++line)
					{
						std::cout << "0x" << std::hex << std::setw(5) << std::setfill('0') << line << "0: ";
						//for (unsigned byte = 0; byte < 16; ++byte)
						for (unsigned byte = 0; byte < 8; ++byte)
						{
							if (line * 16 + 2 * byte < sts - 8u)
							{
								auto thisWord = reinterpret_cast<uint16_t*>(buffer)[4 + line * 8 + byte];
								//uint8_t thisWord = (((uint8_t*)buffer)[8 + (line * 16) + byte]);
								std::cout << std::setw(4) << static_cast<int>(thisWord) << " ";
							}
						}
						std::cout << std::endl;
						if (maxLine > quietCount * 2) {
							if (quiet && line == (quietCount - 1)) {
								line = static_cast<unsigned>(ceil((sts - 8) / 16.0)) - (1 + quietCount);
							}
						}
					}
				}
			}
			else if (checkSERDES) break;
			if (!reallyQuiet) std::cout << std::endl << std::endl;
			device->read_release(DTC_DMA_Engine_DAQ, 1);
			if (delay > 0)
				usleep(delay);
		}

		auto readDevTime = device->GetDeviceTime();
		auto doneTime = std::chrono::steady_clock::now();
		auto totalBytesRead = device->GetReadSize();
		auto totalBytesWritten = device->GetWriteSize();
		auto totalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - startTime).count();
		auto totalInitTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterInit - startTime).count();
		auto totalRequestTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterRequests - afterInit).count();
		auto totalReadTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - afterRequests).count();

		std::cout << "STATS, "
			<< "Total Elapsed Time: " << Utilities::FormatTimeString(totalTime) << "." << std::endl
			<< "Total Init Time: " << Utilities::FormatTimeString(totalInitTime) << "." << std::endl
			<< "Total Readout Request Time: " << Utilities::FormatTimeString(totalRequestTime) << "." << std::endl
			<< "Total Read Time: " << Utilities::FormatTimeString(totalReadTime) << "." << std::endl
			<< "Device Init Time: " << Utilities::FormatTimeString(initTime) << "." << std::endl
			<< "Device Request Time: " << Utilities::FormatTimeString(readoutRequestTime) << "." << std::endl
			<< "Device Read Time: " << Utilities::FormatTimeString(readDevTime) << "." << std::endl
			<< "Total Bytes Written: " << Utilities::FormatByteString(totalBytesWritten) << "." << std::endl
			<< "Total Bytes Read: " << Utilities::FormatByteString(totalBytesRead) << "." << std::endl
			<< "Total PCIe Rate: " << Utilities::FormatByteString((totalBytesWritten + totalBytesRead) / totalTime) << "/s." << std::endl
			<< "Read Rate: " << Utilities::FormatByteString(totalBytesRead / totalReadTime) << "/s." << std::endl
			<< "Device Read Rate: " << Utilities::FormatByteString(totalBytesRead / readDevTime) << "/s." << std::endl;

		delete thisDTC;
	}
	else if (op == "read_release")
	{
		mu2edev device;
		device.init();
		for (unsigned ii = 0; ii < number; ++ii)
		{
			void* buffer;
			auto tmo_ms = 0;
			auto stsRD = device.read_data(DTC_DMA_Engine_DAQ, &buffer, tmo_ms);
			auto stsRL = device.read_release(DTC_DMA_Engine_DAQ, 1);
			TRACE(12, "util - release/read for DAQ and DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
			if (delay > 0)
				usleep(delay);
		}
	}
	else if (op == "DTC")
	{
		auto startTime = std::chrono::steady_clock::now();
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);

		auto initTime = thisDTC->GetDevice()->GetDeviceTime();
		thisDTC->GetDevice()->ResetDeviceTime();
		auto afterInit = std::chrono::steady_clock::now();

		DTCSoftwareCFO theCFO(thisDTC, useCFOEmulator, packetCount, debugType, stickyDebugType, quiet);

		if (genDMABlocks > 0)
		{
			WriteGeneratedData(thisDTC);
		}
		else if (useSimFile)
		{
			auto overwrite = false;
			if (simFile.size() > 0) overwrite = true;
			thisDTC->WriteSimFileToDTC(simFile, false, overwrite);
		}
		else if (readGenerated)
		{
			thisDTC->DisableDetectorEmulator();
			thisDTC->EnableDetectorEmulatorMode();
			thisDTC->SetDetectorEmulationDMACount(number);
			thisDTC->EnableDetectorEmulator();
		}


		if (!syncRequests)
		{
			theCFO.SendRequestsForRange(number, DTC_Timestamp(timestampOffset), incrementTimestamp, cfodelay, requestsAhead);
		}


		uint64_t ii = 0;
		auto retries = 4;
		uint64_t expectedTS = timestampOffset;
		auto packetsProcessed = 0;

		auto afterRequests = std::chrono::steady_clock::now();
		auto readoutRequestTime = thisDTC->GetDevice()->GetDeviceTime();
		thisDTC->GetDevice()->ResetDeviceTime();

		for (; ii < number; ++ii)
		{
			if (!reallyQuiet) std::cout << "util_main: DTC Read " << ii << ": ";
			if (syncRequests)
			{
				auto startRequest = std::chrono::steady_clock::now();
				auto ts = incrementTimestamp ? ii + timestampOffset : timestampOffset;
				theCFO.SendRequestForTimestamp(DTC_Timestamp(ts));
				auto endRequest = std::chrono::steady_clock::now();
				readoutRequestTime += std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(endRequest - startRequest).count();
			}

			auto data = thisDTC->GetData(); //DTC_Timestamp(ts));

			if (data.size() > 0)
			{
				TRACE(19, "util_main %llu DataBlocks returned", (unsigned long long)data.size());
				if (!reallyQuiet) std::cout << data.size() << " DataBlocks returned\n";
				packetsProcessed += static_cast<int>(data.size());
				for (size_t i = 0; i < data.size(); ++i)
				{
					TRACE(19, "util_main constructing DataPacket:");
					auto test = DTC_DataPacket(data[i].blockPointer);
					//TRACE(19, test.toJSON().c_str());
					//if (!reallyQuiet) cout << test.toJSON() << '\n'; // dumps whole databuff_t
					auto h2 = DTC_DataHeaderPacket(test);
					if (expectedTS != h2.GetTimestamp().GetTimestamp(true))
					{
						std::cout << std::dec << h2.GetTimestamp().GetTimestamp(true) << " does not match expected timestamp of " << expectedTS << "!!!" << std::endl;
						if (incrementTimestamp && h2.GetTimestamp().GetTimestamp(true) <= timestampOffset + number)
						{
							auto diff = static_cast<int64_t>(h2.GetTimestamp().GetTimestamp(true)) - static_cast<int64_t>(expectedTS);
							ii += diff > 0 ? diff : 0;
						}
						expectedTS = h2.GetTimestamp().GetTimestamp(true);
					}
					else if (i == data.size() - 1)
					{
						expectedTS += incrementTimestamp ? 1 : 0;
					}
					TRACE(19, h2.toJSON().c_str());
					if (!reallyQuiet)
					{
						std::cout << h2.toJSON() << '\n';
					}
					if (rawOutput)
					{
						auto rawPacket = h2.ConvertToDataPacket();
						outputStream << rawPacket;
						/*for (int ii = 0; ii < 16; ++ii)
						{
							uint8_t word = rawPacket.GetWord(ii);
							outputStream.write((char*)&word, sizeof(uint8_t));
						}*/
					}

					for (auto jj = 0; jj < h2.GetPacketCount(); ++jj)
					{
						auto packet = DTC_DataPacket(reinterpret_cast<uint8_t*>(data[i].blockPointer) + (jj + 1) * 16);
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
				auto eyescan = thisDTC->ReadSERDESEyescanError(DTC_Ring_0);
				if (eyescan)
				{
					//TRACE_CNTL("modeM", 0L);
					std::cout << "SERDES Eyescan Error Detected" << std::endl;
					//return 0;
					break;
				}
				if (static_cast<int>(rxBufferStatus) > 2)
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
			if (delay > 0)
				usleep(delay);
		}


		auto readDevTime = thisDTC->GetDevice()->GetDeviceTime();
		auto doneTime = std::chrono::steady_clock::now();
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
			<< "Total Bytes Written: " << Utilities::FormatByteString(totalBytesWritten) << "." << std::endl
			<< "Total Bytes Read: " << Utilities::FormatByteString(totalBytesRead) << "." << std::endl
			<< "Total PCIe Rate: " << Utilities::FormatByteString((totalBytesWritten + totalBytesRead) / totalTime) << "/s." << std::endl
			<< "Read Rate: " << Utilities::FormatByteString(totalBytesRead / totalReadTime) << "/s." << std::endl
			<< "Device Read Rate: " << Utilities::FormatByteString(totalBytesRead / readDevTime) << "/s." << std::endl;
		delete thisDTC;
	}
	else if (op == "program_clock")
	{
		// ReSharper disable once CppNonReclaimedResourceAcquisition
		auto thisDTC = new DTC(DTC_SimMode_NoCFO, rocMask);
		auto oscillator = clockToProgram == 0 ? DTC_OscillatorType_SERDES : DTC_OscillatorType_DDR;
		thisDTC->SetNewOscillatorFrequency(oscillator, targetFrequency);
		delete thisDTC;
	}
	else
	{
		std::cout << "Unrecognized operation: " << op << std::endl;
		printHelpMsg();
	}

	if (rawOutput) outputStream.close();
	return 0;
} // main


