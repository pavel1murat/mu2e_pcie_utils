// mu2esim.hh: mu2e DTC simulator main file
//
// Eric Flumerfelt
// January 27, 2015
//
#ifndef MU2ESIM_HH
#define MU2ESIM_HH 1

#ifndef _WIN32
#include "mu2e_driver/mu2e_mmap_ioctl.h" //
#else
#include "../mu2e_driver/mu2e_mmap_ioctl.h"
#endif
#include <unordered_map>
#include <mutex>
#include <map>
#include <thread>
#include <memory>
#include <queue>
#include <fstream>
#include "DTC_Types.h"
#include "DTC_Packets.h"

#define SIM_BUFFCOUNT 40U

/// <summary>
/// The mu2esim class emulates a DTC in software. It can be used for hardware-independent testing of software, especially higher-level trigger algorithms.
/// </summary>
class mu2esim
{
public:
	mu2esim();
	~mu2esim();
	int init(DTCLib::DTC_SimMode mode = DTCLib::DTC_SimMode_Tracker);
	int read_data(int chn, void** buffer, int tmo_ms); // return bytes read; error if negative
	int write_data(int chn, void* buffer, size_t bytes);
	int read_release(int chn, unsigned num);
	int release_all(int chn);
	int read_register(uint16_t address, int tmo_ms, uint32_t* output);
	int write_register(uint16_t address, int tmo_ms, uint32_t data);
private:
	unsigned delta_(int chn, int dir);
	static void clearBuffer_(int chn, bool increment = true);
	void openEvent_(DTCLib::DTC_Timestamp ts);
	void closeEvent_();
	void CFOEmulator_();
	void packetSimulator_(DTCLib::DTC_Timestamp ts, DTCLib::DTC_Ring_ID ring, DTCLib::DTC_ROC_ID roc, uint16_t packetCount);
	void dcsPacketSimulator_(DTCLib::DTC_DCSRequestPacket in);

	std::unordered_map<uint16_t, uint32_t> registers_;
	unsigned swIdx_[MU2E_MAX_CHANNELS];
	unsigned hwIdx_[MU2E_MAX_CHANNELS];
	uint32_t detSimLoopCount_;
	mu2e_databuff_t* dmaData_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
	std::fstream ddrFile_;
	DTCLib::DTC_SimMode mode_;
	uint16_t simIndex_[6][6];
	std::thread cfoEmulatorThread_;
	bool cancelCFO_;

	typedef std::bitset<6> readoutRequestData;
	std::map<uint64_t, readoutRequestData> readoutRequestReceived_;
	
	DTCLib::DTC_Timestamp currentTimestamp_;
	uint64_t currentEventSize_;
	std::streampos eventBegin_;
};

#endif

