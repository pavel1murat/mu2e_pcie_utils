#include "DTC.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

int main(int argc, char** argv) {
	DTC::DTC* thisDTC = new DTC::DTC();

	int testsPassed = 0;
	std::cout << "Running DTC Tests." << std::endl << std::endl;

	std::cout << "Test 1: Register R/W" << std::endl;
	try{
		std::cout << "Reading Design Version: " << thisDTC->ReadDesignVersion() << std::endl;
		std::cout << "If simulated, result will be 53494D44 (SIMD in ASCII)" << std::endl;
		std::cout << "Attempting to Toggle Ring 0." << std::endl;
		bool ring0Value = thisDTC->ReadRingEnabled(DTC::DTC_Ring_0);
		if (thisDTC->ToggleRingEnabled(DTC::DTC_Ring_0) != ring0Value || thisDTC->IsSimulatedDTC())
		{
			std::cout << "Test Succeeded" << std::endl;
			++testsPassed;
		}
		else
		{
			std::cout << "Test Failed" << std::endl;
		}
	}
	catch (std::exception ex)
	{
		std::cerr << "Test failed with exception: " << ex.what() << std::endl;
	}
	std::cout << std::endl << std::endl;

	std::cout << "Test 2: PCIe State and Stats" << std::endl;
	try {
		std::cout << "PCIe State: " << std::endl
			<< thisDTC->ReadPCIeState().toString() << std::endl << std::endl;
		std::cout << "PCIe Stats, RX: " << thisDTC->ReadPCIeStats().LRX << ", TX: " << thisDTC->ReadPCIeStats().LTX << std::endl;
		std::cout << "Test Passed" << std::endl;
		++testsPassed;
	}
	catch (std::exception ex)
	{
		std::cerr << "Test failed with exception: " << ex.what() << std::endl;
	}
	std::cout << std::endl << std::endl;

	std::cout << "Test 3: DMA State and Stats" << std::endl;
	try {
		std::cout << "DMA State: " << std::endl
			<< "DAQ Channel, S2C: " << thisDTC->ReadDMAState(DTC::DTC_DMA_Engine_DAQ, DTC::DTC_DMA_Direction_S2C).toString() << std::endl
			<< "DAQ Channel, C2S: " << thisDTC->ReadDMAState(DTC::DTC_DMA_Engine_DAQ, DTC::DTC_DMA_Direction_C2S).toString() << std::endl
			<< "DCS Channel, S2C: " << thisDTC->ReadDMAState(DTC::DTC_DMA_Engine_DCS, DTC::DTC_DMA_Direction_S2C).toString() << std::endl
			<< "DCS Channel, C2S: " << thisDTC->ReadDMAState(DTC::DTC_DMA_Engine_DCS, DTC::DTC_DMA_Direction_C2S).toString() << std::endl;
		std::cout << "DMA Stats: " << std::endl
			<< "DAQ Channel, S2C: " << thisDTC->ReadDMAStats(DTC::DTC_DMA_Engine_DAQ, DTC::DTC_DMA_Direction_S2C).Stats[0].toString() << std::endl
			<< "DAQ Channel, C2S: " << thisDTC->ReadDMAStats(DTC::DTC_DMA_Engine_DAQ, DTC::DTC_DMA_Direction_C2S).Stats[0].toString() << std::endl
			<< "DCS Channel, S2C: " << thisDTC->ReadDMAStats(DTC::DTC_DMA_Engine_DCS, DTC::DTC_DMA_Direction_S2C).Stats[0].toString() << std::endl
			<< "DCS Channel, C2S: " << thisDTC->ReadDMAStats(DTC::DTC_DMA_Engine_DCS, DTC::DTC_DMA_Direction_C2S).Stats[0].toString() << std::endl;
		std::cout << "Test Passed." << std::endl;
		++testsPassed;
	}
	catch (std::exception ex)
	{
		std::cerr << "Test failed with exception: " << ex.what() << std::endl;
	}
	std::cout << std::endl << std::endl;

	std::cout << "Test 4: DMA R/W on DCS Channel" << std::endl;
	try{
		std::cout << "Running DCS Request/Reply Cycle with 0-11 sequence" << std::endl;
		uint8_t testData[12] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		std::cout << "Data in: ";
		for (int i = 0; i < 12; i++)
		{
			std::cout << std::dec << (int)testData[i] << " ";
		}
		std::cout << std::endl;
		thisDTC->DCSRequestReply(DTC::DTC_Ring_0, DTC::DTC_ROC_0, testData);
		std::cout << "Data out: ";
		for (int i = 0; i < 12; i++)
		{
			std::cout << std::dec << (int)testData[i] << " ";
		}
		std::cout << std::endl;
		std::cout << "Simulated DTC should match before/after." << std::endl;
		++testsPassed;
	}
	catch (std::exception ex)
	{
		std::cerr << "Test failed with exception: " << ex.what() << std::endl;
	}
	std::cout << std::endl << std::endl;

	std::cout << "Test 5: DMA R/W on DAQ Channel" << std::endl;
	try{
		std::cout << "Sending Readout Request Packet on Ring 0" << std::endl;
		thisDTC->SendReadoutRequestPacket(DTC::DTC_Ring_0, DTC::DTC_Timestamp((uint64_t)time(0)));
		int length;
		int retry = 3;
		bool err = false;
		do {
			std::vector<void*> data = thisDTC->GetData(DTC::DTC_Ring_0, DTC::DTC_ROC_0, DTC::DTC_Timestamp((uint64_t)time(0)), &length);
			if (data.size() > 0) {
				if (data.size() > 1) {
					std::cout << "Data array is larger than expected! Cowardly refusing to continue the test." << std::endl;
					err = true;
					break;
				}
				else {
					std::cout << "Dumping data..." << std::endl;
					for (int i = 0; i < length; ++i)
					{
						for (int j = 0; j < 8; ++j)
						{
							std::cout <<"0x" << std::setfill('0') << std::setw(4) << std::hex
								<< ((uint16_t*)data[0])[i * 8 + j] << std::endl;
						}
					}
				}
			}
			retry--;
		} while (retry > 0);
		if (err) {
			std::cout << "Test Aborted (fail!)" << std::endl;
		}
		else {
			std::cout << "Test Passed" << std::endl;
			++testsPassed;
		}
	}
	catch (std::exception ex)
	{
		std::cerr << "Test failed with exception: " << ex.what() << std::endl;
	}
	std::cout << std::endl << std::endl;
	std::cout << testsPassed << " of 5 tests passed." << std::endl;
}