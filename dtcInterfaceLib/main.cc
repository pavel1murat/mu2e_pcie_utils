#include "DTC.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

void usage() {
	std::cout << "This program runs several functionality tests of libDTCInterface." << std::endl
		<< "If run with no options, it will run all 5 tests." << std::endl
		<< "Otherwise, it accepts a space-delimited list of the tests to run," << std::endl
		<< "defined either by test number {1,2,3,4,5}, or test name {reg, pcie, stats, dcs, daq}" << std::endl
		<< "It also accepts a -n argument indicating how many iterations of the tests it should run" << std::endl;
}

int main(int argc, char* argv[]) {
	DTC::DTC* thisDTC = new DTC::DTC();

	int testsPassed = 0;
	int nTests = 0;
	int testCount = 1;
	bool registerTest = false,
		pcieTest = false,
		dmaStateTest = false,
		dcsTest = false,
		daqTest = false;
	bool testsSpecified = false;

	if (argc == 1) {
		std::cout << "Running all DTC Tests." << std::endl << std::endl;
	}
	else {

		for (int i = 1; i < argc; ++i) {
			int firstChar = 0;
			if (argv[i][0] == '-') {
				firstChar = 1;
				if (argv[i][1] == 'n' && argc >= i + 1) {
					++i;
					testCount = atoi(argv[i]);
					continue;
				}
			}
			if (isdigit(argv[i][firstChar])) {
				testsSpecified = true;
				switch (argv[i][firstChar] - '0') {
				case 1:
					registerTest = true;
					break;
				case 2:
					pcieTest = true;
					break;
				case 3:
					dmaStateTest = true;
					break;
				case 4:
					dcsTest = true;
					break;
				case 5:
					daqTest = true;
					break;
				}
			}
			else {
				std::string arg(argv[i]);
				arg = arg.substr(firstChar);
				if (arg.find("reg") != std::string::npos) {
					testsSpecified = true;
					registerTest = true;
				}
				else if (arg.find("pcie") != std::string::npos) {
					testsSpecified = true;
					pcieTest = true;
				}
				else if (arg.find("stats") != std::string::npos) {
					dmaStateTest = true;
					testsSpecified = true;
				}
				else if (arg.find("dcs") != std::string::npos) {
					testsSpecified = true;
					dcsTest = true;
				}
				else if (arg.find("daq") != std::string::npos || arg.find("dma") != std::string::npos) {
					daqTest = true;
					testsSpecified = true;
				}
				else {
					usage();
				}
			}
		}
		if (!testsSpecified){
			registerTest = true;
			pcieTest = true;
			dmaStateTest = true;
			dcsTest = true;
			daqTest = true;
		}

		std::cout << "Running tests: " << (registerTest ? "Register I/O " : "") << (pcieTest ? " PCIe State/Stats " : "")
			<< (dmaStateTest ? " DMA State/Stats " : "") << (dcsTest ? " DCS DMA I/O " : "") << (daqTest ? " DAQ DMA I/O" : "")
			<< ", " << testCount << " times." << std::endl;

	}

	int regTestCount = 0;
	while (registerTest && regTestCount < testCount) {
		std::cout << "Test 1: Register R/W" << std::endl;
		++nTests;
		++regTestCount;
		try{
			std::cout << "Reading Design Version: " << thisDTC->ReadDesignVersion() << std::endl;
			std::cout << "If simulated, result will be 53494D44 (SIMD in ASCII)" << std::endl;
			std::cout << "Attempting to Toggle Ring 0." << std::endl;
			bool ring0Value = thisDTC->ReadRingEnabled(DTC::DTC_Ring_0);
			std::cout << "Value before: " << ring0Value << std::endl;
			bool ring0New = thisDTC->ToggleRingEnabled(DTC::DTC_Ring_0);
			std::cout << "Value after: " << ring0New << std::endl;
			// Make sure that the ring is enabled after the test.
			thisDTC->EnableRing(DTC::DTC_Ring_0, DTC::DTC_ROC_0);
			if (ring0New != ring0Value)
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
	}

	int pcieTestCount = 0;
	while (pcieTest && pcieTestCount < testCount)  {
		std::cout << "Test 2: PCIe State and Stats" << std::endl;
		++nTests;
		++pcieTestCount;
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
	}

	int dmaStateTestCount = 0;
	while (dmaStateTest && dmaStateTestCount < testCount) {
		std::cout << "Test 3: DMA State and Stats" << std::endl;
		++nTests;
		++dmaStateTestCount;
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
	}

	int dcsTestCount = 0;
	while (dcsTest && dcsTestCount < testCount) {
		std::cout << "Test 4: DMA R/W on DCS Channel" << std::endl;
		++nTests;
		++dcsTestCount;
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
	}

	int daqTestCount = 0;
	while (daqTest && daqTestCount < testCount) {
		std::cout << "Test 5: DMA R/W on DAQ Channel" << std::endl;
		++nTests;
		++daqTestCount;
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
								std::cout << "0x" << std::setfill('0') << std::setw(4) << std::hex
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
	}
	std::cout <<std::dec << testsPassed << " of " << nTests << " tests passed." << std::endl;
}