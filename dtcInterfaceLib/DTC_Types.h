#ifndef DTC_TYPES_H
#define DTC_TYPES_H

#include <bitset> // std::bitset
#include <cstdint> // uint8_t, uint16_t
#include <vector> // std::vector

#include <iomanip>

namespace DTCLib
{
	const std::string ExpectedDesignVersion = "v1.4_2015-07-01-00";

	enum DTC_Subsystem : uint8_t
	{
		DTC_Subsystem_Tracker = 0,
		DTC_Subsystem_Calorimeter = 1,
		DTC_Subsystem_CRV = 2,
		DTC_Subsystem_Other = 3
	};

	/// <summary>
	/// The DTC_ID field is used to uniquely identify each DTC. The ID consists of 2 bits of Subsystem ID and 6 bits of DTC ID.
	/// </summary>
	class DTC_ID {
		uint8_t idData_;

	public:
		DTC_ID() : idData_(0) {}
		explicit DTC_ID(uint8_t id) : idData_(id) {}
		DTC_Subsystem GetSubsystem() const { return static_cast<DTC_Subsystem>(idData_ >> 6); }
		uint8_t GetID() const { return idData_ & 0x3F; }
		uint8_t GetWord() const { return idData_; }
	};

	enum DTC_Ring_ID : uint8_t
	{
		DTC_Ring_0 = 0,
		DTC_Ring_1 = 1,
		DTC_Ring_2 = 2,
		DTC_Ring_3 = 3,
		DTC_Ring_4 = 4,
		DTC_Ring_5 = 5,
		DTC_Ring_CFO = 6,
		DTC_Ring_EVB = 7,
		DTC_Ring_Unused,
	};

	static const std::vector<DTC_Ring_ID> DTC_Rings{ DTC_Ring_0, DTC_Ring_1, DTC_Ring_2, DTC_Ring_3, DTC_Ring_4, DTC_Ring_5 };

	enum DTC_ROC_ID : uint8_t
	{
		DTC_ROC_0 = 0,
		DTC_ROC_1 = 1,
		DTC_ROC_2 = 2,
		DTC_ROC_3 = 3,
		DTC_ROC_4 = 4,
		DTC_ROC_5 = 5,
		DTC_ROC_Unused,
	};

	static const std::vector<DTC_ROC_ID> DTC_ROCS{ DTC_ROC_Unused, DTC_ROC_0, DTC_ROC_1, DTC_ROC_2, DTC_ROC_3, DTC_ROC_4, DTC_ROC_5 };

	/// <summary>
	/// The DTC_ROCIDConverter converts a DTC_ROC_ID enumeration value to string or JSON representation
	/// </summary>
	struct DTC_ROCIDConverter
	{
		DTC_ROC_ID roc_;

		explicit DTC_ROCIDConverter(DTC_ROC_ID roc) : roc_(roc) { }

		std::string toString() const
		{
			switch (roc_)
			{
			case DTC_ROC_0:
				return "ROC_0";
			case DTC_ROC_1:
				return "ROC_1";
			case DTC_ROC_2:
				return "ROC_2";
			case DTC_ROC_3:
				return "ROC_3";
			case DTC_ROC_4:
				return "ROC_4";
			case DTC_ROC_5:
				return "ROC_5";
			case DTC_ROC_Unused:
			default:
				return "No ROCs";
			}
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_ROCIDConverter& roc)
		{
			stream << "\"DTC_ROC_ID\":\"" << roc.toString() << "\"";
			return stream;
		}
	};

	enum DTC_OscillatorType
	{
		DTC_OscillatorType_SERDES,
		DTC_OscillatorType_DDR,
	};

	enum DTC_SerdesClockSpeed
	{
		DTC_SerdesClockSpeed_25Gbps,
		DTC_SerdesClockSpeed_3125Gbps,
		DTC_SerdesClockSpeed_Unknown
	};

	enum DTC_DebugType
	{
		DTC_DebugType_SpecialSequence = 0,
		DTC_DebugType_ExternalSerial = 1,
		DTC_DebugType_ExternalSerialWithReset = 2,
		DTC_DebugType_RAMTest = 3,
		DTC_DebugType_DDRTest = 4,
		DTC_DebugType_Invalid = 5,
	};

