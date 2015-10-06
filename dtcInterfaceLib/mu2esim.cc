// This file (mu2edev.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

/*
 *    make mu2edev.o CFLAGS='-g -Wall -std=c++0x'
 */

#define TRACE_NAME "MU2EDEV"
#ifndef _WIN32
# include <trace.h>
#else
# ifndef TRACE
#  include <stdio.h>
#  ifdef _DEBUG
#   define TRACE(lvl,...) printf(__VA_ARGS__); printf("\n")
#  else
#   define TRACE(...)
#  endif
# endif
# pragma warning(disable: 4351 6385 6386)
#endif
#include "mu2esim.hh"
#include <ctime>
#include <vector>
#include <forward_list>
#include <iostream>
#include <algorithm>
#include <cmath>

mu2esim::mu2esim()
	: hwIdx_()
	, swIdx_()
	, dmaData_()
	, loopbackData_()
	, mode_(DTCLib::DTC_SimMode_Disabled)
	, cancelCFO_(true)
{
#ifndef _WIN32
	//TRACE_CNTL( "lvlmskM", 0x3 );
	//TRACE_CNTL( "lvlmskS", 0x3 );
#endif
	hwIdx_[0] = 0;
	hwIdx_[1] = 0;
	swIdx_[0] = 0;
	swIdx_[1] = 0;
	for (unsigned ii = 0; ii < SIM_BUFFCOUNT; ++ii)
	{
		dmaData_[0][ii] = (mu2e_databuff_t*)new mu2e_databuff_t();
		dmaData_[1][ii] = (mu2e_databuff_t*)new mu2e_databuff_t();
		buffSize_[0][ii] = 0;
		buffSize_[1][ii] = 0;
	}
	release_all(0);
	release_all(1);
	for (int ring = 0; ring < 6; ++ring)
	{
		for (int roc = 0; roc < 6; ++roc)
		{
			dcsRequestReceived_[ring][roc] = false;
			simIndex_[ring][roc] = 0;
			dcsRequest_[ring][roc] = DTCLib::DTC_DCSRequestPacket((DTCLib::DTC_Ring_ID)ring, (DTCLib::DTC_ROC_ID)roc);
		}
	}
}

mu2esim::~mu2esim()
{
	for (unsigned ii = 0; ii < MU2E_MAX_CHANNELS; ++ii)
	{
		for (unsigned jj = 0; jj < SIM_BUFFCOUNT; ++jj)
		{
			delete[] dmaData_[ii][jj];
		}
	}
}

