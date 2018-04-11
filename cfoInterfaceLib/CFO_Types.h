#ifndef CFO_TYPES_H
#define CFO_TYPES_H

#include <bitset> // std::bitset
#include <cstdint> // uint8_t, uint16_t
#include <vector> // std::vector

#include <iomanip>

namespace CFOLib
{
	enum CFO_Ring_ID : uint8_t
	{
		CFO_Ring_0 = 0,
		CFO_Ring_1 = 1,
		CFO_Ring_2 = 2,
		CFO_Ring_3 = 3,
		CFO_Ring_4 = 4,
		CFO_Ring_5 = 5,
		CFO_Ring_6 = 6,
		CFO_Ring_7 = 7,
		CFO_Ring_Unused,
	};

	static const std::vector<CFO_Ring_ID> CFO_Rings{ CFO_Ring_0, CFO_Ring_1, CFO_Ring_2, CFO_Ring_3, CFO_Ring_4, CFO_Ring_5, CFO_Ring_6, CFO_Ring_7 };
	
	enum CFO_OscillatorType
	{
		CFO_OscillatorType_SERDES,
		CFO_OscillatorType_DDR,
	};

	enum CFO_SerdesClockSpeed
	{
		CFO_SerdesClockSpeed_25Gbps,
		CFO_SerdesClockSpeed_3125Gbps,
		CFO_SerdesClockSpeed_Unknown
	};

	enum CFO_RXBufferStatus
	{
		CFO_RXBufferStatus_Nominal = 0,
		CFO_RXBufferStatus_BufferEmpty = 1,
		CFO_RXBufferStatus_BufferFull = 2,
		CFO_RXBufferStatus_Underflow = 5,
		CFO_RXBufferStatus_Overflow = 6,
		CFO_RXBufferStatus_Unknown = 0x10,
	};

	/// <summary>
	/// The CFO_RXBufferStatusConverter converts a CFO_RXBufferStatus enumeration value to string or JSON representation
	/// </summary>
	struct CFO_RXBufferStatusConverter
	{
		CFO_RXBufferStatus status_; ///< CFO_RXBufferStatus to convert

		/// <summary>
		/// Construct a CFO_RXBufferStatusConverter instance using the given CFO_RXBufferStatus
		/// </summary>
		/// <param name="status">CFO_RXBufferStatus to convert</param>
		explicit CFO_RXBufferStatusConverter(CFO_RXBufferStatus status) : status_(status) {}

		/// <summary>
		/// Convert the CFO_RXBufferStatus to its string representation
		/// </summary>
		/// <returns>String representation of CFO_RXBufferStatus</returns>
		std::string toString() const
		{
			switch (status_)
			{
			case CFO_RXBufferStatus_Nominal:
				return "Nominal";
			case CFO_RXBufferStatus_BufferEmpty:
				return "BufferEmpty";
			case CFO_RXBufferStatus_BufferFull:
				return "BufferFull";
			case CFO_RXBufferStatus_Overflow:
				return "Overflow";
			case CFO_RXBufferStatus_Underflow:
				return "Underflow";
			case CFO_RXBufferStatus_Unknown:
			default:
				return "Unknown";
			}
		}

		/// <summary>
		/// Write a CFO_RXBufferStatusConverter in JSON format to the given stream
		/// </summary>
		/// <param name="stream">Stream to write</param>
		/// <param name="status">CFO_RXBufferStatusConverter to serialize</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_RXBufferStatusConverter& status)
		{
			stream << "\"CFO_RXBufferStatus\":\"" << status.toString() << "\"";
			return stream;
		}
	};

	enum CFO_RXStatus
	{
		CFO_RXStatus_DataOK = 0,
		CFO_RXStatus_SKPAdded = 1,
		CFO_RXStatus_SKPRemoved = 2,
		CFO_RXStatus_ReceiverDetected = 3,
		CFO_RXStatus_DecodeError = 4,
		CFO_RXStatus_ElasticOverflow = 5,
		CFO_RXStatus_ElasticUnderflow = 6,
		CFO_RXStatus_RXDisparityError = 7,
	};

	/// <summary>
	/// The CFO_RXStatusConverter converts a CFO_RXStatus enumeration value to string or JSON representation
	/// </summary>
	struct CFO_RXStatusConverter
	{
		CFO_RXStatus status_; ///< CFO_RXStatus to convert

		/// <summary>
		/// Construct a CFO_RXStatusConverter instance using the given CFO_RXStatus
		/// </summary>
		/// <param name="status">CFO_RXStatus to convert</param>
		explicit CFO_RXStatusConverter(CFO_RXStatus status);

