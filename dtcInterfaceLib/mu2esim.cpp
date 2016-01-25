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
# pragma warning(disable: 4351 6385 6386)
#endif
#include "mu2esim.h"
#include <vector>
#include <forward_list>
#include <cmath>
#include "DTC_Registers.h"
#include "DTC_Packets.h"

#define THREADED_CFO_EMULATOR 1

mu2esim::mu2esim()
	: registers_()
	, hwIdx_()
	, swIdx_()
	, detSimLoopCount_(0)
	, dmaData_()
	, mode_(DTCLib::DTC_SimMode_Disabled)
	, simIndex_()
	, cancelCFO_(true)
	, readoutRequestReceived_()
	, ddrSim_()
	, dcsResponses_()
	, currentOffset_(8)
	, currentBuffer_(reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t()))
{
	TRACE(17, "mu2esim::mu2esim BEGIN");
	hwIdx_[0] = 0;
	hwIdx_[1] = 0;
	swIdx_[0] = 0;
	swIdx_[1] = 0;
	for (unsigned ii = 0; ii < SIM_BUFFCOUNT; ++ii)
	{
		dmaData_[0][ii] = reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t());
		dmaData_[1][ii] = reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t());
	}
	release_all(0);
	release_all(1);
	for (auto ring = 0; ring < 6; ++ring)
	{
		for (auto roc = 0; roc < 6; ++roc)
		{
			simIndex_[ring][roc] = 0;
		}
	}
	TRACE(17, "mu2esim::mu2esim END");
}

mu2esim::~mu2esim()
{
	delete[] currentBuffer_;
	for (unsigned ii = 0; ii < MU2E_MAX_CHANNELS; ++ii)
	{
		for (unsigned jj = 0; jj < SIM_BUFFCOUNT; ++jj)
		{
			delete[] (dmaData_[ii][jj]);
		}
	}
	cancelCFO_ = true;
	if (cfoEmulatorThread_.joinable()) cfoEmulatorThread_.join();
}

