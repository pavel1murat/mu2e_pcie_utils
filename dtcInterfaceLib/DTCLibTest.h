#ifndef DTCLibTest_H
#define DTCLibTest_H

#include "DTC.h"
#include <atomic>
#include <thread>

namespace DTCLib
{
	class DTCLibTest
	{
	public:
		DTCLibTest();
		virtual ~DTCLibTest();

		//Test Control
		void startTest(bool classEnabled, bool regIOEnabled,
		               bool daqEnabled, bool dcsEnabled, int nTests, bool printMessages = false);
		void stopTests();

		// Accessors
		bool isRunning() const
		{
			return running_;
		}

		int classPassed();
		int classFailed();
		int regPassed();
		int regFailed();
		int daqPassed();
		int daqFailed();
		int dcsPassed();
		int dcsFailed();
	private:
		//Test Worker
		void doTests();
		void doClassTest();
		void doRegTest();
		void doDCSTest();
		void doDAQTest();

		static bool DataPacketIntegrityCheck(DTC_DataPacket*);

		//Test Status
		std::atomic<bool> running_;

		std::atomic<int> classPassed_;
		std::atomic<int> classFailed_;
		std::atomic<int> regPassed_;
		std::atomic<int> regFailed_;
		std::atomic<int> daqPassed_;
		std::atomic<int> daqFailed_;
		std::atomic<int> dcsPassed_;
		std::atomic<int> dcsFailed_;

		int classPassedTemp_;
		int classFailedTemp_;
		int regPassedTemp_;
		int regFailedTemp_;
		int daqPassedTemp_;
		int daqFailedTemp_;
		int dcsPassedTemp_;
		int dcsFailedTemp_;

		// Test Internals
		DTC* thisDTC_;
		int nTests_;

		bool runClassTest_;
		bool runRegTest_;
		bool runDAQTest_;
		bool runDCSTest_;

		bool printMessages_;
		std::thread workerThread_;
	};
}
#endif //ifndef DTCLibTest_H