	/// <summary>
	/// The DTC_DebugTypeConverter converts a DTC_DebugType enumeration value to string or JSON representation
	/// </summary>
	struct DTC_DebugTypeConverter
	{
		DTC_DebugType type_;

		explicit DTC_DebugTypeConverter(DTC_DebugType type) : type_(type) { }

		std::string toString() const
		{
			switch (type_)
			{
			case DTC_DebugType_SpecialSequence:
				return "Special Sequence";
			case DTC_DebugType_ExternalSerial:
				return "External Serial";
			case DTC_DebugType_ExternalSerialWithReset:
				return "External Serial with FIFO Reset";
			case DTC_DebugType_RAMTest:
				return "FPGA SRAM Error Checking";
			case DTC_DebugType_DDRTest:
				return "DDR3 Memory Error Checking";
			case DTC_DebugType_Invalid:
				return "INVALID!!!";
			}
			return "Unknown";
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_DebugTypeConverter& type)
		{
			stream << "\"DTC_DebugType\":\"" << type.toString() << "\"";
			return stream;
		}
	};

	enum DTC_RXBufferStatus
	{
		DTC_RXBufferStatus_Nominal = 0,
		DTC_RXBufferStatus_BufferEmpty = 1,
		DTC_RXBufferStatus_BufferFull = 2,
		DTC_RXBufferStatus_Underflow = 5,
		DTC_RXBufferStatus_Overflow = 6,
		DTC_RXBufferStatus_Unknown = 0x10,
	};

	/// <summary>
	/// The DTC_RXBufferStatusConverter converts a DTC_RXBufferStatus enumeration value to string or JSON representation
	/// </summary>
	struct DTC_RXBufferStatusConverter
	{
		DTC_RXBufferStatus status_;

		explicit DTC_RXBufferStatusConverter(DTC_RXBufferStatus status) : status_(status) { }

		std::string toString() const
		{
			switch (status_)
			{
			case DTC_RXBufferStatus_Nominal:
				return "Nominal";
			case DTC_RXBufferStatus_BufferEmpty:
				return "BufferEmpty";
			case DTC_RXBufferStatus_BufferFull:
				return "BufferFull";
			case DTC_RXBufferStatus_Overflow:
				return "Overflow";
			case DTC_RXBufferStatus_Underflow:
				return "Underflow";
			case DTC_RXBufferStatus_Unknown:
			default:
				return "Unknown";
			}
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_RXBufferStatusConverter& status)
		{
			stream << "\"DTC_RXBufferStatus\":\"" << status.toString() << "\"";
			return stream;
		}
	};

	enum DTC_RXStatus
	{
		DTC_RXStatus_DataOK = 0,
		DTC_RXStatus_SKPAdded = 1,
		DTC_RXStatus_SKPRemoved = 2,
		DTC_RXStatus_ReceiverDetected = 3,
		DTC_RXStatus_DecodeError = 4,
		DTC_RXStatus_ElasticOverflow = 5,
		DTC_RXStatus_ElasticUnderflow = 6,
		DTC_RXStatus_RXDisparityError = 7,
	};

	/// <summary>
	/// The DTC_RXStatusConverter converts a DTC_RXStatus enumeration value to string or JSON representation
	/// </summary>
	struct DTC_RXStatusConverter
	{
		DTC_RXStatus status_;

		explicit DTC_RXStatusConverter(DTC_RXStatus status);

		std::string toString() const
		{
			switch (status_)
			{
			case DTC_RXStatus_DataOK:
				return "DataOK";
			case DTC_RXStatus_SKPAdded:
				return "SKPAdded";
			case DTC_RXStatus_SKPRemoved:
				return "SKPRemoved";
			case DTC_RXStatus_ReceiverDetected:
				return "ReceiverDetected";
			case DTC_RXStatus_DecodeError:
				return "DecodeErr";
			case DTC_RXStatus_ElasticOverflow:
				return "ElasticOF";
			case DTC_RXStatus_ElasticUnderflow:
				return "ElasticUF";
			case DTC_RXStatus_RXDisparityError:
				return "RXDisparity";
			}
			return "Unknown";
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_RXStatusConverter& status)
		{
			stream << "\"DTC_RXStatus\":\"" << status.toString() << "\"";
			return stream;
		}
	};

