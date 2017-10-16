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
# define TRACE_CNTL(...)
# endif
# pragma warning(disable: 4351 6385 6386)
#endif
#include "mu2esim.h"
#include <vector>
#include <cmath>
#include "DTC_Registers.h"

#define THREADED_CFO_EMULATOR 1

mu2esim::mu2esim()
	: registers_()
	, swIdx_()
	, detSimLoopCount_(0)
	, dmaData_()
	, ddrFile_("mu2esim.bin", std::ios::binary | std::ios::in | std::ios::out)
	, mode_(DTCLib::DTC_SimMode_Disabled)
	, simIndex_()
	, cancelCFO_(true)
	, readoutRequestReceived_()
	, currentTimestamp_(0xFFFFFFFFFFFF)
	, currentEventSize_(0)
	, eventBegin_(ddrFile_.tellp())
{
	TRACE(17, "mu2esim::mu2esim BEGIN");
	swIdx_[0] = 0;
	swIdx_[1] = 0;
	for (unsigned ii = 0; ii < SIM_BUFFCOUNT; ++ii)
	{
		dmaData_[0][ii] = reinterpret_cast<mu2e_databuff_t*>(new char[0x10000]);
		dmaData_[1][ii] = reinterpret_cast<mu2e_databuff_t*>(new char[0x10000]);
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


	if (!ddrFile_)
	{
		ddrFile_.open("mu2esim.bin", std::fstream::binary | std::fstream::trunc | std::fstream::out);
		ddrFile_.close();
		// re-open with original flags
		ddrFile_.open("mu2esim.bin", std::fstream::binary | std::fstream::in | std::fstream::out);
	}

	TRACE(17, "mu2esim::mu2esim END");
}

mu2esim::~mu2esim()
{
	cancelCFO_ = true;
	if (cfoEmulatorThread_.joinable()) cfoEmulatorThread_.join();
	for (unsigned ii = 0; ii < SIM_BUFFCOUNT; ++ii)
	{
		delete[] dmaData_[0][ii];
		delete[] dmaData_[1][ii];
	}
	ddrFile_.close();
}

int mu2esim::init(DTCLib::DTC_SimMode mode)
{
	TRACE(17, "mu2e Simulator::init");
	mode_ = mode;

	TRACE(17, "mu2esim::init Initializing registers");
	// Set initial register values...
	registers_[DTCLib::DTC_Register_DesignVersion] = 0x00006363; // v99.99
	registers_[DTCLib::DTC_Register_DesignDate] = 0x53494D44; // SIMD in ASCII
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
	registers_[DTCLib::DTC_Register_ClockOscillatorStatus] = 0x20002; // Initialization Complete, no IIC Error
	registers_[DTCLib::DTC_Register_ROCEmulationEnable] = 0x3F; // ROC Emulators enabled (of course!)
	registers_[DTCLib::DTC_Register_RingEnable] = 0x3F3F; // All rings Tx/Rx enabled, CFO and timing disabled
	registers_[DTCLib::DTC_Register_SERDESReset] = 0x0; // No SERDES Reset
	registers_[DTCLib::DTC_Register_SERDESRXDisparityError] = 0x0; // No SERDES Disparity Error
	registers_[DTCLib::DTC_Register_SERDESRXCharacterNotInTableError] = 0x0; // No SERDES CNIT Error
	registers_[DTCLib::DTC_Register_SERDESUnlockError] = 0x0; // No SERDES Unlock Error
	registers_[DTCLib::DTC_Register_SERDESPLLLocked] = 0x7F; // SERDES PLL Locked
	registers_[DTCLib::DTC_Register_SERDESTXBufferStatus] = 0x0; // SERDES TX Buffer Status Normal
	registers_[DTCLib::DTC_Register_SERDESRXBufferStatus] = 0x0; // SERDES RX Buffer Staus Nominal
	registers_[DTCLib::DTC_Register_SERDESRXStatus] = 0x0; // SERDES RX Status Nominal
	registers_[DTCLib::DTC_Register_SERDESResetDone] = 0x7F; // SERDES Resets Done
	registers_[DTCLib::DTC_Register_SERDESEyescanData] = 0x0; // No Eyescan Error
	registers_[DTCLib::DTC_Register_SFPSERDESStatus] = 0x7F00007F; // RX CDR Locked
	registers_[DTCLib::DTC_Register_DMATimeoutPreset] = 0x800; // DMA Timeout Preset
	registers_[DTCLib::DTC_Register_ROCReplyTimeout] = 0x200000; // ROC Timeout Preset
	registers_[DTCLib::DTC_Register_ROCReplyTimeoutError] = 0x0; // ROC Timeout Error
	registers_[DTCLib::DTC_Register_RingPacketLength] = 0x10;
	registers_[DTCLib::DTC_Register_EVBPartitionID] = 0x0;
	registers_[DTCLib::DTC_Register_EVBDestCount] = 0x0;
	registers_[DTCLib::DTC_Register_HeartbeatErrorFlags] = 0x0;
	registers_[DTCLib::DTC_Register_SERDESOscillatorFrequency] = 156250000;
	registers_[DTCLib::DTC_Register_SERDESOscillatorControl] = 0;
	registers_[DTCLib::DTC_Register_SERDESOscillatorParameterLow] = 0xFFFFFFFF;
	registers_[DTCLib::DTC_Register_SERDESOscillatorParameterHigh] = 0x77f3f;
	registers_[DTCLib::DTC_Register_DDROscillatorFrequency] = 0xbebc200;
	registers_[DTCLib::DTC_Register_DDROscillatorControl] = 0;
	registers_[DTCLib::DTC_Register_DDROscillatorParameterLow] = 0x1074f43b;
	registers_[DTCLib::DTC_Register_DDROscillatorParameterHigh] = 0x30303;
	registers_[DTCLib::DTC_Register_TimestampPreset0] = 0x0; // Timestamp preset to 0
	registers_[DTCLib::DTC_Register_TimestampPreset1] = 0x0;
	registers_[DTCLib::DTC_Register_DataPendingTimer] = 0x00002000; // Data pending timeout preset
	registers_[DTCLib::DTC_Register_NUMROCs] = 0x1; // NUMROCs 0 for all rings,except Ring 0 which has 1
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag0] = 0x0; // NO FIFO Full flags
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag1] = 0x0;
	registers_[DTCLib::DTC_Register_FIFOFullErrorFlag2] = 0x0;
	registers_[DTCLib::DTC_Register_ReceivePacketError] = 0x0; // Receive Packet Error
	registers_[DTCLib::DTC_Register_CFOEmulationTimestampLow] = 0x0; // CFO Emulation Registers
	registers_[DTCLib::DTC_Register_CFOEmulationTimestampHigh] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationRequestInterval] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumRequests] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings10] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings32] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings54] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationEventMode1] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationEventMode2] = 0x0;
	registers_[DTCLib::DTC_Register_CFOEmulationDebugPacketType] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationDMACount] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationDelayCount] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationControl0] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationControl1] = 0x0;
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
	registers_[DTCLib::DTC_Register_DetEmulationDataStartAddress] = 0x0;
	registers_[DTCLib::DTC_Register_DetEmulationDataEndAddress] = 0x0;
	registers_[DTCLib::DTC_Register_EthernetFramePayloadSize] = 0x5D4;
	registers_[DTCLib::DTC_Register_FPGAProgramData] = 0x0;
	registers_[DTCLib::DTC_Register_FPGAPROMProgramStatus] = 0x1;
	registers_[DTCLib::DTC_Register_FPGACoreAccess] = 0x0; // FPGA Core Access OK
	registers_[DTCLib::DTC_Register_EventModeLookupTableStart] = 0;
	registers_[DTCLib::DTC_Register_EventModeLookupTableEnd] = 0;

	TRACE(17, "mu2esim::init finished");
	return 0;
}

