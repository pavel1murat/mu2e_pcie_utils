#include<math.h>
#include<iostream>
#include<vector>
#include<random>
#include<bitset>
#include<fstream>
#include<assert.h>
#include<string>

using namespace std;

typedef uint16_t adc_t;

// Waveform for TRK:
double f(double t, double tau, double sigma, double offset);
// Waveform for CAL
double logn(double x, double eta, double sigma, double Epeak, double norm);
// CAL Digitizer specs:
//    200M samples per second => 5ns steps
//    Zero-suppression on board
//    12 bit resolution over 2V dynamic range

vector<adc_t> generateDMABlockHeader(size_t byteCount);


int main() {
  
  bool verbose = false;
  bool save_adc_values = true;
  double nevents = 200000; // Number of hits to generate

  // Packet type to generate:
  //string packetType = "TRK";
  string packetType = "TRK";
  
  size_t numADCSamples = 8;

  // The timestamp count will be offset by the following amount:
  size_t starting_timestamp = 0;

  //size_t max_DMA_block_size = 16 * 20; // Maximum size in bytes of a DMA block
  size_t max_DMA_block_size = 64000; // Maximum size in bytes of a DMA block
  // Normally a DMA block begins when a new timestamp begins, however
  // if the size of the DMA block will exceed the limit within the current
  // timestamp, a new block is created

  string outputFile = "TRK_packets.bin";
  if(packetType == "CAL") {
    outputFile = "CAL_packets.bin";
  }
  
  ofstream binFile;  
  if(save_adc_values) {
	binFile.open(outputFile, ios::out | ios::app | ios::binary);
  }
  
  int number_of_rings = 6; // Used for generating randomized ring IDs
  int rocs_per_ring = 5;   //Used for generating randtomized ROC IDs
  std::uniform_int_distribution<int> Ring_distribution(0,number_of_rings-1);
  std::uniform_int_distribution<int> ROC_distribution(0,rocs_per_ring-1);
  
  int min_datablocks_per_ts = 5; // Min number of datablocks from a single timestamp
  int max_datablocks_per_ts = 20; // Max number of datablocks from a single timestamp
  std::uniform_int_distribution<int> NumDataBlocks_distribution(min_datablocks_per_ts,max_datablocks_per_ts);





  
  double peakVal = 32;
  int noise_level = 140;
  int sigma_noise = 10;
  double threshold = 0.3;
  double nominal_tau = -0.9;
  double nominal_sigma = 55;
  double nominal_offset = 10;
  double nominal_eta = 0.03;
  double nominal_Epeak = 10;
  double nominal_norm = 1000;
  double sigma_tau = 2;
  double sigma_sigma = 2;
  double sigma_offset = 2;
  double sigma_eta = 0.03;
  double sigma_Epeak = 10;
  double sigma_norm = 15;
  double minTime = 0;
  double maxTime = 2000;
  double stepSize = 5;
  if(packetType == "TRK") {
    // Set maximum waveform value for scaling purposes
    peakVal = 0.02;
    
    noise_level = 200;
    sigma_noise = 15;
    
    threshold = 0.0001; // Detection threshold
    
    // Nominal values for hit generation
    nominal_tau   = 20;
    nominal_sigma = 20;  
    nominal_offset = 10;
    // Uncertainties on nominal values for use in randomizing hits
    sigma_tau   = 2;
    sigma_sigma = 2;
    sigma_offset = 2;
    
    minTime = 0;
    maxTime = 300; // nanoseconds
    stepSize = 20; // 20ns => 50M samples per second
    
  } else if(packetType == "CAL") {
    // Set maximum logn value for scaling purposes
    peakVal = 32; // Actually about 28.8372
    noise_level = 140;
    sigma_noise = 10;
    
    threshold = 0.3; // Detection threshold
    
    // Nominal values for hit generation
    nominal_eta   = -0.9;
    nominal_sigma = 55;  
    nominal_Epeak = 200; 
    nominal_norm  = 1000;
    // Uncertainties on nominal values for use in randomizing hits
    sigma_eta   = 0.03;
    sigma_sigma = 5;  
    sigma_Epeak = 10; 
    sigma_norm  = 15;
    
    minTime = 0;
    maxTime = 2000; // nanoseconds
    stepSize = 5; // 5ns => 200M samples per second
  } else {
    cout << "ERROR: Invalid packetType: " << packetType << endl;
    return 1;
  }

  // PRNG initialization
  std::default_random_engine generator;

  std::normal_distribution<double> tau_distribution(nominal_tau,sigma_tau);
  std::normal_distribution<double> sigma_distribution(nominal_sigma,sigma_sigma);
  std::normal_distribution<double> offset_distribution(nominal_offset,sigma_offset);

  std::uniform_int_distribution<int> strawIndex_distribution(0,23039);

  // For now, just use a random value between 0 and 50000 for the TDC
  std::uniform_int_distribution<int> TDC_distribution(0,50000);

  std::normal_distribution<double> eta_distribution(nominal_eta,sigma_eta);
  std::normal_distribution<double> Epeak_distribution(nominal_Epeak,sigma_Epeak);
  std::normal_distribution<double> norm_distribution(nominal_norm,sigma_norm);

  std::uniform_int_distribution<int> crystalID_distribution(0,1860);
  std::uniform_int_distribution<int> apdID_distribution(0,1);

  std::normal_distribution<double> noise_distribution(noise_level,sigma_noise);


  // timeStampVector holds 1 vector per timestamp
  // Each of these vectors contains all the datablocks for that timestamp
  // Each datablock vector contains the adc_t values for the header packet
  // and associated data packets
  vector< vector< vector<adc_t> > > timeStampVector;

  for(size_t eventNum=0; eventNum<nevents; eventNum++) {
    size_t targetNumDataBlocks = NumDataBlocks_distribution(generator);
    vector< vector<adc_t> > curDataBlockVector;
    for(size_t dataBlockNum = 0; dataBlockNum<targetNumDataBlocks; dataBlockNum++) {

      vector<adc_t> curDataBlock;
      // Add the header packet to the DataBlock (leaving including a placeholder for
      // the number of packets in the DataBlock);
      adc_t null_adc = 0;
      // First 16 bits of header (reserved values)
      curDataBlock.push_back(null_adc);
      // Second 16 bits of header (ROC ID, packet type, and ring ID):
      bitset<16> curROCID  = ROC_distribution(generator); // 4 bit ROC ID
      bitset<16> headerPacketType = 5; // 4 bit Data packet header type is 5
      headerPacketType <<= 4; // Shift left by 4
      bitset<16> curRingID = Ring_distribution(generator); // 3 bit ring ID
      curRingID <<= 8; // Shift left by 8
      bitset<16> secondEntry = (curROCID | headerPacketType | curRingID);
      curDataBlock.push_back((adc_t) secondEntry.to_ulong());
      // Third 16 bits of header (place-holder for number of packets in datablock)
      curDataBlock.push_back(null_adc);
      // Fourth through sixth 16 bits of header (timestamp)
      bitset<48> timestamp = eventNum+starting_timestamp;
      bitset<16> timestamp0 = 0;
      bitset<16> timestamp1 = 0;
      bitset<16> timestamp2 = 0;
      for(int i=0; i<16; i++) {
		timestamp0[i] = timestamp[i+16*0];
		timestamp1[i] = timestamp[i+16*1];
		timestamp2[i] = timestamp[i+16*2];
      }
      curDataBlock.push_back((adc_t) timestamp0.to_ulong());
      curDataBlock.push_back((adc_t) timestamp1.to_ulong());
      curDataBlock.push_back((adc_t) timestamp2.to_ulong());
      // Seventh 16 bits of header (data packet format version and status)
      adc_t status = 0; // 0 Corresponds to "Timestamp has valid data"
      adc_t formatVersion = (5 << 8); // Using 5 for now
      curDataBlock.push_back(formatVersion+status);
      // Eighth 16 bits of header (Unassigned)
      curDataBlock.push_back(null_adc);

      // Vector to hold raw digitized waveform
      vector<double> digiVector;
      double hitTime = 0.0;

      // Create a vector of adc_t values corresponding to
      // the content of TRK/CAL data packets.
      vector<adc_t> packetVector;

      
      if(packetType == "TRK") {
		// Generate TRK data packets
		double cur_tau = tau_distribution(generator);
		double cur_sigma = sigma_distribution(generator);
		double cur_offset = offset_distribution(generator);
	
		bool inWindow = false;
		bool exitWindow = false;
		double curVal = 0;
		for(double curTime = minTime; curTime<maxTime && !exitWindow; curTime += stepSize) {
		  curVal = f(curTime, cur_tau, cur_sigma, cur_offset);
		  if(!inWindow && curVal>threshold) {
			inWindow = true;
			curTime -= 2*stepSize; // Save samples starting 2 steps before the threshold is reached
			hitTime = curTime;
			curVal = f(curTime, cur_tau, cur_sigma, cur_offset);
		  } else if(curVal<threshold && inWindow && curTime>hitTime+2*stepSize) {
			inWindow = false;
			exitWindow = true;
		  }
	  
		  if(inWindow) {
			digiVector.push_back(curVal);
		  }
		}
	
		adc_t strawIndex = strawIndex_distribution(generator);
		adc_t TDC0 = TDC_distribution(generator);
		adc_t TDC1 = TDC_distribution(generator);
	
		packetVector.push_back(strawIndex);
		packetVector.push_back(TDC0);
		packetVector.push_back(TDC1);
		for(size_t i=0; i<numADCSamples; i++) {
		  adc_t scaledVal = 0;
		  // Scale the function value relative to a peak
		  // value of around 0.02 and convert to a 12 bit integer
		  // stored in a 16 bit adc_t
		  if(i<digiVector.size()) {
			scaledVal = digiVector[i]/(1.0*peakVal) * (1<<12);
		  } 
		  // Add some noise
		  adc_t noise = noise_distribution(generator);
		  packetVector.push_back(scaledVal+noise);
		}    
		// Pad any empty space in the last packet with 0s
		size_t padding_slots = 8-((numADCSamples-5)%8);
		if(padding_slots<8) {
		  for(size_t i=0; i<padding_slots; i++) {
			packetVector.push_back((adc_t)0);
		  }
		}
	
      } else if(packetType == "CAL") {

		double cur_eta = eta_distribution(generator);
		double cur_sigma = sigma_distribution(generator);
		double cur_Epeak = Epeak_distribution(generator);
		double cur_norm = norm_distribution(generator);
	
		bool inWindow = false;
		bool exitWindow = false;
		double curVal = 0;
		for(double curTime = minTime; curTime<maxTime && !exitWindow; curTime += stepSize) {
		  curVal = logn(curTime, cur_eta, cur_sigma, cur_Epeak, cur_norm);
		  if(!inWindow && curVal>threshold) {
			inWindow = true;
			curTime -= 5*stepSize; // Save samples starting 5 steps before the threshold is reached
			hitTime = curTime;
			curVal = logn(curTime, cur_eta, cur_sigma, cur_Epeak, cur_norm);
		  } else if(curVal<threshold && inWindow && curTime>hitTime+5*stepSize) {
			inWindow = false;
			exitWindow = true;
		  }
	  
		  if(inWindow) {
			digiVector.push_back(curVal);
		  }
		}
	
		adc_t apdID = apdID_distribution(generator);
		adc_t crystalID = crystalID_distribution(generator);
		adc_t IDNum = ((apdID<<12)|crystalID);
		packetVector.push_back(IDNum);
		packetVector.push_back((adc_t)hitTime);
		packetVector.push_back((adc_t)digiVector.size());
		for(size_t i=0; i<digiVector.size(); i++) {
		  // Scale the function value relative to a peak possible
		  // value of around 35 and convert to a 12 bit integer
		  // stored in a 16 bit adc_t
		  adc_t scaledVal = digiVector[i]/(1.0*peakVal) * (1<<12);
		  // Add some noise
		  adc_t noise = noise_distribution(generator);
		  packetVector.push_back(scaledVal+noise);
		}
		// Pad any empty space in the last packet with 0s
		size_t padding_slots = 8-((digiVector.size()-5)%8);
		if(padding_slots<8) {
		  for(size_t i=0; i<padding_slots; i++) {
			packetVector.push_back((adc_t)0);
		  }
		}

      } 
      
      
      // Fill in the number of data packets entry in the header packet
      adc_t numDataPackets = packetVector.size() / 8;
      curDataBlock[2] = numDataPackets;
      
      // Append the data packets after the header packet in the DataBlock
      curDataBlock.insert( curDataBlock.end(), packetVector.begin(), packetVector.end() );      
      curDataBlockVector.push_back(curDataBlock);
      
      
    } // Done generating DataBlocks for this timestamp
    timeStampVector.push_back(curDataBlockVector);
  }
  // Done generating data for all timestamps

  cout << "Size of timestamp vector: " << timeStampVector.size() << endl;

  if(verbose) {
    cout << "\tNumber of DataBlocks for each timestamp vector: {";
    for(size_t i=0; i<timeStampVector.size(); i++) {
      cout << timeStampVector[i].size();
      if(i<timeStampVector.size()-1) {
		cout << ", ";
      }
    }
    cout << "}" << endl;

    cout << "\tNumber of packets in each data block: {";
    for(size_t i=0; i<timeStampVector.size(); i++) {
      cout << "{";
      for(size_t j=0; j<timeStampVector[i].size(); j++) {
		cout << (timeStampVector[i][j].size())/8;
		if(j<timeStampVector[i].size()-1) {
		  cout << ", ";
		}
      }
      cout << "}";
      if(i<timeStampVector.size()-1) {
		cout << ",";
      }
    }
    cout << "}" << endl;
  }


  vector<adc_t> masterVector;
  for(size_t i=0; i<timeStampVector.size(); i++) {

    // Determine how to divide DataBlocks between DMABlocks within each timestamp
    vector<size_t> dataBlockPartition;
    vector<size_t> dataBlockPartitionSizes;
    for(size_t j=0,curDMABlockSize=0,numDataBlocksInCurDMABlock=0; j<timeStampVector[i].size();j++) {
      numDataBlocksInCurDMABlock++; // Increment number of DataBlocks in the current DMA block
      curDMABlockSize+=timeStampVector[i][j].size()*2; // Size of current data block in 8bit words
      assert(curDMABlockSize<=max_DMA_block_size);
      if(j==timeStampVector[i].size()-1) {
		dataBlockPartition.push_back(numDataBlocksInCurDMABlock);
		dataBlockPartitionSizes.push_back(curDMABlockSize);
      } else if(curDMABlockSize+(2*timeStampVector[i][j+1].size()) > max_DMA_block_size){
		dataBlockPartition.push_back(numDataBlocksInCurDMABlock);
		dataBlockPartitionSizes.push_back(curDMABlockSize);
		curDMABlockSize = 0;
		numDataBlocksInCurDMABlock = 0;
      }
    }
    
    // Break the DataBlocks into DMABlocks and add DMABlock headers
    for(size_t curDMABlockNum = 0, curDataBlockNum=0; curDMABlockNum<dataBlockPartition.size(); curDMABlockNum++) {
      size_t numDataBlocksInCurDMABlock = dataBlockPartition[curDMABlockNum];
      size_t curDMABlockSize = dataBlockPartitionSizes[curDMABlockNum];
      vector<adc_t> header = generateDMABlockHeader(curDMABlockSize);
      for(size_t adcNum=0; adcNum<header.size(); adcNum++) {
		masterVector.push_back(header[adcNum]);
      }      
      
      for(size_t j=0; j<numDataBlocksInCurDMABlock; j++) {
		vector<adc_t> curDataBlock = timeStampVector[i][curDataBlockNum];
		for(size_t adcNum=0; adcNum<curDataBlock.size(); adcNum++) {
		  masterVector.push_back(curDataBlock[adcNum]);
		}
		curDataBlockNum++;
      }
    }

    if(verbose) {
      // Print out number of DataBlocks in each DMABlock
      cout << "\tTimestamp " << i+starting_timestamp  << " DataBlock partition: {";
      for(size_t k = 0; k<dataBlockPartition.size(); k++) {
		cout << dataBlockPartition[k];
		if(k<dataBlockPartition.size()-1) {
		  cout << ", ";
		}
      }
      cout << "}" << endl;
    }
    
  } // Close loop over timestamps
  
  cout << endl << "Length of final adc_t array: " << masterVector.size() << endl;

  // Print contents of final adc_t array:
  if(verbose) {
    cout << "Contents of final adc_t array: " << endl;
    for(size_t i=0; i<masterVector.size(); i++) {
      bitset<16> curEntry = masterVector[i];
      cout << "\t" << curEntry.to_string() << " " << curEntry.to_ulong() << endl;
      if(i>0 && (i+1)%4==0) {
		cout << endl;
      }
    }
  }

  // Save contents of final adc_t array to binary file
  if(save_adc_values) {
    for(size_t i = 0; i<masterVector.size(); i++) {
      binFile.write(reinterpret_cast<const char *>(&(masterVector[i])),sizeof(adc_t));
    }
    binFile.close();
  }
  
  return 0;
}

