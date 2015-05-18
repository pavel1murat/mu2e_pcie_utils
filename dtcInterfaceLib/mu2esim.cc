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
#else
#define TRACE(...)
#endif
#include "mu2esim.hh"
#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>

mu2esim::mu2esim() : isActive_(false),
dmaDCSData_(), mode_(DTCLib::DTC_SimMode_Disabled)
{
#ifndef _WIN32  
    //TRACE_CNTL( "lvlmskM", 0x3 );
    //TRACE_CNTL( "lvlmskS", 0x3 );
#endif
    for (int ring = 0; ring < 6; ++ring)
    {
        for (int roc = 0; roc < 6; ++roc)
        {
            dcsRequestRecieved_[ring][roc] = false;
            simIndex_[ring][roc] = 0;
            dcsRequest_[ring][roc] = DTCLib::DTC_DCSRequestPacket((DTCLib::DTC_Ring_ID)ring, (DTCLib::DTC_ROC_ID)roc);
        }
    }
    dmaDAQData_ = (mu2e_databuff_t*)new mu2e_databuff_t();
    olddmaDAQData_ = (mu2e_databuff_t*)new mu2e_databuff_t();
}

mu2esim::~mu2esim()
{
    delete[] dmaDAQData_;
    delete[] olddmaDAQData_;
}

