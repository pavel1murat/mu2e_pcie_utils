#ifndef DTC_PACKETS_H
#define DTC_PACKETS_H

#include <bitset> // std::bitset


#include <cstdint> // uint8_t, uint16_t


#include <vector> // std::vector

#include <iostream>

#include "DTC_Types.h"

#ifndef _WIN32
#include "mu2e_driver/mu2e_mmap_ioctl.h"
#else
#include "../mu2e_driver/mu2e_mmap_ioctl.h"
#endif


namespace DTCLib
{
	enum DTC_PacketType : uint8_t
	{
		DTC_PacketType_DCSRequest = 0,
		DTC_PacketType_Heartbeat = 1,
		DTC_PacketType_DataRequest = 2,
		DTC_PacketType_DCSReply = 4,
		DTC_PacketType_DataHeader = 5,
		DTC_PacketType_Invalid = 0x10,
	};

	enum DTC_DataStatus
	{
		DTC_DataStatus_Valid = 0,
		DTC_DataStatus_NoValid = 1,
		DTC_DataStatus_Invalid = 2,
	};

	enum DTC_DCSOperationType : uint8_t
	{
		DTC_DCSOperationType_Read = 0,
		DTC_DCSOperationType_Write = 1,
		DTC_DCSOperationType_WriteWithAck = 2,
		DTC_DCSOperationType_Unknown = 255
	};

	struct DTC_DCSOperationTypeConverter
	{
	public:
		DTC_DCSOperationType type_;

		DTC_DCSOperationTypeConverter(DTC_DCSOperationType type) : type_(type) { }

		std::string toString() const
		{
			switch (type_)
			{
			case DTC_DCSOperationType_Read:
				return "Read";
			case DTC_DCSOperationType_Write:
				return "Write";
			case DTC_DCSOperationType_WriteWithAck:
				return "WriteWithAck";
			case DTC_DCSOperationType_Unknown:
			default:
				return "Unknown";
			}
		}

		friend std::ostream& operator<<(std::ostream& stream, const DTC_DCSOperationTypeConverter& type)
		{
			switch (type.type_)
			{
			case DTC_DCSOperationType_Read:
				stream << "\"Read\"";
			case DTC_DCSOperationType_Write:
				stream << "\"Write\"";
			case DTC_DCSOperationType_WriteWithAck:
				stream << "\"WriteWithAck\"";
			case DTC_DCSOperationType_Unknown:
			default:
				stream << "\"Unknown\"";
			}
			return stream;
		}
	};

	class DTC_DataPacket
	{
	private:
		uint8_t* dataPtr_;
		uint16_t dataSize_;
		bool memPacket_;
		std::vector<uint8_t> vals_;

	public:
		DTC_DataPacket();

		DTC_DataPacket(mu2e_databuff_t* data) : dataPtr_(*data), dataSize_(16), memPacket_(true) { }

		DTC_DataPacket(void* data) : dataPtr_((uint8_t*)data), dataSize_(16), memPacket_(true) { }

		DTC_DataPacket(uint8_t* data) : dataPtr_(data), dataSize_(16), memPacket_(true) { }

		DTC_DataPacket(const DTC_DataPacket&);
		DTC_DataPacket(DTC_DataPacket&&) = default;

		virtual ~DTC_DataPacket();

		DTC_DataPacket& operator=(const DTC_DataPacket&) = default;
		DTC_DataPacket& operator=(DTC_DataPacket&&) = default;

		void SetWord(uint16_t index, uint8_t data);
		uint8_t GetWord(uint16_t index) const;
		std::string toJSON() const;
		std::string toPacketFormat();
		bool Resize(const uint16_t dmaSize);

		uint16_t GetSize() const
		{
			return dataSize_;
		}

		bool IsMemoryPacket() const
		{
			return memPacket_;
		}

		void CramIn(DTC_DataPacket& other, int offset)
		{
			if (other.dataSize_ + offset <= dataSize_) memcpy(dataPtr_ + offset, other.dataPtr_, other.dataSize_);
		}

		uint8_t* GetData() const
		{
			return dataPtr_;
		}