int mu2esim::init(DTCLib::DTC_SimMode mode)
{
	TRACE(17, "mu2e Simulator::init");
	mode_ = mode;

	TRACE(17, "mu2esim::init Initializing registers");
	// Set initial register values...
	registers_[DTCLib::DTC_Register_DesignVersion] = 0x00006363; // v99.99
	registers_[DTCLib::DTC_Register_DesignDate] = 0x53494D44; // SIMD in ASCII
	registers_[DTCLib::DTC_Register_DesignStatus] = 0x9008;
	registers_[DTCLib::DTC_Register_PerfMonTXByteCount] = 0x00000010; // Send
	registers_[DTCLib::DTC_Register_PerfMonRXByteCount] = 0x00000040; // Recieve
	registers_[DTCLib::DTC_Register_PerfMonTXPayloadCount] = 0x00000100; // SPayload
	registers_[DTCLib::DTC_Register_PerfMonRXPayloadCount] = 0x00000400; // RPayload
	registers_[DTCLib::DTC_Register_PerfMonInitCDC] = 0x901C;
	registers_[DTCLib::DTC_Register_PerfMonInitCHC] = 0x9020;
	registers_[DTCLib::DTC_Register_PerfMonInitNPDC] = 0x9024;
	registers_[DTCLib::DTC_Register_PerfMonInitNPHC] = 0x9028;
	registers_[DTCLib::DTC_Register_PerfMonInitPDC] = 0x902C;
	registers_[DTCLib::DTC_Register_PerfMonInitPHC] = 0x9030;
	registers_[DTCLib::DTC_Register_DTCControl] = 0x00000003; // System Clock, Timing Enable
	registers_[DTCLib::DTC_Register_DMATransferLength] = 0x80000010; //Default value from HWUG
	registers_[DTCLib::DTC_Register_SERDESLoopbackEnable] = 0x00000000; // SERDES Loopback Disabled
	registers_[DTCLib::DTC_Register_SERDESOscillatorStatus] = 0x2; // Initialization Complete, no IIC Error
	registers_[DTCLib::DTC_Register_ROCEmulationEnable] = 0x3F;        // ROC Emulators enabled (of course!)
	registers_[DTCLib::DTC_Register_RingEnable] = 0x3F3F;       // All rings Tx/Rx enabled, CFO and timing disabled
	registers_[DTCLib::DTC_Register_SERDESReset] = 0x0;        // No SERDES Reset
	registers_[DTCLib::DTC_Register_SERDESRXDisparityError] = 0x0;        // No SERDES Disparity Error
	registers_[DTCLib::DTC_Register_SERDESRXCharacterNotInTableError] = 0x0;        // No SERDES CNIT Error
	registers_[DTCLib::DTC_Register_SERDESUnlockError] = 0x0;        // No SERDES Unlock Error
	registers_[DTCLib::DTC_Register_SERDESPLLLocked] = 0x7F;       // SERDES PLL Locked
	registers_[DTCLib::DTC_Register_SERDESTXBufferStatus] = 0x0;        // SERDES TX Buffer Status Normal
	registers_[DTCLib::DTC_Register_SERDESRXBufferStatus] = 0x0;        // SERDES RX Buffer Staus Nominal
	registers_[DTCLib::DTC_Register_SERDESRXStatus] = 0x0;        // SERDES RX Status Nominal
	registers_[DTCLib::DTC_Register_SERDESResetDone] = 0x7F;       // SERDES Resets Done
	registers_[DTCLib::DTC_Register_SERDESEyescanData] = 0x0;        // No Eyescan Error
	registers_[DTCLib::DTC_Register_SERDESRXCDRLock] = 0x7F;       // RX CDR Locked
	registers_[DTCLib::DTC_Register_DMATimeoutPreset] = 0x800;      // DMA Timeout Preset
	registers_[DTCLib::DTC_Register_ROCReplyTimeout] = 0x200000;   // ROC Timeout Preset
	registers_[DTCLib::DTC_Register_ROCReplyTimeoutError] = 0x0;        // ROC Timeout Error
	registers_[DTCLib::DTC_Register_RingPacketLength] = 0x10;
	registers_[DTCLib::DTC_Register_TimestampPreset0] = 0x0;        // Timestamp preset to 0
	registers_[DTCLib::DTC_Register_TimestampPreset1] = 0x0;
	registers_[DTCLib::DTC_Register_DataPendingTimer] = 0x00002000; // Data pending timeout preset
	registers_[DTCLib::DTC_Register_NUMROCs] = 0x1;          // NUMROCs 0 for all rings,except Ring 0 which has 1
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag0] = 0x0;  // NO FIFO Full flags
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag1] = 0x0;
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag2] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketError] = 0x0;        // Receive Packet Error
	registers_[DTCLib::DTC_Register_CFOEmulationTimestampLow] = 0x0;        // CFO Emulation Registers
	registers_[DTCLib::DTC_Register_CFOEmulationTimestampHigh] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationRequestInterval] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumRequests] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing0] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing1] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing2] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing3] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing4] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing5] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationDebugPacketType] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationDMACount] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationDelayCount] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing0] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing1] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing2] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing3] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing4] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataRing5] = 0x0;
	registers_[DTCLib::DTC_Register_ReceiveByteCountDataCFO] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing0] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing1] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing2] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing3] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing4] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataRing5] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketCountDataCFO] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing0] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing1] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing2] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing3] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing4] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataRing5] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitByteCountDataCFO] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing0] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing1] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing2] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing3] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing4] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataRing5] = 0x0;
	registers_[DTCLib::DTC_Register_TransmitPacketCountDataCFO] = 0x0;
	registers_[DTCLib::DTC_Register_DDRLocalStartAddress] = 0x0;
	registers_[DTCLib::DTC_Register_DDRLocalEndAddress] = 0x0;
	registers_[DTCLib::DTC_Regsiter_DDRWriteBurstSize] = 0x0;
	registers_[DTCLib::DTC_Register_DDRReadBurstSize] = 0x0;
	registers_[DTCLib::DTC_Register_FPGAProgramData] = 0x0;
	registers_[DTCLib::DTC_Register_FPGAPROMProgramStatus] = 0x1;
	registers_[DTCLib::DTC_Register_FPGACoreAccess] = 0x0;        // FPGA Core Access OK

	TRACE(17, "mu2esim::init finished");
	return (0);
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
   */
