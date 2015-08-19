// mu2esim.hh: mu2e DTC simulator main file
//
// Eric Flumerfelt
// January 27, 2015
//
#include <cstdint>
#ifndef _WIN32
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h" // 
#else
#include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#endif
#include <unordered_map>
#include <mutex>
#include <set>
#include "DTC_Types.h"

#define SIM_BUFFCOUNT 4

struct mu2esim
{
    mu2esim();
    ~mu2esim();
    int  init(DTCLib::DTC_SimMode mode = DTCLib::DTC_SimMode_Tracker);
    int  read_data(int chn, void **buffer, int tmo_ms); // return bytes read; error if negative
    int  write_data(int chn, void *buffer, size_t bytes);
    int  read_release(int chn, unsigned num);
    int  release_all(int chn);
    int  read_register(uint16_t address, int tmo_ms, uint32_t *output);
    int  write_register(uint16_t address, int tmo_ms, uint32_t data);
    int  read_pcie_state(m_ioc_pcistate_t *output);
    int  read_dma_state(int chn, int dir, m_ioc_engstate_t *output);
    int  read_dma_stats(m_ioc_engstats_t *output);
    int  read_trn_stats(TRNStatsArray *output);
    int  read_test_command(m_ioc_cmd_t *output);
    int  write_test_command(m_ioc_cmd_t input, bool start);
private:
    unsigned delta_(int chn, int dir);
    void clearBuffer_(int chn, bool increment = true);

    //const DTCLib::DTC_Timestamp NULL_TIMESTAMP = DTCLib::DTC_Timestamp(0xffffffffffffffff);
    std::unordered_map<uint16_t, uint32_t> registers_;
    size_t hwIdx_[MU2E_MAX_CHANNELS];
    size_t swIdx_[MU2E_MAX_CHANNELS];
    size_t buffSize_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
    mu2e_databuff_t* dmaData_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
    //mu2e_databuff_t* dmaDAQData_[SIM_BUFFCOUNT];
    //mu2e_databuff_t* dmaDCSData_[SIM_BUFFCOUNT];
    m_ioc_engstate_t dmaState_[MU2E_MAX_CHANNELS][2];
    m_ioc_pcistate_t pcieState_;
    m_ioc_cmd_t testState_;
    bool testStarted_;
    DTCLib::DTC_SimMode mode_;
    uint16_t simIndex_[6][6];
    bool dcsRequestRecieved_[6][6];
    std::set<uint64_t> readoutRequestRecieved_[6];
    std::mutex rrMutex_;
    std::set<uint64_t> dataRequestRecieved_[6][6];
    std::mutex drMutex_;
    DTCLib::DTC_DCSRequestPacket dcsRequest_[6][6];
};
