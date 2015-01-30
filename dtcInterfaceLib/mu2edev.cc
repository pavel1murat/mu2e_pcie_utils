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
#include "mu2edev.hh"

mu2edev::mu2edev() : devfd_(0)
, mu2e_mmap_ptrs_() // extended initializer list; need -std=c++0x
, simulator_()
{
#ifndef _WIN32  
	TRACE_CNTL( "lvlmskM", 0x3 );
	TRACE_CNTL( "lvlmskS", 0x3 );
#endif
}

int mu2edev::init(bool simMode)
{
	if (simMode) {
		simulator_.init();
	}
	else {
#ifndef _WIN32
		int sts;
		devfd_ = open( "/dev/" MU2E_DEV_FILE, O_RDWR );
		if (devfd_ == -1) {
			perror("open /dev/" MU2E_DEV_FILE);
			return init(true);
		}
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
		, chnDirMap2offset( chn, dir, map ) );
		if (mu2e_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
		{   perror( "mmap" ); exit (1);
		}
		TRACE( 1, "mu2edev::init chnDirMap2offset=%lu mu2e_mmap_ptrs_[%d][%d][%d]=%p"
			, chnDirMap2offset( chn, dir, map )
			, chn, dir, map, mu2e_mmap_ptrs_[chn][dir][map] );
		}
			}
#endif
	}
	return (simMode);
}

int mu2edev::read_data(int chn, void **buffer, int tmo_ms)
{
	if (simulator_.active()) {
		return simulator_.read_data(chn, buffer, tmo_ms);
	}
	else {
#ifdef _WIN32
		int retsts = -1;
#else
		int retsts=0;
		unsigned has_recv_data;
		if ((mu2e_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0))
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
			}
			else
			{   // was it a tmo or error
				if (retsts != 0) { perror( "M_IOC_GET_INFO" ); exit (1); }
				// not error... return 0 status
			}

		}
#endif
		return (retsts);
	}
}

int mu2edev::read_release(int chn, unsigned num)
{
	if (simulator_.active())
	{
		return simulator_.read_release(chn, num);
	}
	else {
#ifdef _WIN32
		int retsts = -1;
#else
		int retsts = 0;
		unsigned long arg;
		unsigned has_recv_data;
		has_recv_data = delta_( chn, C2S );
		if (num <= has_recv_data)
		{   arg=(chn<<24)|(C2S<<16)|(num&0xffff);// THIS OBIVOUSLY SHOULD BE A MACRO
		retsts=ioctl( devfd_, M_IOC_BUF_GIVE, arg );
		if (retsts != 0) { perror( "M_IOC_GET_INFO" ); exit (1); }

		// increment our cached info
		mu2e_channel_info_[chn][C2S].swIdx
			= idx_add( mu2e_channel_info_[chn][C2S].swIdx, 1, chn, C2S );
		}
#endif
		return (retsts);
	}
}


int  mu2edev::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
	if (simulator_.active())
	{
		return simulator_.read_register(address, tmo_ms, output);
	}
	else {
#ifdef _WIN32
		int errorCode = -1;
#else
		m_ioc_reg_access_t reg;
		reg.reg_offset = address;
		reg.access_type = 0;
		int errorCode = ioctl(devfd_, M_IOC_REG_ACCESS, &reg);
		*output = reg.val;
#endif
		return errorCode;
	}
}


int  mu2edev::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	if (simulator_.active())
	{
		return simulator_.write_register(address, tmo_ms, data);
	}
	else {
#ifdef _WIN32
		return -1;
#else
		m_ioc_reg_access_t reg;
		reg.reg_offset = address;
		reg.access_type = 1;
		reg.val = data;
		return ioctl(devfd_, M_IOC_REG_ACCESS, &reg);
#endif
	}
}


void mu2edev::meta_dump(int chn, int dir)
{
	if (simulator_.active())
	{
		return;
	}
	else {
#ifdef _WIN32
		return;
#else
		int retsts=0;
		if ((mu2e_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0))
		{
			for (unsigned buf=0; buf<mu2e_channel_info_[chn][dir].num_buffs; ++buf)
			{   int *    BC_p=(int*)mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_META];
			printf( "buf_%02d: %u\n", buf, BC_p[buf] );
			}
		}
#endif
	}
}