int mu2esim::read_data(int chn, void **buffer, int tmo_ms)
{
	auto start = std::chrono::high_resolution_clock::now();
	size_t bytesReturned = 0;
	if (delta_(chn, C2S) == 0)
	{
		TRACE(17, "mu2esim::read_data: Clearing output buffer");
		clearBuffer_(chn, false);

		if (chn == 0)
		{
			TRACE(17, "mu2esim::read_data: Waiting for data");
			while (ddrSim_.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < tmo_ms)
			{
				usleep(1000);
			}
			if (ddrSim_.empty()) return 0;

			auto buf = ddrSim_.pop();
			TRACE(17, "mu2esim::read_data: Done waiting for data (there is data) buf=%p", (void*)buf);
			auto disposeOfBuffer = true;

			TRACE(17, "mu2esim::read_data: Checking conditions for putting this buffer back on the queue");
			if ((registers_[DTCLib::DTC_Register_DTCControl] & 0x4000000) == 0x4000000 && (registers_[DTCLib::DTC_Register_DetEmulationDMACount] == 0 || registers_[DTCLib::DTC_Register_DetEmulationDMACount] >= detSimLoopCount_))
			{
				TRACE(17, "mu2esim::read_data: Conditions met. Putting the buffer back on the queue");
				registers_[DTCLib::DTC_Register_DDRLocalStartAddress]++;
				if (registers_[DTCLib::DTC_Register_DDRLocalStartAddress] == registers_[DTCLib::DTC_Register_DDRLocalEndAddress])
				{
					registers_[DTCLib::DTC_Register_DDRLocalStartAddress] = 0;
					detSimLoopCount_++;
				}
				ddrSim_.push(buf);
				disposeOfBuffer = false;
			}

			TRACE(17, "mu2esim::read_data: Setting bytesReturned to buffer's DMA header: %llu", *(reinterpret_cast<unsigned long long*>(*buf)));
			bytesReturned = *(reinterpret_cast<uint64_t*>(*buf));
			memcpy(dmaData_[chn][swIdx_[chn]], buf, bytesReturned);

			if (disposeOfBuffer)
			{
				spareBuffers_.push(buf);
			}
		}
		else if (chn == 1)
		{
			TRACE(17, "mu2esim::read_data: Waiting for DCS Response");
			while (dcsResponses_.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < tmo_ms)
			{
				usleep(1000);
			}
			if (dcsResponses_.empty()) return 0;

			auto buf = dcsResponses_.pop();
			TRACE(17, "mu2esim::read_data: Done waiting. There is data: buf=%p", (void*)buf);
			bytesReturned = *(reinterpret_cast<uint64_t*>(*buf));
			memcpy(dmaData_[chn][swIdx_[chn]], buf, bytesReturned);

			spareBuffers_.push(buf);
		}
	}

	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::read_data took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	*buffer = dmaData_[chn][swIdx_[chn]];
	swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;

	return bytesReturned;
}

