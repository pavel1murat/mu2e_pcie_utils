// mu2esim.hh: mu2e DTC simulator main file
//
// Eric Flumerfelt
// January 27, 2015
//
#include <cstdint>
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#include <unordered_map>
#include "DTC_Types.h"

struct mu2esim
{
    mu2esim();
    ~mu2esim();
    int  init(DTCLib::DTC_Sim_Mode mode = DTCLib::DTC_Sim_Mode_Tracker);
    int  read_data(int chn, void **buffer, int tmo_ms); // return bytes read; error if negative
    int  write_loopback_data(int chn, void *buffer, size_t bytes);
    int  read_release(int chn, unsigned num);
    int  read_register(uint16_t address, int tmo_ms, uint32_t *output);
    int  write_register(uint16_t address, int tmo_ms, uint32_t data);
    int  read_pcie_state(m_ioc_pcistate_t *output);
    int  read_dma_state(int chn, int dir, m_ioc_engstate_t *output);
    int  read_dma_stats(m_ioc_engstats_t *output);
    int  read_trn_stats(TRNStatsArray *output);
    int  read_test_command(m_ioc_cmd_t *output);
    int  write_test_command(m_ioc_cmd_t input, bool start);
    bool active() { return isActive_; }
private:
    //const DTCLib::DTC_Timestamp NULL_TIMESTAMP = DTCLib::DTC_Timestamp(0xffffffffffffffff);
    bool isActive_;
    std::unordered_map<uint16_t, uint32_t> registers_;
    mu2e_databuff_t* dmaDAQData_;
    mu2e_databuff_t* olddmaDAQData_;
    mu2e_databuff_t dmaDCSData_;
    m_ioc_engstate_t dmaState_[MU2E_MAX_CHANNELS][2];
    m_ioc_pcistate_t pcieState_;
    m_ioc_cmd_t testState_;
    bool testStarted_;
    DTCLib::DTC_Sim_Mode mode_;
    uint16_t simIndex_[6][6];
    bool dcsRequestRecieved_[6][6];
    std::vector<DTCLib::DTC_Timestamp> readoutRequestRecieved_[6];
    std::vector<DTCLib::DTC_Timestamp> dataRequestRecieved_[6][6];
    DTCLib::DTC_DCSRequestPacket dcsRequest_[6][6];
};