int mu2edev::write_loopback_data(int chn, void *buffer, size_t bytes)
{
	if (simulator_.active())
	{
		return simulator_.write_loopback_data(chn, buffer, bytes);
	}
	else
	{
#ifdef _WIN32
		int retsts = -1;
#else
		int dir = S2C;
		int retsts = 0;
		unsigned delta = delta_(chn, dir);
		TRACE(3, "write_loopback_data delta=%u", delta);
		if (delta > 0)
		{
			unsigned idx = mu2e_channel_info_[chn][dir].swIdx;
			void * data = ((mu2e_databuff_t*)(mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_BUFF]))[idx];
			memcpy(data, buffer, bytes);
			unsigned long arg = (chn << 24) | (bytes & 0xffffff);// THIS OBIVOUSLY SHOULD BE A MACRO
			retsts = ioctl(devfd_, M_IOC_BUF_XMIT, arg);
			if (retsts != 0) { perror("M_IOC_BUF_GIVE"); exit(1); }
		}
#endif
		return retsts;
	}
}  // write_loopback_data


int mu2edev::read_pcie_state(m_ioc_pcistate_t *output)
{
	if (simulator_.active())
	{
		return simulator_.read_pcie_state(output);
	}
	else {
#ifdef _WIN32
		int error = -1;
#else
		int error = ioctl(devfd_, M_IOC_GET_PCI_STATE, output);
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}
#endif
		return error;
	}
}

int mu2edev::read_dma_state(int chn, int dir, m_ioc_engstate_t *output)
{
	if (simulator_.active())
	{
		return simulator_.read_dma_state(chn, dir, output);
	}
	else {
#ifdef _WIN32
		int error = -1;
#else
		m_ioc_engstate_t req;
		req.Engine = chn + 0x20 * dir;
		TRACE(0, "Output engine is: %u", req.Engine);
		int error = ioctl(devfd_, M_IOC_GET_ENG_STATE, &req);
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}
		*output = req;
#endif
		return error;
	}
}

int mu2edev::read_dma_stats(m_ioc_engstats_t *output)
{
	if (simulator_.active())
	{
		return simulator_.read_dma_stats(output);
	}
	else {
#ifdef _WIN32
		int error = -1;
#else
		int error = ioctl(devfd_, M_IOC_GET_DMA_STATS, output);
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}
#endif
		return error;
	}
}

int mu2edev::read_trn_stats(TRNStatsArray *output)
{
	if (simulator_.active())
	{
		return simulator_.read_trn_stats(output);
	}
	else {
#ifdef _WIN32
		int error = -1;
#else
		int error = ioctl(devfd_, M_IOC_GET_TRN_STATS, output);
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}
#endif
		return error;
	}
}

int mu2edev::read_test_command(m_ioc_cmd_t *output)
{
	if (simulator_.active())
	{
		return simulator_.read_test_command(output);
	}
	else {
#ifdef _WIN32
		return -1;
#else
		int error = ioctl(devfd_, M_IOC_GET_TST_STATE, output);
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}
		return error;
#endif
	}
}

int mu2edev::write_test_command(m_ioc_cmd_t input, bool start)
{
	if (simulator_.active())
	{
		return simulator_.write_test_command(input, start);
	}
	else {
#ifdef _WIN32
		return -1;
#else
		int error = 0;
		if(start) {
			error = ioctl(devfd_, M_IOC_TEST_START, &input);
		}
		else {
			error = ioctl(devfd_, M_IOC_TEST_STOP, &input);
		}
		if(error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}

		return error;
#endif
	}
}

int mu2edev::release_all(int chn)
{
	int retsts = 0;
	for (int i = 0; i << mu2e_channel_info_[chn][C2S].num_buffs; ++i)
	{
		retsts = retsts || read_release(chn, i);
	}
	return retsts;
}

unsigned mu2edev::delta_(int chn, int dir)
{
	unsigned hw = mu2e_channel_info_[chn][dir].hwIdx;
	unsigned sw = mu2e_channel_info_[chn][dir].swIdx;
	return ((hw >= sw)
		? hw - sw
		: mu2e_channel_info_[chn][C2S].num_buffs + hw - sw);
}
