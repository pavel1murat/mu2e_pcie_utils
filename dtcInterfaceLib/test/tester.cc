#if defined _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <chrono>
#ifdef _WIN32
#include "..\DTC.h"
#include "..\DTCSoftwareCFO.h"
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#ifdef _NDEBUG
#  define TRACE(lvl,...) printf(__VA_ARGS__); printf("\n")
#else
# define TRACE(...)
#endif
# define TRACE_CNTL(...)
#else
# include "dtcInterfaceLib/DTC.h"
# include "dtcInterfaceLib/DTCSoftwareCFO.h"
# include "trace.h"
# include <unistd.h>		// usleep
#endif
#define TRACE_NAME "MU2EDEV"

#include <iostream>
#include "fragmentTester.h"


using namespace DTCLib;

void usage()
{
	std::cout << "Usage: tester [loops = 1000] [simMode = 1] [simFile = \"\"]" << std::endl;
	exit(1);
}

int main(int argc, char* argv[])
{
	int loops = 1000;
	int modeint = 1;
	std::string simFile = "";
	bool badarg = false;
	if (argc > 1)
	{
		int tmp = atoi(argv[1]);
		if (tmp > 0) loops = tmp;
		else badarg = true;
	}
	if (argc > 2)
	{
		int tmp = atoi(argv[2]);
		if (tmp > 0) modeint = tmp;
		else badarg = true;
	}
	if (argc > 3)
	{
		simFile = std::string(argv[3]);
	}
	if (argc > 4) badarg = true;
	if (badarg) usage(); // Exits.

	TRACE(1, "simFile is %s", simFile.c_str());
	DTC_SimMode mode = DTCLib::DTC_SimModeConverter::ConvertToSimMode(std::to_string(modeint));
	DTC* thisDTC = new DTC(mode);
	thisDTC->WriteSimFileToDTC(simFile, true, true);
	TRACE(1, "thisDTC->ReadSimMode: %i", thisDTC->ReadSimMode());
	DTCSoftwareCFO* theCFO = new DTCSoftwareCFO(thisDTC, true);
	long loopCounter = 0;
	long count = 0;
	typedef uint8_t packet_t[16];

	while (loopCounter < loops)
	{
		TRACE(1, "mu2eReceiver::getNext: Starting CFO thread");
		uint64_t z = 0;
		DTCLib::DTC_Timestamp zero(z);
		TRACE(1, "Sending requests for %i timestamps, starting at %lu", BLOCK_COUNT_MAX, BLOCK_COUNT_MAX * loopCounter);
		theCFO->SendRequestsForRange(BLOCK_COUNT_MAX, DTCLib::DTC_Timestamp(BLOCK_COUNT_MAX * loopCounter));

		fragmentTester newfrag(BLOCK_COUNT_MAX * sizeof(packet_t) * 2);

		DTCLib::DTC_Timestamp expected_timestamp;
		bool firstLoop = true;

		//Get data from DTCReceiver
		TRACE(1, "mu2eReceiver::getNext: Starting DTCFragment Loop");
		while (newfrag.hdr_block_count() < BLOCK_COUNT_MAX)
		{
			TRACE(1, "Getting DTC Data");
			std::vector<DTC_DataBlock> data;
			int retryCount = 5;
			while (data.size() == 0 && retryCount >= 0)
			{
				try
				{
					//TRACE(4, "Calling theInterface->GetData(zero)");
					data = thisDTC->GetData(zero);
					//TRACE(4, "Done calling theInterface->GetData(zero)");
				}
				catch (std::exception ex)
				{
					std::cerr << ex.what() << std::endl;
				}
				retryCount--;
				//if (data.size() == 0) { usleep(10000); }
			}
			if (retryCount < 0 && data.size() == 0)
			{
				TRACE(1, "Retry count exceeded. Something is very wrong indeed");
				std::cout << "Had an error with block " << newfrag.hdr_block_count() << " of event " << loopCounter << std::endl;
				break;
			}

			auto first = DTCLib::DTC_DataHeaderPacket(DTCLib::DTC_DataPacket(data[0].blockPointer));
			DTCLib::DTC_Timestamp ts = first.GetTimestamp();
			if(firstLoop)
			{
				expected_timestamp = ts;
				firstLoop = false;
			}
			if(ts != expected_timestamp)
			{
				std::cerr << "WRONG TIMESTAMP DETECTED: 0x" << std::hex << ts.GetTimestamp(true) << " (expected: 0x" << expected_timestamp.GetTimestamp(true)<<")" << std::endl;
			}
			expected_timestamp = ts + 1;
			//int packetCount = first.GetPacketCount() + 1;
			//TRACE(1, "There are %lu data blocks in timestamp %lu. Packet count of first data block: %i", data.size(), ts.GetTimestamp(true), packetCount);

			size_t totalSize = 0;

			for (size_t i = 0; i < data.size(); ++i)
			{
				totalSize += data[i].byteSize;
			}

			int64_t diff = static_cast<int64_t>(totalSize + newfrag.dataSize()) - newfrag.fragSize();
			TRACE(4, "diff=%lli, totalSize=%llu, dataSize=%llu, fragSize=%llu", (long long)diff, (long long unsigned)totalSize, (long long unsigned)newfrag.dataSize(), (long long unsigned)newfrag.fragSize());
			if (diff > 0)
			{
				auto currSize  = newfrag.fragSize();
				auto remaining = 1 - (newfrag.hdr_block_count() / static_cast<double>(BLOCK_COUNT_MAX));
				auto newSize   = static_cast<size_t>(currSize * remaining);
				TRACE(1, "mu2eReceiver::getNext: %lu + %lu > %lu, allocating space for %lu more bytes", totalSize, newfrag.dataSize(), newfrag.fragSize(), static_cast<unsigned long>(newSize + diff));
				newfrag.addSpace(diff + newSize);
			}

			TRACE(3, "Copying DTC packets into Mu2eFragment");
			auto offset = newfrag.dataBegin() + newfrag.dataSize();
			size_t intraBlockOffset = 0;
			for (size_t i = 0; i < data.size(); ++i)
			{
				TRACE(4, "Copying data from %p to %p (sz=%llu)", reinterpret_cast<void*>(data[i].blockPointer), reinterpret_cast<void*>(offset + intraBlockOffset), (unsigned long long)data[i].byteSize);
				memcpy(reinterpret_cast<void*>(offset + intraBlockOffset), data[i].blockPointer, data[i].byteSize);
				intraBlockOffset += data[i].byteSize;
			}

			TRACE(3, "Ending SubEvt %lu", newfrag.hdr_block_count());
			newfrag.endSubEvt(intraBlockOffset);
		}

		loopCounter++;
		count += newfrag.hdr_block_count();
		//auto file = fopen((std::string("tester_") + std::to_string(loopCounter) + ".bin").c_str(),"w+");
		//fwrite((void*)newfrag.dataBegin(), sizeof(uint8_t), newfrag.fragSize(), file);
		std::cout << "Event: " << loopCounter << ": " << newfrag.hdr_block_count() << " timestamps. (" << count << " total)" << std::endl;
	}

	delete theCFO;
	delete thisDTC;

	return 0;
}

