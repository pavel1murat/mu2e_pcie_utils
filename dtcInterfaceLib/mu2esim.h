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
#include "DTC_Types.h"
#include "DTC_Packets.h"

#define SIM_BUFFCOUNT 400U

class DDRSimulator
{
public:
	DDRSimulator() : mutex_(), queue_() {}

	virtual ~DDRSimulator()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		while (!queue_.empty())
		{
			queue_.pop();
		}
	}

	void push(std::shared_ptr<std::vector<uint8_t>> buf)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(buf);
	}

	bool empty()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	std::shared_ptr<std::vector<uint8_t>> pop(bool recycle)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto vec = queue_.front();
		queue_.pop();
		if (recycle) queue_.push(vec);
		return vec;
	}

private:
	std::mutex mutex_;
	std::queue<std::shared_ptr<std::vector<uint8_t>>> queue_;
};

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
	static unsigned delta_(int chn, int dir);
	static void clearBuffer_(int chn, bool increment = true);
	void CFOEmulator_();
	void packetSimulator_(DTCLib::DTC_Timestamp ts, DTCLib::DTC_Ring_ID ring, DTCLib::DTC_ROC_ID roc, uint16_t packetCount);
	void closeBuffer_(bool drop, DTCLib::DTC_Timestamp ts);
	void dcsPacketSimulator_(DTCLib::DTC_DCSRequestPacket in);

	std::unordered_map<uint16_t, uint32_t> registers_;
	unsigned swIdx_[MU2E_MAX_CHANNELS];
	uint32_t detSimLoopCount_;
	mu2e_databuff_t* dmaData_[MU2E_MAX_CHANNELS][SIM_BUFFCOUNT];
	DTCLib::DTC_SimMode mode_;
	uint16_t simIndex_[6][6];
	std::thread cfoEmulatorThread_;
	bool cancelCFO_;

	typedef bool readoutRequestData[6];
	std::map<uint64_t, readoutRequestData> readoutRequestReceived_;

	DDRSimulator ddrSim_;
	DDRSimulator dcsResponses_;

	size_t currentOffset_;
	DTCLib::DTC_Timestamp currentTimestamp_;
	std::shared_ptr<std::vector<uint8_t>> currentBuffer_;
};

#endif