	enum DTC_SERDESLoopbackMode
	{
		DTC_SERDESLoopbackMode_Disabled = 0,
		DTC_SERDESLoopbackMode_NearPCS = 1,
		DTC_SERDESLoopbackMode_NearPMA = 2,
		DTC_SERDESLoopbackMode_FarPMA = 4,
		DTC_SERDESLoopbackMode_FarPCS = 6,
	};

	/// <summary>
	/// The DTC_SERDESLoopbackModeConverter converts a DTC_SERDESLoopbackMode enumeration value to string or JSON representation
	/// </summary>
	struct DTC_SERDESLoopbackModeConverter
	{
		DTC_SERDESLoopbackMode mode_;

		explicit DTC_SERDESLoopbackModeConverter(DTC_SERDESLoopbackMode mode) : mode_(mode) { }

		std::string toString() const
		{
			switch (mode_)
			{
			case DTC_SERDESLoopbackMode_Disabled:
				return "Disabled";
			case DTC_SERDESLoopbackMode_NearPCS:
				return "NearPCS";
			case DTC_SERDESLoopbackMode_NearPMA:
				return "NearPMA";
			case DTC_SERDESLoopbackMode_FarPMA:
				return "FarPMA";
			case DTC_SERDESLoopbackMode_FarPCS:
				return "FarPCS";
			}
			return "Unknown";
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_SERDESLoopbackModeConverter& mode)
		{
			stream << "\"DTC_SERDESLoopbackMode\":\"" << mode.toString() << "\"";
			return stream;
		}
	};

	enum DTC_SimMode
	{
		DTC_SimMode_Disabled = 0,
		DTC_SimMode_Tracker = 1,
		DTC_SimMode_Calorimeter = 2,
		DTC_SimMode_CosmicVeto = 3,
		DTC_SimMode_NoCFO = 4,
		DTC_SimMode_ROCEmulator = 5,
		DTC_SimMode_Loopback = 6,
		DTC_SimMode_Performance = 7,
		DTC_SimMode_LargeFile = 8,
		DTC_SimMode_Invalid,
	};

	/// <summary>
	/// The DTC_SimModeConverter converts a DTC_SimMode enumeration value to string or JSON representation
	/// </summary>
	struct DTC_SimModeConverter
	{
		DTC_SimMode mode_;

		explicit DTC_SimModeConverter(DTC_SimMode mode) : mode_(mode) { }

		static DTC_SimMode ConvertToSimMode(std::string);

		std::string toString() const
		{
			switch (mode_)
			{
			case DTC_SimMode_Tracker:
				return "Tracker";
			case DTC_SimMode_Calorimeter:
				return "Calorimeter";
			case DTC_SimMode_CosmicVeto:
				return "CosmicVeto";
			case DTC_SimMode_NoCFO:
				return "NoCFO";
			case DTC_SimMode_ROCEmulator:
				return "ROCEmulator";
			case DTC_SimMode_Loopback:
				return "Loopback";
			case DTC_SimMode_Performance:
				return "Performance";
			case DTC_SimMode_LargeFile:
				return "LargeFile";
			case DTC_SimMode_Disabled:
			default:
				return "Disabled";
			}
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_SimModeConverter& mode)
		{
			stream << "\"DTC_SimMode\":\"" << mode.toString() << "\"";
			return stream;
		}
	};

	/// <summary>
	/// A DTC_WrongPacketTypeException is thrown when an attempt to decode a DMA packet is made and the type in the DMA header is different than the type of the packet expected
	/// </summary>
	class DTC_WrongPacketTypeException : public std::exception
	{
	public:
		DTC_WrongPacketTypeException(int expected, int encountered) : expected_(expected), encountered_(encountered) {}
		const char* what() const throw()
		{
			return ("Unexpected packet type encountered: " + std::to_string(encountered_) + " != " + std::to_string(expected_) + " (expected)").c_str();
		}
		int expected_;
		int encountered_;
	};

	/// <summary>
	/// A DTC_IOErrorException is thrown when the DTC is not communicating when communication is expected
	/// </summary>
	class DTC_IOErrorException : public std::exception
	{
	public:
		const char* what() const throw()
		{
			return "Unable to communicate with the DTC";
		}
	};

	/// <summary>
	/// A DTC_DataCorruptionException is thrown when corrupt data is detected coming from the DTC
	/// </summary>
	class DTC_DataCorruptionException : public std::exception
	{
	public:
		const char* what() const throw()
		{
			return "Corruption detected in data stream from DTC";
		}
	};

