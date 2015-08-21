#include "DTCSoftwareCFO.h"


#define TRACE_NAME "MU2EDEV"
DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(int debugPacketCount, bool quiet, bool asyncRR) :
debugPacketCount_(debugPacketCount), quiet_(quiet), asyncRR_(asyncRR), requestsSent_(false), abort_(false)
{
    theDTC_ = new DTCLib::DTC();
    ownDTC_ = true;
    for (auto ring : DTCLib::DTC_Rings) { ringMode_[ring] = theDTC_->ReadRingEnabled(ring); }
}

DTCLib::DTCSoftwareCFO::DTCSoftwareCFO(DTCLib::DTC* dtc, int debugPacketCount, bool quiet, bool asyncRR) :
debugPacketCount_(debugPacketCount), quiet_(quiet), asyncRR_(asyncRR), requestsSent_(false), abort_(false)
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
    for (auto ring : DTCLib::DTC_Rings){
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
                            (uint16_t)debugPacketCount_);
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
    requestsSent_ = true;
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRange(int count, DTCLib::DTC_Timestamp start, bool increment, int delayBetweenDataRequests)
{
    requestsSent_ = false;
    if (asyncRR_) {
        theThread_ = std::thread(&DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplAsync, this, start, count, increment, delayBetweenDataRequests);
    }
    else
    {
        theThread_ = std::thread(&DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplSync, this, start, count, increment, delayBetweenDataRequests);
    }
    WaitForRequestsToBeSent();
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplSync(DTCLib::DTC_Timestamp start, int count,
    bool increment, int delayBetweenDataRequests)
{
    TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImplSync Start");
    for (int ii = 0; ii < count; ++ii) {
        DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);

        SendRequestForTimestamp(ts);

        usleep(delayBetweenDataRequests);
        if (abort_) return;
    }
}

void DTCLib::DTCSoftwareCFO::SendRequestsForRangeImplAsync(DTCLib::DTC_Timestamp start, int count,
    bool increment, int delayBetweenDataRequests)
{
    TRACE( 19, "DTCSoftwareCFO::SendRequestsForRangeImplAsync Start");

    // Send Readout Requests first
    for (int ii = 0; ii < count; ++ii) {
        DTCLib::DTC_Timestamp ts = start + (increment ? ii : 0);
        for (auto ring : DTCLib::DTC_Rings){
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
                            TRACE(19, "DTCSoftwareCFO::SendRequestsForRangeImpl before DTC_DataRequestPacket req");
                            DTCLib::DTC_DataRequestPacket req(ring, (DTCLib::DTC_ROC_ID)roc, ts, true,
                                (uint16_t)debugPacketCount_);
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