		/// <summary>
		/// Convert the CFO_RXStatus to its string representation
		/// </summary>
		/// <returns>String representation of CFO_RXStatus</returns>
		std::string toString() const
		{
			switch (status_)
			{
			case CFO_RXStatus_DataOK:
				return "DataOK";
			case CFO_RXStatus_SKPAdded:
				return "SKPAdded";
			case CFO_RXStatus_SKPRemoved:
				return "SKPRemoved";
			case CFO_RXStatus_ReceiverDetected:
				return "ReceiverDetected";
			case CFO_RXStatus_DecodeError:
				return "DecodeErr";
			case CFO_RXStatus_ElasticOverflow:
				return "ElasticOF";
			case CFO_RXStatus_ElasticUnderflow:
				return "ElasticUF";
			case CFO_RXStatus_RXDisparityError:
				return "RXDisparity";
			}
			return "Unknown";
		}

		/// <summary>
		/// Write a CFO_RXStatusConverter in JSON format to the given stream
		/// </summary>
		/// <param name="stream">Stream to write</param>
		/// <param name="status">CFO_RXStatusConverter to serialize</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_RXStatusConverter& status)
		{
			stream << "\"CFO_RXStatus\":\"" << status.toString() << "\"";
			return stream;
		}
	};

	enum CFO_SERDESLoopbackMode
	{
		CFO_SERDESLoopbackMode_Disabled = 0,
		CFO_SERDESLoopbackMode_NearPCS = 1,
		CFO_SERDESLoopbackMode_NearPMA = 2,
		CFO_SERDESLoopbackMode_FarPMA = 4,
		CFO_SERDESLoopbackMode_FarPCS = 6,
	};

	/// <summary>
	/// The CFO_SERDESLoopbackModeConverter converts a CFO_SERDESLoopbackMode enumeration value to string or JSON representation
	/// </summary>
	struct CFO_SERDESLoopbackModeConverter
	{
		CFO_SERDESLoopbackMode mode_; ///< CFO_SERDESLoopbackMode to convert

		/// <summary>
		/// Construct a CFO_SERDESLoopbackModeConverter instance using the given CFO_SERDESLoopbackMode
		/// </summary>
		/// <param name="mode">CFO_SERDESLoopbackMode to convert</param>
		explicit CFO_SERDESLoopbackModeConverter(CFO_SERDESLoopbackMode mode) : mode_(mode) {}

		/// <summary>
		/// Convert the CFO_SERDESLoopbackMode to its string representation
		/// </summary>
		/// <returns>String representation of CFO_SERDESLoopbackMode</returns>
		std::string toString() const
		{
			switch (mode_)
			{
			case CFO_SERDESLoopbackMode_Disabled:
				return "Disabled";
			case CFO_SERDESLoopbackMode_NearPCS:
				return "NearPCS";
			case CFO_SERDESLoopbackMode_NearPMA:
				return "NearPMA";
			case CFO_SERDESLoopbackMode_FarPMA:
				return "FarPMA";
			case CFO_SERDESLoopbackMode_FarPCS:
				return "FarPCS";
			}
			return "Unknown";
		}

		/// <summary>
		/// Write a CFO_SERDESLoopbackModeConverter in JSON format to the given stream
		/// </summary>
		/// <param name="stream">Stream to write</param>
		/// <param name="mode">CFO_SERDESLoopbackModeConverter to serialize</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_SERDESLoopbackModeConverter& mode)
		{
			stream << "\"CFO_SERDESLoopbackMode\":\"" << mode.toString() << "\"";
			return stream;
		}
	};

	/// <summary>
	/// The CFO_SimMode enumeration is used to control the behavior of the CFO class.
	/// 
	/// CFO_SimMode_Tracker, Calorimeter, CosmicVeto, Performance, and LargeFile activate the cfosim CFO emulator
	/// in the corresponding mode. 
	/// CFO_SimMode_Disabled does nothing to set up the CFO beyond basic initialization.
	/// CFO_SimMode_NoCFO enables the CFO CFO Emulator to send ReadoutRequest and DataRequest packets.
	/// CFO_SimMode_ROCEmulator enables the CFO ROC Emulator
	/// CFO_SimMode_Loopback enables the SERDES loopback on the CFO
	/// </summary>
	enum CFO_SimMode
	{
		CFO_SimMode_Disabled = 0,
		CFO_SimMode_Tracker = 1,
		CFO_SimMode_Calorimeter = 2,
		CFO_SimMode_CosmicVeto = 3,
		CFO_SimMode_NoCFO = 4,
		CFO_SimMode_ROCEmulator = 5,
		CFO_SimMode_Loopback = 6,
		CFO_SimMode_Performance = 7,
		CFO_SimMode_LargeFile = 8,
		CFO_SimMode_Invalid,
	};

