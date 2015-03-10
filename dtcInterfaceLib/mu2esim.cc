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
#include <trace.h>
#endif
#include "mu2esim.hh"
#include <ctime>
#include <vector>
#include <iostream>

mu2esim::mu2esim() : isActive_(false), simIndex_(),
dcsRequestRecieved_(false), readoutRequestRecieved_(), dataRequestRecieved_(false),
activeDAQRing_(DTC::DTC_Ring_Unused),
activeDCSRing_(DTC::DTC_Ring_Unused),
dcsRequest_(DTC::DTC_Ring_Unused, DTC::DTC_ROC_Unused)
{
#ifndef _WIN32  
    //	TRACE_CNTL( "lvlmskM", 0x3 );
    //	TRACE_CNTL( "lvlmskS", 0x3 );
#endif
	for (int i = 0; i < 6; ++i)
	{
		readoutRequestRecieved_[i] = false;
	}
}

int mu2esim::init()
{
	// For now, this is the only mode implemented...
	mode_ = mu2e_sim_mode_tracker;

	isActive_ = true;
	// Set initial register values...
	registers_[0x9000] = 0x53494D44; // SIMD in ASCII
	registers_[0x9100] = 0x40000000; // Clear latched errors on
	registers_[0x9108] = 0x3F;       // SERDES Loopback enabled
	registers_[0x9110] = 0x1;        // ROC Emulator enabled (of course!)
	registers_[0x9114] = 0x3F;       // ALl rings enabled
	registers_[0x9118] = 0x0;        // No SERDES Reset
	registers_[0x911C] = 0x0;        // No SERDES Disparity Error
	registers_[0x9120] = 0x0;        // No SERDES CNIT Error
	registers_[0x9124] = 0x0;        // No SERDES Unlock Error
	registers_[0x9128] = 0x3F;       // SERDES PLL Locked
	registers_[0x912C] = 0x0;        // SERDES TX Buffer Status Normal
	registers_[0x9130] = 0x0;        // SERDES RX Buffer Staus Nominal
	registers_[0x9138] = 0x3F;       // SERDES Resets Done
	registers_[0x9180] = 0x0;        // Timestamp preset to 0
	registers_[0x9184] = 0x0;
	registers_[0x91A4] = 0x1;        // FPGA PROM Ready

	// Set DMA State
	dmaState_[0][0].BDerrs = 0;
	dmaState_[0][0].BDSerrs = 0;
	dmaState_[0][0].BDs = 399;
	dmaState_[0][0].Buffers = 4;
	dmaState_[0][0].Engine = 0;
	dmaState_[0][0].IntEnab = 0;
	dmaState_[0][0].MaxPktSize = 0x100000;
	dmaState_[0][0].MinPktSize = 0x40;
	dmaState_[0][0].TestMode = 0;

	dmaState_[0][1].BDerrs = 0;
	dmaState_[0][1].BDSerrs = 0;
	dmaState_[0][1].BDs = 399;
	dmaState_[0][1].Buffers = 4;
	dmaState_[0][1].Engine = 0x20;
	dmaState_[0][1].IntEnab = 0;
	dmaState_[0][1].MaxPktSize = 0x100000;
	dmaState_[0][1].MinPktSize = 0x40;
	dmaState_[0][1].TestMode = 0;

	dmaState_[1][0].BDerrs = 0;
	dmaState_[1][0].BDSerrs = 0;
	dmaState_[1][0].BDs = 399;
	dmaState_[1][0].Buffers = 4;
	dmaState_[1][0].Engine = 1;
	dmaState_[1][0].IntEnab = 0;
	dmaState_[1][0].MaxPktSize = 0x100000;
	dmaState_[1][0].MinPktSize = 0x40;
	dmaState_[1][0].TestMode = 0;

	dmaState_[1][1].BDerrs = 0;
	dmaState_[1][1].BDSerrs = 0;
	dmaState_[1][1].BDs = 399;
	dmaState_[1][1].Buffers = 4;
	dmaState_[1][1].Engine = 0x21;
	dmaState_[1][1].IntEnab = 0;
	dmaState_[1][1].MaxPktSize = 0x100000;
	dmaState_[1][1].MinPktSize = 0x40;
	dmaState_[1][1].TestMode = 0;

	// Set PCIe State
	pcieState_.VendorId = 4334;
	pcieState_.DeviceId = 28738;
	pcieState_.LinkState = true;
	pcieState_.LinkSpeed = 5;
	pcieState_.LinkWidth = 4;
	pcieState_.IntMode = 0;
	pcieState_.MPS = 256;
	pcieState_.MRRS = 512;
	pcieState_.Version = 0x53494D44;
	pcieState_.InitFCCplD = 0;
	pcieState_.InitFCCplH = 0;
	pcieState_.InitFCNPD = 16;
	pcieState_.InitFCNPH = 124;
	pcieState_.InitFCPD = 552;
	pcieState_.InitFCPH = 112;

	// Test State
	testStarted_ = false;
	testState_.Engine = 0;
	testState_.TestMode = 0;

	return (0);
}