int mu2esim::write_data(int chn, void *buffer, size_t bytes)
{
	if (chn == 0)
	{
		TRACE(17, "mu2esim::write_data: adding buffer to simulated DDR memory");
		if (bytes <= sizeof(mu2e_databuff_t) - sizeof(uint64_t))
		{
			uint64_t bufSize = bytes + 8;
			auto* lpBuf = reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t());
			memcpy(*lpBuf, &bufSize, sizeof(uint64_t));
			memcpy(static_cast<unsigned char*>(*lpBuf) + 8, buffer, bytes*sizeof(uint8_t));
			ddrSim_.push(lpBuf);
			registers_[DTCLib::DTC_Register_DDRLocalEndAddress]++;
			return 0;
		}
		TRACE(17, "mu2esim::write_data: I was asked to write more than one buffer's worth of data. Cowardly refusing.");
	}
	else if (chn == 1)
	{
		TRACE(17, "mu2esim::write_data start: chn=%i, buf=%p, bytes=%llu", chn, buffer, (unsigned long long)bytes);
		uint32_t worda;
		memcpy(&worda, buffer, sizeof(worda));
		auto word = static_cast<uint16_t>(worda >> 16);
		TRACE(17, "mu2esim::write_data worda is 0x%x and word is 0x%x", worda, word);
		auto activeRing = static_cast<DTCLib::DTC_Ring_ID>((word & 0x0F00) >> 8);
		auto activeROC = static_cast<DTCLib::DTC_ROC_ID>(word & 0xF);

		DTCLib::DTC_Timestamp ts(reinterpret_cast<uint8_t*>(buffer) + 6);
		if ((word & 0x8010) == 0x8010)
		{
			TRACE(17, "mu2esim::write_data: Readout Request: activeDAQRing=%u, ts=%llu", activeRing, (unsigned long long)ts.GetTimestamp(true));
			readoutRequestReceived_[ts.GetTimestamp(true)][activeRing] = true;
		}
		else if ((word & 0x8020) == 0x8020)
		{
			TRACE(17, "mu2esim::write_data: Data Request: activeDAQRing=%u, activeROC=%u, ts=%llu", activeRing, activeROC, (unsigned long long)ts.GetTimestamp(true));
			if (activeRing != DTCLib::DTC_Ring_Unused)
			{
				if (!readoutRequestReceived_[ts.GetTimestamp(true)][activeRing])
				{
					TRACE(17, "mu2esim::write_data: Data Request Received but missing Readout Request!");
				}
				else if (activeROC < DTCLib::DTC_ROC_Unused)
				{
					if (currentTimestamp_.GetTimestamp(true) == 0 && !readoutRequestReceived_.count(0)) { closeBuffer_(true, ts); }
					auto packetCount = *(reinterpret_cast<uint16_t*>(buffer) + 7);
					packetSimulator_(ts, activeRing, activeROC, packetCount);
				}
			}
		}
		if ((word & 0x0080) == 0)
		{
			TRACE(17, "mu2esim::write_data activeDCSRing is %u, roc is %u", activeRing, activeROC);
			if (activeRing != DTCLib::DTC_Ring_Unused && activeROC != DTCLib::DTC_ROC_Unused)
			{
				DTCLib::DTC_DataPacket packet(buffer);
				DTCLib::DTC_DCSRequestPacket thisPacket(packet);
				if (thisPacket.GetType() == DTCLib::DTC_DCSOperationType_Read ||
					thisPacket.GetType() == DTCLib::DTC_DCSOperationType_WriteWithAck)
				{
					TRACE(17, "mu2esim::write_data: Recieved DCS Request:");
					TRACE(17, thisPacket.toJSON().c_str());
					dcsPacketSimulator_(thisPacket);
				}
			}
		}
	}

	return 0;
}

int mu2esim::read_release(int chn, unsigned num)
{
	//Always succeeds
	TRACE(17, "mu2esim::read_release: Simulating a release of %u buffers of channel %i", num, chn);
	for (unsigned ii = 0; ii < num; ++ii)
	{
		if (delta_(chn, C2S) != 0) swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;
	}
	return 0;
}

int mu2esim::release_all(int chn)
{
	read_release(chn, SIM_BUFFCOUNT);
	hwIdx_[chn] = 0;
	swIdx_[chn] = 0;
	return 0;
}

int  mu2esim::read_register(uint16_t address, int tmo_ms, uint32_t *output)
{
	auto start = std::chrono::high_resolution_clock::now();
	*output = 0;
	if (registers_.count(address) > 0)
	{
		TRACE(17, "mu2esim::read_register: Returning value 0x%x for address 0x%x", registers_[address], address);
		*output = registers_[address];
		return 0;
	}
	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::read_register took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	return 1;
}