	/// <summary>
	/// The mu2e Timestamp is a 48-bit quantity. This class manages all the different ways it could be accessed.
	/// </summary>
	class DTC_Timestamp
	{
		uint64_t timestamp_ : 48;
	public:
		DTC_Timestamp();
		explicit DTC_Timestamp(const uint64_t timestamp);
		DTC_Timestamp(const uint32_t timestampLow, const uint16_t timestampHigh);
		explicit DTC_Timestamp(const uint8_t* timeArr, int offset = 0);
		explicit DTC_Timestamp(const std::bitset<48> timestamp);
		DTC_Timestamp(const DTC_Timestamp&) = default;
		DTC_Timestamp(DTC_Timestamp&&) = default;

		virtual ~DTC_Timestamp() = default;
		DTC_Timestamp& operator=(DTC_Timestamp&&) = default;
		DTC_Timestamp& operator=(const DTC_Timestamp&) = default;

		bool operator==(const DTC_Timestamp r) const
		{
			return r.GetTimestamp(true) == timestamp_;
		}

		bool operator!=(const DTC_Timestamp r) const
		{
			return r.GetTimestamp(true) != timestamp_;
		}

		bool operator<(const DTC_Timestamp r)
		{
			return r.GetTimestamp(true) > timestamp_;
		}

		bool operator<(const DTC_Timestamp r) const
		{
			return r.GetTimestamp(true) > timestamp_;
		}

		DTC_Timestamp operator+(const int r) const
		{
			return DTC_Timestamp(r + timestamp_);
		}

		void SetTimestamp(uint64_t timestamp)
		{
			timestamp_ = timestamp & 0x0000FFFFFFFFFFFF;
		}

		void SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh);

		std::bitset<48> GetTimestamp() const
		{
			return timestamp_;
		}

		uint64_t GetTimestamp(bool dummy) const
		{
			if (dummy)
			{
				return timestamp_;
			}
			return 0;
		}

