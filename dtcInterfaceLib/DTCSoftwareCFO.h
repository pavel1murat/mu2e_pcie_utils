///////////////////////////////////////////////////////////////////////////////
// The DTCSoftwareCFO class is responsible for sending ReadoutRequest and
// DataRequest packets to the DTC in the absence of a functioning CFO.
// Requests are sent asynchronously, and the overall behaviour is meant to
// emulate a system with a CFO as closely as possible.
// 
// Author: Eric Flumerfelt, FNAL RSI
// Date: 8/13/2015
///////////////////////////////////////////////////////////////////////////////


#ifndef DTCSOFTWARECFO_H
#define DTCSOFTWARECFO_H 1

#include "DTC.h"
#include "DTC_Types.h"

#include <thread>

namespace DTCLib {
    class DTCSoftwareCFO {
    public:
        DTCSoftwareCFO(int debugPacketCount = 0, bool quiet = false);
        DTCSoftwareCFO(DTC* dtc, int debugPacketCount = 0, bool quiet = false);
        ~DTCSoftwareCFO();

        void SendRequestForTimestamp(DTC_Timestamp ts = DTC_Timestamp((uint64_t)0));
        void SendRequestsForRange(int count, DTC_Timestamp start = DTC_Timestamp((uint64_t)0),
            bool increment = true, int delayBetweenDataRequests = 0);
        void SendRequestsForRangeImpl(DTC_Timestamp start, int count,
            bool increment = true, int delayBetweenDataRequests = 0);
        void setQuiet(bool quiet) { quiet_ = quiet; }
        void setDebugPacketCount(int dbc) { debugPacketCount_ = dbc; }
    private:
        // Request Parameters
        int debugPacketCount_;
        bool quiet_;


        // Object basic properties (not accessible)
        DTC *theDTC_;
        bool ownDTC_;
        DTC_RingEnableMode ringMode_[6];
        std::thread theThread_;
    };

} // namespace DTCLib
#endif //ifndef DTCSOFTWARECFO_H