#ifndef DTC_PACKETS_H
#define DTC_PACKETS_H

#include <bitset>
#include <cstdint> // uint8_t, uint16_t
#include <vector>

#include "DTC_Types.h"

#ifndef _WIN32
#include "mu2e_driver/mu2e_mmap_ioctl.h"
#else
#include "../mu2e_driver/mu2e_mmap_ioctl.h"
#endif


namespace DTCLib
{

  /// <summary>
  /// Defined Packet Types for the DTC DMA Protocol
  /// </summary>
  enum DTC_PacketType : uint8_t
	{
	  DTC_PacketType_DCSRequest = 0,
		DTC_PacketType_Heartbeat = 1,
		DTC_PacketType_DataRequest = 2,
		DTC_PacketType_DCSReply = 4,
		DTC_PacketType_DataHeader = 5,
		DTC_PacketType_Invalid = 0x10,
		};

  /// <summary>
  /// Possible values for the Status word of the Data Header packet
  /// </summary>
  enum DTC_DataStatus
	{
	  DTC_DataStatus_Valid = 0,
	  DTC_DataStatus_NoValid = 1,
	  DTC_DataStatus_Invalid = 2,
	};

  /// <summary>
  /// Possible values for the Op word of the DCS Reqeust packet.
  /// </summary>
  enum DTC_DCSOperationType : uint8_t
	{
	  DTC_DCSOperationType_Read = 0,
		DTC_DCSOperationType_Write = 1,
		DTC_DCSOperationType_WriteWithAck = 2,
		DTC_DCSOperationType_Unknown = 255
		};

  struct DTC_DCSOperationTypeConverter
  {
	DTC_DCSOperationType type_;

	explicit DTC_DCSOperationTypeConverter(DTC_DCSOperationType type) : type_(type) { }

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
  public:
	DTC_DataPacket();

	explicit DTC_DataPacket(const mu2e_databuff_t* data) : dataPtr_(*data), dataSize_(16), memPacket_(true) { }

	explicit DTC_DataPacket(const void* data) : dataPtr_(static_cast<const uint8_t*>(data)), dataSize_(16), memPacket_(true) { }

	explicit DTC_DataPacket(const uint8_t* data) : dataPtr_(data), dataSize_(16), memPacket_(true) { }

	DTC_DataPacket(const DTC_DataPacket&);
	DTC_DataPacket(DTC_DataPacket&&) = default;

	virtual ~DTC_DataPacket();

	DTC_DataPacket& operator=(const DTC_DataPacket&) = default;
	DTC_DataPacket& operator=(DTC_DataPacket&&) = default;

	void SetWord(uint16_t index, uint8_t data);
	uint8_t GetWord(uint16_t index) const;
	std::string toJSON() const;
	std::string toPacketFormat() const;
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
	  if (other.dataSize_ + offset <= dataSize_) memcpy(const_cast<uint8_t*>(dataPtr_) + offset, other.dataPtr_, other.dataSize_);
	}

	const uint8_t* GetData() const
	{
	  return dataPtr_;
	}

	bool operator==(const DTC_DataPacket& other) const
	{
	  return Equals(other);
	}

	bool operator!=(const DTC_DataPacket& other) const
	{
	  return !Equals(other);
	}

	bool Equals(const DTC_DataPacket& other) const;

	friend std::ostream& operator<<(std::ostream& s, DTC_DataPacket& p)
	{
	  return s.write(reinterpret_cast<const char*>(p.dataPtr_), p.dataSize_);
	}

  private:
	const uint8_t* dataPtr_;
	uint16_t dataSize_;
	bool memPacket_;
	std::vector<uint8_t> vals_;
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


	explicit DTC_DMAPacket(const DTC_DataPacket in);
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

