 // This file (mu2edev.hh) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include "../mymodule/mu2e_mmap_ioctl.h" // 


struct mu2edev
{
    mu2edev();
    int  init( void );
    int  read_data( int chn, void **buffer, int tmo_ms );
    int  read_release( int chn, unsigned num );
    void meta_dump( int chn, int dir );

private:
    unsigned         delta_( int chn, int dir );

    int	             devfd_;
    volatile void *  mu2e_mmap_ptrs_[MU2E_MAX_CHANNELS][2][2];
    m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];
};