int  mu2esim::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	auto start = std::chrono::high_resolution_clock::now();
	// Write the register!!!
	TRACE(17, "mu2esim::write_register: Writing value 0x%x into address 0x%x", data, address);
	registers_[address] = data;
	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	TRACE(18, "mu2esim::write_register took %lli milliseconds out of tmo_ms=%i", duration, tmo_ms);
	if (address == 0x9100) // DTC Control
	{
		std::bitset<32> dataBS(data);
		if (dataBS[30] == 1)
		{
			TRACE(19, "mu2esim::write_register: CFO Emulator Enable Detected!");
			cancelCFO_ = true;
			if (cfoEmulatorThread_.joinable()) cfoEmulatorThread_.join();
			cancelCFO_ = false;
#if THREADED_CFO_EMULATOR
			if (registers_[0x91AC] > 10) {
				cfoEmulatorThread_ = std::thread(&mu2esim::CFOEmulator_, this);
			}
			else
#endif
			{
				CFOEmulator_();
			}
		}
	}
	return 0;
}

void mu2esim::CFOEmulator_()
{
	DTCLib::DTC_Timestamp start(registers_[DTCLib::DTC_Register_CFOEmulationTimestampLow], static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationTimestampHigh]));
	if (currentTimestamp_.GetTimestamp(true) == 0 && start.GetTimestamp(true) != 0 && start.GetTimestamp(true) != 1) { closeBuffer_(true, start); }
	auto count = registers_[DTCLib::DTC_Register_CFOEmulationNumRequests];
	auto ticksToWait = static_cast<long long>(registers_[DTCLib::DTC_Register_CFOEmulationRequestInterval] * 0.0064);
	TRACE(19, "mu2esim::CFOEmulator_ start timestamp=%llu, count=%lu, delayBetween=%lli", (unsigned long long)start.GetTimestamp(true), (unsigned long)count, (long long)ticksToWait);
	DTCLib::DTC_ROC_ID numROCS[6]{ DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused,
								   DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused,
								   DTCLib::DTC_ROC_Unused,  DTCLib::DTC_ROC_Unused };
	for (auto ring : DTCLib::DTC_Rings)
	{
		std::bitset<32> ringRocs(registers_[DTCLib::DTC_Register_NUMROCs]);
		auto number = ringRocs[ring * 3] + (ringRocs[ring * 3 + 1] << 1) + (ringRocs[ring * 3 + 2] << 2);
		numROCS[ring] = DTCLib::DTC_ROCS[number];
		TRACE(19, "mu2esim::CFOEmulator_ ringRocs[%u]=%u", ring, numROCS[ring]);
	}
	unsigned sentCount = 0;
	while (sentCount < count && !cancelCFO_)
	{
		for (auto ring : DTCLib::DTC_Rings)
		{
			auto packetCount = 0;
			switch (ring)
			{
			case DTCLib::DTC_Ring_0:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing0]);
				break;
			case DTCLib::DTC_Ring_1:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing1]);
				break;
			case DTCLib::DTC_Ring_2:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing2]);
				break;
			case DTCLib::DTC_Ring_3:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing3]);
				break;
			case DTCLib::DTC_Ring_4:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing4]);
				break;
			case DTCLib::DTC_Ring_5:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRing5]);
				break;
			default:
				break;
			}
			if (numROCS[ring] != DTCLib::DTC_ROC_Unused)
			{
				TRACE(19, "mu2esim::CFOEmulator_ ringRocs[%u]=%u", ring, numROCS[ring]);
				for (uint8_t roc = 0; roc <= numROCS[ring]; ++roc)
				{
					TRACE(21, "mu2esim::CFOEmulator_ activating packet simulator, ring=%u, roc=%u, for timestamp=%llu", ring, roc, (unsigned long long)(start + sentCount).GetTimestamp(true));
					packetSimulator_((start + sentCount), ring, static_cast<DTCLib::DTC_ROC_ID>(roc), packetCount);
				}
			}
		}
		if (ticksToWait > 100) {
			usleep(ticksToWait);
		}
		sentCount++;
	}
	closeBuffer_(false, start + sentCount);
	std::bitset<32> ctrlReg(registers_[0x9100]);
	ctrlReg[30] = 0;
	registers_[0x9100] = ctrlReg.to_ulong();
}

