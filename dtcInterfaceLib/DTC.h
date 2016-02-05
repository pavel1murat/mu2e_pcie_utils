#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include "DTC_Packets.h"
#include "DTC_Registers.h"
#include <vector>
#include <cstdlib>
#include <atomic>

namespace DTCLib
{
	class DTC : public DTC_Registers
	{
	public:
		DTC(DTC_SimMode mode = DTC_SimMode_Disabled, std::string simFile = "");
		virtual ~DTC();
		
		//
		// DMA Functions
		//
		// Data read-out
		std::vector<void*> GetData(DTC_Timestamp when = DTC_Timestamp());
		std::string GetJSONData(DTC_Timestamp when = DTC_Timestamp());


		// DCS Register R/W
		uint16_t ReadROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address);
		void WriteROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address, const uint16_t data);
		uint16_t ReadExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint16_t address);
		void WriteExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint8_t address, const uint16_t data);
		std::string ROCRegDump(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc);

		// Broadcast Readout
		void SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet = true);
		void SendDCSRequestPacket(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_DCSOperationType type, const uint8_t address, const uint16_t data = 0x0, bool quiet = true);

		// For loopback testing...
		void SetFirstRead(bool read)
		{
			first_read_ = read;
		}

		void WriteDMAPacket(const DTC_DMAPacket& packet);
		void WriteDetectorEmulatorData(mu2e_databuff_t* buf, size_t sz);
		DTC_DataHeaderPacket* ReadNextDAQPacket(int tmo_ms = 0);
		DTC_DCSReplyPacket* ReadNextDCSPacket();

		void ReleaseAllBuffers(const DTC_DMA_Engine& channel)
		{
			device_.release_all(channel);
		}

	private:
		int ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms = 0);
		void WriteDataPacket(const DTC_DataPacket& packet);

	private:
		mu2e_databuff_t* daqbuffer_;
		int buffers_used_;
		mu2e_databuff_t* dcsbuffer_;
		uint32_t bufferIndex_;
		bool first_read_;
		uint16_t daqDMAByteCount_;
		uint16_t dcsDMAByteCount_;
		void* lastReadPtr_;
		void* nextReadPtr_;
		void* dcsReadPtr_;
	};
}
#endif

