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
# ifndef TRACE
#  include <stdio.h>
#  ifdef _DEBUG
#   define TRACE(lvl,...) printf(__VA_ARGS__); printf("\n")
#  else
#   define TRACE(...)
#  endif
# endif
# pragma warning(disable: 4351)
#endif
#include "mu2edev.hh"

mu2edev::mu2edev() : devfd_(0), buffers_held_(0), simulator_(nullptr)
{
	//TRACE_CNTL( "lvlmskM", 0x3 );
	//TRACE_CNTL( "lvlmskS", 0x3 );
}

int mu2edev::init(DTCLib::DTC_SimMode simMode)
{
	if (simMode != DTCLib::DTC_SimMode_Disabled && simMode != DTCLib::DTC_SimMode_NoCFO
		&& simMode != DTCLib::DTC_SimMode_ROCEmulator && simMode != DTCLib::DTC_SimMode_Loopback)
	{
		simulator_ = new mu2esim();
		simulator_->init(simMode);
	}
	else
	{
		if (simulator_ != nullptr)
		{
			delete simulator_;
			simulator_ = nullptr;
		}
#ifndef _WIN32
		int sts;
		devfd_ = open("/dev/" MU2E_DEV_FILE, O_RDWR);
		if (devfd_ == -1 || devfd_ == 0)
		{
			perror("open /dev/" MU2E_DEV_FILE);
			TRACE(1, "mu2e Device file not found and DTCLIB_SIM_ENABLE not set! Exiting.");
			exit(1);
		}
		for (unsigned chn = 0; chn < MU2E_MAX_CHANNELS; ++chn)
			for (unsigned dir = 0; dir < 2; ++dir)
			{
				m_ioc_get_info_t get_info;
				get_info.chn = chn; get_info.dir = dir; get_info.tmo_ms = 0;
				TRACE(17, "mu2edev::init before ioctl( devfd_, M_IOC_GET_INFO, &get_info ) chn=%u dir=%u", chn, dir);
				sts = ioctl(devfd_, M_IOC_GET_INFO, &get_info);
				if (sts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
				mu2e_channel_info_[chn][dir] = get_info;
				TRACE(4, "mu2edev::init %u:%u - num=%u size=%u hwIdx=%u, swIdx=%u delta=%u"
					  , chn, dir
					  , get_info.num_buffs, get_info.buff_size
					  , get_info.hwIdx, get_info.swIdx
					  , mu2e_chn_info_delta_(chn, dir, &mu2e_channel_info_));
				for (unsigned map = 0; map < 2; ++map)
				{
					size_t length = get_info.num_buffs * ((map == MU2E_MAP_BUFF)
														  ? get_info.buff_size
														  : sizeof(int));
					//int prot = (((dir == S2C) && (map == MU2E_MAP_BUFF))? PROT_WRITE : PROT_READ);
					int prot = (((map == MU2E_MAP_BUFF)) ? PROT_WRITE : PROT_READ);
					off64_t offset = chnDirMap2offset(chn, dir, map);
					mu2e_mmap_ptrs_[chn][dir][map]
						= mmap(0 /* hint address */
							   , length, prot, MAP_SHARED, devfd_, offset);
					if (mu2e_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
					{
						perror("mmap"); exit(1);
					}
					TRACE(4, "mu2edev::init chnDirMap2offset=%lu mu2e_mmap_ptrs_[%d][%d][%d]=%p p=%c l=%lu"
						  , offset
						  , chn, dir, map
						  , mu2e_mmap_ptrs_[chn][dir][map]
						  , prot == PROT_READ ? 'R' : 'W'
						  , length);
				}
				if (dir == DTC_DMA_Direction_C2S)
				{
					release_all(chn);
				}

				// Enable DMA Engines
				{
					uint16_t addr = DTC_Register_Engine_Control(chn, dir);
					TRACE(17, "mu2edev::init write Engine_Control reg 0x%x", addr);
					write_register(DTC_Register_Engine_Control(chn, dir), 0, 0x100);//bit 8 enable=1
				}
			}
#endif
	}
	return (simMode);
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
   */
int mu2edev::read_data(int chn, void **buffer, int tmo_ms)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_data(chn, buffer, tmo_ms);
	}
	else
	{
#ifdef _WIN32
		int retsts = -1;
#else
		int retsts = 0;
		unsigned has_recv_data;
		TRACE(18, "mu2edev::read_data before (mu2e_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0)");
		if ((mu2e_mmap_ptrs_[0][0][0] != NULL) || ((retsts = init()) == 0))
		{
			if (buffers_held_ >= 2)
				read_release(chn, 1);
			has_recv_data = mu2e_chn_info_delta_(chn, C2S, &mu2e_channel_info_);
			TRACE(18, "mu2edev::read_data after %u=has_recv_data = delta_( chn, C2S )", has_recv_data);
			mu2e_channel_info_[chn][C2S].tmo_ms = tmo_ms; // in case GET_INFO is called
			if ((has_recv_data > 0)
				|| ((retsts = ioctl(devfd_, M_IOC_GET_INFO, &mu2e_channel_info_[chn][C2S])) == 0
					&& (has_recv_data = mu2e_chn_info_delta_(chn, C2S, &mu2e_channel_info_)) > 0))
			{   // have data
// get byte count from new/next sw
				unsigned newNxtIdx = idx_add(mu2e_channel_info_[chn][C2S].swIdx, 1, chn, C2S);
				int *    BC_p = (int*)mu2e_mmap_ptrs_[chn][C2S][MU2E_MAP_META];
				retsts = BC_p[newNxtIdx];
				*buffer = ((mu2e_databuff_t*)(mu2e_mmap_ptrs_[chn][C2S][MU2E_MAP_BUFF]))[newNxtIdx];
				TRACE(3, "mu2edev::read_data chn%d hIdx=%u, sIdx=%u "
					  "%u hasRcvDat=%u %p[newNxtIdx=%d]=retsts=%d buf(%p)[0]=0x%08x"
					  , chn
					  , mu2e_channel_info_[chn][C2S].hwIdx
					  , mu2e_channel_info_[chn][C2S].swIdx
					  , mu2e_channel_info_[chn][C2S].num_buffs, has_recv_data
					  , (void*)BC_p, newNxtIdx, retsts, *buffer, *(uint32_t*)*buffer);
				++buffers_held_;
			}
			else
			{   // was it a tmo or error
				if (retsts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
				TRACE(18, "mu2edev::read_data not error... return 0 status");
			}
		}
#endif
		return (retsts);
	}
}   // read_data

/* read_release
   release a number of buffers (usually 1)
   */
int mu2edev::read_release(int chn, unsigned num)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_release(chn, num);
	}
	else
	{
#ifdef _WIN32
		int retsts = -1;
#else
		int retsts = 0;
		unsigned long arg;
		unsigned has_recv_data;
		has_recv_data = mu2e_chn_info_delta_(chn, C2S, &mu2e_channel_info_);
		if (num <= has_recv_data)
		{
			arg = (chn << 24) | (C2S << 16) | (num & 0xffff);// THIS OBIVOUSLY SHOULD BE A MACRO
			retsts = ioctl(devfd_, M_IOC_BUF_GIVE, arg);
			if (retsts != 0) { perror("M_IOC_BUF_GIVE"); exit(1); }

			// increment our cached info
			mu2e_channel_info_[chn][C2S].swIdx
				= idx_add(mu2e_channel_info_[chn][C2S].swIdx, (int)num, chn, C2S);
			if (num <= buffers_held_)
				buffers_held_ -= num;
			else
				buffers_held_ = 0;
		}
#endif
		return (retsts);
	}
}