	/// <summary>
	/// The CFO_SimModeConverter converts a CFO_SimMode enumeration value to string or JSON representation
	/// </summary>
	struct CFO_SimModeConverter
	{
		CFO_SimMode mode_; ///< CFO_SimMode to convert to string

		/// <summary>
		/// Construct a CFO_SimModeConverter instance using the given CFO_SimMode
		/// </summary>
		/// <param name="mode">CFO_SimMode to convert</param>
		explicit CFO_SimModeConverter(CFO_SimMode mode) : mode_(mode) {}

		/// <summary>
		/// Parse a string and return the CFO_SimMode which corresponds to it
		/// 
		/// Will search for SimMode name (see CFO_SimModeConverter::toString(),), or integer value (i.e. 1 = CFO_SimMode_Tracker, see enumeration definition)
		/// </summary>
		/// <param name="s">String to parse</param>
		/// <returns>CFO_SimMode corresponding to string</returns>
		static CFO_SimMode ConvertToSimMode(std::string s);

		/// <summary>
		/// Convert the CFO_SimMode to its string representation
		/// </summary>
		/// <returns>String representation of CFO_SimMode</returns>
		std::string toString() const
		{
			switch (mode_)
			{
			case CFO_SimMode_Tracker:
				return "Tracker";
			case CFO_SimMode_Calorimeter:
				return "Calorimeter";
			case CFO_SimMode_CosmicVeto:
				return "CosmicVeto";
			case CFO_SimMode_NoCFO:
				return "NoCFO";
			case CFO_SimMode_ROCEmulator:
				return "ROCEmulator";
			case CFO_SimMode_Loopback:
				return "Loopback";
			case CFO_SimMode_Performance:
				return "Performance";
			case CFO_SimMode_LargeFile:
				return "LargeFile";
			case CFO_SimMode_Disabled:
			default:
				return "Disabled";
			}
		}

		/// <summary>
		/// Write a CFO_SimModeConverter in JSON format to the given stream
		/// </summary>
		/// <param name="stream">Stream to write</param>
		/// <param name="mode">CFO_SimModeConverter to serialize</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_SimModeConverter& mode)
		{
			stream << "\"CFO_SimMode\":\"" << mode.toString() << "\"";
			return stream;
		}
	};

	/// <summary>
	/// A CFO_WrongVersionException is thrown when an attempt to initialize a CFO is made with a certain firmware version expected, and the firmware does not match that version
	/// </summary>
	class CFO_WrongVersionException : public std::exception
	{
	public:
		/// <summary>
		/// A CFO_WrongVersionException is thrown when an attempt is made to construct a CFO packet with data that does not match the packet type
		/// </summary>
		/// <param name="expected">Expected firmware version string</param>
		/// <param name="encountered">Encountered firmware version string</param>
		CFO_WrongVersionException(std::string expected, std::string encountered) : expected_(expected), encountered_(encountered) {}
		/// <summary>
		/// Describe the exception
		/// </summary>
		/// <returns>String describing the exception</returns>
		const char* what() const throw()
		{
			return ("Unexpected firmware version encountered: " + encountered_ + " != " + expected_ + " (expected)").c_str();
		}
		std::string expected_; ///< Expected Firmware version string
		std::string encountered_; ///< Firmware version string of CFO
	};

	/// <summary>
	/// A CFO_WrongPacketTypeException is thrown when an attempt to decode a DMA packet is made and the type in the DMA header is different than the type of the packet expected
	/// </summary>
	class CFO_WrongPacketTypeException : public std::exception
	{
	public:
		/// <summary>
		/// A CFO_WrongPacketTypeException is thrown when an attempt is made to construct a CFO packet with data that does not match the packet type
		/// </summary>
		/// <param name="expected">Expected packet type</param>
		/// <param name="encountered">Encountered packet type</param>
		CFO_WrongPacketTypeException(int expected, int encountered) : expected_(expected), encountered_(encountered) {}
		/// <summary>
		/// Describe the exception
		/// </summary>
		/// <returns>String describing the exception</returns>
		const char* what() const throw()
		{
			return ("Unexpected packet type encountered: " + std::to_string(encountered_) + " != " + std::to_string(expected_) + " (expected)").c_str();
		}
		int expected_; ///< Packet type which was expected based on the type of CFO packet constructed
		int encountered_; ///< Packet type encountered in data
	};