/*****************************
   read_data
   returns number of bytes read; negative value indicates an error
   */
int mu2esim::read_data(int chn, void** buffer, int tmo_ms)
{
	auto start = std::chrono::steady_clock::now();
	size_t bytesReturned = 0;
	if (delta_(chn, C2S) == 0)
	{
		TRACE(17, "mu2esim::read_data: Clearing output buffer");
		clearBuffer_(chn, false);

		if (chn == 0)
		{
			TRACE(17, "mu2esim::read_data: Reading size from memory file");
			uint64_t size;
			ddrFile_.read(reinterpret_cast<char*>(&size), sizeof(uint64_t) / sizeof(char));

			TRACE(17, "mu2esim::read_data: Size is %llu", (unsigned long long)size);

			if (ddrFile_.eof() || size == 0)
			{
				TRACE(17, "mu2esim::read_data: End of file reached, looping back to start");
				ddrFile_.clear();
				ddrFile_.seekg(std::ios::beg);

				TRACE(17, " mu2esim::read_data: Re-reading size from memory file");
				ddrFile_.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
				TRACE(17, "mu2esim::read_data: Size is %llu", (unsigned long long)size);
				if (ddrFile_.eof())
				{
					TRACE(17, "mu2esim::read_data: 0-size file detected!");
					return -1;
				}
			}
			TRACE(17, "Size of data is %zu, reading into buffer %u, at %p", size, swIdx_[chn], (void*)dmaData_[chn][swIdx_[chn]]);
			memcpy(dmaData_[chn][swIdx_[chn]], &size, sizeof(uint64_t));
			ddrFile_.read(reinterpret_cast<char*>(dmaData_[chn][swIdx_[chn]]) + sizeof(uint64_t), size);
			bytesReturned = size + sizeof(uint64_t);
		}
		else if (chn == 1)
		{
			// Data should already be in appropriate buffer
		}
	}

	*buffer = dmaData_[chn][swIdx_[chn]];
	TRACE(18, "mu2esim::read_data: *buffer (%p) should now be equal to dmaData_[%d][%u] (%p)", (void*)*buffer, chn, swIdx_[chn], (void*)dmaData_[chn][swIdx_[chn]]);
	swIdx_[chn] = (swIdx_[chn] + 1) % SIM_BUFFCOUNT;

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	TRACE(18, "mu2esim::read_data took %lli milliseconds out of tmo_ms=%i", static_cast<long long>(duration), tmo_ms);
	return static_cast<int>(bytesReturned);
}

