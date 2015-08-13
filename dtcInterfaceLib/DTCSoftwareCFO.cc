#include "DTCSoftwareCFO.h"

#include <thread>

DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(int debugPacketCount, bool quiet) :
debugPacketCount_(debugPacketCount), quiet_(quiet)
{
    theDTC_ = new DTCLib::DTC();
    ownDTC_ = true;
}

DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(DTCLib::DTC* dtc, int debugPacketCount, bool quiet) :
debugPacketCount_(debugPacketCount), quiet_(quiet)
{
    theDTC_ = dtc;
    ownDTC_ = false;
}

DTCLib::DTCSoftwareCFO::~DTCSoftwareCFO()
{
    if (ownDTC_) delete theDTC_;
}

void DTCLib::DTCSoftwareCFO::SendRequestForTimestamp(DTCLib::DTC_Timestamp ts)
{
    for (auto ring : DTCLib::DTC_Rings) { ringMode_[ring] = theDTC_->ReadRingEnabled(ring); }
    for (auto ring : DTCLib::DTC_Rings){
        if (!ringMode_[ring].TimingEnable)
        {
            if (ringMode_[ring].TransmitEnable)
            {
                TRACE(19, "DTC::GetData before SendReadoutRequestPacket");
                theDTC_->SendReadoutRequestPacket(ring, ts, quiet_);
                int maxRoc;
                if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTCLib::DTC_ROC_Unused)
                {
                    for (uint8_t roc = 0; roc <= maxRoc; ++roc)
                    {
                        TRACE(19, "DTC::GetData before DTC_DataRequestPacket req");
                        DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, ts, true,
                            (uint16_t)debugPacketCount_);
                        TRACE(19, "DTC::GetData before WriteDMADAQPacket - DTC_DataRequestPacket");
                        if (!quiet_) std::cout << req.toJSON() << std::endl;
                        theDTC_->WriteDMADAQPacket(req);
                        TRACE(19, "DTC::GetData after  WriteDMADAQPacket - DTC_DataRequestPacket");
                    }
                }
                //usleep(2000);
            }
        }
    }
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRange(int count, DTCLib::DTC_Timestamp start, bool increment, int delayBetweenDataRequests)
{
    std::thread(&DTCLib::DTCSoftwareCFO::SendRequestsForRangeImpl, this, start, count, increment, delayBetweenDataRequests);
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImpl(DTCLib::DTC_Timestamp start, int count,
    bool increment, int delayBetweenDataRequests)
{
    for (auto ring : DTCLib::DTC_Rings) { ringMode_[ring] = theDTC_->ReadRingEnabled(ring); }

    // Send Readout Requests first
    for (int ii = 0; ii < count; ++ii) {
        DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);
        for (auto ring : DTCLib::DTC_Rings){
            if (!ringMode_[ring].TimingEnable)
            {
                if (ringMode_[ring].TransmitEnable)
                {
                    TRACE(19, "DTC::GetData before SendReadoutRequestPacket");
                    theDTC_->SendReadoutRequestPacket(ring, ts, quiet_);
                }
            }
        }
    }

    // Now do the DataRequests, sleeping for the required delay between each.
    for (int ii = 0; ii < count; ++ii) {
        DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);
        for (auto ring : DTCLib::DTC_Rings){
            if (!ringMode_[ring].TimingEnable)
            {
                if (ringMode_[ring].TransmitEnable)
                {
                    int maxRoc;
                    if ((maxRoc = theDTC_->ReadRingROCCount(ring)) != DTCLib::DTC_ROC_Unused)
                    {
                        for (uint8_t roc = 0; roc <= maxRoc; ++roc)
                        {
                            TRACE(19, "DTC::GetData before DTC_DataRequestPacket req");
                            DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, ts, true,
                                (uint16_t)debugPacketCount_);
                            TRACE(19, "DTC::GetData before WriteDMADAQPacket - DTC_DataRequestPacket");
                            if (!quiet_) std::cout << req.toJSON() << std::endl;
                            theDTC_->WriteDMADAQPacket(req);
                            TRACE(19, "DTC::GetData after  WriteDMADAQPacket - DTC_DataRequestPacket");
                        }
                    }
                    //usleep(2000);
                }
            }
        }
        usleep(delayBetweenDataRequests);
    }

}