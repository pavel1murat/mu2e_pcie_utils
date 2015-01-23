#include "DTC.h"

#include <iostream>

int main(int argc, char** argv) {
	DTC::DTC* thisDTC = new DTC::DTC();
	std::cout << "DMA Stats Length: " << thisDTC->ReadDMAStats(DTC::DTC_DMA_Engine_DAQ, DTC::DTC_DMA_Direction_C2S).size() << std::endl;
	DTC::DTC_PCIeStat stat = thisDTC->ReadPCIeStats();
	std::cout << "PCIe Stats: TX: " << stat.LTX  << ", RX: " << stat.LRX << std::endl;

}