int mu2esim::write_data(int chn, void* buffer, size_t bytes)
{
	if (chn == 0)
	{
		TRACE(17, "mu2esim::write_data: adding buffer to simulated DDR memory sz=%llu, *buffer=%llu", (unsigned long long)bytes, (unsigned long long)*((uint64_t*)buffer));
		if (bytes <= sizeof(mu2e_databuff_t))
		{
			if (currentEventSize_ > 0) closeEvent_();

			// Strip off first 64-bit word
			auto writeBytes = *reinterpret_cast<uint64_t*>(buffer) - sizeof(uint64_t);
			auto ptr = reinterpret_cast<char*>(buffer) + (sizeof(uint64_t) / sizeof(char));
			ddrFile_.write(ptr, writeBytes);
			registers_[DTCLib::DTC_Register_DetEmulationDataEndAddress] += static_cast<uint32_t>(writeBytes);
			ddrFile_.flush();
			return 0;
		}
		TRACE(17, "mu2esim::write_data: I was asked to write more than one buffer's worth of data. Cowardly refusing.");
	}
	else if (chn == 1)
	{
		TRACE(17, "mu2esim::write_data start: chn=%i, buf=%p, bytes=%llu", chn, buffer, (unsigned long long)bytes);
		uint32_t worda;
		memcpy(&worda, buffer, sizeof worda);
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
					openEvent_(ts);

					auto packetCount = *(reinterpret_cast<uint16_t*>(buffer) + 7);
					packetSimulator_(ts, activeRing, activeROC, packetCount);

					readoutRequestReceived_[ts.GetTimestamp(true)][activeRing] = false;
					if(readoutRequestReceived_[ts.GetTimestamp(true)].count() == 0)
					{
						closeEvent_();
						readoutRequestReceived_.erase(ts.GetTimestamp(true));
					}
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
	swIdx_[chn] = 0;
	return 0;
}

int mu2esim::read_register(uint16_t address, int tmo_ms, uint32_t* output)
{
	auto start = std::chrono::steady_clock::now();
	*output = 0;
	if (registers_.count(address) > 0)
	{
		TRACE(17, "mu2esim::read_register: Returning value 0x%x for address 0x%x", registers_[address], address);
		*output = registers_[address];
		return 0;
	}
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	TRACE(18, "mu2esim::read_register took %lli milliseconds out of tmo_ms=%i", static_cast<long long>(duration), tmo_ms);
	return 1;
}

int mu2esim::write_register(uint16_t address, int tmo_ms, uint32_t data)
{
	auto start = std::chrono::steady_clock::now();
	// Write the register!!!
	TRACE(17, "mu2esim::write_register: Writing value 0x%x into address 0x%x", data, address);
	registers_[address] = data;
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	TRACE(18, "mu2esim::write_register took %lli milliseconds out of tmo_ms=%i", static_cast<long long>(duration), tmo_ms);
	std::bitset<32> dataBS(data);
	if (address == DTCLib::DTC_Register_DTCControl)
	{
		auto detectorEmulationMode = (registers_[DTCLib::DTC_Register_DetEmulationControl0] & 0x3) != 0;
		if (dataBS[30] == 1 && !detectorEmulationMode)
		{
			TRACE(19, "mu2esim::write_register: CFO Emulator Enable Detected!");
			cancelCFO_ = true;
			if (cfoEmulatorThread_.joinable()) cfoEmulatorThread_.join();
			cancelCFO_ = false;
#if THREADED_CFO_EMULATOR
			if (registers_[0x91AC] > 10)
			{
				cfoEmulatorThread_ = std::thread(&mu2esim::CFOEmulator_, this);
			}
			else
#endif
			{
				CFOEmulator_();
			}
		}
		else if (dataBS[26] == 1 && detectorEmulationMode)
		{
			TRACE(19, "mu2esim::write_register: IGNORING CFO Emulator Enable because we're in Detector Simulator mode!");
		}
		if (dataBS[31] == 1)
		{
			TRACE(19, "mu2esim::write_register: RESETTING DTC EMULATOR!");
			init(mode_);
			ddrFile_.close();
			ddrFile_.open("mu2esim.bin", std::ios::trunc | std::ios::binary | std::ios::out | std::ios::in);
			eventBegin_ = ddrFile_.tellp();
		}
	}
	if (address == DTCLib::DTC_Register_DetEmulationControl0)
	{
		if (dataBS[0] == 0)
		{
			ddrFile_.close();
			ddrFile_.open("mu2esim.bin", std::ios::trunc | std::ios::binary | std::ios::out | std::ios::in);
			eventBegin_ = ddrFile_.tellp();
		}
	}
	if (address == DTCLib::DTC_Register_DetEmulationDataStartAddress)
	{
		ddrFile_.seekg(data);
	}
	return 0;
}

void mu2esim::CFOEmulator_()
{
	if (cancelCFO_)
	{
		std::bitset<32> ctrlReg(registers_[0x9100]);
		ctrlReg[30] = 0;
		registers_[0x9100] = ctrlReg.to_ulong();
		return;
	}
	DTCLib::DTC_Timestamp start(registers_[DTCLib::DTC_Register_CFOEmulationTimestampLow], static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationTimestampHigh]));
	auto count = registers_[DTCLib::DTC_Register_CFOEmulationNumRequests];
	auto ticksToWait = static_cast<long long>(registers_[DTCLib::DTC_Register_CFOEmulationRequestInterval] * 0.0064);
	TRACE(19, "mu2esim::CFOEmulator_ start timestamp=%llu, count=%lu, delayBetween=%lli", (unsigned long long)start.GetTimestamp(true), (unsigned long)count, (long long)ticksToWait);
	DTCLib::DTC_ROC_ID numROCS[6]{ DTCLib::DTC_ROC_Unused, DTCLib::DTC_ROC_Unused,
		DTCLib::DTC_ROC_Unused, DTCLib::DTC_ROC_Unused,
		DTCLib::DTC_ROC_Unused, DTCLib::DTC_ROC_Unused };
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
		openEvent_(start + sentCount);
		for (auto ring : DTCLib::DTC_Rings)
		{
			uint16_t packetCount;
			switch (ring)
			{
			case DTCLib::DTC_Ring_0:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings10]);
				break;
			case DTCLib::DTC_Ring_1:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings10] >> 16);
				break;
			case DTCLib::DTC_Ring_2:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings32]);
				break;
			case DTCLib::DTC_Ring_3:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings32] >> 16);
				break;
			case DTCLib::DTC_Ring_4:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings54]);
				break;
			case DTCLib::DTC_Ring_5:
				packetCount = static_cast<uint16_t>(registers_[DTCLib::DTC_Register_CFOEmulationNumPacketsRings54] >> 16);
				break;
			default:
				packetCount = 0;
				break;
			}
			if (numROCS[ring] != DTCLib::DTC_ROC_Unused)
			{
				TRACE(19, "mu2esim::CFOEmulator_ ringRocs[%u]=%u", ring, numROCS[ring]);
				for (uint8_t roc = 0; roc <= numROCS[ring]; ++roc)
				{
					TRACE(21, "mu2esim::CFOEmulator_ activating packet simulator, ring=%u, roc=%u, for timestamp=%llu", ring, roc, (unsigned long long)(start + sentCount).GetTimestamp(true));
					packetSimulator_(start + sentCount, ring, static_cast<DTCLib::DTC_ROC_ID>(roc), packetCount);
				}
			}
		}

		closeEvent_();

		if (ticksToWait > 100)
		{
			usleep(ticksToWait);
		}
		sentCount++;
	}
	std::bitset<32> ctrlReg(registers_[0x9100]);
	ctrlReg[30] = 0;
	registers_[0x9100] = ctrlReg.to_ulong();
}