int mu2esim::init(DTCLib::DTC_SimMode mode)
{
    TRACE(17, "mu2e Simulator::init");
    mode_ = mode;

    TRACE(17, "mu2esim::init Initializing registers");
    isActive_ = true;
    // Set initial register values...
    registers_[0x9000] = 0x00006363; // v99.99
    registers_[0x9004] = 0x53494D44; // SIMD in ASCII
    registers_[0x900C] = 0x00000010; // Send
    registers_[0x9010] = 0x00000040; // Recieve
    registers_[0x9014] = 0x00000100; // SPayload
    registers_[0x9018] = 0x00000400; // RPayload
    registers_[0x9100] = 0x40000003; // Clear latched errors, System Clock, Timing Enable
    registers_[0x9104] = 0x80000040; //Default value from HWUG
    registers_[0x9108] = 0x00049249; // SERDES Loopback PCS Near-End
    registers_[0x910C] = 0x2; // Initialization Complete, no IIC Error
    registers_[0x9168] = 0x00049249;
    registers_[0x9110] = 0x3F;        // ROC Emulators enabled (of course!)
    registers_[0x9114] = 0x3F3F;       // ALl rings Tx/Rx enabled, CFO and timing disabled
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
    registers_[0x9180] = 0x0;        // Timestamp preset to 0
    registers_[0x9184] = 0x0;
    registers_[0x9188] = 0x00002000; // Data pending timeout preset
    registers_[0x918C] = 0x0;          // NUMROCs 0 for all rings
    registers_[0x9190] = 0x0;  // NO FIFO Full flags
    registers_[0x9194] = 0x0;
    registers_[0x9198] = 0x0;
    registers_[0x9204] = 0x0010;     // Packet Size Bytes
    registers_[0x91A4] = 0x1;        // FPGA PROM Ready
    registers_[0x9404] = 0x1;
    registers_[0x9408] = 0x0;        // FPGA Core Access OK

    TRACE(17, "mu2esim::init Initializing DMA State Objects");
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

    TRACE(17, "mu2esim::init Initializing PCIe State Object");
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

    TRACE(17, "mu2esim::init finished");
    return (0);
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
 */
int mu2esim::read_data(int chn, void **buffer, int tmo_ms)
{
    // Clear the buffer:
    TRACE(17, "mu2esim::read_data Clearing output buffer");
    if (chn == 0) {
        delete[] olddmaDAQData_;
        olddmaDAQData_ = dmaDAQData_;
        dmaDAQData_ = (mu2e_databuff_t*)new mu2e_databuff_t();
        memset(dmaDAQData_, 0, sizeof(*dmaDAQData_));
    }
    else {
        memset(&dmaDCSData_, 0, sizeof(dmaDCSData_));
    }

    int bytesReturned = 0;

    if (chn == 0)
    {
        std::vector<DTCLib::DTC_Timestamp> activeTimestamps;
        for (int ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
        {
            for (auto i : readoutRequestRecieved_[ring])
            {
                bool found = false;
                for (auto ts : activeTimestamps) { found = found || ts == i; }
                if (!found) { activeTimestamps.push_back(i); }
            }
        }
        std::sort(activeTimestamps.begin(), activeTimestamps.end());
        size_t bufferIndex = 0;
        size_t bufferIndexMax = sizeof(mu2e_databuff_t) / (16 * sizeof(uint8_t));

        for (auto ts : activeTimestamps)
        {
            for (int ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
            {
                auto rrIter = std::find(readoutRequestRecieved_[ring].begin(), readoutRequestRecieved_[ring].end(), ts);
                if (rrIter != readoutRequestRecieved_[ring].end() && bufferIndex < bufferIndexMax)
                {
                    for (int roc = 0; roc <= DTCLib::DTC_ROC_5; ++roc)
                    {
                        auto drIter = std::find(dataRequestRecieved_[ring][roc].begin(), dataRequestRecieved_[ring][roc].end(), ts);
                        if (drIter != dataRequestRecieved_[ring][roc].end() && bufferIndex < bufferIndexMax)
                        {
                            TRACE(17, "mu2esim::read_data, DAQ Channel w/Requests recieved");
                            uint8_t packet[16];

                            // Add a Data Header packet to the reply
                            packet[0] = 32;
                            packet[1] = 0;
                            packet[2] = 0x50 + (roc & 0x0F);
                            packet[3] = 0x80 + (ring & 0x0F);
                            packet[4] = 1; //1 data packet
                            packet[5] = 0;
                            (*rrIter).GetTimestamp(packet, 6);
                            packet[12] = 0;
                            packet[13] = 0;
                            packet[14] = 0;
                            packet[15] = 0;

                            TRACE(17, "mu2esim::read_data Copying Data Header packet into buffer, idx=%li, buf=%p, packet=%p, off=%li"
				  , bufferIndex, (void*)dmaDAQData_, (void*)packet, bufferIndex * sizeof(packet));
                            memcpy((char*)dmaDAQData_ + bufferIndex * sizeof(packet), &packet[0], sizeof(packet));
                            bufferIndex++;
                            if (bufferIndex >= bufferIndexMax) { break; }

                            switch (mode_)
                            {
                            case DTCLib::DTC_SimMode_CosmicVeto:
                            {
                                int nSamples = 4;
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

                                TRACE(17, "mu2esim::read_data Copying Data packet into buffer, idx=%li, buf=%p, packet=%p, off=%li"
				      , bufferIndex, (void*)dmaDAQData_, (void*)packet, bufferIndex * sizeof(packet));
                                memcpy(((char*)dmaDAQData_ + bufferIndex * sizeof(packet)), &packet, sizeof(packet));
                                bufferIndex++;

                            }
                            break;
                            case DTCLib::DTC_SimMode_Calorimeter:
                            {
                                int nSamples = rand() % 10 + 10;
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

                                TRACE(17, "mu2esim::read_data Copying Data packet into buffer, idx=%li, buf=%p, packet=%p, off=%li"
				      , bufferIndex, (void*)dmaDAQData_, (void*)packet, bufferIndex * sizeof(packet));
                                memcpy(((char*)dmaDAQData_ + bufferIndex * sizeof(packet)), &packet, sizeof(packet));
                                bufferIndex++;

                                int samplesProcessed = 5;
                                while (samplesProcessed < nSamples)
                                {
                                    if (bufferIndex >= bufferIndexMax) { break; }

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
                                    TRACE(17, "mu2esim::read_data Copying Data packet into buffer, idx=%li, buf=%p, packet=%p, off=%li"
					  , bufferIndex, (void*)dmaDAQData_, (void*)packet, bufferIndex * sizeof(packet));
                                    memcpy(((char*)dmaDAQData_ + bufferIndex * sizeof(packet)), &packet, sizeof(packet));
                                    bufferIndex++;
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

                                TRACE(17, "mu2esim::read_data Copying Data packet into buffer, idx=%li, buf=%p, packet=%p, off=%li"
				      , bufferIndex, (void*)dmaDAQData_, (void*)packet, bufferIndex * sizeof(packet));
                                memcpy(((char*)dmaDAQData_ + bufferIndex * sizeof(packet)), &packet, sizeof(packet));
                                bufferIndex++;
                            }
                            break;
                            case DTCLib::DTC_SimMode_Disabled:
                            default:
                                break;
                            }
                            simIndex_[ring][roc] = (simIndex_[ring][roc] + 1) % 0x3FF;
                            TRACE(17, "mu2esim::read_data: Erasing DTC_Timestamp from DataRequestReceived list");
                            dataRequestRecieved_[ring][roc].erase(drIter);
                        }
                    }
                    TRACE(17, "mu2esim::read_data: Erasing DTC_Timestamp from ReadoutRequestReceived list");
                    readoutRequestRecieved_[ring].erase(rrIter);
                }
            }
        }
	bytesReturned = 16 * bufferIndex;
    }
    else if (chn == 1)
    {
        for (int ring = 0; ring <= DTCLib::DTC_Ring_5; ++ring)
        {
            for (int roc = 0; roc <= DTCLib::DTC_ROC_5; ++roc)
            {
                if (dcsRequestRecieved_[ring][roc])
                {
                    TRACE(17, "mu2esim::read_data DCS Request Recieved, Sending Response");
                    uint8_t replyPacket[16];
                    replyPacket[0] = 16;
                    replyPacket[1] = 0;
                    replyPacket[2] = 0x40;
                    replyPacket[3] = (ring & 0x0F) + 0x80;
                    for (int i = 4; i < 16; ++i)
                    {
                        replyPacket[i] = dcsRequest_[ring][roc].GetData()[i - 4];
                    }
                    TRACE(17, "mu2esim::read_data Copying reply into buffer");
                    memcpy(&dmaDCSData_, &replyPacket[0], sizeof(replyPacket));
                    dcsRequestRecieved_[ring][roc] = false;
		    bytesReturned=16;
		    goto out;
                }
            }
        }
 out:;
    }

    if (chn == 0)
    {
        TRACE(17, "mu2esim::read_data Setting output buffer to dmaDAQData_=%p", (void*)dmaDAQData_);
        *buffer = dmaDAQData_;
    }
    else if (chn == 1)
    {
        TRACE(17, "mu2esim::read_data Setting output buffer to dmaDCSData_=%p", (void*)&dmaDCSData_);
        *buffer = &dmaDCSData_;
    }

    TRACE(17, "mu2esim::read_data RETURN: dmaDCSData_=%p, olddmaDAQData_=%p, dmaDAQData_=%p"
	  , (void*)&dmaDCSData_, (void*)olddmaDAQData_, (void*)dmaDAQData_);
    return bytesReturned;
}

int mu2esim::write_loopback_data(int chn, void *buffer, size_t bytes)
{
    TRACE(17, "mu2esim::write_loopback_data start");
    uint32_t worda;
    memcpy(&worda, buffer, sizeof(worda));
    uint16_t word = static_cast<uint16_t>(worda >> 16);
    TRACE(17, "mu2esim::write_loopback_data worda is 0x%x and word is 0x%x", worda, word);

    switch (chn) {
    case 0: // DAQ Channel
    {
        DTCLib::DTC_Timestamp ts((uint8_t*)buffer + 6);
        if ((word & 0x8010) == 0x8010) {
            int activeDAQRing = (word & 0x0F00) >> 8;
            TRACE(17, "mu2esim::write_loopback_data activeDAQRing is %i", activeDAQRing);
            if (std::find(readoutRequestRecieved_[activeDAQRing].begin(), readoutRequestRecieved_[activeDAQRing].end(), ts) == readoutRequestRecieved_[activeDAQRing].end())
            {
                readoutRequestRecieved_[activeDAQRing].push_back(ts);
            }
        }
        else if ((word & 0x8020) == 0x8020) {
            int activeDAQRing = (word & 0x0F00) >> 8;
            TRACE(17, "mu2esim::write_loopback_data activeDAQRing is %i", activeDAQRing);
            if (std::find(readoutRequestRecieved_[activeDAQRing].begin(), readoutRequestRecieved_[activeDAQRing].end(), ts) != readoutRequestRecieved_[activeDAQRing].end()) {
                int activeROC = word & 0xF;
                if (std::find(dataRequestRecieved_[activeDAQRing][activeROC].begin(), dataRequestRecieved_[activeDAQRing][activeROC].end(), ts) == dataRequestRecieved_[activeDAQRing][activeROC].end())
                {
                    dataRequestRecieved_[activeDAQRing][activeROC].push_back(ts);
                }
            }
        }
        break;
    }
    case 1:
    {
        if ((word & 0x0080) == 0) {
            int activeDCSRing = (word & 0x0F00) >> 8;
            int activeDCSROC = word & 0xF;
            TRACE(17, "mu2esim::write_loopback_data activeDCSRing is %i, roc is %i", activeDCSRing, activeDCSROC);
            dcsRequestRecieved_[activeDCSRing][activeDCSROC] = true;
            uint8_t data[12];
            memcpy(&data[0], (char*)buffer + (2 * sizeof(uint16_t)), sizeof(data));
            dcsRequest_[activeDCSRing][activeDCSROC] = DTCLib::DTC_DCSRequestPacket((DTCLib::DTC_Ring_ID)activeDCSRing, (DTCLib::DTC_ROC_ID)activeDCSROC, data);
            TRACE(17, "mu2esim::write_loopback_data: Recieved DCS Request:");
            TRACE(17, dcsRequest_[activeDCSRing][activeDCSROC].toJSON().c_str());
        }
        break;
    }
    }

    return 0;
}

int mu2esim::read_release(int chn, unsigned num)
{
    //Always succeeds
    TRACE(0, "Simulating a release of buffer %u of channel %i", num, chn);
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
