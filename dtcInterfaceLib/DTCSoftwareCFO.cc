#include "DTCSoftwareCFO.h"


#define TRACE_NAME "MU2EDEV"
DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(bool useCFOEmulator, uint16_t debugPacketCount, 
	DTCLib::DTC_DebugType debugType, bool stickyDebugType,
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
	theDTC_ = new DTCLib::DTC();
	ownDTC_ = true;
	for (auto ring : DTCLib::DTC_Rings) { ringMode_[ring] = theDTC_->ReadRingEnabled(ring); }
}

DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(DTCLib::DTC* dtc, bool useCFOEmulator, uint16_t debugPacketCount,
	DTCLib::DTC_DebugType debugType, bool stickyDebugType,
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
	ownDTC_ = false;
	for (auto ring : DTCLib::DTC_Rings) { ringMode_[ring] = theDTC_->ReadRingEnabled(ring); }
}

DTCLib::DTCSoftwareCFO::~DTCSoftwareCFO()
{
	if (ownDTC_) delete theDTC_;
	abort_ = true;
	theThread_.join();
}

void DTCLib::DTCSoftwareCFO::WaitForRequestsToBeSent()
{
	while (!requestsSent_) {
		usleep(1000);
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestForTimestamp(DTCLib::DTC_Timestamp ts)
{
	if (!useCFOEmulator_) {
		for (auto ring : DTCLib::DTC_Rings) {
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before SendReadoutRequestPacket");
					theDTC_->SendReadoutRequestPacket(ring, ts, quiet_);
					int maxRoc;
					if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTCLib::DTC_ROC_Unused)
					{
						for (uint8_t roc = 0; roc <= maxRoc; ++roc)
						{
							TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before DTC_DataRequestPacket req");
							DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, ts, true,
								debugPacketCount_, debugType_);
							if (debugType_ == DTCLib::DTC_DebugType_ExternalSerialWithReset && !stickyDebugType_) {
								debugType_ = DTCLib::DTC_DebugType_ExternalSerial;
							}
							TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp before WriteDMADAQPacket - DTC_DataRequestPacket");
							if (!quiet_) std::cout << req.toJSON() << std::endl;
							theDTC_->WriteDMADAQPacket(req);
							TRACE(19, "DTCSoftwareCFO::SendRequestForTimestamp after  WriteDMADAQPacket - DTC_DataRequestPacket");
						}
					}
					//usleep(2000);
				}
			}
		}
	}
	else
	{
		theDTC_->DisableCFOEmulation();
		theDTC_->SetCFOEmulationTimestamp(ts);
		theDTC_->SetCFOEmulationNumPackets(debugPacketCount_);
		theDTC_->SetCFOEmulationNumRequests(1);
		theDTC_->SetCFOEmulationRequestInterval(0);
		theDTC_->EnableCFOEmulation();
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRange(int count, DTCLib::DTC_Timestamp start, bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	if (!useCFOEmulator_)
	{
		requestsSent_ = false;
		if (asyncRR_) {
			theThread_ = std::thread(&DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplAsync, this, start, count, increment, delayBetweenDataRequests);
		}
		else
		{
			theThread_ = std::thread(&DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplSync, this, start, count, increment, delayBetweenDataRequests, requestsAhead);
		}
		WaitForRequestsToBeSent();
	}
	else
	{
		theDTC_->DisableCFOEmulation();
		theDTC_->SetCFOEmulationTimestamp(start);
		theDTC_->SetCFOEmulationNumPackets(debugPacketCount_);
		theDTC_->SetCFOEmulationNumRequests(count);
		theDTC_->SetCFOEmulationRequestInterval(delayBetweenDataRequests);
		theDTC_->EnableCFOEmulation();
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplSync(DTCLib::DTC_Timestamp start, int count,
	bool increment, uint32_t delayBetweenDataRequests, int requestsAhead)
{
	TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImplSync Start");
	for (int ii = 0; ii < count; ++ii) {
		DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);

		SendRequestForTimestamp(ts);
		if (ii >= requestsAhead || ii == count - 1) { requestsSent_ = true; }

		usleep(delayBetweenDataRequests);
		if (abort_) return;
	}
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplAsync(DTCLib::DTC_Timestamp start, int count,
	bool increment, uint32_t delayBetweenDataRequests)
{
	TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImplAsync Start");

	// Send Readout Requests first
	for (int ii = 0; ii < count; ++ii) {
		DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);
		for (auto ring : DTCLib::DTC_Rings) {
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
	for (int ii = 0; ii < count; ++ii) {
		DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);
		for (auto ring : DTCLib::DTC_Rings) {
			if (!ringMode_[ring].TimingEnable)
			{
				if (ringMode_[ring].TransmitEnable)
				{
					int maxRoc;
					if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTCLib::DTC_ROC_Unused)
					{
						for (uint8_t roc = 0; roc <= maxRoc; ++roc)
						{
							TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before DTC_DataRequestPacket req");
							DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, ts, true,
								(uint16_t)debugPacketCount_, debugType_);
							if (debugType_ == DTCLib::DTC_DebugType_ExternalSerialWithReset && !stickyDebugType_) {
								debugType_ = DTCLib::DTC_DebugType_ExternalSerial;
							}
							TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before WriteDMADAQPacket - DTC_DataRequestPacket");
							if (!quiet_) std::cout << req.toJSON() << std::endl;
							theDTC_->WriteDMADAQPacket(req);
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