unsigned mu2esim::delta_(int chn, int dir)
{
	if (chn == 0) return 0;
	TRACE(25, "delta_ %i %i = 0", chn, dir);
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
	/*
	TRACE(17, "mu2esim::clearBuffer_: Clearing output buffer");
	if (increment)
	{
		hwIdx_[chn] = (hwIdx_[chn] + 1) % SIM_BUFFCOUNT;
	}
	memset(dmaData_[chn][hwIdx_[chn]], 0, sizeof(mu2e_databuff_t));
	*/
	TRACE(17, "mu2esim::clearBuffer_(%i, %u): NOP", chn, increment);
}

void mu2esim::openEvent_(DTCLib::DTC_Timestamp ts)
{
	TRACE(18, "mu2esim::openEvent_ Checking timestamp %llu vs current timestamp %llu", (unsigned long long) ts.GetTimestamp(true), (unsigned long long)currentTimestamp_.GetTimestamp(true));
	if (ts == currentTimestamp_) return;
	if (currentEventSize_ > 0) closeEvent_();

	TRACE(18, "mu2esim::openEvent_: Setting up initial buffer");
	eventBegin_ = ddrFile_.tellp();
	ddrFile_.write(reinterpret_cast<char*>(&currentEventSize_), sizeof(uint64_t) / sizeof(char));

	TRACE(18, "mu2esim::openEvent_: updating current timestamp");
	currentTimestamp_ = ts;
}