unsigned mu2esim::delta_(int chn, int dir)
{
	unsigned hw = hwIdx_[chn];
	unsigned sw = swIdx_[chn];
	TRACE(21, "mu2esim::delta_ chn=%i dir=%i hw=%u sw=%u num_buffs=%u"
		, chn, dir, hw, sw, SIM_BUFFCOUNT);
	if (dir == C2S)
		return ((hw >= sw)
			? hw - sw
			: SIM_BUFFCOUNT + hw - sw);
	else
		return ((sw >= hw)
			? SIM_BUFFCOUNT - (sw - hw)
			: hw - sw);
}

void mu2esim::clearBuffer_(int chn, bool increment)
{
	// Clear the buffer:
	TRACE(17, "mu2esim::clearBuffer_: Clearing output buffer");
	if (increment)
	{
		hwIdx_[chn] = (hwIdx_[chn] + 1) % SIM_BUFFCOUNT;
	}
	//memset(dmaData_[chn][hwIdx_[chn]], 0, sizeof(mu2e_databuff_t));
}

void mu2esim::dcsPacketSimulator_(DTCLib::DTC_DCSRequestPacket in)
{
	TRACE(17, "mu2esim::dcsPacketSimulator_: Constructing DCS Response");
	DTCLib::DTC_DCSReplyPacket packet(in.GetRingID(), 0, in.GetType(), in.GetAddress(), in.GetData(), true);

	TRACE(17, "mu2esim::dcsPacketSimulator_: copying response into new buffer");
	mu2e_databuff_t* buf = reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t());
	DTCLib::DTC_DataPacket dataPacket = packet.ConvertToDataPacket();
	size_t packetSize = dataPacket.GetSize();
	memcpy(static_cast<unsigned char*>(*buf) + 8, dataPacket.GetData(), packetSize);
	memcpy(buf, &packetSize, sizeof(uint64_t));
	dcsResponses_.push(buf);
}

