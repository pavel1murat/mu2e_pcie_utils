#ifndef CFO_H
#define CFO_H

#include "CFO_Types.h"
#include "CFO_Packets.h"
#include "CFO_Registers.h"
#include <vector>
#include <memory>
#include <list>

namespace CFOLib
{
	/// <summary>
	/// The CFO class implements the data transfers to the CFO card. It derives from CFO_Registers, the class representing the CFO register space.
	/// </summary>
	class CFO : public CFO_Registers
	{
	public:
		/// <summary>
		/// Construct an instance of the CFO class
		/// </summary>
		/// <param name="expectedDesignVersion">Expected CFO Firmware Design Version. If set, will throw an exception if the CFO firmware does not match (Default: "")</param>
		/// <param name="mode">The desired simulation mode for the CFO (Default: Disabled)</param>
		/// <param name="rocMask">Which ROCs should be active. Each hex digit corresponds to a Ring, and the number indicates how many ROCs are active on that ring. (Default: 0x1)</param>
		explicit CFO(std::string expectedDesignVersion = "", CFO_SimMode mode = CFO_SimMode_Disabled, unsigned rocMask = 0x1);
		virtual ~CFO();

		//
		// DMA Functions
		//
		/// <summary>
		/// Read a file into the CFO memory. Will truncate the file so that it fits in the CFO memory.
		/// </summary>
		/// <param name="file">File name to read into CFO memory</param>
		/// <param name="goForever">Whether readout should loop through the file</param>
		/// <param name="overwriteEnvrionment">Whether to use file instead of CFOLIB_SIM_FILE</param>
		/// <param name="outputFileName">Name of binary file to write expected output (Default: "", no file created)</param>
		/// <param name="skipVerify">Skip the verify stage of WriteSimFileToCFO</param>
		void WriteSimFileToCFO(std::string file, bool goForever, bool overwriteEnvrionment = false, std::string outputFileName = "", bool skipVerify = false);
		/// <summary>
		/// Read the CFO memory and determine whether the file was written correctly.
		/// </summary>
		/// <param name="file">File to verify against</param>
		/// <param name="rawOutputFilename">Default: "". If set, file to write data retrieved from the CFO to</param>
		/// <returns>True if file is in CFO memory without errors</returns>
		bool VerifySimFileInCFO(std::string file, std::string rawOutputFilename = "");


		// For loopback testing...
		/// <summary>
		/// The "first read" flag resets the cached data, allowing the CFO class to recover from invalid data from the CFO
		/// </summary>
		/// <param name="read">Value of the flag</param>
		void SetFirstRead(bool read)
		{
			first_read_ = read;
		}

		/// <summary>
		/// Writes the given data buffer to the CFO's DDR memory, via the DAQ channel.
		/// </summary>
		/// <param name="buf">DMA buffer to write. Must have an inclusive 64-bit byte count at the beginning, followed by an exclusive 64-bit block count.</param>
		/// <param name="sz">Size ofthe data in the buffer</param>
		void WriteDetectorEmulatorData(cfo_databuff_t* buf, size_t sz);
		
		
	private:

		uint32_t bufferIndex_;
		bool first_read_;
		uint16_t daqDMAByteCount_;
		uint16_t dcsDMAByteCount_;
	};
}
#endif