int mu2esim::read_data(int chn, void **buffer, int tmo_ms)
{
	// Clear the buffer:
	memset(&dmaData_, 0, sizeof(dmaData_));

	if (readoutRequestRecieved_ && dataRequestRecieved_ && chn == 0)
	{
		uint16_t packet[8];

		// Add a Data Header packet to the reply
		packet[0] = ((activeDAQRing_ & 0x0F) << 8) + 0x50;
		packet[1] = 0x0001;
		time_t currentTime = time(0); // On Windows, at least, this is 64-bits. We'll drop the first 16.
		memcpy(&packet[2], &currentTime, 3 * sizeof(packet[2]));
		packet[5] = 0x0001;
		packet[6] = 0x0;
		packet[7] = 0x0;
		memcpy(&dmaData_, &packet[0], sizeof(packet));

		switch (mode_)
		{
		case mu2e_sim_mode_tracker:
		default:
			packet[0] = simIndex_[activeDAQRing_];
			packet[1] = 0x0; // No TDC value!
			packet[2] = 0x0;
			packet[3] = ((simIndex_[activeDAQRing_] & 0x3F) << 10) + 0x0;
			packet[4] = (((simIndex_[activeDAQRing_] * 3) & 0x3) << 14) + (0x2 << 4) + ((simIndex_[activeDAQRing_] << 6) & 0xF);
			packet[5] = (0x4 << 8) + ((simIndex_[activeDAQRing_] * 3) & 0xFF);
			packet[6] = (0x6 << 12) + (((simIndex_[activeDAQRing_] * 5) & 0x3FF) << 2);
			packet[7] = ((simIndex_[activeDAQRing_] * 7) & 0x3FF) << 6;
			memcpy(((char*)&dmaData_ + sizeof(packet)), &packet, sizeof(packet));
			break;
		}
		simIndex_[activeDAQRing_] = (simIndex_[activeDAQRing_] + 1) % 0x3FF;
		dataRequestRecieved_ = false;
	}
	else if (dcsRequestRecieved_ && chn == 1)
	{
		uint16_t replyPacket[8];
		replyPacket[0] = ((activeDCSRing_ & 0x0F) << 8) + 0x40;
		replyPacket[1] = 0x0;
		for (int i = 2; i < 8; ++i)
		{
			int j = (i - 2) * 2;
			replyPacket[i] = (dcsRequest_.GetData()[j + 1] << 8) + dcsRequest_.GetData()[j];
		}
		memcpy(&dmaData_, &replyPacket[0], sizeof(replyPacket));
		dcsRequestRecieved_ = false;
	}
	*buffer = &dmaData_;
	return 0;
}

int mu2esim::write_loopback_data(int chn, void *buffer, size_t bytes)
{
	uint16_t word;
	memcpy(&word, buffer, sizeof(word));

	switch (chn) {
	case 0: // DAQ Channel
		if ((word & 0xF01F) == 0x10) {
			activeDAQRing_ = (DTC::DTC_Ring_ID)((word & 0x0F00) >> 8);
			readoutRequestRecieved_[activeDAQRing_] = true;
		}
		else if ((word & 0xF02F) == 0x20) {
			activeDAQRing_ = (DTC::DTC_Ring_ID)((word & 0x0F00) >> 8);
			if (readoutRequestRecieved_[activeDAQRing_]) {
				dataRequestRecieved_ = true;
			}
		}
		break;
	case 1:
		if ((word & 0xF00F) == 0) {
			activeDCSRing_ = (DTC::DTC_Ring_ID)((word & 0x0F00) >> 8);
			dcsRequestRecieved_ = true;
			uint8_t data[12];
			memcpy(&data[0], (char*)buffer + (2 * sizeof(uint16_t)), sizeof(data));
			dcsRequest_ = DTC::DTC_DCSRequestPacket(activeDCSRing_, (DTC::DTC_ROC_ID)(word & 0xF), data);

		}
		break;
	}

	return 0;
}

int mu2esim::read_release(int chn, unsigned num)
{
	//Always succeeds
#ifndef _WIN32
	TRACE(0, "Simulating a release of buffer %u of channel %i", num, chn);
#endif
	return 0;
}

int  mu2esim::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
	*output = 0;
	if (registers_.count(address) > 0)
	{
		*output = registers_[address];
		return 0;
	}
	return 1;
}

int  mu2esim::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	// Write the register!!!
	registers_[address] = data;
	return 0;
}

int mu2esim::read_pcie_state(m_ioc_pcistate_t *output)
{
	*output = pcieState_;
	return 0;
}

int mu2esim::read_dma_state(int chn, int dir, m_ioc_engstate_t *output)
{
	*output = dmaState_[chn][dir];
	return 0;
}

int mu2esim::read_dma_stats(m_ioc_engstats_t *output)
{
	int engi[4] {0, 0, 0, 0};

	for (int i = 0; i < output->Count; ++i)
	{
		DMAStatistics thisStat;
		int eng = rand() % 4;
		++(engi[eng]);
		switch (eng) {
		case 0:
			thisStat.Engine = 0;
			break;
		case 1:
			thisStat.Engine = 1;
			break;
		case 2:
			thisStat.Engine = 32;
			break;
		case 3:
			thisStat.Engine = 33;
			break;
		}
		thisStat.LWT = 0;
		thisStat.LBR = engi[eng] * 1000000;
		thisStat.LAT = thisStat.LBR / 1000000;
		output->engptr[i] = thisStat;
	}

	return 0;
}

int mu2esim::read_trn_stats(TRNStatsArray *output)
{
	for (int i = 0; i < output->Count; ++i){
		TRNStatistics thisStat;
		thisStat.LRX = (i + 1) * 1000000;
		thisStat.LTX = (i + 1) * 1000000;
		output->trnptr[i] = thisStat;
	}

	return 0;
}

int mu2esim::read_test_command(m_ioc_cmd_t *output)
{
	*output = testState_;
	return 0;
}

int mu2esim::write_test_command(m_ioc_cmd_t input, bool start)
{
	testState_ = input;
	testStarted_ = start;
	return 0;
}