int mu2esim::init(DTCLib::DTC_SimMode mode)
{
	TRACE(17, "mu2e Simulator::init");
	mode_ = mode;

	TRACE(17, "mu2esim::init Initializing registers");
	// Set initial register values...
	registers_[0x9000] = 0x00006363; // v99.99
	registers_[0x9004] = 0x53494D44; // SIMD in ASCII
	registers_[0x900C] = 0x00000010; // Send
	registers_[0x9010] = 0x00000040; // Recieve
	registers_[0x9014] = 0x00000100; // SPayload
	registers_[0x9018] = 0x00000400; // RPayload
	registers_[0x9100] = 0x00000003; // System Clock, Timing Enable
	registers_[0x9104] = 0x80000010; //Default value from HWUG
	registers_[0x9108] = 0x00000000; // SERDES Loopback Disabled
	registers_[0x910C] = 0x2; // Initialization Complete, no IIC Error
	registers_[0x9110] = 0x3F;        // ROC Emulators enabled (of course!)
	registers_[0x9114] = 0x3F3F;       // All rings Tx/Rx enabled, CFO and timing disabled
	registers_[0x9118] = 0x0;        // No SERDES Reset
	registers_[0x911C] = 0x0;        // No SERDES Disparity Error
	registers_[0x9120] = 0x0;        // No SERDES CNIT Error
	registers_[0x9124] = 0x0;        // No SERDES Unlock Error
	registers_[0x9128] = 0x7F;       // SERDES PLL Locked
	registers_[0x912C] = 0x0;        // SERDES TX Buffer Status Normal
	registers_[0x9130] = 0x0;        // SERDES RX Buffer Staus Nominal
	registers_[0x9134] = 0x0;        // SERDES RX Status Nominal
	registers_[0x9138] = 0x7F;       // SERDES Resets Done
	registers_[0x913C] = 0x0;        // No Eyescan Error
	registers_[0x9140] = 0x7F;       // RX CDR Locked
	registers_[0x9144] = 0x800;      // DMA Timeout Preset
	registers_[0x9148] = 0x200000;   // ROC Timeout Preset
	registers_[0x914C] = 0x0;        // ROC Timeout Error
	registers_[0x9180] = 0x0;        // Timestamp preset to 0
	registers_[0x9184] = 0x0;
	registers_[0x9188] = 0x00002000; // Data pending timeout preset
	registers_[0x918C] = 0x1;          // NUMROCs 0 for all rings,except Ring 0 which has 1
	registers_[0x9190] = 0x0;  // NO FIFO Full flags
	registers_[0x9194] = 0x0;
	registers_[0x9198] = 0x0;
	registers_[0x919C] = 0x0;        // Receive Packet Error
	registers_[0x91A0] = 0x0;        // CFO Emulation Registers
	registers_[0x91A4] = 0x0;
	registers_[0x91A8] = 0x0;
	registers_[0x91AC] = 0x0;
	registers_[0x91B0] = 0x0;
	registers_[0x9204] = 0x0010;     // Packet Size Bytes
	registers_[0x91A4] = 0x1;        // FPGA PROM Ready
	registers_[0x9404] = 0x1;
	registers_[0x9408] = 0x0;        // FPGA Core Access OK

	TRACE(17, "mu2esim::init finished");
	return (0);
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
   */
int mu2esim::read_data(int chn, void **buffer, int tmo_ms)
{
	auto start = std::chrono::high_resolution_clock::now();
	if (delta_(chn, C2S) == 0)
	{
		clearBuffer_(chn, false);

		uint64_t currentOffset = 8;
		buffSize_[chn][swIdx_[chn]] = currentOffset;

		if (chn == 0)
		{
			if (!loopbackData_.empty())
			{
				memcpy((char*)dmaData_[chn][swIdx_[chn]], loopbackData_.front(), sizeof(mu2e_databuff_t) - sizeof(uint64_t));
				*buffer = dmaData_[chn][swIdx_[chn]];
				delete loopbackData_.front();
				loopbackData_.pop();
				swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;
				return static_cast<int>(*(uint64_t*)buffer[0]);
			}
			std::set<uint64_t> activeTimestamps;
			rrMutex_.lock();
			for (unsigned ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
			{
				//TRACE(21, "mu2esim::read_data ring=%u RR list size=%llu p=%p this=%p", ring, (unsigned long long)readoutRequestReceived_[ring].size(), (void*)&readoutRequestReceived_, (void*)this);
				if (readoutRequestReceived_[ring].size() > 0)
				{
					bool active = true;
					auto ii = readoutRequestReceived_[ring].begin();
					while (active && ii != readoutRequestReceived_[ring].end())
					{
						bool found = false;
						uint64_t ts = *ii;
						//TRACE(21, "mu2esim::read_data checking if there are DataRequests for ts=%llu", (unsigned long long)ts);
						for (auto roc : DTCLib::DTC_ROCS)
						{
							drMutex_.lock();
							if (dataRequestReceived_[ring][roc].count(ts))
							{
								drMutex_.unlock();
								activeTimestamps.insert(ts);
								found = true;
								break;
							}
							drMutex_.unlock();
						}
						if (!found) active = false;
						++ii;
					}
				}
			}
			rrMutex_.unlock();

			bool exitLoop = false;
			for (auto ts : activeTimestamps)
			{
				TRACE(17, "mu2esim::read_data, checking timestamp %llu", (unsigned long long)ts);
				if (exitLoop) break;
				for (int ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
				{
					if (exitLoop) break;
					rrMutex_.lock();
					if (readoutRequestReceived_[ring].count(ts) > 0 && currentOffset < sizeof(mu2e_databuff_t))
					{
						for (int roc = 0; roc <= DTCLib::DTC_ROC_5; ++roc)
						{
							drMutex_.lock();
							if (dataRequestReceived_[ring][roc].count(ts) > 0 && currentOffset < sizeof(mu2e_databuff_t))
							{
								TRACE(17, "mu2esim::read_data, DAQ Channel w/Requests recieved");
								uint8_t packet[16];

								int nSamples = rand() % 10 + 10;
								uint16_t nPackets = 1;
								if (mode_ == DTCLib::DTC_SimMode_Calorimeter)
								{
									if (nSamples <= 5) { nPackets = 1; }
									else { nPackets = static_cast<uint16_t>(floor((nSamples - 6) / 8 + 2)); }
								}
								else if (mode_ == DTCLib::DTC_SimMode_Performance)
								{
									nPackets = dataRequestReceived_[ring][roc][ts];
									// Safety Check
									if ((uint32_t)((nPackets + 1) * 16) >= sizeof(mu2e_databuff_t))
									{
										nPackets = (sizeof(mu2e_databuff_t) / 16) - 3;
									}
								}
								if ((currentOffset + (nPackets + 1) * 16) > sizeof(mu2e_databuff_t))
								{
									exitLoop = true;
									drMutex_.unlock();
									break;
								}

								// Record the DataBlock size
								uint16_t dataBlockByteCount = static_cast<uint16_t>((nPackets + 1) * 16);
								TRACE(17, "mu2esim::read_data DataBlock size is %u", dataBlockByteCount);

								// Add a Data Header packet to the reply
								packet[0] = static_cast<uint8_t>(dataBlockByteCount & 0xFF);
								packet[1] = static_cast<uint8_t>(dataBlockByteCount >> 8);
								packet[2] = 0x50 + (roc & 0x0F);
								packet[3] = 0x80 + (ring & 0x0F);
								packet[4] = static_cast<uint8_t>(nPackets & 0xFF);
								packet[5] = static_cast<uint8_t>(nPackets >> 8);
								DTCLib::DTC_Timestamp dts(ts);
								dts.GetTimestamp(packet, 6);
								packet[12] = 0;
								packet[13] = 0;
								packet[14] = 0;
								packet[15] = 0;

								TRACE(17, "mu2esim::read_data Copying Data Header packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
									  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
								memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet[0], sizeof(packet));
								currentOffset += sizeof(packet);
								buffSize_[chn][hwIdx_[chn]] = currentOffset;

								switch (mode_)
								{
								case DTCLib::DTC_SimMode_CosmicVeto:
								{
									nSamples = 4;
									packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
									packet[1] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
									packet[2] = 0x0; // No TDC value!
									packet[3] = 0x0;
									packet[4] = static_cast<uint8_t>(nSamples);
									packet[5] = static_cast<uint8_t>(nSamples >> 8);

									packet[6] = 0;
									packet[7] = 0;
									packet[8] = static_cast<uint8_t>(simIndex_[ring][roc]);
									packet[9] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
									packet[10] = 2;
									packet[11] = 2;
									packet[12] = static_cast<uint8_t>(3 * simIndex_[ring][roc]);
									packet[13] = static_cast<uint8_t>((3 * simIndex_[ring][roc]) >> 8);
									packet[14] = 0;
									packet[15] = 0;

									TRACE(17, "mu2esim::read_data Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
										  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
									memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet, sizeof(packet));
									currentOffset += sizeof(packet);
									buffSize_[chn][hwIdx_[chn]] = currentOffset;
								}
								break;
								case DTCLib::DTC_SimMode_Calorimeter:
								{
									packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
									packet[1] = ((simIndex_[ring][roc] >> 8) & 0xF) + ((simIndex_[ring][roc] & 0xF) << 4);
									packet[2] = 0x0; // No TDC value!
									packet[3] = 0x0;
									packet[4] = static_cast<uint8_t>(nSamples);
									packet[5] = static_cast<uint8_t>(nSamples >> 8);

									packet[6] = 0;
									packet[7] = 0;
									packet[8] = static_cast<uint8_t>(simIndex_[ring][roc]);
									packet[9] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
									packet[10] = 2;
									packet[11] = 2;
									packet[12] = static_cast<uint8_t>(3 * simIndex_[ring][roc]);
									packet[13] = static_cast<uint8_t>((3 * simIndex_[ring][roc]) >> 8);
									packet[14] = 4;
									packet[15] = 4;

									TRACE(17, "mu2esim::read_data Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
										  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
									memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet, sizeof(packet));
									currentOffset += sizeof(packet);
									buffSize_[chn][hwIdx_[chn]] = currentOffset;

									int samplesProcessed = 5;
									for (int i = 1; i < nPackets; ++i)
									{
										packet[0] = static_cast<uint8_t>(samplesProcessed * simIndex_[ring][roc]);
										packet[1] = static_cast<uint8_t>((samplesProcessed * simIndex_[ring][roc]) >> 8);
										packet[2] = static_cast<uint8_t>(samplesProcessed + 1);
										packet[3] = static_cast<uint8_t>(samplesProcessed + 1);
										packet[4] = static_cast<uint8_t>((2 + samplesProcessed) * simIndex_[ring][roc]);
										packet[5] = static_cast<uint8_t>(((2 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
										packet[6] = static_cast<uint8_t>(samplesProcessed + 3);
										packet[7] = static_cast<uint8_t>(samplesProcessed + 3);
										packet[8] = static_cast<uint8_t>((4 + samplesProcessed) * simIndex_[ring][roc]);
										packet[9] = static_cast<uint8_t>(((4 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
										packet[10] = static_cast<uint8_t>(samplesProcessed + 5);
										packet[11] = static_cast<uint8_t>(samplesProcessed + 5);
										packet[12] = static_cast<uint8_t>((6 + samplesProcessed) * simIndex_[ring][roc]);
										packet[13] = static_cast<uint8_t>(((6 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
										packet[14] = static_cast<uint8_t>(samplesProcessed + 7);
										packet[15] = static_cast<uint8_t>(samplesProcessed + 7);

										samplesProcessed += 8;
										TRACE(17, "mu2esim::read_data Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
											  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
										memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet, sizeof(packet));
										currentOffset += sizeof(packet);
										buffSize_[chn][hwIdx_[chn]] = currentOffset;
									}
								}
								break;
								case DTCLib::DTC_SimMode_Tracker:
								{
									packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
									packet[1] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);

									packet[2] = 0x0; // No TDC value!
									packet[3] = 0x0;
									packet[4] = 0x0;
									packet[5] = 0x0;

									uint16_t pattern0 = 0;
									uint16_t pattern1 = simIndex_[ring][roc];
									uint16_t pattern2 = 2;
									uint16_t pattern3 = (simIndex_[ring][roc] * 3) % 0x3FF;
									uint16_t pattern4 = 4;
									uint16_t pattern5 = (simIndex_[ring][roc] * 5) % 0x3FF;
									uint16_t pattern6 = 6;
									uint16_t pattern7 = (simIndex_[ring][roc] * 7) % 0x3FF;

									packet[6] = static_cast<uint8_t>(pattern0);
									packet[7] = static_cast<uint8_t>((pattern0 >> 8) + (pattern1 << 2));
									packet[8] = static_cast<uint8_t>((pattern1 >> 6) + (pattern2 << 4));
									packet[9] = static_cast<uint8_t>((pattern2 >> 4) + (pattern3 << 6));
									packet[10] = static_cast<uint8_t>((pattern3 >> 2));
									packet[11] = static_cast<uint8_t>(pattern4);
									packet[12] = static_cast<uint8_t>((pattern4 >> 8) + (pattern5 << 2));
									packet[13] = static_cast<uint8_t>((pattern5 >> 6) + (pattern6 << 4));
									packet[14] = static_cast<uint8_t>((pattern6 >> 4) + (pattern7 << 6));
									packet[15] = static_cast<uint8_t>((pattern7 >> 2));

									TRACE(17, "mu2esim::read_data Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
										  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
									memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet, sizeof(packet));
									currentOffset += sizeof(packet);
									buffSize_[chn][hwIdx_[chn]] = currentOffset;
								}
								break;
								case DTCLib::DTC_SimMode_Performance:
									for (uint16_t ii = 0; ii < nPackets; ++ii)
									{
										packet[0] = static_cast<uint8_t>(ii);
										packet[1] = 0x11;
										packet[2] = 0x22;
										packet[3] = 0x33;
										packet[4] = 0x44;
										packet[5] = 0x55;
										packet[6] = 0x66;
										packet[7] = 0x77;
										packet[8] = 0x88;
										packet[9] = 0x99;
										packet[10] = 0xaa;
										packet[11] = 0xbb;
										packet[12] = 0xcc;
										packet[13] = 0xdd;
										packet[14] = 0xee;
										packet[15] = 0xff;

										TRACE(17, "mu2esim::read_data Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
											  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)packet, (unsigned long long)currentOffset);
										memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &packet, sizeof(packet));
										currentOffset += sizeof(packet);
										buffSize_[chn][hwIdx_[chn]] = currentOffset;
									}
									break;
								case DTCLib::DTC_SimMode_Disabled:
								default:
									break;
								}
								simIndex_[ring][roc] = (simIndex_[ring][roc] + 1) % 0x3FF;
								TRACE(17, "mu2esim::read_data: Erasing DTC_Timestamp %llu from DataRequestReceived list", (unsigned long long)ts);
								dataRequestReceived_[ring][roc].erase(ts);
							}
							drMutex_.unlock();
						}
						if (exitLoop)
						{
							rrMutex_.unlock();
							break;
						}
						TRACE(17, "mu2esim::read_data: Erasing DTC_Timestamp %llu from ReadoutRequestReceived list", (unsigned long long)ts);
						readoutRequestReceived_[ring].erase(ts);
					}
					rrMutex_.unlock();
				}
			}
		}
		else if (chn == 1)
		{
			bool exitLoop = false;
			for (int ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
			{
				if (exitLoop) break;
				for (int roc = 0; roc <= DTCLib::DTC_ROC_5; ++roc)
				{
					if (dcsRequestReceived_[ring][roc])
					{
						if (currentOffset + 16 >= sizeof(mu2e_databuff_t))
						{
							exitLoop = true;
							break;
						}
						TRACE(17, "mu2esim::read_data DCS Request Recieved, Sending Response");
						uint8_t replyPacket[16];
						replyPacket[0] = 16;
						replyPacket[1] = 0;
						replyPacket[2] = 0x40;
						replyPacket[3] = (ring & 0x0F) + 0x80;
						replyPacket[4] = (uint8_t)dcsRequest_[ring][roc].GetType();
						replyPacket[5] = 0x3E;
						replyPacket[6] = dcsRequest_[ring][roc].GetAddress();
						replyPacket[7] = 0x8;
						replyPacket[8] = 0;
						replyPacket[9] = 0;
						replyPacket[10] = dcsRequest_[ring][roc].GetData() & 0xFF;
						replyPacket[11] = (dcsRequest_[ring][roc].GetData() & 0xFF00) >> 8;
						for (int i = 12; i < 16; ++i)
						{
							replyPacket[i] = 0;
						}

						TRACE(17, "mu2esim::read_data Copying DCS Reply packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
							  , chn, hwIdx_[chn], (void*)dmaData_[chn][hwIdx_[chn]], (void*)replyPacket, (unsigned long long)currentOffset);
						memcpy((char*)dmaData_[chn][hwIdx_[chn]] + currentOffset, &replyPacket, sizeof(replyPacket));
						currentOffset += sizeof(replyPacket);
						buffSize_[chn][hwIdx_[chn]] = currentOffset;
						dcsRequestReceived_[ring][roc] = false;
					}
				}
			}
		}
	}

	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::read_data took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	TRACE(17, "mu2esim::read_data Setting output buffer to dmaData_[%i][%u]=%p, retsts=%llu", chn, swIdx_[chn], (void*)dmaData_[chn][swIdx_[chn]], (unsigned long long)buffSize_[chn][swIdx_[chn]]);
	uint64_t bytesReturned = buffSize_[chn][swIdx_[chn]];
	memcpy(dmaData_[chn][swIdx_[chn]], (uint64_t*)&bytesReturned, sizeof(uint64_t));
	*buffer = dmaData_[chn][swIdx_[chn]];
	swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;

	return static_cast<int>(bytesReturned);
}

int mu2esim::write_data(int chn, void *buffer, size_t bytes)
{
	TRACE(17, "mu2esim::write_data start: chn=%i, buf=%p, bytes=%llu", chn, buffer, (unsigned long long)bytes);
	uint32_t worda;
	memcpy(&worda, buffer, sizeof(worda));
	uint16_t word = static_cast<uint16_t>(worda >> 16);
	TRACE(17, "mu2esim::write_data worda is 0x%x and word is 0x%x", worda, word);

	switch (chn)
	{
	case 0: // DAQ Channel
	{
		DTCLib::DTC_Ring_ID activeDAQRing = static_cast<DTCLib::DTC_Ring_ID>((word & 0x0F00) >> 8);
		if (registers_[0x9108] != 0)
		{
			TRACE(17, "mu2esim::write_data: loopback mode is %u", registers_[0x9108]);
			if ((registers_[0x9108] & (0x7 << activeDAQRing * 3)) != 0)
			{
				TRACE(17, "mu2esim::write_data: adding buffer to loopback queue");
				if (bytes <= sizeof(mu2e_databuff_t) - sizeof(uint64_t))
				{
					uint64_t bufSize = bytes + 8;
					mu2e_databuff_t* lpBuf((mu2e_databuff_t*)new mu2e_databuff_t());
					memcpy(lpBuf, (uint64_t*)&bufSize, sizeof(uint64_t));
					memcpy((char*)lpBuf + 8, buffer, bytes*sizeof(uint8_t));
					loopbackData_.push(lpBuf);
					return 0;
				}
			}
		}
		DTCLib::DTC_Timestamp ts((uint8_t*)buffer + 6);
		if ((word & 0x8010) == 0x8010)
		{
			TRACE(17, "mu2esim::write_data: Readout Request: activeDAQRing=%u, ts=%llu", activeDAQRing, (unsigned long long)ts.GetTimestamp(true));
			rrMutex_.lock();
			readoutRequestReceived_[activeDAQRing].insert(ts.GetTimestamp(true));
			rrMutex_.unlock();
		}
		else if ((word & 0x8020) == 0x8020)
		{
			DTCLib::DTC_ROC_ID activeROC = static_cast<DTCLib::DTC_ROC_ID>(word & 0xF);
			TRACE(17, "mu2esim::write_data: Data Request: activeDAQRing=%u, activeROC=%u, ts=%llu", activeDAQRing, activeROC, (unsigned long long)ts.GetTimestamp(true));
			if (activeDAQRing != DTCLib::DTC_Ring_Unused)
			{
				rrMutex_.lock();
				if (readoutRequestReceived_[activeDAQRing].count(ts.GetTimestamp(true)) == 0)
				{
					TRACE(17, "mu2esim::write_data: Data Request Received but missing Readout Request!");
				}
				rrMutex_.unlock();
				drMutex_.lock();
				if (activeROC < DTCLib::DTC_ROC_Unused)
				{
					uint16_t packetCount = *((uint16_t*)buffer + 7);
					dataRequestReceived_[activeDAQRing][activeROC][ts.GetTimestamp(true)] = packetCount;
				}
				drMutex_.unlock();
			}
		}
		break;
	}
	case 1:
	{
		if ((word & 0x0080) == 0)
		{
			DTCLib::DTC_Ring_ID activeDCSRing = static_cast<DTCLib::DTC_Ring_ID>((word & 0x0F00) >> 8);
			DTCLib::DTC_ROC_ID activeDCSROC = static_cast<DTCLib::DTC_ROC_ID>(word & 0xF);
			TRACE(17, "mu2esim::write_data activeDCSRing is %u, roc is %u", activeDCSRing, activeDCSROC);
			if (activeDCSRing != DTCLib::DTC_Ring_Unused && activeDCSROC != DTCLib::DTC_ROC_Unused)
			{
				DTCLib::DTC_DataPacket packet(buffer);
				DTCLib::DTC_DCSRequestPacket thisPacket(packet);
				if (thisPacket.GetType() == DTCLib::DTC_DCSOperationType_Read ||
					thisPacket.GetType() == DTCLib::DTC_DCSOperationType_WriteWithAck)
				{
					dcsRequestReceived_[activeDCSRing][activeDCSROC] = true;
					dcsRequest_[activeDCSRing][activeDCSROC] = std::move(thisPacket);
					TRACE(17, "mu2esim::write_data: Recieved DCS Request:");
					TRACE(17, dcsRequest_[activeDCSRing][activeDCSROC].toJSON().c_str());
				}
			}
		}
		break;
	}
	}

	return 0;
}

int mu2esim::read_release(int chn, unsigned num)
{
	//Always succeeds
	TRACE(17, "mu2esim::read_release: Simulating a release of %u buffers of channel %i", num, chn);
	for (unsigned ii = 0; ii < num; ++ii)
	{
		if (delta_(chn, C2S) != 0) swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;
	}
	return 0;
}

int mu2esim::release_all(int chn)
{
	read_release(chn, SIM_BUFFCOUNT);
	hwIdx_[chn] = 0;
	swIdx_[chn] = 0;
	return 0;
}

int  mu2esim::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
	auto start = std::chrono::high_resolution_clock::now();
	*output = 0;
	if (registers_.count(address) > 0)
	{
		TRACE(17, "mu2esim::read_register: Returning value 0x%x for address 0x%x", registers_[address], address);
		*output = registers_[address];
		return 0;
	}
	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::read_register took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	return 1;
}

int  mu2esim::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	auto start = std::chrono::high_resolution_clock::now();
	// Write the register!!!
	TRACE(17, "mu2esim::write_register: Writing value 0x%x into address 0x%x", data, address);
	registers_[address] = data;
	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::write_register took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	if (address == 0x9100) // DTC Control
	{
		std::bitset<32> dataBS(data);
		if (dataBS[30] == 1)
		{
			TRACE(19, "mu2esim::write_register: CFO Emulator Enable Detected!");
			cancelCFO_ = true;
			if (cfoEmulatorThread_.joinable()) cfoEmulatorThread_.join();
			cancelCFO_ = false;
			cfoEmulatorThread_ = std::thread(&mu2esim::CFOEmulator_, this);
		}
	}
	return 0;
}

void mu2esim::CFOEmulator_()
{
	DTCLib::DTC_Timestamp start(registers_[0x91A0], static_cast<uint16_t>(registers_[0x91A4]));
	uint32_t count = registers_[0x91AC];
	uint16_t debugCount = static_cast<uint16_t>(registers_[0x91B0]);
	long long ticksToWait = static_cast<long long>(registers_[0x91A8] * 0.0064);
	TRACE(19, "mu2esim::CFOEmulator_ start timestamp=%llu, count=%lu, delayBetween=%lli", (unsigned long long)start.GetTimestamp(true), (unsigned long)count, (long long)ticksToWait);
	DTCLib::DTC_ROC_ID numROCS[6]{ DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused,
								   DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused,
								   DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused };
	for (auto ring : DTCLib::DTC_Rings)
	{
		std::bitset<32> ringRocs(registers_[0x918C]);
		int number = ringRocs[ring * 3] + (ringRocs[ring * 3 + 1] << 1) + (ringRocs[ring * 3 + 2] << 2);
		numROCS[ring] = DTCLib::DTC_ROCS[number];
		TRACE(19, "mu2esim::CFOEmulator_ ringRocs[%u]=%u", ring, numROCS[ring]);
	}
	unsigned sentCount = 0;
	while (sentCount < count && !cancelCFO_)
	{
		for (auto ring : DTCLib::DTC_Rings)
		{
			if (numROCS[ring] != DTCLib::DTC_ROC_Unused)
			{
				TRACE(21, "mu2esim::CFOEmulator_ writing DTC_ReadoutRequestPacket to ring=%u for timestamp=%llu", ring, (unsigned long long)(start + sentCount).GetTimestamp(true));
				DTCLib::DTC_ReadoutRequestPacket packet(ring, start + sentCount, numROCS[ring], true);
				DTCLib::DTC_DataPacket thisPacket = packet.ConvertToDataPacket();
				write_data(0, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
			}
		}
		sentCount++;
	}
	sentCount = 0;
	while (sentCount < count && !cancelCFO_)
	{
		for (auto ring : DTCLib::DTC_Rings)
		{
			if (numROCS[ring] != DTCLib::DTC_ROC_Unused)
			{
				for (uint8_t roc = 0; roc <= numROCS[ring]; ++roc)
				{
					TRACE(21, "mu2esim::CFOEmulator_ writing DTC_DataRequestPacket to ring=%u, roc=%u, for timestamp=%llu", ring, roc, (unsigned long long)(start + sentCount).GetTimestamp(true));
					DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, start + sentCount, true,
													  (uint16_t)debugCount);
					DTCLib::DTC_DataPacket thisPacket = req.ConvertToDataPacket();
					write_data(0, thisPacket.GetData(), thisPacket.GetSize() * sizeof(uint8_t));
				}
			}
		}
		usleep(ticksToWait);
		sentCount++;
	}
}

unsigned mu2esim::delta_(int chn, int dir)
{
	unsigned hw = hwIdx_[chn];
	unsigned sw = swIdx_[chn];
	TRACE(21, "mu2esim::delta_ chn=%i dir=%i hw=%u sw=%u num_buffs=%u"
		  , chn, dir, hw, sw, SIM_BUFFCOUNT);
	if (dir == C2S)
		return ((hw >= sw)
				? hw - sw
				: SIM_BUFFCOUNT + hw - sw);
	else
		return ((sw >= hw)
				? SIM_BUFFCOUNT - (sw - hw)
				: hw - sw);
}

void mu2esim::clearBuffer_(int chn, bool increment)
{
	// Clear the buffer:
	TRACE(17, "mu2esim::clearBuffer_: Clearing output buffer");
	if (increment)
	{
		hwIdx_[chn] = (hwIdx_[chn] + 1) % SIM_BUFFCOUNT;
	}
	memset(dmaData_[chn][hwIdx_[chn]], 0, sizeof(mu2e_databuff_t));
}
