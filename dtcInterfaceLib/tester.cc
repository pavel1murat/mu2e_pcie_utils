#include <cstdio>		// printf
#include <cstdlib>		// strtoul
#include <iostream>
#include <chrono>
#include "DTC.h"
#include "DTCSoftwareCFO.h"
#include "fragmentTester.h"

#ifdef _WIN32
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# ifndef TRACE
#  include <stdio.h>
#  define TRACE(...)
# endif
# define TRACE_CNTL(...)
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif
#define TRACE_NAME "MU2EDEV"

using namespace DTCLib;
int main(int argc, char* argv[])
{
	DTC *thisDTC = new DTC(DTC_SimMode_Tracker);
	DTCSoftwareCFO *theCFO = new DTCSoftwareCFO(thisDTC, true);
	long loopCounter = 0;
	long count = 0;
	typedef uint8_t packet_t[16];

	while (loopCounter < 1000)
	{
		TRACE(1, "mu2eReceiver::getNext: Starting CFO thread");
		uint64_t z = 0;
		DTCLib::DTC_Timestamp zero(z);
		TRACE(1, "Sending requests for %i timestamps, starting at %lu", BLOCK_COUNT_MAX, BLOCK_COUNT_MAX * loopCounter);
		theCFO->SendRequestsForRange(BLOCK_COUNT_MAX, DTCLib::DTC_Timestamp(BLOCK_COUNT_MAX * loopCounter));

		fragmentTester newfrag(BLOCK_COUNT_MAX * sizeof(packet_t) * 2);

		//Get data from DTCReceiver
		TRACE(1, "mu2eReceiver::getNext: Starting DTCFragment Loop");
		while (newfrag.hdr_block_count() < BLOCK_COUNT_MAX)
		{

			TRACE(1, "Getting DTC Data");
			std::vector<void*> data;
			int retryCount = 5;
			while (data.size() == 0 && retryCount >= 0)
			{
				try
				{
					TRACE(4, "Calling theInterface->GetData(zero)");
					data = thisDTC->GetData(zero);
					TRACE(4, "Done calling theInterface->GetData(zero)");
				}
				catch (std::exception ex)
				{
					std::cerr << ex.what() << std::endl;
				}
				retryCount--;
				if (data.size() == 0) { usleep(10000); }
			}
			if (retryCount < 0 && data.size() == 0) {
				TRACE(1, "Retry count exceeded. Something is very wrong indeed");
				std::cout << "Had an error with block " << newfrag.hdr_block_count() << " of event " << loopCounter << std::endl;
				break;
			}

			auto first = DTCLib::DTC_DataHeaderPacket(DTCLib::DTC_DataPacket(data[0]));
			DTCLib::DTC_Timestamp ts = first.GetTimestamp();
			int packetCount = first.GetPacketCount() + 1;
			TRACE(1, "There are %lu data blocks in timestamp %lu. Packet count of first data block: %i", data.size(), ts.GetTimestamp(true), packetCount);

			for (size_t i = 1; i < data.size(); ++i)
			{
				auto packet = DTCLib::DTC_DataHeaderPacket(DTCLib::DTC_DataPacket(data[i]));
				packetCount += packet.GetPacketCount() + 1;
			}

			auto dataSize = packetCount * sizeof(packet_t);
			int64_t diff = dataSize + newfrag.dataSize() - newfrag.fragSize();
			if (diff > 0) {
				TRACE(1, "mu2eReceiver::getNext: %lu + %lu > %lu, allocating space for 1%% BLOCK_COUNT_MAX more packets", dataSize, newfrag.dataSize(), newfrag.fragSize());
				newfrag.addSpace(diff + (BLOCK_COUNT_MAX / 100) * sizeof(packet_t));
			}

			TRACE(3, "Copying DTC packets into Mu2eFragment");
			size_t packetsProcessed = 0;
			packet_t* offset = reinterpret_cast<packet_t*>((uint8_t*)newfrag.dataBegin() + newfrag.dataSize());
			for (size_t i = 0; i < data.size(); ++i)
			{
				TRACE(3, "Creating packet object to determine data block size: i=%lu, data=%p", i, data[i]);
				auto packet = DTCLib::DTC_DataHeaderPacket(DTCLib::DTC_DataPacket(data[i]));
				TRACE(3, "Copying packet %lu. src=%p, dst=%p, sz=%lu off=%p processed=%lu", i, data[i], 
					(void*)(offset + packetsProcessed), (1 + packet.GetPacketCount())*sizeof(packet_t), 
					(void*)offset, packetsProcessed);
				memcpy((void*)(offset + packetsProcessed), data[i], (1 + packet.GetPacketCount())*sizeof(packet_t));
				TRACE(3, "Incrementing packet counter");
				packetsProcessed += 1 + packet.GetPacketCount();
			}

			TRACE(3, "Ending SubEvt");
			newfrag.endSubEvt(packetsProcessed * sizeof(packet_t));
		}

		loopCounter++;
		count += newfrag.hdr_block_count();
		std::cout << "Event: " << loopCounter << ": " << newfrag.hdr_block_count() << " timestamps. (" << count << " total)" << std::endl;
	}


	return 0;
}