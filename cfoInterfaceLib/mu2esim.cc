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
# define TRACE(...)
# pragma warning(disable: 4351)
#endif
#include "mu2esim.hh"
#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>

mu2esim::mu2esim()
    : hwIdx_(0)
    , swIdx_(0)
    , dmaData_()
{
#ifndef _WIN32  
    //TRACE_CNTL( "lvlmskM", 0x3 );
    //TRACE_CNTL( "lvlmskS", 0x3 );
#endif
    release_all();
}

mu2esim::~mu2esim()
{
        for (unsigned jj = 0; jj < SIM_BUFFCOUNT; ++jj) {
            delete[] dmaData_[jj];
        }
}

int mu2esim::init()
{
    TRACE(17, "mu2e CFOSimulator::init");

    TRACE(17, "mu2esim::init Initializing registers");
    // Set initial register values...
    registers_[0x9000] = 0x00006363; // v99.99
    registers_[0x9004] = 0x53494D44; // SIMD in ASCII
    registers_[0x900C] = 0x00000010; // Send
    registers_[0x9010] = 0x00000040; // Recieve
    registers_[0x9014] = 0x00000100; // SPayload
    registers_[0x9018] = 0x00000400; // RPayload
    registers_[0x9100] = 0x00000002; // System Clock
    registers_[0x9104] = 0x80000040; //Default value from HWUG
    registers_[0x9108] = 0x00249249; // SERDES Loopback PCS Near-End
    registers_[0x9168] = 0x00249249;
    registers_[0x9114] = 0xFFFF;       // All rings Tx/Rx enabled
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
    registers_[0x918C] = 0x1;          // NUMCFOs 0 for all rings,except Ring 0 which has 1
    registers_[0x9190] = 0x0;  // NO FIFO Full flags
    registers_[0x9194] = 0x0;
    registers_[0x9198] = 0x0;
    registers_[0x9204] = 0x0010;     // Packet Size Bytes
    registers_[0x91A4] = 0x1;        // FPGA PROM Ready
    registers_[0x9304] = 0x02FFFFFF; // Readout Request Information Table Size (bytes)
    registers_[0x9404] = 0x1;
    registers_[0x9408] = 0x0;        // FPGA Core Access OK

    TRACE(17, "mu2esim::init Initializing DMA State Objects");
    // Set DMA State
    dmaState_[0].BDerrs = 0;
    dmaState_[0].BDSerrs = 0;
    dmaState_[0].BDs = 399;
    dmaState_[0].Buffers = 4;
    dmaState_[0].Engine = 0;
    dmaState_[0].IntEnab = 0;
    dmaState_[0].MaxPktSize = 0x100000;
    dmaState_[0].MinPktSize = 0x40;
    dmaState_[0].TestMode = 0;

    dmaState_[1].BDerrs = 0;
    dmaState_[1].BDSerrs = 0;
    dmaState_[1].BDs = 399;
    dmaState_[1].Buffers = 4;
    dmaState_[1].Engine = 0x20;
    dmaState_[1].IntEnab = 0;
    dmaState_[1].MaxPktSize = 0x100000;
    dmaState_[1].MinPktSize = 0x40;
    dmaState_[1].TestMode = 0;

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
int mu2esim::read_data(void **buffer, int tmo_ms)
{
    if (delta_(C2S) == 0 && tmo_ms >= 0) {
        clearBuffer_(false);
        
    }

    TRACE(17, "mu2esim::read_data Setting output buffer to dmaData_[%li]=%p, retsts=%lu", swIdx_, (void*)dmaData_[swIdx_], buffSize_[swIdx_]);
    size_t bytesReturned = buffSize_[swIdx_];
    *buffer = dmaData_[swIdx_];
    swIdx_ = (swIdx_ + 1) % SIM_BUFFCOUNT;

    return bytesReturned;
}

int mu2esim::write_data(void *buffer, size_t bytes)
{
    TRACE(17, "mu2esim::write_data start");
    uint32_t worda;
    memcpy(&worda, buffer, sizeof(worda));
    //uint16_t word = static_cast<uint16_t>(worda >> 16);
    TRACE(17, "mu2esim::write_data worda is 0x%x", worda);
 
    return 0;
}

int mu2esim::read_release(unsigned num)
{
    //Always succeeds
    TRACE(17, "mu2esim::read_release: Simulating a release of %u buffers of channel %i", num, 0);
    for (unsigned ii = 0; ii < num; ++ii) {
        delete[] dmaData_[swIdx_];
        dmaData_[swIdx_] = (mu2e_databuff_t*)new mu2e_databuff_t();
        swIdx_ = (swIdx_ + 1) % SIM_BUFFCOUNT;
    }
    return 0;
}

int mu2esim::release_all()
{
    return read_release(SIM_BUFFCOUNT);
}

int  mu2esim::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
    *output = 0;
    if (registers_.count(address) > 0)
    {
        TRACE(17, "mu2esim::read_register: Returning value 0x%x for address 0x%x", registers_[address], address);
        *output = registers_[address];
        return 0;
    }
    return 1;
}

int  mu2esim::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
    // Write the register!!!
    TRACE(17, "mu2esim::write_register: Writing value 0x%x into address 0x%x", data, address);
    registers_[address] = data;
    return 0;
}

int mu2esim::read_pcie_state(m_ioc_pcistate_t *output)
{
    *output = pcieState_;
    return 0;
}

int mu2esim::read_dma_state(int dir, m_ioc_engstate_t *output)
{
    *output = dmaState_[dir];
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

unsigned mu2esim::delta_(int dir)
{
    unsigned hw = hwIdx_;
    unsigned sw = swIdx_;
    TRACE(21, "mu2edev::delta_ dir=%d hw=%u sw=%u num_buffs=%u"
       , dir, hw, sw, SIM_BUFFCOUNT);
    if (dir == C2S)
        return ((hw >= sw)
        ? hw - sw
        : SIM_BUFFCOUNT + hw - sw);
    else
        return ((sw >= hw)
        ? SIM_BUFFCOUNT - (sw - hw)
        : hw - sw);
}

void mu2esim::clearBuffer_(bool increment) {
    // Clear the buffer:
    TRACE(17, "mu2esim::clearBuffer_: Clearing output buffer");
    if (increment) {
        hwIdx_ = (hwIdx_ + 1) % SIM_BUFFCOUNT;
    }
    memset(dmaData_[hwIdx_], 0, sizeof(mu2e_databuff_t));
}