	/// <summary>
	/// A CFO_IOErrorException is thrown when the CFO is not communicating when communication is expected
	/// </summary>
	class CFO_IOErrorException : public std::exception
	{
	public:
		/// <summary>
		/// Describe the exception
		/// </summary>
		/// <returns>String describing the exception</returns>
		const char* what() const throw()
		{
			return "Unable to communicate with the CFO";
		}
	};

	/// <summary>
	/// A CFO_DataCorruptionException is thrown when corrupt data is detected coming from the CFO
	/// </summary>
	class CFO_DataCorruptionException : public std::exception
	{
	public:
		/// <summary>
		/// Describe the exception
		/// </summary>
		/// <returns>String describing the exception</returns>
		const char* what() const throw()
		{
			return "Corruption detected in data stream from CFO";
		}
	};

	/// <summary>
	/// The mu2e Timestamp is a 48-bit quantity. This class manages all the different ways it could be accessed.
	/// </summary>
	class CFO_Timestamp
	{
		uint64_t timestamp_ : 48;
	public:
		/// <summary>
		/// Default Constructor. Initializes timestamp to value 0
		/// </summary>
		CFO_Timestamp();
		/// <summary>
		/// Construct a timestamp using the given quad word
		/// </summary>
		/// <param name="timestamp">64-bit unsigned integer representing timestamp. Top 16 bits will be discarded</param>
		explicit CFO_Timestamp(const uint64_t timestamp);
		/// <summary>
		/// Construct a timestamp using the given low and high words
		/// </summary>
		/// <param name="timestampLow">Lower 32 bits of timestamp</param>
		/// <param name="timestampHigh">Upper 16 bits of timestamp</param>
		CFO_Timestamp(const uint32_t timestampLow, const uint16_t timestampHigh);
		/// <summary>
		/// Construct a CFO_Timestamp using the given byte array. Length of the array must be greater than 6 + offset!
		/// </summary>
		/// <param name="timeArr">Byte array to read timestamp from (i.e. CFO_DataPacket::GetData())</param>
		/// <param name="offset">Location of timestamp byte 0 in array (Default 0)</param>
		explicit CFO_Timestamp(const uint8_t* timeArr, int offset = 0);
		/// <summary>
		/// Construct a CFO_Timestamp using a std::bitset of the 48-bit timestamp
		/// </summary>
		/// <param name="timestamp">std::bitset containing timestamp</param>
		explicit CFO_Timestamp(const std::bitset<48> timestamp);
		/// <summary>
		/// Default copy constructor
		/// </summary>
		/// <param name="r">CFO_Timestamp to copy</param>
		CFO_Timestamp(const CFO_Timestamp& r) = default;
		/// <summary>
		/// Default move constructor
		/// </summary>
		/// <param name="r">CFO_Timestamp rvalue</param>
		CFO_Timestamp(CFO_Timestamp&& r) = default;

		/// <summary>
		/// CFO_Timestamp Default Destructor
		/// </summary>
		virtual ~CFO_Timestamp() = default;
		/// <summary>
		/// Default move assignment operator
		/// </summary>
		/// <param name="r">CFO_Timestamp rvalue</param>
		/// <returns>CFO_Timestamp reference</returns>
		CFO_Timestamp& operator=(CFO_Timestamp&& r) = default;
		/// <summary>
		/// Default copy assignment operator
		/// </summary>
		/// <param name="r">CFO_Timestamp to copy</param>
		/// <returns>CFO_Timestamp reference</returns>
		CFO_Timestamp& operator=(const CFO_Timestamp& r) = default;

		/// <summary>
		/// Compare two CFO_Timestamp instances
		/// </summary>
		/// <param name="r">Other timestamp</param>
		/// <returns>Result of comparison</returns>
		bool operator==(const CFO_Timestamp r) const
		{
			return r.GetTimestamp(true) == timestamp_;
		}

		/// <summary>
		/// Compare two CFO_Timestamp instances
		/// </summary>
		/// <param name="r">Other timestamp</param>
		/// <returns>Result of comparison</returns>
		bool operator!=(const CFO_Timestamp r) const
		{
			return r.GetTimestamp(true) != timestamp_;
		}

		/// <summary>
		/// Compare two CFO_Timestamp instances
		/// </summary>
		/// <param name="r">Other timestamp</param>
		/// <returns>Result of comparison</returns>
		bool operator<(const CFO_Timestamp r)
		{
			return r.GetTimestamp(true) > timestamp_;
		}