		bool operator==(const DTC_DataPacket& other)
		{
			return Equals(other);
		}

		bool operator!=(const DTC_DataPacket& other)
		{
			return !Equals(other);
		}

		bool Equals(const DTC_DataPacket& other);

		friend std::ostream& operator<<(std::ostream& s, DTC_DataPacket& p)
		{
			return s.write(reinterpret_cast<char*>(p.dataPtr_), p.dataSize_);
		}
	};

	class DTC_DMAPacket
	{
	protected:
		uint16_t byteCount_;
		bool valid_;
		DTC_Ring_ID ringID_;
		DTC_PacketType packetType_;
		DTC_ROC_ID rocID_;

	public:
		DTC_DMAPacket() : byteCount_(0), valid_(false), ringID_(DTC_Ring_Unused), packetType_(DTC_PacketType_Invalid), rocID_(DTC_ROC_Unused) { }

		DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t byteCount = 64, bool valid = true);

		DTC_DMAPacket(const DTC_DataPacket in);
		DTC_DMAPacket(const DTC_DMAPacket&) = default;
		DTC_DMAPacket(DTC_DMAPacket&&) = default;

		virtual ~DTC_DMAPacket() = default;

		DTC_DMAPacket& operator=(const DTC_DMAPacket&) = default;
		DTC_DMAPacket& operator=(DTC_DMAPacket&&) = default;

		virtual DTC_DataPacket ConvertToDataPacket() const;

		DTC_PacketType GetPacketType() const
		{
			return packetType_;
		}

		std::string headerJSON();
		std::string headerPacketFormat();

		uint16_t GetByteCount() const
		{
			return byteCount_;
		}

		DTC_Ring_ID GetRingID() const
		{
			return ringID_;
		}

		virtual std::string toPacketFormat();
		virtual std::string toJSON();

