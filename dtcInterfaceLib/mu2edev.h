// This file (mu2edev.hh) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 13, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#ifndef _WIN32
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h" //
#else
#include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#endif

#include <atomic>
#include "mu2esim.h"

class mu2edev
{
public:
	mu2edev();

	double GetDeviceTime() const
	{
		return deviceTime_ / 1000000000.0;
	}

	void ResetDeviceTime()
	{
		deviceTime_ = 0;
	}

	size_t GetWriteSize() const
	{
		return writeSize_;
	}

	void ResetWriteSize()
	{
		writeSize_ = 0;
	}

	size_t GetReadSize() const
	{
		return readSize_;
	}

	void ResetReadSize()
	{
		readSize_ = 0;
	}

	int init(DTCLib::DTC_SimMode simMode = DTCLib::DTC_SimMode_Disabled);
	int read_data(int chn, void** buffer, int tmo_ms); // return bytes read; error if negative
	int read_release(int chn, unsigned num);
	int release_all(int chn);
	int read_register(uint16_t address, int tmo_ms, uint32_t* output);
	int write_register(uint16_t address, int tmo_ms, uint32_t data);
	void meta_dump(int chn, int dir);
	int write_data(int chn, void* buffer, size_t bytes);
	void close();
	//int  read_pcie_state(m_ioc_pcistate_t *output);
	//int  read_dma_state(int chn, int dir, m_ioc_engstate_t *output);
	//int  read_dma_stats(m_ioc_engstats_t *output);
	//int  read_trn_stats(TRNStatsArray *output);
	//int  read_test_command(m_ioc_cmd_t *output);
	//int  write_test_command(m_ioc_cmd_t input, bool start);

private:
	//unsigned delta_(int chn, int dir);

	int devfd_;
	volatile void* mu2e_mmap_ptrs_[MU2E_MAX_CHANNELS][2][2];
	m_ioc_get_info_t mu2e_channel_info_[MU2E_MAX_CHANNELS][2];
	unsigned buffers_held_;
	mu2esim* simulator_;
	std::atomic<long long> deviceTime_;
	std::atomic<size_t> writeSize_;
	std::atomic<size_t> readSize_;
};

