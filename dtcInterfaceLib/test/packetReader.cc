#include <math.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <fstream>
#include <string>

int main()
{
	bool verbose = true;

	std::string packetType = "TRK";
	//	std::string packetType = "CAL";

	// Number of tracker adc samples
	//	size_t numADCSamples = 8;
	size_t numADCSamples = 12;

	std::string inputFile = "TRK_packets.bin";
	if (packetType == "CAL")
	{
		inputFile = "CAL_packets.bin";
	}

	std::ifstream binFile;
	binFile.open(inputFile, std::ios::in | std::ios::binary | std::ios::ate);

	typedef uint16_t adc_t;

	std::vector<adc_t> masterVector;
	std::vector< std::vector< std::vector<adc_t> > > timeStampVector;
	if (binFile.is_open())
	{
		std::streampos size = binFile.tellg();
		char* memblock = new char[size];
		binFile.seekg(0, std::ios::beg);
		binFile.read(memblock, size);
		binFile.close();

		// Read input file into master adc_t vector
		for (adc_t* curPos = reinterpret_cast<adc_t *>(memblock); curPos != reinterpret_cast<adc_t *>(memblock + (sizeof(char) * size)); curPos++)
		{
			masterVector.push_back((adc_t)(*curPos));
		}
		std::cout << "Number of adc_t entries in input dataset: " << masterVector.size() << std::endl;

		bool exitLoop = false;
		size_t curPos = 0;

		std::vector< std::vector<adc_t> > dataBlockVector; // Vector of adc_t vectors containing DataBlocks
		while (!exitLoop && curPos < masterVector.size())
		{
			std::bitset<64> byteCount = 0;
			std::bitset<16> byteCount0 = masterVector[curPos + 0];
			std::bitset<16> byteCount1 = masterVector[curPos + 1];
			std::bitset<16> byteCount2 = masterVector[curPos + 2];
			std::bitset<16> byteCount3 = masterVector[curPos + 3];
			for (int i = 0; i < 16; i++)
			{
				byteCount[i + 16 * 0] = byteCount0[i];
				byteCount[i + 16 * 1] = byteCount1[i];
				byteCount[i + 16 * 2] = byteCount2[i];
				byteCount[i + 16 * 3] = byteCount3[i];
			}
			size_t theCount = byteCount.to_ulong();

			if (verbose)
			{
				std::cout << "Number of bytes in DMABlock: " << theCount << std::endl;
			}

			size_t blockStartIdx = curPos;
			size_t blockEndIdx = curPos + (theCount / 2);
			size_t posInBlock = 4;

			std::vector<adc_t> curDataBlock;
			while (posInBlock < blockEndIdx - blockStartIdx)
			{
				size_t numDataPackets = masterVector[blockStartIdx + posInBlock + 2];
				for (size_t i = 0; i < 8 + numDataPackets * 8; i++)
				{
					curDataBlock.push_back(masterVector[blockStartIdx + posInBlock + i]);
				}
				dataBlockVector.push_back(curDataBlock);
				curDataBlock.clear();
				posInBlock += 8 + numDataPackets * 8;
			}
			// Skip over the 4*16 bit DMABlock header and skip theCount/2 adc_t values to
			// get to the next DMABlock
			curPos += (theCount / 2);
		}


		std::cout << "DataBlock vector size: " << dataBlockVector.size() << std::endl;
		if (verbose)
		{
			for (size_t eventNum = 0; eventNum < dataBlockVector.size(); eventNum++)
			{
				std::vector<adc_t> packetVector = dataBlockVector[eventNum];
				std::cout << "================================================" << std::endl;
				std::cout << "Hit num: " << eventNum << std::endl;

				// Print out header info
				std::bitset<48> timestamp;
				std::bitset<16> timestamp0 = packetVector[3];
				std::bitset<16> timestamp1 = packetVector[4];
				std::bitset<16> timestamp2 = packetVector[5];
				for (int i = 0; i < 16; i++)
				{
					timestamp[i + 16 * 0] = timestamp0[i];
					timestamp[i + 16 * 1] = timestamp1[i];
					timestamp[i + 16 * 2] = timestamp2[i];
				}
				size_t ts = timestamp.to_ulong();
				std::cout << "\tTimestamp: " << ts << std::endl;

				std::bitset<16> rocring = packetVector[1];
				std::bitset<4> ring;
				std::bitset<4> roc;
				for (size_t i = 0; i < 4; i++)
				{
					roc[i] = rocring[i];
					ring[i] = rocring[i + 8];
				}
				size_t ROC_ID = roc.to_ulong();
				size_t Ring_ID = ring.to_ulong();
				std::cout << "\tROC ID: " << ROC_ID << std::endl;
				std::cout << "\tRing ID: " << Ring_ID << std::endl;

				std::cout << "\tNumber of non-header data packets: " << (packetVector.size() / 8) - 1 << std::endl;

				// Print out payload packet info


				if (packetType == "TRK")
				{
					std::cout << "\tNumber of ADC samples: " << numADCSamples << std::endl;
					std::cout << "\tscaledNoisyVector: {";
					for (size_t i = 8 + 4; i < 8 + 4 + numADCSamples; i++)
					{
						double curVal = packetVector[i];
						if (i > 8 + 4)
						{
							std::cout << ",";
						}
						std::cout << curVal;
					}
					std::cout << "}" << std::endl;
				}
				else if (packetType == "CAL")
				{

				  if((packetVector.size() / 8) - 1 > 0) { // At least 1 data packet following the header packet
				                std::bitset<16> IDNum = packetVector[8];
				                std::bitset<12> crystalID;
				                for(int i= 11; i>=0; i--) {
				                  crystalID[i] = IDNum[i];
				                }
				                std::cout << "\tCrystalID: " << crystalID.to_ulong() << std::endl;

					        adc_t apdID = adc_t(packetVector[8]) >> 12;
					        std::cout << "\tapdID: " << apdID << std::endl;
						
				                adc_t numSamples = packetVector[8 + 2];
						std::cout << "\tNumber of waveform samples: " << numSamples << std::endl;
					        std::cout << "\tscaledNoisyVector: {";
					        for (size_t i = 8U + 3U; i < 8U + 3U + packetVector[8 + 2]; i++)
					        {
					        	double curVal = packetVector[i];
					        	if (i > 8 + 3)
					        	{
					        		std::cout << ",";
					        	}
					        	std::cout << curVal;
					        }
					        std::cout << "}" << std::endl;
					}
				}

				// Print out raw datablock contents, including the header packet
				std::bitset<16> curEntry;
				std::cout << std::endl;
				std::cout << "\tRaw Packets:" << std::endl;
				for (size_t i = 0; i < packetVector.size(); i++)
				{
					curEntry = packetVector[i];
					std::cout << "\t\t" << curEntry.to_string() << " " << curEntry.to_ulong() << std::endl;
					if (i > 0 && (i + 1) % 8 == 0)
					{
						std::cout << std::endl;
					}
				}
			}
		}

		delete[] memblock;
	}
	else
	{
		std::cout << "ERROR: Could not open input file.";
		return 1;
	}

	return 0;
}