		void GetTimestamp(const uint8_t* timeArr, int offset = 0) const;
		std::string toJSON(bool arrayMode = false) const;
		std::string toPacketFormat() const;
	};

	/// <summary>
	/// This class is used to decode the SERDES RX Disparity Error register
	/// </summary>
	class DTC_SERDESRXDisparityError
	{
		std::bitset<2> data_;

	public:
		/// <summary>
		/// Default Constructor
		/// </summary>
		DTC_SERDESRXDisparityError();
		/// <summary>
		/// Construct a DTC_SERDESRXDisparityError using the given bitset
		/// </summary>
		/// <param name="data">Bits to use for initialization</param>
		explicit DTC_SERDESRXDisparityError(std::bitset<2> data);
		/// <summary>
		/// Construct a DTC_SERDESRXDisparityError using the register value and a Ring ID
		/// </summary>
		/// <param name="data">Register value to read error bits from</param>
		/// <param name="ring">Ring to read</param>
		DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring);
		/// <summary>
		/// Default Copy Constructor
		/// </summary>
		/// <param name="in">DTC_SERDESRXDisparityError to copy</param>
		DTC_SERDESRXDisparityError(const DTC_SERDESRXDisparityError& in) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="in">DTC_SERDESRXDisparityError rvalue</param>
		DTC_SERDESRXDisparityError(DTC_SERDESRXDisparityError&& in) = default;

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="in">DTC_SERDESRXDisparityError to copy</param>
		/// <returns>DTC_SERDESRXDisparityError Reference</returns>
		DTC_SERDESRXDisparityError& operator=(const DTC_SERDESRXDisparityError& in) = default;
		/// <summary>
		/// Default Move Assignment Operator
		/// </summary>
		/// <param name="in">DTC_SERDESRXDisparityError rvalue</param>
		/// <returns>DTC_SERDESRXDisparityError reference</returns>
		DTC_SERDESRXDisparityError& operator=(DTC_SERDESRXDisparityError&& in) = default;

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
		/// Write the DTC_SERDESRXDisparityError to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="error">DTC_SERDESRXDisparityError to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, DTC_SERDESRXDisparityError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	/// <summary>
	/// This structure is used to decode the SERDES Character Not In Table Error register
	/// </summary>
	class DTC_CharacterNotInTableError
	{
		std::bitset<2> data_;

	public:
		/// <summary>
		/// Default Constructor
		/// Initializes data bits to 0,0
		/// </summary>
		DTC_CharacterNotInTableError();
		/// <summary>
		/// Constructor using data bits
		/// </summary>
		/// <param name="data">data bits for this instance</param>
		explicit DTC_CharacterNotInTableError(std::bitset<2> data);
		/// <summary>
		/// DTC_CharacterNotInTableError Constructor
		/// </summary>
		/// <param name="data">Register value</param>
		/// <param name="ring">Specific ring to read</param>
		DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring);
		/// <summary>
		/// Default Copy Constructor
		/// </summary>
		/// <param name="in">DTC_CharacterNotInTableError to copy</param>
		DTC_CharacterNotInTableError(const DTC_CharacterNotInTableError& in) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="in">DTC_CharacterNotInTableError rvalue</param>
		DTC_CharacterNotInTableError(DTC_CharacterNotInTableError&& in) = default;

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="in">DTC_CharacterNotInTableError to copy</param>
		/// <returns>DTC_CharacterNotInTableError reference</returns>
		DTC_CharacterNotInTableError& operator=(const DTC_CharacterNotInTableError& in) = default;
		/// <summary>
		/// Default Move Assignment Operator
		/// </summary>
		/// <param name="in">DTC_CharacterNotInTableError rvalue</param>
		/// <returns>DTC_CharacterNotInTableError reference</returns>
		DTC_CharacterNotInTableError& operator=(DTC_CharacterNotInTableError&& in) = default;

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
		/// Write the DTC_CharacterNotInTableError to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="error">DTC_CharacterNotInTableError to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, DTC_CharacterNotInTableError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	/// <summary>
	/// This structure is used to decode the RingEnable register value
	/// </summary>
	struct DTC_RingEnableMode
	{
		bool TransmitEnable; ///< Whether transmit is enabled on this link
		bool ReceiveEnable; ///< Whether receive is enabled on this link
		bool TimingEnable; ///< Whether timing is enabled on this link

		/// <summary>
		/// Default constructor. Sets all enable bits to true.
		/// </summary>
		DTC_RingEnableMode() : TransmitEnable(true), ReceiveEnable(true), TimingEnable(true) { }

		/// <summary>
		/// Construct a DTC_RingEnableMode instance with the given flags
		/// </summary>
		/// <param name="transmit">Enable TX</param>
		/// <param name="receive">Enable RX</param>
		/// <param name="timing">Enable CFO</param>
		DTC_RingEnableMode(bool transmit, bool receive, bool timing) : TransmitEnable(transmit), ReceiveEnable(receive), TimingEnable(timing) { }

		/// <summary>
		/// Write the DTC_RingEnableMode to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="mode">DTC_RingEnableMode to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const DTC_RingEnableMode& mode)
		{
			auto formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
			stream.setf(std::ios_base::boolalpha);
			stream << "{\"TransmitEnable\":" << mode.TransmitEnable << ",\"ReceiveEnable\":" << mode.ReceiveEnable << ",\"TimingEnable\":" << mode.TimingEnable << "}";
			if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
			return stream;
		}

		/// <summary>
		/// Determine if two DTC_RingEnableMode objects are equal
		/// They are equal if TX, RX and Timing bits are all equal
		/// </summary>
		/// <param name="left">LHS of compare</param>
		/// <param name="right">RHS of compare</param>
		/// <returns>Whether all three bits of both sides are equal</returns>
		friend bool operator==(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right)
		{
			return left.TransmitEnable == right.TransmitEnable && left.ReceiveEnable == right.ReceiveEnable && left.TimingEnable == right.TimingEnable;
		}

		/// <summary>
		/// Determine if two DTC_RingEnableMode objects are not equal
		/// </summary>
		/// <param name="left">LHS of compare</param>
		/// <param name="right">RHS of compare</param>
		/// <returns>!(left == right)</returns>
		friend bool operator!=(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right)
		{
			return !(left == right);
		}
	};

	/// <summary>
	/// This structure is used to decode the FIFOFullErrorFlags register values
	/// </summary>
	struct DTC_FIFOFullErrorFlags
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
		DTC_FIFOFullErrorFlags()
			: OutputData(false)
			, CFOLinkInput(false)
			, ReadoutRequestOutput(false)
			, DataRequestOutput(false)
			, OtherOutput(false)
			, OutputDCS(false)
			, OutputDCSStage2(false)
			, DataInput(false)
			, DCSStatusInput(false) { }

		/// <summary>
		/// Construct a DTC_FIFOFUllErrorFlags instance with the given values
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
		DTC_FIFOFullErrorFlags(bool outputData, bool cfoLinkInput, bool readoutRequest, bool dataRequest,
							   bool otherOutput, bool outputDCS, bool outputDCS2, bool dataInput, bool dcsInput)
			: OutputData(outputData)
			, CFOLinkInput(cfoLinkInput)
			, ReadoutRequestOutput(readoutRequest)
			, DataRequestOutput(dataRequest)
			, OtherOutput(otherOutput)
			, OutputDCS(outputDCS)
			, OutputDCSStage2(outputDCS2)
			, DataInput(dataInput)
			, DCSStatusInput(dcsInput) { }

		/// <summary>
		/// Write the DTC_FIFOFullErrorFlags to stream in JSON format.
		/// Note that the JSON represents an object, calling code should add a key if there's other objects to stream
		/// </summary>
		/// <param name="stream">Stream to write to</param>
		/// <param name="flags">Flags to parse</param>
		/// <returns>Stream object for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const DTC_FIFOFullErrorFlags& flags)
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
	/// The DTC_RegisterFormatter class is used to print a DTC register in a human-readable format
	/// </summary>
	struct DTC_RegisterFormatter
	{
		/// <summary>
		/// Default Constructor with zero values
		/// </summary>
		DTC_RegisterFormatter() : address(0), value(0), descWidth(28), description(""), vals() {}

		/// <summary>
		/// Default Copy Consturctor
		/// </summary>
		/// <param name="r">DTC_RegisterFormatter to copy</param>
		DTC_RegisterFormatter(const DTC_RegisterFormatter& r) = default;
		/// <summary>
		/// Default Move Constructor
		/// </summary>
		/// <param name="r">DTC_RegisterFormatter rvalue</param>
		DTC_RegisterFormatter(DTC_RegisterFormatter&& r) = default;
		uint16_t address; ///< Address of the register
		uint32_t value; ///< Value of the register
		int descWidth; ///< Display width of description field
		std::string description; ///< Description of the register (name)
		std::vector<std::string> vals; ///< Human-readable descriptions of register values (bits, etc)

		/// <summary>
		/// Default Copy Assignment Operator
		/// </summary>
		/// <param name="r">RHS</param>
		/// <returns>DTC_RegisterFormatter regerence</returns>
		DTC_RegisterFormatter& operator=(const DTC_RegisterFormatter& r) = default;

		/// <summary>
		/// Write out the DTC_RegisterFormatter to the given stream. This function uses setw to make sure that fields for
		/// different registers still line up.
		/// Format is: "    0xADDR  | 0xVALUEXXX | DESCRIPTION: Variable size, minimum of 28 chars | Value 1
		///                                                                                        | Value 2 ..."
		/// </summary>
		/// <param name="stream">Stream to write register data to</param>
		/// <param name="reg">RegisterFormatter to write</param>
		/// <returns>Stream reference for continued streaming</returns>
		friend std::ostream& operator<<(std::ostream& stream, const DTC_RegisterFormatter& reg)
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
	/// A Data Block object (DataHeader packet plus associated Data Packets)
	/// Constructed as a pointer to a region of memory
	/// </summary>
	struct DTC_DataBlock
	{
		typedef uint64_t pointer_t; ///< DataBlock pointers should be 64-bit aligned
		pointer_t* blockPointer; ///< Pointer to DataBlock in Memeory
		size_t byteSize; ///< Size of DataBlock

		/// <summary>
		/// Create a DTC_DataBlock pointing to the given location in memory with the given size
		/// </summary>
		/// <param name="ptr">Pointer to DataBlock in memory</param>
		/// <param name="sz">Size of DataBlock</param>
		DTC_DataBlock(pointer_t* ptr, size_t sz) : blockPointer(ptr), byteSize(sz) {}
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
		/// <returns>String with converted value</returns>
		static std::string FormatByteString(double bytes);
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

#endif //DTC_TYPES_H