void mu2esim::packetSimulator_(DTCLib::DTC_Timestamp ts, DTCLib::DTC_Ring_ID ring, DTCLib::DTC_ROC_ID roc, uint16_t packetCount)
{
	TRACE(17, "mu2esim::packetSimulator_: Generating data for timestamp %llu", (unsigned long long)ts.GetTimestamp(true));
	uint8_t packet[16];

	auto nSamples = rand() % 10 + 10;
	uint16_t nPackets = 1;
	if (mode_ == DTCLib::DTC_SimMode_Calorimeter)
	{
		if (nSamples <= 5) { nPackets = 1; }
		else { nPackets = static_cast<uint16_t>(floor((nSamples - 6) / 8 + 2)); }
	}
	else if (mode_ == DTCLib::DTC_SimMode_Performance)
	{
		nPackets = packetCount;
	}
	if ((currentOffset_ + (nPackets + 1) * registers_[DTCLib::DTC_Register_RingPacketLength]) > sizeof(mu2e_databuff_t) || currentTimestamp_ != ts)
	{
		closeBuffer_(false, ts);
	}

	// Record the DataBlock size
	auto dataBlockByteCount = static_cast<uint16_t>((nPackets + 1) * 16);
	TRACE(17, "mu2esim::packetSimulator_ DataBlock size is %u", dataBlockByteCount);

	// Add a Data Header packet to the reply
	packet[0] = static_cast<uint8_t>(dataBlockByteCount & 0xFF);
	packet[1] = static_cast<uint8_t>(dataBlockByteCount >> 8);
	packet[2] = 0x50 + (roc & 0x0F);
	packet[3] = 0x80 + (ring & 0x0F);
	packet[4] = static_cast<uint8_t>(nPackets & 0xFF);
	packet[5] = static_cast<uint8_t>(nPackets >> 8);
	ts.GetTimestamp(packet, 6);
	packet[12] = 0;
	packet[13] = 0;
	packet[14] = 0;
	packet[15] = 0;

	TRACE(17, "mu2esim::packetSimulator_ Copying Data Header packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
		, 0, hwIdx_[0], (void*)currentBuffer_, (void*)packet, (unsigned long long)currentOffset_);
	memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet[0], sizeof(packet));
	currentOffset_ += sizeof(packet);

	switch (mode_)
	{
	case DTCLib::DTC_SimMode_CosmicVeto:
	{
		nSamples = 4;
		packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
		packet[1] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
		packet[2] = 0x0; // No TDC value!
		packet[3] = 0x0;
		packet[4] = static_cast<uint8_t>(nSamples);
		packet[5] = static_cast<uint8_t>(nSamples >> 8);
		packet[6] = 0;
		packet[7] = 0;
		packet[8] = static_cast<uint8_t>(simIndex_[ring][roc]);
		packet[9] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
		packet[10] = 2;
		packet[11] = 2;
		packet[12] = static_cast<uint8_t>(3 * simIndex_[ring][roc]);
		packet[13] = static_cast<uint8_t>((3 * simIndex_[ring][roc]) >> 8);
		packet[14] = 0;
		packet[15] = 0;

		TRACE(17, "mu2esim::packetSimulator_ Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
			, 0, hwIdx_[0], (void*)(currentBuffer_), (void*)packet, (unsigned long long)currentOffset_);
		memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet, sizeof(packet));
	}
	break;
	case DTCLib::DTC_SimMode_Calorimeter:
	{
		packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
		packet[1] = static_cast<uint8_t>((simIndex_[ring][roc] >> 8) & 0xF) + ((simIndex_[ring][roc] & 0xF) << 4);
		packet[2] = 0x0; // No TDC value!
		packet[3] = 0x0;
		packet[4] = static_cast<uint8_t>(nSamples);
		packet[5] = static_cast<uint8_t>(nSamples >> 8);
		packet[6] = 0;
		packet[7] = 0;
		packet[8] = static_cast<uint8_t>(simIndex_[ring][roc]);
		packet[9] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);
		packet[10] = 2;
		packet[11] = 2;
		packet[12] = static_cast<uint8_t>(3 * simIndex_[ring][roc]);
		packet[13] = static_cast<uint8_t>((3 * simIndex_[ring][roc]) >> 8);
		packet[14] = 4;
		packet[15] = 4;

		TRACE(17, "mu2esim::packetSimulator_ Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
			, 0, hwIdx_[0], (void*)(currentBuffer_), (void*)packet, (unsigned long long)currentOffset_);
		memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet, sizeof(packet));
		currentOffset_ += sizeof(packet);

		auto samplesProcessed = 5;
		for (auto i = 1; i < nPackets; ++i)
		{
			packet[0] = static_cast<uint8_t>(samplesProcessed * simIndex_[ring][roc]);
			packet[1] = static_cast<uint8_t>((samplesProcessed * simIndex_[ring][roc]) >> 8);
			packet[2] = static_cast<uint8_t>(samplesProcessed + 1);
			packet[3] = static_cast<uint8_t>(samplesProcessed + 1);
			packet[4] = static_cast<uint8_t>((2 + samplesProcessed) * simIndex_[ring][roc]);
			packet[5] = static_cast<uint8_t>(((2 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
			packet[6] = static_cast<uint8_t>(samplesProcessed + 3);
			packet[7] = static_cast<uint8_t>(samplesProcessed + 3);
			packet[8] = static_cast<uint8_t>((4 + samplesProcessed) * simIndex_[ring][roc]);
			packet[9] = static_cast<uint8_t>(((4 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
			packet[10] = static_cast<uint8_t>(samplesProcessed + 5);
			packet[11] = static_cast<uint8_t>(samplesProcessed + 5);
			packet[12] = static_cast<uint8_t>((6 + samplesProcessed) * simIndex_[ring][roc]);
			packet[13] = static_cast<uint8_t>(((6 + samplesProcessed) * simIndex_[ring][roc]) >> 8);
			packet[14] = static_cast<uint8_t>(samplesProcessed + 7);
			packet[15] = static_cast<uint8_t>(samplesProcessed + 7);

			samplesProcessed += 8;
			TRACE(17, "mu2esim::packetSimulator_ Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
				, 0, hwIdx_[0], (void*)(currentBuffer_), (void*)packet, (unsigned long long)currentOffset_);
			memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet, sizeof(packet));
			currentOffset_ += sizeof(packet);
		}
	}
	break;
	case DTCLib::DTC_SimMode_Tracker:
	{
		packet[0] = static_cast<uint8_t>(simIndex_[ring][roc]);
		packet[1] = static_cast<uint8_t>(simIndex_[ring][roc] >> 8);

		packet[2] = 0x0; // No TDC value!
		packet[3] = 0x0;
		packet[4] = 0x0;
		packet[5] = 0x0;

		uint16_t pattern0 = 0;
		uint16_t pattern1 = simIndex_[ring][roc];
		uint16_t pattern2 = 2;
		uint16_t pattern3 = (simIndex_[ring][roc] * 3) % 0x3FF;
		uint16_t pattern4 = 4;
		uint16_t pattern5 = (simIndex_[ring][roc] * 5) % 0x3FF;
		uint16_t pattern6 = 6;
		uint16_t pattern7 = (simIndex_[ring][roc] * 7) % 0x3FF;

		packet[6] = static_cast<uint8_t>(pattern0);
		packet[7] = static_cast<uint8_t>((pattern0 >> 8) + (pattern1 << 2));
		packet[8] = static_cast<uint8_t>((pattern1 >> 6) + (pattern2 << 4));
		packet[9] = static_cast<uint8_t>((pattern2 >> 4) + (pattern3 << 6));
		packet[10] = static_cast<uint8_t>((pattern3 >> 2));
		packet[11] = static_cast<uint8_t>(pattern4);
		packet[12] = static_cast<uint8_t>((pattern4 >> 8) + (pattern5 << 2));
		packet[13] = static_cast<uint8_t>((pattern5 >> 6) + (pattern6 << 4));
		packet[14] = static_cast<uint8_t>((pattern6 >> 4) + (pattern7 << 6));
		packet[15] = static_cast<uint8_t>((pattern7 >> 2));

		TRACE(17, "mu2esim::packetSimulator_ Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
			, 0, hwIdx_[0], (void*)(currentBuffer_), (void*)packet, (unsigned long long)currentOffset_);
		memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet, sizeof(packet));
		currentOffset_ += sizeof(packet);

	}
	break;
	case DTCLib::DTC_SimMode_Performance:
		for (uint16_t ii = 0; ii < nPackets; ++ii)
		{
			packet[0] = static_cast<uint8_t>(ii);
			packet[1] = 0x11;
			packet[2] = 0x22;
			packet[3] = 0x33;
			packet[4] = 0x44;
			packet[5] = 0x55;
			packet[6] = 0x66;
			packet[7] = 0x77;
			packet[8] = 0x88;
			packet[9] = 0x99;
			packet[10] = 0xaa;
			packet[11] = 0xbb;
			packet[12] = 0xcc;
			packet[13] = 0xdd;
			packet[14] = 0xee;
			packet[15] = 0xff;

			TRACE(17, "mu2esim::packetSimulator_ Copying Data packet into buffer, chn=%i, idx=%u, buf=%p, packet=%p, off=%llu"
				, 0, hwIdx_[0], (void*)(currentBuffer_), (void*)packet, (unsigned long long)currentOffset_);
			memcpy(reinterpret_cast<char*>(*currentBuffer_) + currentOffset_, &packet, sizeof(packet));
			currentOffset_ += sizeof(packet);
		}
		break;
	case DTCLib::DTC_SimMode_Disabled:
	default:
		break;

	}
	simIndex_[ring][roc] = (simIndex_[ring][roc] + 1) % 0x3FF;
}

void mu2esim::closeBuffer_(bool drop, DTCLib::DTC_Timestamp ts)
{
	memcpy(currentBuffer_, &currentOffset_, sizeof(uint64_t));
	if (!drop) {
		ddrSim_.push(currentBuffer_);
	}
	else
	{
		spareBuffers_.push(currentBuffer_);
	}
	if (spareBuffers_.empty()) {
		currentBuffer_ = reinterpret_cast<mu2e_databuff_t*>(new mu2e_databuff_t());
	}
	else
	{
		currentBuffer_ = spareBuffers_.pop();
	}
	currentOffset_ = 8;
	currentTimestamp_ = ts;
}
