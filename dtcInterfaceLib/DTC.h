#ifndef DTC_H
#define DTC_H

#include "DTC_Types.h"
#include "DTC_Packets.h"
#include "DTC_Registers.h"
#include <vector>
#include <memory>
#include <list>

namespace DTCLib
{
	/// <summary>
	/// The DTC class implements the data transfers to the DTC card. It derives from DTC_Registers, the class representing the DTC register space.
	/// </summary>
	class DTC : public DTC_Registers
	{
	public:
		/// <summary>
		/// Construct an instance of the DTC class
		/// </summary>
		/// <param name="expectedDesignVersion">Expected DTC Firmware Design Version. If set, will throw an exception if the DTC firmware does not match (Default: "")</param>
		/// <param name="mode">The desired simulation mode for the DTC (Default: Disabled)</param>
		/// <param name="rocMask">Which ROCs should be active. Each hex digit corresponds to a Ring, and the number indicates how many ROCs are active on that ring. (Default: 0x1)</param>
		explicit DTC(std::string expectedDesignVersion = "", DTC_SimMode mode = DTC_SimMode_Disabled, unsigned rocMask = 0x1);
		virtual ~DTC();

		//
		// DMA Functions
		//
		// Data read-out
		/// <summary>
		/// Reads data from the DTC, and returns all data blocks with the same timestamp. If timestamp is specified, will look for data
		/// with that timestamp.
		/// </summary>
		/// <param name="when">Desired timestamp for readout. Default means use whatever timestamp is next</param>
		/// <returns>A vector of DTC_DataBlock objects</returns>
		std::vector<DTC_DataBlock> GetData(DTC_Timestamp when = DTC_Timestamp());
		/// <summary>
		/// Reads data from the DTC and returns all data blocks with the same timestamp, as a JSON string.
		/// </summary>
		/// <param name="when">Desired timestamp for readout. Default means use whatever timestamp is next</param>
		/// <returns>JSON data string</returns>
		std::string GetJSONData(DTC_Timestamp when = DTC_Timestamp());
		/// <summary>
		/// Read a file into the DTC memory. Will truncate the file so that it fits in the DTC memory.
		/// </summary>
		/// <param name="file">File name to read into DTC memory</param>
		/// <param name="goForever">Whether readout should loop through the file</param>
		/// <param name="overwriteEnvrionment">Whether to use file instead of DTCLIB_SIM_FILE</param>
		/// <param name="outputFileName">Name of binary file to write expected output (Default: "", no file created)</param>
		void WriteSimFileToDTC(std::string file, bool goForever, bool overwriteEnvrionment = false, std::string outputFileName = "");


