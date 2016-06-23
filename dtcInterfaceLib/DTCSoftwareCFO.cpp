#include "DTCSoftwareCFO.h"
#include <iostream>

#define TRACE_NAME "MU2EDEV"

DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(DTC* dtc, bool useCFOEmulator, uint16_t debugPacketCount,
									   DTC_DebugType debugType, bool stickyDebugType,
									   bool quiet, bool asyncRR)
	: useCFOEmulator_(useCFOEmulator)
	  , debugPacketCount_(debugPacketCount)
	  , debugType_(debugType)
	  , stickyDebugType_(stickyDebugType)
	  , quiet_(quiet)
	  , asyncRR_(asyncRR)
	  , requestsSent_(false)
	  , abort_(false)
{
	theDTC_ = dtc;
	for (auto ring : DTC_Rings)
	{
		ringMode_[ring] = theDTC_->ReadRingEnabled(ring);
	}
	theDTC_->EnableAutogenDRP();
	theDTC_->SetAllEventModeWords(1U);
	theDTC_->SetEventModeWord(0, 0U);
}

DTCLib::DTCSoftwareCFO::~DTCSoftwareCFO()
{
	abort_ = true;
}

void DTCLib::DTCSoftwareCFO::WaitForRequestsToBeSent() const
{
	while (!requestsSent_)
	{
		usleep(1000);
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestForTimestamp(DTC_Timestamp ts)
{
	if (theDTC_->IsDetectorEmulatorInUse()) {
		TRACE(19, "DTCSoftwareCFO: Enabling Detector Emulator for 1 DMA");
		theDTC_->DisableDetectorEmulator();
		theDTC_->SetDetectorEmulationDMACount(1);
		theDTC_->EnableDetectorEmulator();
		return;
	}
	if (!useCFOEmulator_)
	{
		for (auto ring : DTC_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before SendReadoutRequestPacket");
				theDTC_->SendReadoutRequestPacket(ring, ts, quiet_);
				int maxRoc;
				if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTC_ROC_Unused)
				{
					for (uint8_t roc = 0; roc <= maxRoc; ++roc)
					{
						TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before DTC_DataRequestPacket req");
						DTC_DataRequestPacket req(ring, static_cast<DTC_ROC_ID>(roc), ts, true,
												  debugPacketCount_, debugType_);
						if (debugType_ == DTC_DebugType_ExternalSerialWithReset && !stickyDebugType_)
						{
							debugType_ = DTC_DebugType_ExternalSerial;
						}
						TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before WriteDMADAQPacket - DTC_DataRequestPacket");
						if (!quiet_) std::cout << req.toJSON() << std::endl;
						theDTC_->WriteDMAPacket(req);
						TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp after  WriteDMADAQPacket - DTC_DataRequestPacket");
					}
				}
				//usleep(2000);
			}
		}
	}
	else
	{
		TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp setting up DTC CFO Emulator");
		theDTC_->DisableCFOEmulation();
		theDTC_->SetCFOEmulationTimestamp(ts);
		for (auto ring : DTC_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				theDTC_->SetCFOEmulationNumPackets(ring, debugPacketCount_);
			}
		}
		theDTC_->SetCFOEmulationNumRequests(1);
		theDTC_->SetCFOEmulationRequestInterval(1000);
		theDTC_->SetCFOEmulationDebugType(debugType_);
		theDTC_->SetCFOEmulationModeByte(5, 1);
		theDTC_->EnableDebugPacketMode();
		TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp enabling DTC CFO Emulator");
		theDTC_->EnableCFOEmulation();
		TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp done");
	}
	requestsSent_ = true;
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRange(int count, DTC_Timestamp start, bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	if (theDTC_->IsDetectorEmulatorInUse()) {
		TRACE(19, "DTCSoftwareCFO: Enabling Detector Emulator for %i DMAs", count);
		theDTC_->DisableDetectorEmulator();
		theDTC_->SetDetectorEmulationDMACount(count);
		theDTC_->EnableDetectorEmulator();
		return;
	}
	if (delayBetweenDataRequests < 1000)
	{
		delayBetweenDataRequests = 1000;
	}
	if (!useCFOEmulator_)
	{
		requestsSent_ = false;
		if (asyncRR_)
		{
			theThread_ = std::thread(&DTCSoftwareCFO::SendRequestsForRangeImplAsync, this, start, count, increment, delayBetweenDataRequests);
		}
		else
		{
			theThread_ = std::thread(&DTCSoftwareCFO::SendRequestsForRangeImplSync, this, start, count, increment, delayBetweenDataRequests, requestsAhead);
		}
		WaitForRequestsToBeSent();
	}
	else
	{
		TRACE(19, "DTCSoftwareCFO::SendRequestsForRange setting up DTC CFO Emulator");
		theDTC_->DisableCFOEmulation();
		theDTC_->SetCFOEmulationTimestamp(start);
		for (auto ring : DTC_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				theDTC_->SetCFOEmulationNumPackets(ring, debugPacketCount_);
			}
		}
		theDTC_->SetCFOEmulationNumRequests(count);
		theDTC_->SetCFOEmulationDebugType(debugType_);
		theDTC_->SetCFOEmulationModeByte(5, 1);
		theDTC_->EnableDebugPacketMode();
		theDTC_->SetCFOEmulationRequestInterval(delayBetweenDataRequests);
		TRACE(19, "DTCSoftwareCFO::SendRequestsForRange enabling DTC CFO Emulator");
		theDTC_->EnableCFOEmulation();
		TRACE(19, "DTCSoftwareCFO::SendRequestsForRange done");
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplSync(DTC_Timestamp start, int count,
														  bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImplSync Start");
	for (auto ii = 0; ii < count; ++ii)
	{
		auto ts = start + (increment ? ii : 0);

		SendRequestForTimestamp(ts);
		if (ii >= requestsAhead || ii == count - 1)
		{
			requestsSent_ = true;
		}

		usleep(delayBetweenDataRequests);
		if (abort_) return;
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplAsync(DTC_Timestamp start, int count,
														   bool increment, uint32_t delayBetweenDataRequests)
{
	TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImplAsync Start");

	// Send Readout Requests first
	for (auto ii = 0; ii < count; ++ii)
	{
		auto ts = start + (increment ? ii : 0);
		for (auto ring : DTC_Rings)
		{
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before SendReadoutRequestPacket");
					theDTC_->SendReadoutRequestPacket(ring, ts, quiet_);
				}
			}
			if (abort_) return;
		}
	}
	TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl setting RequestsSent flag");
	requestsSent_ = true;

	// Now do the DataRequests, sleeping for the required delay between each.
	for (auto ii = 0; ii < count; ++ii)
	{
		auto ts = start + (increment ? ii : 0);
		for (auto ring : DTC_Rings)
		{
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					int maxRoc;
					if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTC_ROC_Unused)
					{
						for (uint8_t roc = 0; roc <= maxRoc; ++roc)
						{
							TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before DTC_DataRequestPacket req");
							DTC_DataRequestPacket req(ring, static_cast<DTC_ROC_ID>(roc), ts, true,
													  static_cast<uint16_t>(debugPacketCount_), debugType_);
							if (debugType_ == DTC_DebugType_ExternalSerialWithReset && !stickyDebugType_)
							{
								debugType_ = DTC_DebugType_ExternalSerial;
							}
							TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before WriteDMADAQPacket - DTC_DataRequestPacket");
							if (!quiet_) std::cout << req.toJSON() << std::endl;
							theDTC_->WriteDMAPacket(req);
							TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl after  WriteDMADAQPacket - DTC_DataRequestPacket");
							if (abort_) return;
						}
					}
					//usleep(2000);
				}
			}
			if (abort_) return;
		}
		usleep(delayBetweenDataRequests);
	}
}

