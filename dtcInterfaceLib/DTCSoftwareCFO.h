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
#include <atomic>

namespace DTCLib {
    class DTCSoftwareCFO {
    public:
		DTCSoftwareCFO(bool useCFOEmulator, uint16_t debugPacketCount = 0,
			DTCLib::DTC_DebugType debugType = DTCLib::DTC_DebugType_ExternalSerialWithReset, bool stickyDebugType = false,
			bool quiet = false, bool asyncRR = false);
        DTCSoftwareCFO(DTC* dtc, bool useCFOEmulator, uint16_t debugPacketCount = 0, 
			DTCLib::DTC_DebugType debugType = DTCLib::DTC_DebugType_ExternalSerialWithReset, bool stickyDebugType = false,
			bool quiet = false, bool asyncRR = false);
        ~DTCSoftwareCFO();

        void SendRequestForTimestamp(DTC_Timestamp ts = DTC_Timestamp((uint64_t)0));
        void SendRequestsForRange(int count, DTC_Timestamp start = DTC_Timestamp((uint64_t)0),
            bool increment = true, uint32_t delayBetweenDataRequests = 0, int requestsAhead = 1);
        void setQuiet(bool quiet) { quiet_ = quiet; }
        void setDebugPacketCount(uint16_t dbc) { debugPacketCount_ = dbc; }
        void WaitForRequestsToBeSent();
	private:
        void SendRequestsForRangeImplAsync(DTC_Timestamp start, int count,
            bool increment = true, uint32_t delayBetweenDataRequests = 0);
        void SendRequestsForRangeImplSync(DTC_Timestamp start, int count,
            bool increment = true, uint32_t delayBetweenDataRequests = 0, int requestsAhead = 1);
    private:
        // Request Parameters
		bool useCFOEmulator_;
        uint16_t debugPacketCount_;
		DTCLib::DTC_DebugType debugType_;
		bool stickyDebugType_;
        bool quiet_; // Don't print as much
        bool asyncRR_;

        // Object basic properties (not accessible)
        DTC *theDTC_;
        bool ownDTC_;
        DTC_RingEnableMode ringMode_[6];
        std::thread theThread_;
        std::atomic<bool> requestsSent_;
        std::atomic<bool> abort_;
    };

} // namespace DTCLib
#endif //ifndef DTCSOFTWARECFO_H