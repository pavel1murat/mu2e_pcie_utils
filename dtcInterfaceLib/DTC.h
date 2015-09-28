#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include <vector>
#include <cstdlib>
#include <atomic>
#include "mu2edev.hh"

namespace DTCLib {
class DTC
{
public:
	DTC(DTC_SimMode mode = DTC_SimMode_Disabled);
	virtual ~DTC() = default;

	DTC_SimMode ReadSimMode() { return simMode_; }
	DTC_SimMode SetSimMode(DTC_SimMode mode);

	double GetDeviceTime() { return deviceTime_ / 1000000000.0; }
	void ResetDeviceTime() { deviceTime_ = 0; }

	//
	// DMA Functions
	//
	// Data read-out
	std::vector<void*> GetData(DTC_Timestamp when = DTC_Timestamp());
	std::string GetJSONData(DTC_Timestamp when = DTC_Timestamp());
	mu2edev GetDevice() { return device_; }

	// DCS Read/Write Cycle
	void DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t *dataIn);

	// Broadcast Readout
	void SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet = true);

	// For loopback testing...
	void SetFirstRead(bool read) { first_read_ = read; }
	void WriteDMADAQPacket(const DTC_DMAPacket& packet);
	void WriteDMADCSPacket(const DTC_DMAPacket& packet);
	DTC_DataHeaderPacket* ReadNextDAQPacket(int tmo_ms = 0);
	DTC_DCSReplyPacket* ReadNextDCSPacket();
	void ReleaseAllBuffers(const DTC_DMA_Engine& channel)
	{
		auto start = std::chrono::high_resolution_clock::now();
		device_.release_all(channel);
		deviceTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>
			(std::chrono::high_resolution_clock::now() - start).count();
	}

	//
	// Register IO Functions
	//
	std::string RegDump();
	std::string RingRegDump(const DTC_Ring_ID& ring, std::string id);
	std::string CFORegDump();
	std::string ConsoleFormatRegDump();
	std::string FormatRegister(const DTC_Register& address);
	std::string RegisterRead(const DTC_Register& address);

	std::string ReadDesignVersion();
	std::string ReadDesignDate();
	std::string ReadDesignVersionNumber();

	void ResetDTC();
	bool ReadResetDTC();

	void EnableCFOEmulation();
	void DisableCFOEmulation();
	bool ReadCFOEmulation();

	void ResetSERDESOscillator();
	bool ReadResetSERDESOscillator();

	void ToggleSERDESOscillatorClock();
	bool ReadSERDESOscillatorClock();

	bool SetExternalSystemClock();
	bool SetInternalSystemClock();
	bool ToggleSystemClockEnable();
	bool ReadSystemClock();

	bool EnableTiming();
	bool DisableTiming();
	bool ToggleTimingEnable();
	bool ReadTimingEnable();

	int SetTriggerDMATransferLength(uint16_t length);
	uint16_t ReadTriggerDMATransferLength();

	int SetMinDMATransferLength(uint16_t length);
	uint16_t ReadMinDMATransferLength();

	DTC_SERDESLoopbackMode SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode);
	DTC_SERDESLoopbackMode ReadSERDESLoopback(const DTC_Ring_ID& ring);

	bool ReadSERDESOscillatorIICError();
	bool ReadSERDESOscillatorInitializationComplete();

	bool EnableROCEmulator(const DTC_Ring_ID& ring);
	bool DisableROCEmulator(const DTC_Ring_ID& ring);
	bool ToggleROCEmulator(const DTC_Ring_ID& ring);
	bool ReadROCEmulator(const DTC_Ring_ID& ring);

	DTC_RingEnableMode EnableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode(), const DTC_ROC_ID& lastRoc = DTC_ROC_Unused);
	DTC_RingEnableMode DisableRing(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
	DTC_RingEnableMode ToggleRingEnabled(const DTC_Ring_ID& ring, const DTC_RingEnableMode& mode = DTC_RingEnableMode());
	DTC_RingEnableMode ReadRingEnabled(const DTC_Ring_ID& ring);

	bool ResetSERDES(const DTC_Ring_ID& ring, int interval = 100);
	bool ReadResetSERDES(const DTC_Ring_ID& ring);
	bool ReadResetSERDESDone(const DTC_Ring_ID& ring);

	DTC_SERDESRXDisparityError ReadSERDESRXDisparityError(const DTC_Ring_ID& ring);
	DTC_SERDESRXDisparityError ClearSERDESRXDisparityError(const DTC_Ring_ID& ring);
	DTC_CharacterNotInTableError ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);
	DTC_CharacterNotInTableError ClearSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring);

	bool ReadSERDESUnlockError(const DTC_Ring_ID& ring);
	bool ClearSERDESUnlockError(const DTC_Ring_ID& ring);
	bool ReadSERDESPLLLocked(const DTC_Ring_ID& ring);
	bool ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring);
	bool ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring);

	DTC_RXBufferStatus ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring);

	DTC_RXStatus ReadSERDESRXStatus(const DTC_Ring_ID& ring);

	bool ReadSERDESEyescanError(const DTC_Ring_ID& ring);
	bool ClearSERDESEyescanError(const DTC_Ring_ID& ring);
	bool ReadSERDESRXCDRLock(const DTC_Ring_ID& ring);

	int WriteDMATimeoutPreset(uint32_t preset);
	uint32_t ReadDMATimeoutPreset();
	int WriteDataPendingTimer(uint32_t timer);
	uint32_t ReadDataPendingTimer();
	int SetPacketSize(uint16_t packetSize);
	uint16_t ReadPacketSize();

	// Set number of ROCs in a Ring
	DTC_ROC_ID SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc);
	DTC_ROC_ID ReadRingROCCount(const DTC_Ring_ID& ring, bool local = true);

	DTC_Timestamp WriteTimestampPreset(const DTC_Timestamp& preset);
	DTC_Timestamp ReadTimestampPreset();

	DTC_FIFOFullErrorFlags WriteFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
	DTC_FIFOFullErrorFlags ToggleFIFOFullErrorFlags(const DTC_Ring_ID& ring, const DTC_FIFOFullErrorFlags& flags);
	DTC_FIFOFullErrorFlags ReadFIFOFullErrorFlags(const DTC_Ring_ID& ring);

	void SetCFOEmulationTimestamp(const DTC_Timestamp& ts);
	DTC_Timestamp ReadCFOEmulationTimestamp();
	void SetCFOEmulationNumPackets(uint16_t numPackets) { WriteRegister(numPackets, DTC_Register_CFOEmulationNumPackets); }
	uint16_t ReadCFOEmulationNumPackets() { return static_cast<uint16_t>(ReadRegister(DTC_Register_CFOEmulationNumPackets)); }
	void SetCFOEmulationNumRequests(uint32_t numRequests) { WriteRegister(numRequests, DTC_Register_CFOEmulationNumRequests); }
	uint32_t ReadCFOEmulationNumRequests() { return ReadRegister(DTC_Register_CFOEmulationNumRequests); }
	void SetCFOEmulationRequestInterval(uint32_t interval) { WriteRegister(interval, DTC_Register_CFOEmulationRequestInterval); }
	uint32_t ReadCFOEmulationRequestInterval() { return ReadRegister(DTC_Register_CFOEmulationRequestInterval); }

	uint32_t ReadROCTimeoutPreset();
	int WriteROCTimeoutPreset(uint32_t preset);

	bool ReadROCTimeoutError(const DTC_Ring_ID& ring);
	bool ClearROCTimeoutError(const DTC_Ring_ID& ring);

	bool ReadRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
	bool ClearRXElasticBufferUnderrun(const DTC_Ring_ID& ring);
	bool ReadRXElasticBufferOverrun(const DTC_Ring_ID& ring);
	bool ClearRXElasticBufferOverrun(const DTC_Ring_ID& ring);
	bool ReadPacketError(const DTC_Ring_ID& ring);
	bool ClearPacketError(const DTC_Ring_ID& ring);
	bool ReadPacketCRCError(const DTC_Ring_ID& ring);
	bool ClearPacketCRCError(const DTC_Ring_ID& ring);

	bool ReadFPGAPROMProgramFIFOFull();
	bool ReadFPGAPROMReady();

	void ReloadFPGAFirmware();
	bool ReadFPGACoreAccessFIFOFull();
	bool ReadFPGACoreAccessFIFOEmpty();

private:
	void ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms = 0);
	void WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet);
	void WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet);

	void WriteRegister(uint32_t data, const DTC_Register& address);
	uint32_t ReadRegister(const DTC_Register& address);

private:
	mu2edev device_;
	mu2e_databuff_t* daqbuffer_;
	int buffers_used_;
	mu2e_databuff_t* dcsbuffer_;
	DTC_SimMode simMode_;
	DTC_ROC_ID maxROCs_[6];
	uint16_t dmaSize_;
	uint32_t bufferIndex_;
	bool first_read_;
	uint16_t daqDMAByteCount_;
	uint16_t dcsDMAByteCount_;
	void* lastReadPtr_;
	void* nextReadPtr_;
	void* dcsReadPtr_;
	std::atomic<long long> deviceTime_;
};
}
#endif