	std::string headerJSON() const;
	std::string headerPacketFormat() const;

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
  public:
	DTC_DCSRequestPacket();
	DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc);
	DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_DCSOperationType type, uint8_t address, uint16_t data);
	DTC_DCSRequestPacket(const DTC_DCSRequestPacket&) = default;
	DTC_DCSRequestPacket(DTC_DCSRequestPacket&&) = default;
	explicit DTC_DCSRequestPacket(const DTC_DataPacket in);

	DTC_DCSRequestPacket& operator=(const DTC_DCSRequestPacket&) = default;
	DTC_DCSRequestPacket& operator=(DTC_DCSRequestPacket&&) = default;

	virtual ~DTC_DCSRequestPacket() = default;

	uint16_t GetData() const
	{
	  return data_;
	}

	void SetData(uint16_t data)
	{
	  data_ = data;
	}

	uint8_t GetAddress() const
	{
	  return address_;
	}

	void SetAddress(uint8_t address)
	{
	  address_ = address & 0x1F;
	}

	DTC_DCSOperationType GetType() const
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
  private:
	DTC_DCSOperationType type_;
	uint8_t address_ : 5;
	uint16_t data_;
  };

  class DTC_HeartbeatPacket : public DTC_DMAPacket
  {
  public:
	explicit DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC = DTC_ROC_5);
	DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC = DTC_ROC_5, uint8_t* eventMode = nullptr);
	DTC_HeartbeatPacket(const DTC_HeartbeatPacket& right) = default;
	DTC_HeartbeatPacket(DTC_HeartbeatPacket&& right) = default;
	explicit DTC_HeartbeatPacket(const DTC_DataPacket in);

	virtual ~DTC_HeartbeatPacket() = default;

	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	virtual uint8_t* GetData()
	{
	  return eventMode_;
	}

	DTC_DataPacket ConvertToDataPacket() const override;
	std::string toJSON() override;
	std::string toPacketFormat() override;
  private:
	DTC_Timestamp timestamp_;
	uint8_t eventMode_[6];
  };

  class DTC_DataRequestPacket : public DTC_DMAPacket
  {
  public:
	DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
	DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp, bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
	DTC_DataRequestPacket(const DTC_DataRequestPacket&) = default;
	DTC_DataRequestPacket(DTC_DataRequestPacket&&) = default;
	explicit DTC_DataRequestPacket(const DTC_DataPacket in);

	bool GetDebug() const
	{
	  return debug_;
	}

	DTC_DebugType GetDebugType() const
	{
	  return type_;
	}

	uint16_t GetDebugPacketCount() const
	{
	  return debugPacketCount_;
	}

	void SetDebugPacketCount(uint16_t count);

	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	DTC_DataPacket ConvertToDataPacket() const override;
	std::string toJSON() override;
	std::string toPacketFormat() override;
  private:
	DTC_Timestamp timestamp_;
	bool debug_;
	uint16_t debugPacketCount_;
	DTC_DebugType type_;
  };

  class DTC_DCSReplyPacket : public DTC_DMAPacket
  {
  public:
	explicit DTC_DCSReplyPacket(DTC_Ring_ID ring);
	DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t counter, DTC_DCSOperationType type, uint8_t address, uint16_t data, bool fifoEmpty);
	DTC_DCSReplyPacket(const DTC_DCSReplyPacket&) = default;
	DTC_DCSReplyPacket(DTC_DCSReplyPacket&&) = default;
	explicit DTC_DCSReplyPacket(const DTC_DataPacket in);

	uint8_t GetRequestCounter() const
	{
	  return requestCounter_;
	}

	void SetRequestCounter(uint8_t counter)
	{
	  requestCounter_ = counter;
	}

	uint16_t GetData() const
	{
	  return data_;
	}

	void SetData(uint16_t data)
	{
	  data_ = data;
	}

	uint8_t GetAddress() const
	{
	  return address_;
	}

	void SetAddress(uint8_t address)
	{
	  address_ = address & 0x1F;
	}

	DTC_DCSOperationType GetType() const
	{
	  return type_;
	}

	void SetType(DTC_DCSOperationType type)
	{
	  type_ = type;
	}

	bool DCSReceiveFIFOEmpty() const
	{
	  return dcsReceiveFIFOEmpty_;
	}

	DTC_DataPacket ConvertToDataPacket() const override;
	std::string toJSON() override;
	std::string toPacketFormat() override;
  private:
	uint8_t requestCounter_;
	DTC_DCSOperationType type_;
	bool dcsReceiveFIFOEmpty_;
	uint8_t address_ : 5;
	uint16_t data_;
  };

  class DTC_DataHeaderPacket : public DTC_DMAPacket
  {
  public:
	DTC_DataHeaderPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t packetCount, DTC_DataStatus status, uint8_t dtcid, uint8_t packetVersion, DTC_Timestamp timestamp = DTC_Timestamp(), uint8_t evbMode = 0);
	DTC_DataHeaderPacket(const DTC_DataHeaderPacket&) = default;
	DTC_DataHeaderPacket(DTC_DataHeaderPacket&&) = default;
	explicit DTC_DataHeaderPacket(const DTC_DataPacket in);

	DTC_DataPacket ConvertToDataPacket() const override;

	uint8_t GetEVBMode() const
	{
	  return evbMode_;
	}

	DTC_Subsystem GetSubsystem() const
	{
		return dtcId_.GetSubsystem();
	}

	uint8_t GetID() const
	{
		return dtcId_.GetID();
	}

	uint16_t GetPacketCount() const
	{
	  return packetCount_;
	}

	uint8_t GetVersion() const
	{
		return dataPacketVersion_;
	}

	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	DTC_DataStatus GetStatus() const
	{
	  return status_;
	}

	std::string toJSON() override;
	std::string toPacketFormat() override;

	bool operator==(const DTC_DataHeaderPacket& other) const
	{
	  return Equals(other);
	}

	bool operator!=(const DTC_DataHeaderPacket& other) const
	{
	  return !Equals(other);
	}

	bool Equals(const DTC_DataHeaderPacket& other) const;
  private:
	uint16_t packetCount_;
	DTC_Timestamp timestamp_;
	DTC_DataStatus status_;
	uint8_t dataPacketVersion_;
	DTC_ID dtcId_;
	uint8_t evbMode_;
  };
}

#endif //DTC_PACKETS_H