double f(double t, double tau, double sigma, double offset) {  
  double E = 2.718281828459045;
  double retval = (pow(E,(pow(sigma,2) + 2*offset*tau - 2*t*tau)/(2.*pow(tau,2)))*(-pow(sigma,2) + (-offset + t)*tau))/pow(tau,3);
  if(retval<0) {
    retval = 0;
  }
  return retval;
}

double logn(double x, double eta, double sigma, double Epeak, double norm){  
  double Aterm;
  double logterms0,s0;
  double logn,logterm;
  double expterm;
  double pigreco=3.14159265;

  //double f = 2.35;
  double f = 20.0;
  
  logterms0 = eta*f/2+sqrt(1+pow((eta*f/2),2));
  s0 = (2/f)*log(logterms0);
    
  Aterm = eta/(sqrt(2*pigreco)*sigma*s0);
  
  logterm = 1-(eta/sigma)*(x-Epeak);  
      
  if(logterm<0){
    logterm = 0.0001;
  }
  expterm = log(logterm)/s0;
  expterm = -0.5*pow(expterm,2);
  
  logn = norm*Aterm *exp(expterm);      
  return logn;
}


vector<adc_t> generateDMABlockHeader(size_t theCount) {  
  bitset<64> byteCount = theCount;
  bitset<16> byteCount0 = 0;
  bitset<16> byteCount1 = 0;
  bitset<16> byteCount2 = 0;
  bitset<16> byteCount3 = 0;
  for(int i=0; i<16; i++) {
	byteCount0[i] = byteCount[i+16*0];
	byteCount1[i] = byteCount[i+16*1];
	byteCount2[i] = byteCount[i+16*2];
	byteCount3[i] = byteCount[i+16*3];
  }
  vector<adc_t> header;
  header.push_back((adc_t) byteCount0.to_ulong());
  header.push_back((adc_t) byteCount1.to_ulong());
  header.push_back((adc_t) byteCount2.to_ulong());
  header.push_back((adc_t) byteCount3.to_ulong());

  return header;
}