		/// <summary>
		/// Compare two CFO_Timestamp instances
		/// </summary>
		/// <param name="r">Other timestamp</param>
		/// <returns>Result of comparison</returns>
		bool operator<(const CFO_Timestamp r) const
		{
			return r.GetTimestamp(true) > timestamp_;
		}

		/// <summary>
		/// Add an integer to a timestamp instance
		/// </summary>
		/// <param name="r">Integer to add to timestamp</param>
		/// <returns>New timestamp with result</returns>
		CFO_Timestamp operator+(const int r) const
		{
			return CFO_Timestamp(r + timestamp_);
		}

		/// <summary>
		/// Set the timestamp using the given quad word
		/// </summary>
		/// <param name="timestamp">64-bit unsigned integer representing timestamp. Top 16 bits will be discarded</param>
		void SetTimestamp(uint64_t timestamp)
		{
			timestamp_ = timestamp & 0x0000FFFFFFFFFFFF;
		}

		/// <summary>
		/// Set the timestamp using the given low and high words
		/// </summary>
		/// <param name="timestampLow">Lower 32 bits of the timestamp</param>
		/// <param name="timestampHigh">Upper 16 bits of the timstamp</param>
		void SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh);

		/// <summary>
		/// Returns the timstamp as a 48-bit std::bitset
		/// </summary>
		/// <returns>The timestamp as a 48-bit std::bitset</returns>
		std::bitset<48> GetTimestamp() const
		{
			return timestamp_;
		}

		/// <summary>
		/// Returns the timestamp as a 64-bit unsigned integer
		/// </summary>
		/// <param name="dummy">Whether to return a timestamp (used to distinguish signature)</param>
		/// <returns>Timestamp as a 64-bit unsigned integer</returns>
		uint64_t GetTimestamp(bool dummy) const
		{
			if (dummy)
			{
				return timestamp_;
			}
			return 0;
		}

		/// <summary>
		/// Copies the timestamp into the given byte array, starting at some offset.
		/// Size of input array MUST be larger than offset + 6
		/// </summary>
		/// <param name="timeArr">Byte array for output</param>
		/// <param name="offset">Target index of byte 0 of the timestamp</param>
		void GetTimestamp(const uint8_t* timeArr, int offset = 0) const;

		/// <summary>
		/// Convert the timestamp to a JSON representation
		/// </summary>
		/// <param name="arrayMode">(Default: false) If true, will create a JSON array of the 6 bytes. Otherwise, represents timestamp as a single number</param>
		/// <returns>JSON-formatted string containing timestamp</returns>
		std::string toJSON(bool arrayMode = false) const;