		// DCS Register R/W
		/// <summary>
		/// Sends a DCS Request Packet with fields filled in such that the given ROC register will be read out.
		/// This function reads from the main ROC register space, use ReadExtROCRegister to access other firmware blocks' register spaces.
		/// </summary>
		/// <param name="ring">Ring of the ROC to read</param>
		/// <param name="roc">ROC ID of the ROC to read</param>
		/// <param name="address">Address of the register</param>
		/// <returns>Value of the ROC register from a DCS Reply packet</returns>
		uint16_t ReadROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address);
		/// <summary>
		/// Sends a DCS Request Packet with the fields filled in such that the given ROC register will be written.
		/// This function writes to the main ROC register space, use WriteExtROCRegister to access other firmware blocks' register spaces.
		/// </summary>
		/// <param name="ring">Ring of the ROC to write to</param>
		/// <param name="roc">ROC ID of the ROC to write to</param>
		/// <param name="address">Address of the register</param>
		/// <param name="data">Value to write</param>
		void WriteROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t address, const uint16_t data);
		/// <summary>
		/// Sends a DCS Request Packet with fields filled in such that the given ROC firmware block register will be read out.
		/// This funcion reads from firmware blocks' register spaces.
		/// </summary>
		/// <param name="ring">Ring of the ROC to read</param>
		/// <param name="roc">ROC ID of the ROC to read</param>
		/// <param name="block">Block ID to read from</param>
		/// <param name="address">Address of the register</param>
		/// <returns>Value of the ROC register from a DCS Reply packet</returns>
		uint16_t ReadExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint16_t address);
		/// <summary>
		/// Sends a DCS Request Packet with fields filled in such that the given ROC firmware block register will be written.
		/// This funcion writes to firmware blocks' register spaces.
		/// </summary>
		/// <param name="ring">Ring of the ROC to write to</param>
		/// <param name="roc">ROC ID of the ROC to write to</param>
		/// <param name="block">Block ID to write to</param>
		/// <param name="address">Address of the register</param>
		/// <param name="data">Value to write</param>
		void WriteExtROCRegister(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const uint8_t block, const uint8_t address, const uint16_t data);
		/// <summary>
		/// Dump all known registers from the given ROC, via DCS Request packets.
		/// </summary>
		/// <param name="ring">Ring of the ROC</param>
		/// <param name="roc">ROC ID of the ROC</param>
		/// <returns>JSON-formatted register dump</returns>
		std::string ROCRegDump(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc);

		// Broadcast Readout
		/// <summary>
		/// DEPRECATED
		/// Sends a Readout Request broadcast to given Ring.
		/// </summary>
		/// <param name="ring">Ring to send to</param>
		/// <param name="when">Timestamp for the Readout Request</param>
		/// <param name="quiet">Whether to not print the JSON representation of the Readout Request (Default: true, no JSON printed)</param>
		void SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when, bool quiet = true);
		/// <summary>
		/// Send a DCS Request Packet to the given ROC. Use the Read/Write ROC Register functions for more convinient register access.
		/// </summary>
		/// <param name="ring">Ring of the ROC</param>
		/// <param name="roc">ROC ID of the ROC</param>
		/// <param name="type">Operation to perform</param>
		/// <param name="address">Target address</param>
		/// <param name="data">Data to write, if operation is write</param>
		/// <param name="quiet">Whether to not print the JSON representation of the Readout Request (Default: true, no JSON printed)</param>
		void SendDCSRequestPacket(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_DCSOperationType type, const uint8_t address, const uint16_t data = 0x0, bool quiet = true);

		// For loopback testing...
		/// <summary>
		/// The "first read" flag resets the cached data, allowing the DTC class to recover from invalid data from the DTC
		/// </summary>
		/// <param name="read">Value of the flag</param>
		void SetFirstRead(bool read)
		{
			first_read_ = read;
		}

		/// <summary>
		/// Writes a packet to the DTC on the DCS channel
		/// </summary>
		/// <param name="packet">Packet to write</param>
		void WriteDMAPacket(const DTC_DMAPacket& packet);
		/// <summary>
		/// Writes the given data buffer to the DTC's DDR memory, via the DAQ channel.
		/// </summary>
		/// <param name="buf">DMA buffer to write. Must have an inclusive 64-bit byte count at the beginning, followed by an exclusive 64-bit block count.</param>
		/// <param name="sz">Size ofthe data in the buffer</param>
		void WriteDetectorEmulatorData(mu2e_databuff_t* buf, size_t sz);
		/// <summary>
		/// Reads the next available DataHeaderPacket. If no data is available in the cache, a new DMA buffer is obtained.
		/// Performs several validity checks on the data, if no data is available or data is invalid, will return nullptr.
		/// </summary>
		/// <param name="tmo_ms">Timeout before returning nullptr. Default: 0, no timeout</param>
		/// <returns>Pointer to DataHeaderPacket. Will be nullptr if no data available.</returns>
		DTC_DataHeaderPacket* ReadNextDAQPacket(int tmo_ms = 0);
		/// <summary>
		/// DCS packets are read one-at-a-time, this function reads the next one from the DTC
		/// </summary>
		/// <returns>Pointer to read DCSReplyPacket. Will be nullptr if no data available.</returns>
		DTC_DCSReplyPacket* ReadNextDCSPacket();

		/// <summary>
		/// Releases all buffers to the hardware, from both the DAQ and DCS channels
		/// </summary>
		void ReleaseAllBuffers()
		{
			ReleaseAllBuffers(DTC_DMA_Engine_DAQ);
			ReleaseAllBuffers(DTC_DMA_Engine_DCS);
		}

		/// <summary>
		/// Release all buffers to the hardware on the given channel
		/// </summary>
		/// <param name="channel">Channel to release</param>
		void ReleaseAllBuffers(const DTC_DMA_Engine& channel)
		{
			if (channel == DTC_DMA_Engine_DAQ) daqbuffer_.clear();
			else if (channel == DTC_DMA_Engine_DCS) dcsbuffer_.clear();
			device_.release_all(channel);
		}

	private:
		int ReadBuffer(const DTC_DMA_Engine& channel, int tmo_ms = 0);
		void WriteDataPacket(const DTC_DataPacket& packet);

		std::list<mu2e_databuff_t*> daqbuffer_;
		std::list<mu2e_databuff_t*> dcsbuffer_;
		bool lastDAQBufferActive_;
		bool lastDCSBufferActive_;
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

