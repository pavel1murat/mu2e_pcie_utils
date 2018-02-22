///////////////////////////////////////////////////////////////////////////////
// The CFOSoftwareCFO class is responsible for sending ReadoutRequest and
// DataRequest packets to the CFO in the absence of a functioning CFO.
// Requests are sent asynchronously, and the overall behaviour is meant to
// emulate a system with a CFO as closely as possible.
//
// Author: Eric Flumerfelt, FNAL RSI
// Date: 8/13/2015
///////////////////////////////////////////////////////////////////////////////

#ifndef CFOSOFTWARECFO_H
#define CFOSOFTWARECFO_H 1

#include "CFO.h"
#include "CFO_Types.h"

#include <thread>
#include <atomic>

namespace CFOLib
{
	/// <summary>
	/// DEPRECATED
	/// The CFOSoftwareCFO class is responsible for sending ReadoutRequest and
	/// DataRequest packets to the CFO in the absence of a functioning CFO.
	/// Requests are sent asynchronously, and the overall behaviour is meant to
	/// emulate a system with a CFO as closely as possible.
	/// </summary>
	class CFOSoftwareCFO
	{
	public:
		/// <summary>
		/// Construct an instance of the CFOSoftwareCFO class
		/// </summary>
		/// <param name="CFO">Pointer to CFO object to use for sending requests</param>
		/// <param name="useCFOEmulator">Whether to use the CFO CFO emulator</param>
		/// <param name="debugPacketCount">Number of debug packets to request</param>
		/// <param name="debugType">Flag for the debug packets</param>
		/// <param name="stickyDebugType">Whether the debug type should stay as the flag or revert to default</param>
		/// <param name="quiet">When true, don't print debug information to screen</param>
		/// <param name="asyncRR">Whether to send ReadoutRequests asynchronously</param>
		CFOSoftwareCFO(CFO* CFO, bool useCFOEmulator, uint16_t debugPacketCount = 0,
		               CFO_DebugType debugType = CFO_DebugType_ExternalSerialWithReset, bool stickyDebugType = false,
		               bool quiet = false, bool asyncRR = false);
		/// <summary>
		/// CFOSoftwareCFO Destructor
		/// </summary>
		~CFOSoftwareCFO();

		/// <summary>
		/// Send a readout request and data request for a given timestamp
		/// </summary>
		/// <param name="ts">Timestamp for requests</param>
		void SendRequestForTimestamp(CFO_Timestamp ts = CFO_Timestamp(static_cast<uint64_t>(0)));
		/// <summary>
		/// Send requests for a range of timestamps.
		/// </summary>
		/// <param name="count">Number of requests</param>
		/// <param name="start">Starting timestamp (Default: 0)</param>
		/// <param name="increment">Whether to increment the timestamp for each request (Default: true)</param>
		/// <param name="delayBetweenDataRequests">Number of microseconds to wait between requests</param>
		/// <param name="requestsAhead">Number of readout requests to send ahead of data requests</param>
		void SendRequestsForRange(int count, CFO_Timestamp start = CFO_Timestamp(static_cast<uint64_t>(0)),
		                          bool increment = true, uint32_t delayBetweenDataRequests = 0, int requestsAhead = 1);

		/// <summary>
		/// Enable quiet mode.
		/// 
		/// When in quiet mode, CFOSoftwareCFO will not print ReadoutRequest packets or DataRequest packets to screen.
		/// </summary>
		/// <param name="quiet">Whether to enable quiet mode.</param>
		void setQuiet(bool quiet)
		{
			quiet_ = quiet;
		}

		/// <summary>
		/// Set the number of packets requested
		/// </summary>
		/// <param name="dbc">Packet count</param>
		void setDebugPacketCount(uint16_t dbc)
		{
			debugPacketCount_ = dbc;
		}

		/// <summary>
		/// Blocks until all ReadoutRequest packets have been sent
		/// </summary>
		void WaitForRequestsToBeSent() const;

		/// <summary>
		/// Get a pointer to the CFO instance
		/// </summary>
		/// <returns>Pointer to the CFO instance</returns>
		CFO* GetCFO() const
		{
			return theCFO_;
		}

	private:
		void SendRequestsForRangeImplAsync(CFO_Timestamp start, int count,
		                                   bool increment = true, uint32_t delayBetweenDataRequests = 0);
		void SendRequestsForRangeImplSync(CFO_Timestamp start, int count,
		                                  bool increment = true, uint32_t delayBetweenDataRequests = 0, int requestsAhead = 1);
		// Request Parameters
		bool useCFOEmulator_;
		uint16_t debugPacketCount_;
		CFO_DebugType debugType_;
		bool stickyDebugType_;
		bool quiet_; // Don't print as much
		bool asyncRR_;

		// Object basic properties (not accessible)
		CFO* theCFO_;
		CFO_RingEnableMode ringMode_[6];
		std::thread theThread_;
		std::atomic<bool> requestsSent_;
		std::atomic<bool> abort_;
	};
} // namespace CFOLib
#endif //ifndef CFOSOFTWARECFO_H