		friend std::ostream& operator<<(std::ostream& stream, DTC_DMAPacket& packet)
		{
			stream << packet.toJSON();
			return stream;
		}
	};

	class DTC_DCSRequestPacket : public DTC_DMAPacket
	{
	private:
		DTC_DCSOperationType type_;
		uint8_t address_ : 5;
		uint16_t data_;
	public:
		DTC_DCSRequestPacket();
		DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc);
		DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_DCSOperationType type, uint8_t address, uint16_t data);
		DTC_DCSRequestPacket(const DTC_DCSRequestPacket&) = default;
		DTC_DCSRequestPacket(DTC_DCSRequestPacket&&) = default;
		DTC_DCSRequestPacket(DTC_DataPacket in);

		DTC_DCSRequestPacket& operator=(const DTC_DCSRequestPacket&) = default;
		DTC_DCSRequestPacket& operator=(DTC_DCSRequestPacket&&) = default;

		virtual ~DTC_DCSRequestPacket() = default;

		uint16_t GetData()
		{
			return data_;
		}

		void SetData(uint16_t data)
		{
			data_ = data;
		}

		uint8_t GetAddress()
		{
			return address_;
		}

		void SetAddress(uint8_t address)
		{
			address_ = address & 0x1F;
		}

		DTC_DCSOperationType GetType()
		{
			return type_;
		}

		void SetType(DTC_DCSOperationType type)
		{
			type_ = type;
		}

		DTC_DataPacket ConvertToDataPacket() const override;
		std::string toJSON() override;
		std::string toPacketFormat() override;
	};

	class DTC_HeartbeatPacket : public DTC_DMAPacket
	{
	private:
		DTC_Timestamp timestamp_;
		uint8_t eventMode_[6];
	public:
		DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC = DTC_ROC_5);
		DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC = DTC_ROC_5, uint8_t* eventMode = nullptr);
		DTC_HeartbeatPacket(const DTC_HeartbeatPacket& right) = default;
		DTC_HeartbeatPacket(DTC_HeartbeatPacket&& right) = default;
		DTC_HeartbeatPacket(DTC_DataPacket in);

		virtual ~DTC_HeartbeatPacket() = default;
		
		DTC_Timestamp GetTimestamp()
		{
			return timestamp_;
		}

		virtual uint8_t* GetData()
		{
			return eventMode_;
		}

		DTC_DataPacket ConvertToDataPacket() const;
		std::string toJSON();
		std::string toPacketFormat();
	};

	class DTC_DataRequestPacket : public DTC_DMAPacket
	{
	private:
		DTC_Timestamp timestamp_;
		bool debug_;
		uint16_t debugPacketCount_;
		DTC_DebugType type_;

	public:
		DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
		DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp, bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
		DTC_DataRequestPacket(const DTC_DataRequestPacket&) = default;
		DTC_DataRequestPacket(DTC_DataRequestPacket&&) = default;
		DTC_DataRequestPacket(DTC_DataPacket in);

		bool GetDebug()
		{
			return debug_;
		}

		DTC_DebugType GetDebugType()
		{
			return type_;
		}

		uint16_t GetDebugPacketCount()
		{
			return debugPacketCount_;
		}

		void SetDebugPacketCount(uint16_t count);

		DTC_Timestamp GetTimestamp()
		{
			return timestamp_;
		}

		DTC_DataPacket ConvertToDataPacket() const;
		std::string toJSON();
		std::string toPacketFormat();
	};

	class DTC_DCSReplyPacket : public DTC_DMAPacket
	{
	private:
		uint8_t requestCounter_;
		DTC_DCSOperationType type_;
		bool dcsReceiveFIFOEmpty_;
		uint8_t address_ : 5;
		uint16_t data_;
	public:
		DTC_DCSReplyPacket(DTC_Ring_ID ring);
		DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t counter, DTC_DCSOperationType type, uint8_t address, uint16_t data, bool fifoEmpty);
		DTC_DCSReplyPacket(const DTC_DCSReplyPacket&) = default;
		DTC_DCSReplyPacket(DTC_DCSReplyPacket&&) = default;
		DTC_DCSReplyPacket(DTC_DataPacket in);

		uint8_t GetRequestCounter()
		{
			return requestCounter_;
		}

		void SetRequestCounter(uint8_t counter)
		{
			requestCounter_ = counter;
		}

		uint16_t GetData()
		{
			return data_;
		}

		void SetData(uint16_t data)
		{
			data_ = data;
		}

		uint8_t GetAddress()
		{
			return address_;
		}

		void SetAddress(uint8_t address)
		{
			address_ = address & 0x1F;
		}

		DTC_DCSOperationType GetType()
		{
			return type_;
		}

		void SetType(DTC_DCSOperationType type)
		{
			type_ = type;
		}

		bool DCSReceiveFIFOEmpty()
		{
			return dcsReceiveFIFOEmpty_;
		}

		DTC_DataPacket ConvertToDataPacket() const;
		std::string toJSON();
		std::string toPacketFormat();
	};

	class DTC_DataHeaderPacket : public DTC_DMAPacket
	{
	private:
		uint16_t packetCount_;
		DTC_Timestamp timestamp_;
		DTC_DataStatus status_[3];
		uint8_t evbMode_;

	public:
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status);
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp);
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp, uint8_t evbMode_);
		DTC_DataHeaderPacket(const DTC_DataHeaderPacket&) = default;
		DTC_DataHeaderPacket(DTC_DataHeaderPacket&&) = default;
		DTC_DataHeaderPacket(DTC_DataPacket in);

		DTC_DataPacket ConvertToDataPacket() const;

		virtual uint8_t GetData()
		{
			return evbMode_;
		}

		uint16_t GetPacketCount()
		{
			return packetCount_;
		}

		DTC_Timestamp GetTimestamp()
		{
			return timestamp_;
		}

		DTC_DataStatus GetStatus()
		{
			return status_[0];
		}

		std::string toJSON();
		std::string toPacketFormat();

		bool operator==(const DTC_DataHeaderPacket& other)
		{
			return Equals(other);
		}

		bool operator!=(const DTC_DataHeaderPacket& other)
		{
			return !Equals(other);
		}

		bool Equals(const DTC_DataHeaderPacket& other);
	};
}

#endif //DTC_PACKETS_H


