// mu2esim.hh: mu2e DTC simulator main file
//
// Eric Flumerfelt
// January 27, 2015
//
#ifndef MU2ESIM_HH
#define MU2ESIM_HH 1

#include <cstdint>
#ifndef _WIN32
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h" //
#else
#include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#endif
#include <unordered_map>
#include <mutex>
#include <set>
#include <map>
#include <thread>
#include <queue>
#include "DTC_Types.h"

#define SIM_BUFFCOUNT 4U

struct mu2esim
{
	mu2esim();
	~mu2esim();
	int init(DTCLib::DTC_SimMode mode = DTCLib::DTC_SimMode_Tracker);
	int read_data(int chn, void **buffer, int tmo_ms); // return bytes read; error if negative
	int write_data(int chn, void *buffer, size_t bytes);
	int read_release(int chn, unsigned num);
	int release_all(int chn);
	int read_register(uint16_t address, int tmo_ms, uint32_t *output);
	int write_register(uint16_t address, int tmo_ms, uint32_t data);
private:
	unsigned delta_(int chn, int dir);
	void clearBuffer_(int chn, bool increment = true);
	void CFOEmulator_();

	//const DTCLib::DTC_Timestamp NULL_TIMESTAMP = DTCLib::DTC_Timestamp(0xffffffffffffffff);
	std::unordered_map<uint16_t, uint32_t> registers_;
	unsigned hwIdx_[MU2E_MAX_CHANNELS];
	unsigned swIdx_[MU2E_MAX_CHANNELS];
	uint64_t buffSize_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
	mu2e_databuff_t* dmaData_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
	std::queue<mu2e_databuff_t*> loopbackData_;
	//mu2e_databuff_t* dmaDAQData_[SIM_BUFFCOUNT];
	//mu2e_databuff_t* dmaDCSData_[SIM_BUFFCOUNT];
	DTCLib::DTC_SimMode mode_;
	uint16_t simIndex_[6][6];
	bool dcsRequestReceived_[6][6];
	std::set<uint64_t> readoutRequestReceived_[6];
	std::mutex rrMutex_;
	std::map<uint64_t, uint16_t> dataRequestReceived_[6][6];
	std::mutex drMutex_;
	DTCLib::DTC_DCSRequestPacket dcsRequest_[6][6];
	std::thread cfoEmulatorThread_;
	bool cancelCFO_;
};

#endif
