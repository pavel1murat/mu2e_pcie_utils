#ifndef DTC_TYPES_H
#define DTC_TYPES_H

#include <bitset> // std::bitset


#include <cstdint> // uint8_t, uint16_t


#include <vector> // std::vector


#include <iomanip>

namespace DTCLib
{
	const std::string ExpectedDesignVersion = "v1.4_2015-07-01-00";

	enum DTC_Ring_ID : uint8_t
	{
		DTC_Ring_0 = 0,
		DTC_Ring_1 = 1,
		DTC_Ring_2 = 2,
		DTC_Ring_3 = 3,
		DTC_Ring_4 = 4,
		DTC_Ring_5 = 5,
		DTC_Ring_CFO = 6,
		DTC_Ring_Unused,
	};

	static const std::vector<DTC_Ring_ID> DTC_Rings = { DTC_Ring_0, DTC_Ring_1, DTC_Ring_2, DTC_Ring_3, DTC_Ring_4, DTC_Ring_5 };

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

	static const std::vector<DTC_ROC_ID> DTC_ROCS = { DTC_ROC_Unused, DTC_ROC_0, DTC_ROC_1, DTC_ROC_2, DTC_ROC_3, DTC_ROC_4, DTC_ROC_5 };

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
			switch (roc.roc_)
			{
			case DTC_ROC_0:
				stream << "ROC_0";
				break;
			case DTC_ROC_1:
				stream << "ROC_1";
				break;
			case DTC_ROC_2:
				stream << "ROC_2";
				break;
			case DTC_ROC_3:
				stream << "ROC_3";
				break;
			case DTC_ROC_4:
				stream << "ROC_4";
				break;
			case DTC_ROC_5:
				stream << "ROC_5";
				break;
			case DTC_ROC_Unused:
			default:
				stream << "No ROCs";
				break;
			}
			return stream;
		}
	};

	enum DTC_SerdesClockSpeed
	{
		DTC_SerdesClockSpeed_25Gbps,
		DTC_SerdesClockSpeed_3125Gbps,
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
			switch (type.type_)
			{
			case DTC_DebugType_SpecialSequence:
				stream << "\"Special Sequence\"";
				break;
			case DTC_DebugType_ExternalSerial:
				stream << "\"External Serial\"";
				break;
			case DTC_DebugType_ExternalSerialWithReset:
				stream << "\"External Serial with FIFO Reset\"";
				break;
			case DTC_DebugType_RAMTest:
				stream << "\"FPGA SRAM Error Checking\"";
				break;
			case DTC_DebugType_DDRTest:
				stream << "\"DDR3 Memory Error Checking\"";
				break;
			case DTC_DebugType_Invalid:
				stream << "\"\"";
				break;
			}
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
			switch (status.status_)
			{
			case DTC_RXBufferStatus_Nominal:
				stream << "{\"Nominal\":1,";
				stream << "\"Empty\":0,";
				stream << "\"Full\":0,";
				stream << "\"Underflow\":0,";
				stream << "\"Overflow\":0}";
				break;
			case DTC_RXBufferStatus_BufferEmpty:
				stream << "{\"Nominal\":0,";
				stream << "\"Empty\":1,";
				stream << "\"Full\":0,";
				stream << "\"Underflow\":0,";
				stream << "\"Overflow\":0}";
				break;
			case DTC_RXBufferStatus_BufferFull:
				stream << "{\"Nominal\":0,";
				stream << "\"Empty\":0,";
				stream << "\"Full\":1,";
				stream << "\"Underflow\":0,";
				stream << "\"Overflow\":0}";
				break;
			case DTC_RXBufferStatus_Overflow:
				stream << "{\"Nominal\":0,";
				stream << "\"Empty\":0,";
				stream << "\"Full\":0,";
				stream << "\"Underflow\":1,";
				stream << "\"Overflow\":0}";
				break;
			case DTC_RXBufferStatus_Underflow:
				stream << "{\"Nominal\":0,";
				stream << "\"Empty\":0,";
				stream << "\"Full\":0,";
				stream << "\"Underflow\":0,";
				stream << "\"Overflow\":1}";
				break;
			case DTC_RXBufferStatus_Unknown:
			default:
				stream << "{\"Nominal\":0,";
				stream << "\"Empty\":0,";
				stream << "\"Full\":0,";
				stream << "\"Underflow\":0,";
				stream << "\"Overflow\":0}";
				break;
			}
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
			switch (status.status_)
			{
			case DTC_RXStatus_DataOK:
				stream << "{\"DataOK\":1,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_SKPAdded:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":1,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_SKPRemoved:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":1,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_ReceiverDetected:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":1,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_DecodeError:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":1,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_ElasticOverflow:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":1,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_ElasticUnderflow:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":1,";
				stream << "\"DisparityError\":0}";
				break;
			case DTC_RXStatus_RXDisparityError:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":1}";
				break;
			default:
				stream << "{\"DataOK\":0,";
				stream << "\"SKPAdded\":0,";
				stream << "\"SKPRemoved\":0,";
				stream << "\"ReceiverDetected\":0,";
				stream << "\"DecodeError\":0,";
				stream << "\"EOverflow\":0,";
				stream << "\"EUnderflow\":0,";
				stream << "\"DisparityError\":0}";
				break;
			}
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
			switch (mode.mode_)
			{
			case DTC_SERDESLoopbackMode_NearPCS:
				stream << "{\"NEPCS\":1,";
				stream << "\"NEMPA\":0,";
				stream << "\"FEPMA\":0,";
				stream << "\"FEPCS\":0}";
				break;
			case DTC_SERDESLoopbackMode_NearPMA:
				stream << "{\"NEPCS\":0,";
				stream << "\"NEMPA\":1,";
				stream << "\"FEPMA\":0,";
				stream << "\"FEPCS\":0}";
				break;
			case DTC_SERDESLoopbackMode_FarPMA:
				stream << "{\"NEPCS\":0,";
				stream << "\"NEMPA\":0,";
				stream << "\"FEPMA\":1,";
				stream << "\"FEPCS\":0}";
				break;
			case DTC_SERDESLoopbackMode_FarPCS:
				stream << "{\"NEPCS\":0,";
				stream << "\"NEMPA\":0,";
				stream << "\"FEPMA\":0,";
				stream << "\"FEPCS\":1}";
				break;
			case DTC_SERDESLoopbackMode_Disabled:
			default:
				stream << "{\"NEPCS\":0,";
				stream << "\"NEMPA\":0,";
				stream << "\"FEPMA\":0,";
				stream << "\"FEPCS\":0}";
				break;
			}
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
		DTC_SimMode_Invalid,
	};

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
			case DTC_SimMode_Disabled:
			default:
				return "Disabled";
			}
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_SimModeConverter& mode)
		{
			switch (mode.mode_)
			{
			case DTC_SimMode_Disabled:
			default:
				stream << "{\"Disabled\":1,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_Tracker:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":1,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_Calorimeter:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":1,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_CosmicVeto:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":1,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_NoCFO:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":1,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_ROCEmulator:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":1,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_Loopback:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":1,";
				stream << "\"Performance\":0}";
				break;
			case DTC_SimMode_Performance:
				stream << "{\"Disabled\":0,";
				stream << "\"Tracker\":0,";
				stream << "\"Calorimeter\":0,";
				stream << "\"CosmicVeto\":0,";
				stream << "\"NoCFO\":0,";
				stream << "\"ROCEmulator\":0,";
				stream << "\"Loopback\":0,";
				stream << "\"Performance\":1}";
				break;
			}
			return stream;
		}
	};

	class DTC_WrongPacketTypeException : public std::exception
	{
	public:
		const char* what() const throw()
		{
			return "Unexpected packet type encountered!";
		}
	};

	class DTC_IOErrorException : public std::exception
	{
	public:
		const char* what() const throw()
		{
			return "Unable to communicate with the DTC";
		}
	};

	class DTC_DataCorruptionException : public std::exception
	{
	public:
		const char* what() const throw()
		{
			return "Corruption detected in data stream from DTC";
		}
	};

	class DTC_Timestamp
	{
		uint64_t timestamp_ : 48;
	public:
		DTC_Timestamp();
		explicit DTC_Timestamp(uint64_t timestamp);
		DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh);
		explicit DTC_Timestamp(uint8_t* timeArr, int offset = 0);
		explicit DTC_Timestamp(std::bitset<48> timestamp);
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

		void GetTimestamp(uint8_t* timeArr, int offset = 0) const;
		std::string toJSON(bool arrayMode = false) const;
		std::string toPacketFormat() const;
	};

	class DTC_SERDESRXDisparityError
	{
		std::bitset<2> data_;

	public:
		DTC_SERDESRXDisparityError();
		explicit DTC_SERDESRXDisparityError(std::bitset<2> data);
		DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring);
		DTC_SERDESRXDisparityError(const DTC_SERDESRXDisparityError&) = default;
		DTC_SERDESRXDisparityError(DTC_SERDESRXDisparityError&&) = default;

		DTC_SERDESRXDisparityError& operator=(const DTC_SERDESRXDisparityError&) = default;
		DTC_SERDESRXDisparityError& operator=(DTC_SERDESRXDisparityError&&) = default;

		void SetData(std::bitset<2> data)
		{
			data_ = data;
		}

		std::bitset<2> GetData() const
		{
			return data_;
		}

		int GetData(bool output) const
		{
			if (output) return static_cast<int>(data_.to_ulong());
			return 0;
		}

		friend std::ostream& operator<<(std::ostream& stream, DTC_SERDESRXDisparityError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	class DTC_CharacterNotInTableError
	{
		std::bitset<2> data_;

	public:
		DTC_CharacterNotInTableError();
		explicit DTC_CharacterNotInTableError(std::bitset<2> data);
		DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring);
		DTC_CharacterNotInTableError(const DTC_CharacterNotInTableError&) = default;
		DTC_CharacterNotInTableError(DTC_CharacterNotInTableError&&) = default;

		DTC_CharacterNotInTableError& operator=(const DTC_CharacterNotInTableError&) = default;
		DTC_CharacterNotInTableError& operator=(DTC_CharacterNotInTableError&&) = default;

		void SetData(std::bitset<2> data)
		{
			data_ = data;
		}

		std::bitset<2> GetData() const
		{
			return data_;
		}

		int GetData(bool output) const
		{
			if (output) return static_cast<int>(data_.to_ulong());
			return 0;
		}

		friend std::ostream& operator<<(std::ostream& stream, DTC_CharacterNotInTableError error)
		{
			stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
			return stream;
		}
	};

	struct DTC_RingEnableMode
	{
		bool TransmitEnable;
		bool ReceiveEnable;
		bool TimingEnable;

		DTC_RingEnableMode() : TransmitEnable(true), ReceiveEnable(true), TimingEnable(true) { }

		DTC_RingEnableMode(bool transmit, bool receive, bool timing) : TransmitEnable(transmit), ReceiveEnable(receive), TimingEnable(timing) { }

		friend std::ostream& operator<<(std::ostream& stream, const DTC_RingEnableMode& mode)
		{
			auto formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
			stream.setf(std::ios_base::boolalpha);
			stream << "{\"TransmitEnable\":" << mode.TransmitEnable << ",\"ReceiveEnable\":" << mode.ReceiveEnable << ",\"TimingEnable\":" << mode.TimingEnable << "}";
			if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
			return stream;
		}

		friend bool operator==(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right)
		{
			return left.TransmitEnable == right.TransmitEnable && left.ReceiveEnable == right.ReceiveEnable && left.TimingEnable == right.TimingEnable;
		}

		friend bool operator!=(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right)
		{
			return !(left == right);
		}
	};

	struct DTC_FIFOFullErrorFlags
	{
		bool OutputData;
		bool CFOLinkInput;
		bool ReadoutRequestOutput;
		bool DataRequestOutput;
		bool OtherOutput;
		bool OutputDCS;
		bool OutputDCSStage2;
		bool DataInput;
		bool DCSStatusInput;

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

	struct DTC_RegisterFormatter
	{
		DTC_RegisterFormatter() : address(0), value(0), descWidth(28), description(""), vals() {}

		DTC_RegisterFormatter(const DTC_RegisterFormatter& r) = default;
		DTC_RegisterFormatter(DTC_RegisterFormatter&& r) = default;
		uint16_t address;
		uint32_t value;
		int descWidth;
		std::string description;
		std::vector<std::string> vals;

		DTC_RegisterFormatter& operator=(const DTC_RegisterFormatter& r) = default;

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

	struct DTC_DataBlock
	{
		typedef uint64_t pointer_t;
		pointer_t* blockPointer;
		size_t byteSize;

		DTC_DataBlock(pointer_t* ptr, size_t sz) : blockPointer(ptr), byteSize(sz) {}
	};

	struct Utilities
	{
		static std::string FormatByteString(double bytes);
		static std::pair<double, std::string> FormatBytes(double bytes);
	};
}

#endif //DTC_TYPES_H


