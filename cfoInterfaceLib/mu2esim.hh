// mu2esim.hh: mu2e CFO simulator main file
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
#include "CFO_Types.h"

#define SIM_BUFFCOUNT 4

struct mu2esim
{
    mu2esim();
    ~mu2esim();
    int  init();
    int  read_data(void **buffer, int tmo_ms); // return bytes read; error if negative
    int  write_data(void *buffer, size_t bytes);
    int  read_release(unsigned num);
    int  release_all();
    int  read_register(uint16_t address, int tmo_ms, uint32_t *output);
    int  write_register(uint16_t address, int tmo_ms, uint32_t data);
    int  read_pcie_state(m_ioc_pcistate_t *output);
    int  read_dma_state(int dir, m_ioc_engstate_t *output);
    int  read_dma_stats(m_ioc_engstats_t *output);
    int  read_trn_stats(TRNStatsArray *output);
    int  read_test_command(m_ioc_cmd_t *output);
    int  write_test_command(m_ioc_cmd_t input, bool start);
private:
    unsigned delta_(int dir);
    void clearBuffer_(bool increment = true);

    std::unordered_map<uint16_t, uint32_t> registers_;
    size_t hwIdx_;
    size_t swIdx_;
    size_t buffSize_[SIM_BUFFCOUNT];
    mu2e_databuff_t* dmaData_[SIM_BUFFCOUNT];
    //mu2e_databuff_t* dmaDAQData_[SIM_BUFFCOUNT];
    //mu2e_databuff_t* dmaDCSData_[SIM_BUFFCOUNT];
    m_ioc_engstate_t dmaState_[2];
    m_ioc_pcistate_t pcieState_;
    m_ioc_cmd_t testState_;
    bool testStarted_;
};