		/// <summary>
		/// Convert the 48-bit timestamp to the format used in the Packet format definitions.
		/// Byte 1 | Byte 0
		/// Byte 3 | Byte 2
		/// Byte 5 | Byte 4
		/// </summary>
		/// <returns>String representing timestamp in "packet format"</returns>
		std::string toPacketFormat() const;
	};

	/// <summary>
	/// This class is used to decode the SERDES RX Disparity Error register
	/// </summary>
	class CFO_SERDESRXDisparityError
	{
		std::bitset<2> data_;

	public:
		/// <summary>
		/// Default Constructor
		/// </summary>
		CFO_SERDESRXDisparityError();
		/// <summary>
		/// Construct a CFO_SERDESRXDisparityError using the given bitset
		/// </summary>
		/// <param name="data">Bits to use for initialization</param>
		explicit CFO_SERDESRXDisparityError(std::bitset<2> data);
		/// <summary>
		/// Construct a CFO_SERDESRXDisparityError using the register value and a Ring ID
		/// </summary>
		/// <param name="data">Register value to read error bits from</param>
		/// <param name="ring">Ring to read</param>
		CFO_SERDESRXDisparityError(uint32_t data, CFO_Ring_ID ring);
		/// <summary>
		/// Default Copy Constructor
		/// </summary>
		/// <param name="in">CFO_SERDESRXDisparityError to copy</param>
		CFO_SERDESRXDisparityError(const CFO_SERDESRXDisparityError& in) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="in">CFO_SERDESRXDisparityError rvalue</param>
		CFO_SERDESRXDisparityError(CFO_SERDESRXDisparityError&& in) = default;

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="in">CFO_SERDESRXDisparityError to copy</param>
		/// <returns>CFO_SERDESRXDisparityError Reference</returns>
		CFO_SERDESRXDisparityError& operator=(const CFO_SERDESRXDisparityError& in) = default;
		/// <summary>
		/// Default Move Assignment Operator
		/// </summary>
		/// <param name="in">CFO_SERDESRXDisparityError rvalue</param>
		/// <returns>CFO_SERDESRXDisparityError reference</returns>
		CFO_SERDESRXDisparityError& operator=(CFO_SERDESRXDisparityError&& in) = default;

		/// <summary>
		/// Sets the data bits
		/// </summary>
		/// <param name="data">Data bits to set</param>
		void SetData(std::bitset<2> data)
		{
			data_ = data;
		}

		/// <summary>
		/// Gets the data bitset
		/// </summary>
		/// <returns>Data bits</returns>
		std::bitset<2> GetData() const
		{
			return data_;
		}

		/// <summary>
		/// Get the integer representation of the error flags
		/// </summary>
		/// <param name="output">Whether to output data (signature different than bitset version)</param>
		/// <returns>Data as int</returns>
		int GetData(bool output) const
		{
			if (output) return static_cast<int>(data_.to_ulong());
			return 0;
		}

		/// <summary>
		/// Write the CFO_SERDESRXDisparityError to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="error">CFO_SERDESRXDisparityError to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, CFO_SERDESRXDisparityError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	/// <summary>
	/// This structure is used to decode the SERDES Character Not In Table Error register
	/// </summary>
	class CFO_CharacterNotInTableError
	{
		std::bitset<2> data_;

	public:
		/// <summary>
		/// Default Constructor
		/// Initializes data bits to 0,0
		/// </summary>
		CFO_CharacterNotInTableError();
		/// <summary>
		/// Constructor using data bits
		/// </summary>
		/// <param name="data">data bits for this instance</param>
		explicit CFO_CharacterNotInTableError(std::bitset<2> data);
		/// <summary>
		/// CFO_CharacterNotInTableError Constructor
		/// </summary>
		/// <param name="data">Register value</param>
		/// <param name="ring">Specific ring to read</param>
		CFO_CharacterNotInTableError(uint32_t data, CFO_Ring_ID ring);
		/// <summary>
		/// Default Copy Constructor
		/// </summary>
		/// <param name="in">CFO_CharacterNotInTableError to copy</param>
		CFO_CharacterNotInTableError(const CFO_CharacterNotInTableError& in) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="in">CFO_CharacterNotInTableError rvalue</param>
		CFO_CharacterNotInTableError(CFO_CharacterNotInTableError&& in) = default;

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="in">CFO_CharacterNotInTableError to copy</param>
		/// <returns>CFO_CharacterNotInTableError reference</returns>
		CFO_CharacterNotInTableError& operator=(const CFO_CharacterNotInTableError& in) = default;
		/// <summary>
		/// Default Move Assignment Operator
		/// </summary>
		/// <param name="in">CFO_CharacterNotInTableError rvalue</param>
		/// <returns>CFO_CharacterNotInTableError reference</returns>
		CFO_CharacterNotInTableError& operator=(CFO_CharacterNotInTableError&& in) = default;

		/// <summary>
		/// Sets the data bits
		/// </summary>
		/// <param name="data">Data bits to set</param>
		void SetData(std::bitset<2> data)
		{
			data_ = data;
		}

		/// <summary>
		/// Gets the data bitset
		/// </summary>
		/// <returns>Data bits</returns>
		std::bitset<2> GetData() const
		{
			return data_;
		}

		/// <summary>
		/// Get the integer representation of the error flags
		/// </summary>
		/// <param name="output">Whether to output data (signature different than bitset version)</param>
		/// <returns>Data as int</returns>
		int GetData(bool output) const
		{
			if (output) return static_cast<int>(data_.to_ulong());
			return 0;
		}

		/// <summary>
		/// Write the CFO_CharacterNotInTableError to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="error">CFO_CharacterNotInTableError to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, CFO_CharacterNotInTableError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	/// <summary>
	/// This structure is used to decode the RingEnable register value
	/// </summary>
	struct CFO_RingEnableMode
	{
		bool TransmitEnable; ///< Whether transmit is enabled on this link
		bool ReceiveEnable; ///< Whether receive is enabled on this link
		bool TimingEnable; ///< Whether timing is enabled on this link

		/// <summary>
		/// Default constructor. Sets all enable bits to true.
		/// </summary>
		CFO_RingEnableMode() : TransmitEnable(true), ReceiveEnable(true), TimingEnable(true) {}

		/// <summary>
		/// Construct a CFO_RingEnableMode instance with the given flags
		/// </summary>
		/// <param name="transmit">Enable TX</param>
		/// <param name="receive">Enable RX</param>
		/// <param name="timing">Enable CFO</param>
		CFO_RingEnableMode(bool transmit, bool receive, bool timing) : TransmitEnable(transmit), ReceiveEnable(receive), TimingEnable(timing) {}

		/// <summary>
		/// Write the CFO_RingEnableMode to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="mode">CFO_RingEnableMode to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_RingEnableMode& mode)
		{
			auto formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
			stream.setf(std::ios_base::boolalpha);
			stream << "{\"TransmitEnable\":" << mode.TransmitEnable << ",\"ReceiveEnable\":" << mode.ReceiveEnable << ",\"TimingEnable\":" << mode.TimingEnable << "}";
			if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
			return stream;
		}

		/// <summary>
		/// Determine if two CFO_RingEnableMode objects are equal
		/// They are equal if TX, RX and Timing bits are all equal
		/// </summary>
		/// <param name="left">LHS of compare</param>
		/// <param name="right">RHS of compare</param>
		/// <returns>Whether all three bits of both sides are equal</returns>
		friend bool operator==(const CFO_RingEnableMode& left, const CFO_RingEnableMode& right)
		{
			return left.TransmitEnable == right.TransmitEnable && left.ReceiveEnable == right.ReceiveEnable && left.TimingEnable == right.TimingEnable;
		}

		/// <summary>
		/// Determine if two CFO_RingEnableMode objects are not equal
		/// </summary>
		/// <param name="left">LHS of compare</param>
		/// <param name="right">RHS of compare</param>
		/// <returns>!(left == right)</returns>
		friend bool operator!=(const CFO_RingEnableMode& left, const CFO_RingEnableMode& right)
		{
			return !(left == right);
		}
	};

	/// <summary>
	/// This structure is used to decode the FIFOFullErrorFlags register values
	/// </summary>
	struct CFO_FIFOFullErrorFlags
	{
		bool OutputData; ///< Output Data FIFO Full
		bool CFOLinkInput; ///< CFO Link Input FIFO Full
		bool ReadoutRequestOutput; ///< Readout Request Output FIFO Full
		bool DataRequestOutput; ///< Data Request Output FIFO Full
		bool OtherOutput; ///< Other Output FIFO Full
		bool OutputDCS; ///< Output DCS FIFO Full
		bool OutputDCSStage2; ///< Output DCS Stage 2 FIFO Full
		bool DataInput; ///< Data Input FIFO Full
		bool DCSStatusInput; ///< DCS Status Input FIFO Full

		/// <summary>
		/// Default Constructor, sets all flags to false
		/// </summary>
		CFO_FIFOFullErrorFlags()
			: OutputData(false)
			, CFOLinkInput(false)
			, ReadoutRequestOutput(false)
			, DataRequestOutput(false)
			, OtherOutput(false)
			, OutputDCS(false)
			, OutputDCSStage2(false)
			, DataInput(false)
			, DCSStatusInput(false)
		{}

		/// <summary>
		/// Construct a CFO_FIFOFUllErrorFlags instance with the given values
		/// </summary>
		/// <param name="outputData">Output Data FIFO Full</param>
		/// <param name="cfoLinkInput">CFO Link Input FIFO Full</param>
		/// <param name="readoutRequest">Readout Request FIFO Full</param>
		/// <param name="dataRequest">Data Request FIFO Full</param>
		/// <param name="otherOutput"> Other Output FIFO Full</param>
		/// <param name="outputDCS">Output DCS FIFO Full</param>
		/// <param name="outputDCS2">Output DCS Stage 2 FIFO Full</param>
		/// <param name="dataInput">Data Input FIFO Full</param>
		/// <param name="dcsInput">DCS Status Input FIFO Full</param>
		CFO_FIFOFullErrorFlags(bool outputData, bool cfoLinkInput, bool readoutRequest, bool dataRequest,
							   bool otherOutput, bool outputDCS, bool outputDCS2, bool dataInput, bool dcsInput)
			: OutputData(outputData)
			, CFOLinkInput(cfoLinkInput)
			, ReadoutRequestOutput(readoutRequest)
			, DataRequestOutput(dataRequest)
			, OtherOutput(otherOutput)
			, OutputDCS(outputDCS)
			, OutputDCSStage2(outputDCS2)
			, DataInput(dataInput)
			, DCSStatusInput(dcsInput)
		{}

		/// <summary>
		/// Write the CFO_FIFOFullErrorFlags to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="flags">Flags to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_FIFOFullErrorFlags& flags)
		{
			auto formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
			stream.setf(std::ios_base::boolalpha);
			stream << "{\"OutputData\":" << flags.OutputData
				<< ",\"CFOLinkInput\":" << flags.CFOLinkInput
				<< ",\"ReadoutRequestOutput\":" << flags.ReadoutRequestOutput
				<< ",\"DataRequestOutput\":" << flags.DataRequestOutput
				<< ",\"OtherOutput\":" << flags.OtherOutput
				<< ",\"OutputDCS\":" << flags.OutputDCS
				<< ",\"OutputDCSStage2\":" << flags.OutputDCSStage2
				<< ",\"DataInput\":" << flags.DataInput
				<< ",\"DCSStatusInput\":" << flags.DCSStatusInput << "}";
			if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
			return stream;
		}
	};

	/// <summary>
	/// The CFO_RegisterFormatter class is used to print a CFO register in a human-readable format
	/// </summary>
	struct CFO_RegisterFormatter
	{
		/// <summary>
		/// Default Constructor with zero values
		/// </summary>
		CFO_RegisterFormatter() : address(0), value(0), descWidth(28), description(""), vals() {}

		/// <summary>
		/// Default Copy Consturctor
		/// </summary>
		/// <param name="r">CFO_RegisterFormatter to copy</param>
		CFO_RegisterFormatter(const CFO_RegisterFormatter& r) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="r">CFO_RegisterFormatter rvalue</param>
		CFO_RegisterFormatter(CFO_RegisterFormatter&& r) = default;
		uint16_t address; ///< Address of the register
		uint32_t value; ///< Value of the register
		int descWidth; ///< Display width of description field
		std::string description; ///< Description of the register (name)
		std::vector<std::string> vals; ///< Human-readable descriptions of register values (bits, etc)

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="r">RHS</param>
		/// <returns>CFO_RegisterFormatter regerence</returns>
		CFO_RegisterFormatter& operator=(const CFO_RegisterFormatter& r) = default;

		/// <summary>
		/// Write out the CFO_RegisterFormatter to the given stream. This function uses setw to make sure that fields for
		/// different registers still line up.
		/// Format is: "    0xADDR  | 0xVALUEXXX | DESCRIPTION: Variable size, minimum of 28 chars | Value 1
		///                                                                                        | Value 2 ..."
		/// </summary>
		/// <param name="stream">Stream to write register data to</param>
		/// <param name="reg">RegisterFormatter to write</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const CFO_RegisterFormatter& reg)
		{
			stream << std::hex << std::setfill('0');
			stream << "    0x" << std::setw(4) << static_cast<int>(reg.address) << "  | 0x" << std::setw(8) << static_cast<int>(reg.value) << " | ";
			auto tmp = reg.description;
			tmp.resize(reg.descWidth, ' ');
			stream << tmp << " | ";

			auto first = true;
			for (auto i : reg.vals)
			{
				if (!first)
				{
					std::string placeholder = "";
					placeholder.resize(reg.descWidth, ' ');
					stream << "                           " << placeholder << " | ";
				}
				stream << i << std::endl;
				first = false;
			}

			return stream;
		}
	};

	/// <summary>
	/// Several useful data manipulation utilities
	/// </summary>
	struct Utilities
	{
		/// <summary>
		/// Create a string with "[value] [unit]" for a given number of bytes, using FormatBytes(bytes)
		/// </summary>
		/// <param name="bytes">Number of bytes to convert to string</param>
		/// <param name="extraUnit">Extra units to be applied after converted byte unit (i.e. "/s")</param>
		/// <returns>String with converted value</returns>
		static std::string FormatByteString(double bytes, std::string extraUnit);
		/// <summary>
		/// Determine the best units for describing a given number of bytes.
		/// Algorithm will divide by 1024, and if the result is greater than 1 and less than 1024, use that unit.
		/// Maximum unit is TB.
		/// </summary>
		/// <param name="bytes">Number of bytes to format</param>
		/// <returns>Pair of Value in "best unit" and string representation of unit (i.e. "KB", "MB", etc)</returns>
		static std::pair<double, std::string> FormatBytes(double bytes);
		/// <summary>
		/// Create a string with "[value] [unit]" for a given number of seconds, using FormatTime(seconds)
		/// </summary>
		/// <param name="seconds">Number of seconds to convert to string</param>
		/// <returns>String with converted value</returns>
		static std::string FormatTimeString(double seconds);
		/// <summary>
		/// Determine the best units for describing a given number of seconds.
		/// Will find largest time span which has unit greater than 1, up to days, down to ns.
		/// </summary>
		/// <param name="seconds">Number of seconds to convert</param>
		/// <returns>Pair of Value in "best unit" and string representation of unit (i.e. "ns", "us", "hours", etc)</returns>
		static std::pair<double, std::string> FormatTime(double seconds);
	};
}

#endif //CFO_TYPES_H
