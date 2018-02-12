#include "CFOSoftwareCFO.h"
#include <iostream>


CFOLib::CFOSoftwareCFO::CFOSoftwareCFO(CFO* CFO, bool useCFOEmulator, uint16_t debugPacketCount,
									   CFO_DebugType debugType, bool stickyDebugType,
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
	theCFO_ = CFO;
	for (auto ring : CFO_Rings)
	{
		ringMode_[ring] = theCFO_->ReadRingEnabled(ring);
	}
	theCFO_->EnableAutogenDRP();
	theCFO_->SetAllEventModeWords(1U);
	theCFO_->SetEventModeWord(0, 0U);
}

CFOLib::CFOSoftwareCFO::~CFOSoftwareCFO()
{
	abort_ = true;
}

void CFOLib::CFOSoftwareCFO::WaitForRequestsToBeSent() const
{
	while (!requestsSent_)
	{
		usleep(1000);
	}
}

void CFOLib::CFOSoftwareCFO::SendRequestForTimestamp(CFO_Timestamp ts)
{
	if (theCFO_->IsDetectorEmulatorInUse()) {
		TRACE(19, "CFOSoftwareCFO: Enabling Detector Emulator for 1 DMA");
		//theCFO_->ResetCFO();
		theCFO_->DisableDetectorEmulator();
		theCFO_->SetDetectorEmulationDMACount(1);
		theCFO_->EnableDetectorEmulator();
		return;
	}
	if (!useCFOEmulator_)
	{
		for (auto ring : CFO_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp before SendReadoutRequestPacket");
				theCFO_->SendReadoutRequestPacket(ring, ts, quiet_);
				int maxRoc;
				if ((maxRoc = theCFO_->ReadRingROCCount(ring)) != CFO_ROC_Unused)
				{
					for (uint8_t roc = 0; roc <= maxRoc; ++roc)
					{
						TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp before CFO_DataRequestPacket req");
						CFO_DataRequestPacket req(ring, static_cast<CFO_ROC_ID>(roc), ts, true,
												  debugPacketCount_, debugType_);
						if (debugType_ == CFO_DebugType_ExternalSerialWithReset && !stickyDebugType_)
						{
							debugType_ = CFO_DebugType_ExternalSerial;
						}
						TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp before WriteDMADAQPacket - CFO_DataRequestPacket");
						if (!quiet_) std::cout << req.toJSON() << std::endl;
						theCFO_->WriteDMAPacket(req);
						TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp after  WriteDMADAQPacket - CFO_DataRequestPacket");
					}
				}
				//usleep(2000);
			}
		}
	}
	else
	{
		TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp setting up CFO CFO Emulator");
		theCFO_->DisableCFOEmulation();
		theCFO_->SetCFOEmulationTimestamp(ts);
		for (auto ring : CFO_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				theCFO_->SetCFOEmulationNumPackets(ring, debugPacketCount_);
			}
		}
		theCFO_->SetCFOEmulationNumRequests(1);
		theCFO_->SetCFOEmulationRequestInterval(1000);
		theCFO_->SetCFOEmulationDebugType(debugType_);
		theCFO_->SetCFOEmulationModeByte(5, 1);
		theCFO_->EnableDebugPacketMode();
		TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp enabling CFO CFO Emulator");
		theCFO_->EnableCFOEmulation();
		TRACE(19, "CFOSoftwareCFO::SendRequestForTimestamp done");
	}
	requestsSent_ = true;
}

void CFOLib::CFOSoftwareCFO::SendRequestsForRange(int count, CFO_Timestamp start, bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	if (theCFO_->IsDetectorEmulatorInUse()) {
		TRACE(19, "CFOSoftwareCFO: Enabling Detector Emulator for %i DMAs", count);
		//theCFO_->ResetCFO();
		theCFO_->DisableDetectorEmulator();
		theCFO_->SetDetectorEmulationDMACount(count + 1);
		theCFO_->EnableDetectorEmulator();
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
			theThread_ = std::thread(&CFOSoftwareCFO::SendRequestsForRangeImplAsync, this, start, count, increment, delayBetweenDataRequests);
		}
		else
		{
			theThread_ = std::thread(&CFOSoftwareCFO::SendRequestsForRangeImplSync, this, start, count, increment, delayBetweenDataRequests, requestsAhead);
		}
		WaitForRequestsToBeSent();
	}
	else
	{
		TRACE(19, "CFOSoftwareCFO::SendRequestsForRange setting up CFO CFO Emulator");
		theCFO_->DisableCFOEmulation();
		theCFO_->SetCFOEmulationTimestamp(start);
		for (auto ring : CFO_Rings)
		{
			if (!ringMode_[ring].TimingEnable && ringMode_[ring].TransmitEnable)
			{
				theCFO_->SetCFOEmulationNumPackets(ring, debugPacketCount_);
			}
		}
		theCFO_->SetCFOEmulationNumRequests(count);
		theCFO_->SetCFOEmulationDebugType(debugType_);
		theCFO_->SetCFOEmulationModeByte(5, 1);
		theCFO_->EnableDebugPacketMode();
		theCFO_->SetCFOEmulationRequestInterval(delayBetweenDataRequests);
		TRACE(19, "CFOSoftwareCFO::SendRequestsForRange enabling CFO CFO Emulator");
		theCFO_->EnableCFOEmulation();
		TRACE(19, "CFOSoftwareCFO::SendRequestsForRange done");
	}
}

void CFOLib::CFOSoftwareCFO::SendRequestsForRangeImplSync(CFO_Timestamp start, int count,
														  bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImplSync Start");
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

void CFOLib::CFOSoftwareCFO::SendRequestsForRangeImplAsync(CFO_Timestamp start, int count,
														   bool increment, uint32_t delayBetweenDataRequests)
{
	TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImplAsync Start");

	// Send Readout Requests first
	for (auto ii = 0; ii < count; ++ii)
	{
		auto ts = start + (increment ? ii : 0);
		for (auto ring : CFO_Rings)
		{
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImpl before SendReadoutRequestPacket");
					theCFO_->SendReadoutRequestPacket(ring, ts, quiet_);
				}
			}
			if (abort_) return;
		}
	}
	TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImpl setting RequestsSent flag");
	requestsSent_ = true;

	// Now do the DataRequests, sleeping for the required delay between each.
	for (auto ii = 0; ii < count; ++ii)
	{
		auto ts = start + (increment ? ii : 0);
		for (auto ring : CFO_Rings)
		{
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					int maxRoc;
					if ((maxRoc = theCFO_->ReadRingROCCount(ring)) != CFO_ROC_Unused)
					{
						for (uint8_t roc = 0; roc <= maxRoc; ++roc)
						{
							TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImpl before CFO_DataRequestPacket req");
							CFO_DataRequestPacket req(ring, static_cast<CFO_ROC_ID>(roc), ts, true,
													  static_cast<uint16_t>(debugPacketCount_), debugType_);
							if (debugType_ == CFO_DebugType_ExternalSerialWithReset && !stickyDebugType_)
							{
								debugType_ = CFO_DebugType_ExternalSerial;
							}
							TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImpl before WriteDMADAQPacket - CFO_DataRequestPacket");
							if (!quiet_) std::cout << req.toJSON() << std::endl;
							theCFO_->WriteDMAPacket(req);
							TRACE(19, "CFOSoftwareCFO::SendRequestsForRangeImpl after  WriteDMADAQPacket - CFO_DataRequestPacket");
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

