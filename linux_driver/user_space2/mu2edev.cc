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
#include "trace.h"
#include "mu2e_mmap_ioctl.h" // MU2E_DEV_FILE, M_IOC_*, etc
#include "mu2edev.hh"

mu2edev::mu2edev() : devfd_(0)
		   , mu2e_mmap_ptrs_() // extended initializer list; need -std=c++0x
{   TRACE_CNTL( "lvlmskM", 0xffff );
    TRACE_CNTL( "lvlmskS", 0xf );
}

int mu2edev::init()
{   int sts;
    devfd_ = open( "/dev/" MU2E_DEV_FILE, O_RDWR );
    if (devfd_ == -1) { perror("open /dev/" MU2E_DEV_FILE); exit (1); }
    for (unsigned chn=0; chn<MU2E_MAX_CHANNELS; ++chn)
	for (unsigned dir=0; dir<2; ++dir)
	{   m_ioc_get_info_t get_info;
	    get_info.chn = chn; get_info.dir = dir;
	    sts = ioctl( devfd_, M_IOC_GET_INFO, &get_info );
	    if (sts != 0) { perror( "M_IOC_GET_INFO" ); exit (1); }
	    mu2e_channel_info_[chn][dir] = get_info;
	    TRACE( 1, "mu2edev::init %u:%u - num=%u size=%u hwIdx=%u, swIdx=%u"
		  , chn,dir
		  , get_info.num_buffs, get_info.buff_size
		  , get_info.hwIdx, get_info.swIdx );
	    for (unsigned map=0; map<2; ++map)
	    {   mu2e_mmap_ptrs_[chn][dir][map]
		    = mmap( 0 /* hint address */
			   , get_info.num_buffs * ((map==MU2E_MAP_BUFF)
						   ? get_info.buff_size
						   : sizeof(int) )
			   , (((dir==S2C)&&(map==MU2E_MAP_BUFF))
			      ? PROT_WRITE : PROT_READ )
			   , MAP_SHARED
			   , devfd_
			   , chnDirMap2offset(chn,dir,map) // trick to encode chn,dir,map into "offset"
			   );
		if (mu2e_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
		{   perror( "mmap" ); exit (1);
		}
		TRACE( 1, "mu2edev::init chnDirMap2offset=%lu mu2e_mmap_ptrs_[%d][%d][%d]=%p"
		      , chnDirMap2offset( chn, dir, map )
		      , chn, dir, map, mu2e_mmap_ptrs_[chn][dir][map] );
	    }
	}
    return (0);
}  // init

static uint64_t get_us_timeofday()
{   struct timeval tv;
    gettimeofday( &tv, NULL );
    return (uint64_t)tv.tv_sec*1000000+tv.tv_usec;
}  // get_us_timeofday


int mu2edev::read_data( int chn, void **buffer, int tmo_ms )
{                                            // FIXME - tmo_ms not used
    int retsts=0;
    unsigned has_recv_data;
    if ((mu2e_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0))
    {   uint64_t expire_time=get_us_timeofday()+tmo_ms*1000;
	do
	{
	    has_recv_data = delta_( chn, C2S );
	    if (  (has_recv_data > 0)
		||(  (retsts=ioctl(devfd_,M_IOC_GET_INFO,&mu2e_channel_info_[chn][C2S]))==0
		   &&(has_recv_data=delta_(chn,C2S))>0 ) )
	    {   // have data
		// get byte count from new/next sw
		unsigned newNxtIdx = idx_add(mu2e_channel_info_[chn][C2S].swIdx,1,chn,C2S);
		int *    BC_p=(int*)mu2e_mmap_ptrs_[chn][C2S][MU2E_MAP_META];
		retsts = BC_p[newNxtIdx];
		*buffer = ((mu2e_databuff_t*)(mu2e_mmap_ptrs_[chn][C2S][MU2E_MAP_BUFF]))[newNxtIdx];
		TRACE( 1, "mu2edev::read_data chn%d hIdx=%u, sIdx=%u "
		      "%u hasRcvDat=%u %p[newNxtIdx=%d]=retsts=%d"
		      , chn
		      , mu2e_channel_info_[chn][C2S].hwIdx, mu2e_channel_info_[chn][C2S].swIdx
		      , mu2e_channel_info_[chn][C2S].num_buffs, has_recv_data
		       , (void*)BC_p, newNxtIdx
		      , retsts );
		break;
	    }
	    else
	    {   // was it a tmo or error
		if (retsts != 0) { perror( "M_IOC_GET_INFO" ); exit (1); }
		// not error...
	    }
	} while ((get_us_timeofday()<expire_time) && (usleep(1000)==0));
    }
    return (retsts);
}  // read_data

int mu2edev::read_release( int chn, unsigned num )
{
    int retsts;
    unsigned long arg;
    unsigned has_recv_data;
    has_recv_data = delta_( chn, C2S );
    if (num <= has_recv_data)
    {   arg=(chn<<24)|(C2S<<16)|(num&0xffff);// THIS OBIVOUSLY SHOULD BE A MACRO
	retsts=ioctl( devfd_, M_IOC_BUF_GIVE, arg );
	if (retsts != 0) { perror( "M_IOC_BUF_GIVE" ); exit (1); }

	// increment our cached info
	mu2e_channel_info_[chn][C2S].swIdx
	    = idx_add( mu2e_channel_info_[chn][C2S].swIdx, 1, chn, C2S );
    }
    return (retsts);
}  // read_release


void mu2edev::meta_dump( int chn, int dir )
{
    int retsts=0;
    if ((mu2e_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0))
    {
	for (unsigned buf=0; buf<mu2e_channel_info_[chn][dir].num_buffs; ++buf)
	{   int *    BC_p=(int*)mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_META];
	    printf( "buf_%02d: %u\n", buf, BC_p[buf] );
	}
    }
}  // meta_dump

int mu2edev::write_loopback_data( int chn, void *buffer, size_t bytes )
{
    int dir=S2C;
    int retsts=0;
    unsigned delta = delta_( chn, dir );
    TRACE( 3, "write_loopback_data delta=%u", delta );
    if (delta > 0)
    {   unsigned idx = mu2e_channel_info_[chn][dir].swIdx;
	void * data=((mu2e_databuff_t*)(mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_BUFF]))[idx];
	memcpy( data, buffer, bytes );
	unsigned long arg=(chn<<24)|(bytes&0xffffff);// THIS OBIVOUSLY SHOULD BE A MACRO
	retsts=ioctl( devfd_, M_IOC_BUF_XMIT, arg );
	if (retsts != 0) { perror( "M_IOC_BUF_GIVE" ); exit (1); }
    }
    return retsts;
}  // write_loopback_data

unsigned mu2edev::delta_( int chn, int dir )
{   unsigned hw=mu2e_channel_info_[chn][dir].hwIdx;
    unsigned sw=mu2e_channel_info_[chn][dir].swIdx;
    if (dir == C2S)
	return ( (hw>=sw)
		? hw-sw
		: mu2e_channel_info_[chn][dir].num_buffs+hw-sw );
    else
	return ( (sw>=hw)
		? mu2e_channel_info_[chn][dir].num_buffs-(sw-hw)
		: hw-sw );
}  // delta_
