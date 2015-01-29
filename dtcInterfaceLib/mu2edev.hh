 // This file (mu2edev.hh) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include "mu2e_mmap_ioctl.h" // 
#include <cstdint>
#include "mu2esim.hh"


struct mu2edev
{
    mu2edev();
    int  init( bool simMode = false );
    int  read_data( int chn, void **buffer, int tmo_ms );
    int  read_release( int chn, unsigned num );
    int  read_register(uint16_t address, int tmo_ms, uint32_t *output);
    int  write_register(uint16_t address, int tmo_ms, uint32_t data);
    void meta_dump( int chn, int dir );
	int  write_loopback_data(int chn, void *buffer, size_t bytes);
    int  read_pcie_state(m_ioc_pcistate_t *output);
    int  read_dma_state(int chn, int dir, m_ioc_engstate_t *output);
    int  read_dma_stats(m_ioc_engstats_t *output);
    int  read_trn_stats(TRNStatsArray *output);
	int  read_test_command(m_ioc_cmd_t *output);
	int  write_test_command(m_ioc_cmd_t input, bool start);

private:
    unsigned         delta_( int chn, int dir );

    int	             devfd_;
    volatile void *  mu2e_mmap_ptrs_[MU2E_MAX_CHANNELS][2][2];
    m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];
	mu2esim          simulator_;
};
