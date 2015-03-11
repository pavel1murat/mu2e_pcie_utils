#ifndef DTCLibTest_H
#define DTCLibTest_H

#include "DTC.h"
#include <bitset>
#include <atomic>
#include <thread>

namespace DTC {
	class DTCLibTest {
	public:
		DTCLibTest();
		virtual ~DTCLibTest();

		//Test Control
		void startTest(bool regIOEnabled, bool pcieEnabled, bool dmaStateEnabled,
			       bool daqEnabled, bool dcsEnabled, bool loopbackEnabled, int nTests, bool printMessages = false);
		void stopTests();
		
		// Accessors
		bool isRunning() { return running_; }
		int regPassed();
		int regFailed();
		int pciePassed();
		int pcieFailed();
		int dmaStatePassed();
		int dmaStateFailed();
		int daqPassed();
		int daqFailed();
		int dcsPassed();
		int dcsFailed();
                int loopbackPassed();
                int loopbackFailed();
	private:
		//Test Worker
		void doTests();
		void doRegTest();
		void doPCIeTest();
		void doDMAStateTest();
		void doDCSTest();
		void doDAQTest();
                void doLoopbackTest();

		//Test Status
		std::atomic<bool> running_;
		std::atomic<int> regPassed_;
		std::atomic<int> regFailed_;
		std::atomic<int> pciePassed_;
		std::atomic<int> pcieFailed_;
		std::atomic<int> dmaStatePassed_;
		std::atomic<int> dmaStateFailed_;
		std::atomic<int> daqPassed_;
		std::atomic<int> daqFailed_;
		std::atomic<int> dcsPassed_;
		std::atomic<int> dcsFailed_;
		std::atomic<int> loopbackPassed_;
		std::atomic<int> loopbackFailed_;
		int regPassedTemp_;
		int regFailedTemp_;
		int pciePassedTemp_;
		int pcieFailedTemp_;
		int dmaStatePassedTemp_;
		int dmaStateFailedTemp_;
		int daqPassedTemp_;
		int daqFailedTemp_;
		int dcsPassedTemp_;
		int dcsFailedTemp_;
                int loopbackPassedTemp_;
                int loopbackFailedTemp_;

		// Test Internals
		DTC* thisDTC_;
		int nTests_;
		bool runRegTest_;
		bool runPCIeTest_;
		bool runDMAStateTest_;
		bool runDAQTest_;
		bool runDCSTest_;
                bool runLoopbackTest_;
		bool printMessages_;
		std::thread* workerThread_;
	};
}
#endif  //ifndef DTCLibTest_H