void mu2esim::closeEvent_()
{
	TRACE(18, "mu2esim::closeEvent_: Checking current event size %llu", (unsigned long long)currentEventSize_);
	if (currentEventSize_ > 0) {
		auto currentPos = ddrFile_.tellp();
		ddrFile_.seekp(eventBegin_);
		TRACE(18, "mu2esim::closeEvent_: Writing event size word");
		ddrFile_.write(reinterpret_cast<char*>(&currentEventSize_), sizeof(uint64_t) / sizeof(char));
		ddrFile_.seekp(currentPos);
		currentEventSize_ = 0;
		ddrFile_.flush();
	}
	TRACE(18, "mu2esim::closeEvent_ FINISH");
}

void mu2esim::dcsPacketSimulator_(DTCLib::DTC_DCSRequestPacket in)
{
	TRACE(17, "mu2esim::dcsPacketSimulator_: Constructing DCS Response");
	DTCLib::DTC_DCSReplyPacket packet(in.GetRingID(), 0, in.GetType(), in.GetAddress(), in.GetData(), true);

	TRACE(17, "mu2esim::dcsPacketSimulator_: copying response into new buffer");
	auto dataPacket = packet.ConvertToDataPacket();
	size_t packetSize = dataPacket.GetSize();
	*reinterpret_cast<uint64_t*>(dmaData_[1][hwIdx_[1]]) = packetSize;
	memcpy(reinterpret_cast<uint64_t*>(dmaData_[1][hwIdx_[1]]) + 1, dataPacket.GetData(), packetSize);
	hwIdx_[1] = (hwIdx_[1] + 1) % SIM_BUFFCOUNT;
}

