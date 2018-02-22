// This file (cfodev.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

/*
 *    make cfodev.o CFLAGS='-g -Wall -std=c++0x'
 */

#include <trace.h>
#include <chrono>
#include "cfodev.h"

cfodev::cfodev() : devfd_(0), buffers_held_(0), simulator_(nullptr), deviceTime_(0LL), writeSize_(0), readSize_(0)
{
	//TRACE_CNTL( "lvlmskM", 0x3 );
	//TRACE_CNTL( "lvlmskS", 0x3 );
}

cfodev::~cfodev()
{
	delete simulator_;
}

int cfodev::init(CFOLib::CFO_SimMode simMode)
{
	auto start = std::chrono::steady_clock::now();
	if (simMode != CFOLib::CFO_SimMode_Disabled && simMode != CFOLib::CFO_SimMode_NoCFO
		&& simMode != CFOLib::CFO_SimMode_ROCEmulator && simMode != CFOLib::CFO_SimMode_Loopback)
	{
		simulator_ = new cfosim();
		simulator_->init(simMode);
	}
	else
	{
		if (simulator_ != nullptr)
		{
			delete simulator_;
			simulator_ = nullptr;
		}
		int sts;
		devfd_ = open("/dev/" CFO_DEV_FILE, O_RDWR);
		if (devfd_ == -1 || devfd_ == 0)
		{
			perror("open /dev/" CFO_DEV_FILE);
			TRACE(1, "cfo Device file not found and CFOLIB_SIM_ENABLE not set! Exiting.");
			exit(1);
		}
		for (unsigned chn = 0; chn < CFO_MAX_CHANNELS; ++chn)
			for (unsigned dir = 0; dir < 2; ++dir)
			{
				m_ioc_get_info_t get_info;
				get_info.chn = chn; get_info.dir = dir; get_info.tmo_ms = 0;
				TRACE(17, "cfodev::init before ioctl( devfd_, M_IOC_GET_INFO, &get_info ) chn=%u dir=%u", chn, dir);
				sts = ioctl(devfd_, M_IOC_GET_INFO, &get_info);
				if (sts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
				cfo_channel_info_[chn][dir] = get_info;
				TRACE(4, "cfodev::init %u:%u - num=%u size=%u hwIdx=%u, swIdx=%u delta=%u"
					  , chn, dir
					  , get_info.num_buffs, get_info.buff_size
					  , get_info.hwIdx, get_info.swIdx
					  , cfo_chn_info_delta_(chn, dir, &cfo_channel_info_));
				for (unsigned map = 0; map < 2; ++map)
				{
					size_t length = get_info.num_buffs * ((map == CFO_MAP_BUFF)
														  ? get_info.buff_size
														  : sizeof(int));
					//int prot = (((dir == S2C) && (map == CFO_MAP_BUFF))? PROT_WRITE : PROT_READ);
					int prot = (((map == CFO_MAP_BUFF)) ? PROT_WRITE : PROT_READ);
					off64_t offset = chnDirMap2offset(chn, dir, map);
					cfo_mmap_ptrs_[chn][dir][map]
						= mmap(0 /* hint address */
							   , length, prot, MAP_SHARED, devfd_, offset);
					if (cfo_mmap_ptrs_[chn][dir][map] == MAP_FAILED)
					{
						perror("mmap"); exit(1);
					}
					TRACE(4, "cfodev::init chnDirMap2offset=%lu cfo_mmap_ptrs_[%d][%d][%d]=%p p=%c l=%lu"
						  , offset
						  , chn, dir, map
						  , cfo_mmap_ptrs_[chn][dir][map]
						  , prot == PROT_READ ? 'R' : 'W'
						  , length);
				}
				if (dir == CFO_DMA_Direction_C2S)
				{
					release_all(chn);
				}

				// Reset the CFO
				//{
				//	write_register(0x9100, 0, 0xa0000000);
				//	write_register(0x9118, 0, 0x0000003f);
				//	write_register(0x9100, 0, 0x00000000);
				//	write_register(0x9100, 0, 0x10000000);
				//	write_register(0x9100, 0, 0x30000000);
				//	write_register(0x9100, 0, 0x10000000);
				//	write_register(0x9118, 0, 0x00000000);
				//}

				// Enable DMA Engines
				{
					//uint16_t addr = CFO_Register_Engine_Control(chn, dir);
					//TRACE(17, "cfodev::init write Engine_Control reg 0x%x", addr);
					//write_register(addr, 0, 0x100);//bit 8 enable=1
				}
			}
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	return simMode;
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
   */
int cfodev::read_data(int chn, void** buffer, int tmo_ms)
{
	auto start = std::chrono::steady_clock::now();
	auto retsts = -1;
	if (simulator_ != nullptr)
	{
		retsts = simulator_->read_data(chn, buffer, tmo_ms);
	}
	else
	{
		retsts = 0;
		unsigned has_recv_data;
		TRACE(18, "cfodev::read_data before (cfo_mmap_ptrs_[0][0][0]!=NULL) || ((retsts=init())==0)");
		if ((cfo_mmap_ptrs_[0][0][0] != NULL) || ((retsts = init()) == 0))
		{
			has_recv_data = cfo_chn_info_delta_(chn, C2S, &cfo_channel_info_);
			TRACE(18, "cfodev::read_data after %u=has_recv_data = delta_( chn, C2S )", has_recv_data);
			cfo_channel_info_[chn][C2S].tmo_ms = tmo_ms; // in case GET_INFO is called
			if ((has_recv_data > 0)
				|| ((retsts = ioctl(devfd_, M_IOC_GET_INFO, &cfo_channel_info_[chn][C2S])) == 0
					&& (has_recv_data = cfo_chn_info_delta_(chn, C2S, &cfo_channel_info_)) > 0))
			{   // have data
	// get byte count from new/next
				unsigned newNxtIdx = idx_add(cfo_channel_info_[chn][C2S].swIdx, 1, chn, C2S);
				int *    BC_p = (int*)cfo_mmap_ptrs_[chn][C2S][CFO_MAP_META];
				retsts = BC_p[newNxtIdx];
				*buffer = ((cfo_databuff_t*)(cfo_mmap_ptrs_[chn][C2S][CFO_MAP_BUFF]))[newNxtIdx];
				TRACE(3, "cfodev::read_data chn%d hIdx=%u, sIdx=%u "
					  "%u hasRcvDat=%u %p[newNxtIdx=%d]=retsts=%d buf(%p)[0]=0x%08x"
					  , chn
					  , cfo_channel_info_[chn][C2S].hwIdx
					  , cfo_channel_info_[chn][C2S].swIdx
					  , cfo_channel_info_[chn][C2S].num_buffs, has_recv_data
					  , (void*)BC_p, newNxtIdx, retsts, *buffer, *(uint32_t*)*buffer);
				++buffers_held_;
			}
			else
			{   // was it a tmo or error
				if (retsts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
				TRACE(18, "cfodev::read_data not error... return 0 status");
			}
		}
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	if (retsts > 0) readSize_ += retsts;
	return retsts;
} // read_data

/* read_release
   release a number of buffers (usually 1)
   */
int cfodev::read_release(int chn, unsigned num)
{
	auto start = std::chrono::steady_clock::now();
	auto retsts = -1;
	if (simulator_ != nullptr)
	{
		retsts = simulator_->read_release(chn, num);
	}
	else
	{
		retsts = 0;
		unsigned long arg;
		unsigned has_recv_data;
		has_recv_data = cfo_chn_info_delta_(chn, C2S, &cfo_channel_info_);
		if (num <= has_recv_data)
		{
			arg = (chn << 24) | (C2S << 16) | (num & 0xffff);// THIS OBIVOUSLY SHOULD BE A MACRO
			retsts = ioctl(devfd_, M_IOC_BUF_GIVE, arg);
			if (retsts != 0) { perror("M_IOC_BUF_GIVE"); }//exit(1); } // Don't exit for now

	// increment our cached info
			cfo_channel_info_[chn][C2S].swIdx
				= idx_add(cfo_channel_info_[chn][C2S].swIdx, (int)num, chn, C2S);
			if (num <= buffers_held_)
				buffers_held_ -= num;
			else
				buffers_held_ = 0;
		}
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	return retsts;
}

int cfodev::read_register(uint16_t address, int tmo_ms, uint32_t* output)
{
	auto start = std::chrono::steady_clock::now();
	if (simulator_ != nullptr)
	{
		return simulator_->read_register(address, tmo_ms, output);
	}
	m_ioc_reg_access_t reg;
	reg.reg_offset = address;
	reg.access_type = 0;
	int errorCode = ioctl(devfd_, M_IOC_REG_ACCESS, &reg);
	*output = reg.val;
	TRACE(24, "Read value 0x%x from register 0x%x", reg.val, address);
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	return errorCode;
}

int cfodev::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	auto start = std::chrono::steady_clock::now();
	auto retsts = -1;
	if (simulator_ != nullptr)
	{
		retsts = simulator_->write_register(address, tmo_ms, data);
	}
	else
	{
		m_ioc_reg_access_t reg;
		reg.reg_offset = address;
		reg.access_type = 1;
		reg.val = data;
		TRACE(24, "Writing value 0x%x to register 0x%x", data, address);
		retsts = ioctl(devfd_, M_IOC_REG_ACCESS, &reg);
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	return retsts;
}

void cfodev::meta_dump(int chn, int dir)
{
	TRACE(17, "cfodev::meta_dump: chn=%i, dir=%i", chn, dir);
	auto start = std::chrono::steady_clock::now();
	if (simulator_ == nullptr)
	{
		int retsts = 0;
		if ((cfo_mmap_ptrs_[0][0][0] != NULL) || ((retsts = init()) == 0))
		{
			for (unsigned buf = 0; buf < cfo_channel_info_[chn][dir].num_buffs; ++buf)
			{
				int *    BC_p = (int*)cfo_mmap_ptrs_[chn][dir][CFO_MAP_META];
				printf("buf_%02d: %u\n", buf, BC_p[buf]);
			}
		}
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
}

int cfodev::write_data(int chn, void* buffer, size_t bytes)
{
	auto start = std::chrono::steady_clock::now();
	auto retsts = -1;
	if (simulator_ != nullptr)
	{
		retsts = simulator_->write_data(chn, buffer, bytes);
	}
	else
	{
		int dir = S2C;
		retsts = 0;
		unsigned delta = cfo_chn_info_delta_(chn, dir, &cfo_channel_info_); // check cached info
		TRACE(3, "write_data delta=%u chn=%d dir=S2C", delta, chn);
		while (delta <= 1 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < 1000)
		{
			m_ioc_get_info_t get_info;
			get_info.chn = chn; get_info.dir = dir; get_info.tmo_ms = 0;
			int sts = ioctl(devfd_, M_IOC_GET_INFO, &get_info);
			if (sts != 0) { perror("M_IOC_GET_INFO"); exit(1); }
			cfo_channel_info_[chn][dir] = get_info; // copy info struct
			delta = cfo_chn_info_delta_(chn, dir, &cfo_channel_info_);
			usleep(1000);
		}

		if (delta <= 1)
		{
			TRACE( 0, "HW_NOT_READING_BUFS");
			perror("HW_NOT_READING_BUFS");
			exit(1);
		}

		unsigned idx = cfo_channel_info_[chn][dir].swIdx;
		void * data = ((cfo_databuff_t*)(cfo_mmap_ptrs_[chn][dir][CFO_MAP_BUFF]))[idx];
		memcpy(data, buffer, bytes);
		unsigned long arg = (chn << 24) | (bytes & 0xffffff);// THIS OBIVOUSLY SHOULD BE A MACRO

		int retry = 15;
		do
		{
			retsts = ioctl(devfd_, M_IOC_BUF_XMIT, arg);
			if (retsts != 0)
			{
			TRACE(3, "write_data ioctl returned %d, errno=%d (%s), retrying.", retsts,errno, strerror(errno));
				//perror("M_IOC_BUF_XMIT");
				usleep(50000);
			} // exit(1); } // Take out the exit call for now
			retry--;
		} while (retry > 0 && retsts != 0);
// increment our cached info
		if (retsts == 0)
		{
			cfo_channel_info_[chn][dir].swIdx
				= idx_add(cfo_channel_info_[chn][dir].swIdx, 1, chn, dir);
		}
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	if (retsts >= 0) writeSize_ += bytes;
	return retsts;
} // write_data

// applicable for recv.
int cfodev::release_all(int chn)
{
	auto start = std::chrono::steady_clock::now();
	auto retsts = 0;
	if (simulator_ != nullptr)
	{
		retsts = simulator_->release_all(chn);
	}
	else
	{
		auto has_recv_data = cfo_chn_info_delta_(chn, C2S, &cfo_channel_info_);
		if (has_recv_data) read_release(chn, has_recv_data);
	}
	deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
		(std::chrono::steady_clock::now() - start).count();
	return retsts;
}

void cfodev::close()
{
	if (simulator_ != nullptr)
	{
		delete simulator_;
		simulator_ = nullptr;
	}
}
