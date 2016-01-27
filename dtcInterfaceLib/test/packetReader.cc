#include<math.h>
#include<iostream>
#include<vector>
#include<random>
#include<bitset>
#include<fstream>
#include<string>

using namespace std;

int main() {

  bool verbose = false;

  string packetType = "TRK";
  //string packetType = "CAL";

  // Number of tracker adc samples
  size_t numADCSamples = 8;

  string inputFile = "TRK_packets.bin";
  if(packetType=="CAL") {
    inputFile = "CAL_packets.bin";
  }
  
  ifstream binFile;  
  binFile.open(inputFile, ios::in | ios::binary | ios::ate);

  typedef uint16_t adc_t;

  vector<adc_t> masterVector;
  vector< vector< vector<adc_t> > > timeStampVector;
  if(binFile.is_open()) {
    streampos size =  binFile.tellg();
    char * memblock = new char[size];
    binFile.seekg (0, ios::beg);
    binFile.read (memblock, size);
    binFile.close();

    // Read input file into master adc_t vector
    for(adc_t* curPos = reinterpret_cast<adc_t *>(memblock); curPos != reinterpret_cast<adc_t *>(memblock+(sizeof(char)*size)); curPos ++) {
      masterVector.push_back((adc_t)(*curPos));
    }
    cout << "Number of adc_t entries in input dataset: " << masterVector.size() << endl;

    bool exitLoop = false;
    size_t curPos=0;

    vector< vector<adc_t> > dataBlockVector; // Vector of adc_t vectors containing DataBlocks
    while(!exitLoop && curPos<masterVector.size()) {
      bitset<64> byteCount = 0;
      bitset<16> byteCount0 = masterVector[curPos+0];
      bitset<16> byteCount1 = masterVector[curPos+1];
      bitset<16> byteCount2 = masterVector[curPos+2];
      bitset<16> byteCount3 = masterVector[curPos+3];
      for(int i=0; i<16; i++) {
	byteCount[i+16*0] = byteCount0[i];
	byteCount[i+16*1] = byteCount1[i];
	byteCount[i+16*2] = byteCount2[i];
	byteCount[i+16*3] = byteCount3[i];
      }
      size_t theCount = byteCount.to_ulong();

      if(verbose) {
	cout << "Number of bytes in DMABlock: " << theCount << endl;
      }
      
      size_t blockStartIdx = curPos + 4;
      size_t blockEndIdx = curPos + 4 + (theCount/2);
      size_t posInBlock = 0;

      vector<adc_t> curDataBlock;
      while(posInBlock<blockEndIdx-blockStartIdx) {
	size_t numDataPackets = masterVector[blockStartIdx + posInBlock + 2];
	for(size_t i=0; i<8+numDataPackets*8; i++) {
	  curDataBlock.push_back(masterVector[blockStartIdx + posInBlock + i]);
	}
	dataBlockVector.push_back(curDataBlock);
	curDataBlock.clear();
	posInBlock += 8+numDataPackets*8;
      }
      // Skip over the 4*16 bit DMABlock header and skip theCount/2 adc_t values to
      // get to the next DMABlock
      curPos += 4 + (theCount/2);
    }


    cout << "DataBlock vector size: " << dataBlockVector.size() << endl;  
    if(verbose) {
      for(size_t eventNum = 0; eventNum<dataBlockVector.size(); eventNum++) {    
	vector<adc_t> packetVector = dataBlockVector[eventNum];
	cout << "================================================" << endl;
	cout << "Hit num: " << eventNum << endl;

	// Print out header info
	bitset<48> timestamp;
	bitset<16> timestamp0 = packetVector[3];
	bitset<16> timestamp1 = packetVector[4];
	bitset<16> timestamp2 = packetVector[5];
	for(int i=0; i<16; i++) {
	  timestamp[i+16*0] = timestamp0[i];
	  timestamp[i+16*1] = timestamp1[i];
	  timestamp[i+16*2] = timestamp2[i];
	}
	size_t ts = timestamp.to_ulong();
	cout << "\tTimestamp: " << ts << endl;

	bitset<16> rocring = packetVector[1];
	bitset<4> ring;
	bitset<4> roc;
	for(size_t i=0; i<4; i++) {
	  roc[i] = rocring[i];
	  ring[i] = rocring[i+8];
	}
	size_t ROC_ID = roc.to_ulong();
	size_t Ring_ID = ring.to_ulong();
	cout << "\tROC ID: " << ROC_ID << endl;
	cout << "\tRing ID: " << Ring_ID << endl;
	  
	cout << "\tNumber of non-header data packets: " << (packetVector.size()/8)-1 << endl;

	// Print out payload packet info

	
	if(packetType == "TRK") {
	  cout << "\tNumber of ADC samples: " << numADCSamples << endl;
	  cout << "\tscaledNoisyVector: {";
	  for(size_t i = 8+3; i<8+3+numADCSamples; i++) {
	    double curVal = packetVector[i];
	    if(i>8+3) {
	      cout << ",";
	    }
	    cout << curVal;
	  }
	  cout << "}" << endl;
	  	  
	} else if(packetType == "CAL") {

	  cout << "\tNumber of waveform samples: " << packetVector[8+2] << endl;
	  cout << "\tscaledNoisyVector: {";
	  for(size_t i = 8U+3U; i<8U+3U+packetVector[8+2]; i++) {
	    double curVal = packetVector[i];
	    if(i>8+3) {
	      cout << ",";
	    }
	    cout << curVal;
	  }
	  cout << "}" << endl;
	}

	// Print out raw datablock contents, including the header packet
	bitset<16> curEntry;
	cout << endl;
	cout << "\tRaw Packets:" << endl;
	for(size_t i=0; i<packetVector.size(); i++) {
	  curEntry = packetVector[i];
	  cout << "\t\t" << curEntry.to_string() << " " << curEntry.to_ulong() << endl;
	  if(i>0 && (i+1)%8==0) {
	    cout << endl;
	  }
	}

      }
    }
    
    delete[] memblock;  
  } else {
    cout << "ERROR: Could not open input file.";
    return 1;
  }
  
  return 0;
}