int  mu2edev::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_register(address, tmo_ms, output);
	}
	else
	{
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
	if (simulator_ != nullptr)
	{
		return simulator_->write_register(address, tmo_ms, data);
	}
	else
	{
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
	if (simulator_ != nullptr)
	{
		return;
	}
	else
	{
#ifdef _WIN32
		TRACE(17, "mu2edev::meta_dump: chn=%i, dir=%i", chn, dir);
		return;
#else
		int retsts = 0;
		if ((mu2e_mmap_ptrs_[0][0][0] != NULL) || ((retsts = init()) == 0))
		{
			for (unsigned buf = 0; buf < mu2e_channel_info_[chn][dir].num_buffs; ++buf)
			{
				int *    BC_p = (int*)mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_META];
				printf("buf_%02d: %u\n", buf, BC_p[buf]);
			}
		}
#endif
	}
}

int mu2edev::write_data(int chn, void *buffer, size_t bytes)
{
	if (simulator_ != nullptr)
	{
		return simulator_->write_data(chn, buffer, bytes);
	}
	else
	{
#ifdef _WIN32
		int retsts = -1;
#else
		int dir = S2C;
		int retsts = 0;
		unsigned delta = mu2e_chn_info_delta_(chn, dir, &mu2e_channel_info_); // check cached info
		TRACE(3, "write_loopback_data delta=%u chn=%d dir=S2C", delta, chn);
		if (delta == 0)  // recheck with module
		{
			m_ioc_get_info_t get_info;
			get_info.chn = chn; get_info.dir = dir; get_info.tmo_ms = 0;
			int sts = ioctl(devfd_, M_IOC_GET_INFO, &get_info);
			if (sts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
			mu2e_channel_info_[chn][dir] = get_info; // copy info struct
			delta = mu2e_chn_info_delta_(chn, dir, &mu2e_channel_info_);
		}
		if (delta > 0)
		{
			unsigned idx = mu2e_channel_info_[chn][dir].swIdx;
			void * data = ((mu2e_databuff_t*)(mu2e_mmap_ptrs_[chn][dir][MU2E_MAP_BUFF]))[idx];
			memcpy(data, buffer, bytes);
			unsigned long arg = (chn << 24) | (bytes & 0xffffff);// THIS OBIVOUSLY SHOULD BE A MACRO
			retsts = ioctl(devfd_, M_IOC_BUF_XMIT, arg);
			if (retsts != 0) { perror("M_IOC_BUF_GIVE"); exit(1); }
			// increment our cached info
			mu2e_channel_info_[chn][dir].swIdx
				= idx_add(mu2e_channel_info_[chn][dir].swIdx, 1, chn, dir);
		}
#endif
		return retsts;
	}
}   // write_data

/*
int mu2edev::read_pcie_state(m_ioc_pcistate_t *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_pcie_state(output);
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
*/
/*
int mu2edev::read_dma_state(int chn, int dir, m_ioc_engstate_t *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_dma_state(chn, dir, output);
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
*/
/*
int mu2edev::read_dma_stats(m_ioc_engstats_t *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_dma_stats(output);
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
*/
/*
int mu2edev::read_trn_stats(TRNStatsArray *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_trn_stats(output);
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
*/
/*
int mu2edev::read_test_command(m_ioc_cmd_t *output)
{
	if (simulator_ != nullptr)
	{
		return simulator_->read_test_command(output);
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
*/
/*
int mu2edev::write_test_command(m_ioc_cmd_t input, bool start)
{
	if (simulator_ != nullptr)
	{
		return simulator_->write_test_command(input, start);
	}
	else {
#ifdef _WIN32
		return -1;
#else
		int error = 0;
		if (start) {
			error = ioctl(devfd_, M_IOC_TEST_START, &input);
		}
		else {
			error = ioctl(devfd_, M_IOC_TEST_STOP, &input);
		}
		if (error < 0)
		{
			printf("Error! : %s \n", strerror(errno));
		}

		return error;
#endif
	}
}
*/

// applicable for recv.
int mu2edev::release_all(int chn)
{
	if (simulator_ != nullptr)
	{
		return simulator_->release_all(chn);
	}
	else
	{
		int retsts = 0;
		unsigned has_recv_data = mu2e_chn_info_delta_(chn, C2S, &mu2e_channel_info_);
		if (has_recv_data) read_release(chn, has_recv_data);
		return retsts;
	}
}

#if 0
unsigned mu2edev::delta_(int chn, int dir)
{
	unsigned hw = mu2e_channel_info_[chn][dir].hwIdx;
	unsigned sw = mu2e_channel_info_[chn][dir].swIdx;
	TRACE(21, "mu2edev::delta_ chn=%d dir=%d hw=%u sw=%u num_buffs=%u"
		  , chn, dir, hw, sw, mu2e_channel_info_[chn][C2S].num_buffs);
	if (dir == C2S)
		return ((hw >= sw)
				? hw - sw
				: mu2e_channel_info_[chn][dir].num_buffs + hw - sw);
	else
		return ((sw >= hw)
				? mu2e_channel_info_[chn][dir].num_buffs - (sw - hw)
				: hw - sw);
}
#endif