void mu2esim::packetSimulator_(DTCLib::DTC_Timestamp ts, DTCLib::DTC_Ring_ID ring, DTCLib::DTC_ROC_ID roc, uint16_t packetCount)
{
	TRACE(17, "mu2esim::packetSimulator_: Generating data for timestamp %llu", (unsigned long long)ts.GetTimestamp(true));

	uint8_t packet[16];
	auto packetSize = sizeof(packet) / sizeof(char);

	auto nSamples = rand() % 10 + 10;
	uint16_t nPackets = 1;
	if (mode_ == DTCLib::DTC_SimMode_Calorimeter)
	{
		if (nSamples <= 5)
		{
			nPackets = 1;
		}
		else
		{
			nPackets = static_cast<uint16_t>(floor((nSamples - 6) / 8 + 2));
		}
	}
	else if (mode_ == DTCLib::DTC_SimMode_Performance)
	{
		nPackets = packetCount;
	}

	// Record the DataBlock size
	uint16_t dataBlockByteCount = static_cast<uint16_t>((nPackets + 1) * 16);
	currentEventSize_ += dataBlockByteCount;

	// Add a Data Header packet to the reply
	packet[0] = static_cast<uint8_t>(dataBlockByteCount);
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

	TRACE(17, "mu2esim::packetSimulator_ Writing Data Header packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
	ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);

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

		TRACE(17, "mu2esim::packetSimulator_ Writing Data packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
		ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);
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

		TRACE(17, "mu2esim::packetSimulator_ Writing Data packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
		ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);

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
			TRACE(17, "mu2esim::packetSimulator_ Writing Data packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
			ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);
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
		auto pattern1 = simIndex_[ring][roc];
		uint16_t pattern2 = 2;
		uint16_t pattern3 = simIndex_[ring][roc] * 3 % 0x3FF;
		uint16_t pattern4 = 4;
		uint16_t pattern5 = simIndex_[ring][roc] * 5 % 0x3FF;
		uint16_t pattern6 = 6;
		uint16_t pattern7 = simIndex_[ring][roc] * 7 % 0x3FF;

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

		TRACE(17, "mu2esim::packetSimulator_ Writing Data packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
		ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);
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

			TRACE(17, "mu2esim::packetSimulator_ Writing Data packet to memory file, chn=%i, packet=%p", 0, (void*)packet);
			ddrFile_.write(reinterpret_cast<char*>(packet), packetSize);
		}
		break;
	case DTCLib::DTC_SimMode_Disabled:
	default:
		break;
	}
	simIndex_[ring][roc] = (simIndex_[ring][roc] + 1) % 0x3FF;
}


