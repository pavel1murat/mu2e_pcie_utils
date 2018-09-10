#ifndef DTC_PACKETS_H
#define DTC_PACKETS_H

#include <bitset>
#include <cstdint> // uint8_t, uint16_t
#include <vector>

#include "DTC_Types.h"

#include "mu2e_driver/mu2e_mmap_ioctl.h"


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

  /// <summary>
  /// Convert a DTC_DCSOperationType eunumeration value to its string or JSON representation
  /// </summary>
  struct DTC_DCSOperationTypeConverter
  {
	DTC_DCSOperationType type_; ///< DTC)DCSOperationType to convert

	/// <summary>
	/// DTC_DCSOperationTypeConverter Constructor
	/// </summary>
	/// <param name="type">DTC_DCSOperationType to convert</param>
	explicit DTC_DCSOperationTypeConverter(DTC_DCSOperationType type) : type_(type) { }

	/// <summary>
	/// Convert the type to its string representation
	/// </summary>
	/// <returns></returns>
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

	/// <summary>
	/// Write a DTC_DCSOperationTypeConverter in JSON format to the given stream
	/// </summary>
	/// <param name="stream">Stream to write</param>
	/// <param name="type">DTC_DCSOperationTypeConverter to serialize</param>
	/// <returns>Stream reference for continued streaming</returns>
	friend std::ostream& operator<<(std::ostream& stream, const DTC_DCSOperationTypeConverter& type)
	{
	  switch (type.type_)
		{
		case DTC_DCSOperationType_Read:
		  stream << "\"Read\"";break;
		case DTC_DCSOperationType_Write:
		  stream << "\"Write\"";break;
		case DTC_DCSOperationType_WriteWithAck:
		  stream << "\"WriteWithAck\"";break;
		case DTC_DCSOperationType_Unknown:
		default:
		  stream << "\"Unknown\"";break;
		}
	  return stream;
	}
  };

  /// <summary>
  /// The DTC_DataPacket class represents the 16 bytes of raw data for all DTC packets.
  /// The class works in two modes: "overlay" mode, where the data is in a fixed location in memory and modification is restricted, and
  /// "owner" mode, where the DataPacket is a concrete instance.
  /// </summary>
  class DTC_DataPacket
  {
  public:
	  /// <summary>
	  /// Construct a DTC_DataPacket in owner mode
	  /// </summary>
	DTC_DataPacket();

	/// <summary>
	/// Construct a DTC_DataPacket in "overlay" mode using the given DMA buffer pointer. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer data</param>
	explicit DTC_DataPacket(const mu2e_databuff_t* data) : dataPtr_(*data), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Construct a DTC_DataPacket using a pointer to data. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer to data</param>
	explicit DTC_DataPacket(const void* data) : dataPtr_(static_cast<const uint8_t*>(data)), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Construct a DTC_DataPacket in "overlay" mode using the given byte pointer. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer to data</param>
	explicit DTC_DataPacket(const uint8_t* data) : dataPtr_(data), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Creates a copy of the DTC_DataPacket. Mode is preserved, if the existing DataPacket was in "owner" mode, a deep copy is made, otherwise
	/// the reference to the read-only memory will be copied.
	/// </summary>
	/// <param name="in">Input DTC_DataPacket</param>
	DTC_DataPacket(const DTC_DataPacket& in);
	/// <summary>
	/// Default move constructor
	/// </summary>
	/// <param name="in">DTC_DataPacket rvalue</param>
	DTC_DataPacket(DTC_DataPacket&& in) = default;

	virtual ~DTC_DataPacket();

	/// <summary>
	/// Default copy-assignment operator
	/// </summary>
	/// <param name="in">DTC_DataPacket lvalue</param>
	/// <returns>DTC_DataPacket reference</returns>
	DTC_DataPacket& operator=(const DTC_DataPacket& in) = default;
	/// <summary>
	/// Default move-assignment operator
	/// </summary>
	/// <param name="in">DTC_DataPacket rvalue</param>
	/// <returns>DTC_DataPacket reference</returns>
	DTC_DataPacket& operator=(DTC_DataPacket&& in) = default;

	/// <summary>
	/// Set the given word of the DataPacket.
	/// No-op if the DataPacket is in overlay mode
	/// </summary>
	/// <param name="index">Index of the word to change</param>
	/// <param name="data">Value of the word</param>
	void SetWord(uint16_t index, uint8_t data);
	/// <summary>
	/// Gets the current value of the given word
	/// </summary>
	/// <param name="index">Index of the word</param>
	/// <returns>Value of the word</returns>
	uint8_t GetWord(uint16_t index) const;
	/// <summary>
	/// Creates a JSON represenation of the DTC_DataPacket
	/// </summary>
	/// <returns>JSON-formatted string representation of the DTC_DataPacket</returns>
	std::string toJSON() const;
	/// <summary>
	/// Create a "packet format" representation of the DTC_DataPacket. See "DTC Hardware User's Guide" for "packet format" representation.
	/// </summary>
	/// <returns>"packet format" string representation of the DTC_DataPacket</returns>
	std::string toPacketFormat() const;
	/// <summary>
	/// Resize a DTC_DataPacket in "owner" mode. New size must be larger than current.
	/// </summary>
	/// <param name="dmaSize">Size in bytes of the new packet</param>
	/// <returns>If the resize operation was successful</returns>
	bool Resize(const uint16_t dmaSize);

	/// <summary>
	/// Get the current size, in bytes, of the DTC_DataPacket (default: 16)
	/// </summary>
	/// <returns></returns>
	uint16_t GetSize() const
	{
	  return dataSize_;
	}

	/// <summary>
	/// Determine whether the DataPacket is in owner mode or overlay mode
	/// </summary>
	/// <returns>True if the DataPacket is in overlay mode</returns>
	bool IsMemoryPacket() const
	{
	  return memPacket_;
	}

	/// <summary>
	/// Add a DTC_DataPacket's data to this DataPacket. DataPacket must be large enough to accomodate data and in "owner" mode.
	/// </summary>
	/// <param name="other">Packet to add</param>
	/// <param name="offset">Where to copy packet in this DataPacket</param>
	/// <returns>True if successful</returns>
	bool CramIn(DTC_DataPacket& other, int offset)
	{
		if (other.dataSize_ + offset <= dataSize_)
		{
			memcpy(const_cast<uint8_t*>(dataPtr_) + offset, other.dataPtr_, other.dataSize_);
			return true;
		}
		return false;
	}

	/// <summary>
	/// Gets the pointer to the data
	/// </summary>
	/// <returns>Pointer to DTC_DataPacket data. Use GetSize() to determine the valid range of this pointer</returns>
	const uint8_t* GetData() const
	{
	  return dataPtr_;
	}

	/// <summary>
	/// Comparison operator. Returns this.Equals(other)
	/// </summary>
	/// <param name="other">DataPacket to compare</param>
	/// <returns>this.Equals(other)</returns>
	bool operator==(const DTC_DataPacket& other) const
	{
	  return Equals(other);
	}

	/// <summary>
	/// Comparison operator. Returns !this.Equals(other)
	/// </summary>
	/// <param name="other">DataPacket to compare</param>
	/// <returns>!this.Equals(other)</returns>
	bool operator!=(const DTC_DataPacket& other) const
	{
	  return !Equals(other);
	}

	/// <summary>
	/// Compare the contents of two DataPackets. Ignores the first two bytes as they may differ (reserved words in most DMA packets).
	/// </summary>
	/// <param name="other">Data packet to compare</param>
	/// <returns>Whether the two DataPackets contents are equal</returns>
	bool Equals(const DTC_DataPacket& other) const;

	/// <summary>
	/// Serialize a DTC_DataPacket to the given ostream
	/// </summary>
	/// <param name="s">Stream to write to</param>
	/// <param name="p">DataPacket to stream, in binary</param>
	/// <returns>Stream reference for continued streaming</returns>
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

  /// <summary>
  /// Header information common to all DTC Packets (except Data Packets)
  /// </summary>
  class DTC_DMAPacket
  {
  protected:
	uint16_t byteCount_; ///< Byte count of current block
	bool valid_; ///< Whether the DTC believes the packet to be valid
	DTC_Link_ID linkID_; ///< Link identifier of packet
	DTC_PacketType packetType_; ///< Packet type

  public:
	  /// <summary>
	  /// DTC_DMAPacket default constructor. Fills in header fields with default (invalid) values.
	  /// </summary>
	DTC_DMAPacket() : byteCount_(0), valid_(false), linkID_(DTC_Link_Unused), packetType_(DTC_PacketType_Invalid) { }

	/// <summary>
	/// Create a DTC_DMAPacket with the given parameters
	/// </summary>
	/// <param name="type">Packet Type</param>
	/// <param name="link">Link ID</param>
	/// <param name="roc">ROC ID</param>
	/// <param name="byteCount">Block byte count. Default is the minimum value, 64 bytes</param>
	/// <param name="valid">Valid flag for packet, default true</param>
	DTC_DMAPacket(DTC_PacketType type, DTC_Link_ID link, uint16_t byteCount = 64, bool valid = true);

	/// <summary>
	/// Construct a DTC_DMAPacket using the data in the given DataPacket
	/// </summary>
	/// <param name="in">DTC_DataPacket to interpret</param>
	explicit DTC_DMAPacket(const DTC_DataPacket in);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="in">DTC_DMAPacket to copy</param>
	DTC_DMAPacket(const DTC_DMAPacket& in) = default;
	/// <summary>
	/// Default move Constructor
	/// </summary>
	/// <param name="in">DTC_DMAPacket rvalue</param>
	DTC_DMAPacket(DTC_DMAPacket&& in) = default;

	virtual ~DTC_DMAPacket() = default;

	/// <summary>
	/// Default Copy Assignment Operator
	/// </summary>
	/// <param name="in">DTC_DMAPacket to copy</param>
	/// <returns>DTC_DMAPacket reference</returns>
	DTC_DMAPacket& operator=(const DTC_DMAPacket& in) = default;
	/// <summary>
	/// Default Move Assignment Operator
	/// </summary>
	/// <param name="in">DTC_DMAPacket rvalue</param>
	/// <returns>DTC_DMAPacket reference</returns>
	DTC_DMAPacket& operator=(DTC_DMAPacket&& in) = default;

	/// <summary>
	/// Convert a DTC_DMAPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DMA Header bytes set</returns>
	virtual DTC_DataPacket ConvertToDataPacket() const;

	/// <summary>
	/// Packet Type accessor
	/// </summary>
	/// <returns>Packet Type of DMA Packet</returns>
	DTC_PacketType GetPacketType() const
	{
	  return packetType_;
	}

	/// <summary>
	/// Gets the DMA Header in JSON
	/// </summary>
	/// <returns>JSON-formatted string representation of DMA Header information</returns>
	std::string headerJSON() const;
	/// <summary>
	/// Gets the DMA header in "packet format" (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DMA header information</returns>
	std::string headerPacketFormat() const;

	/// <summary>
	/// Gets the block byte count
	/// </summary>
	/// <returns>Block byte count of DMA Header</returns>
	uint16_t GetByteCount() const
	{
	  return byteCount_;
	}

	/// <summary>
	/// Gets the Link ID of the packet
	/// </summary>
	/// <returns>The Link ID of the packet</returns>
	DTC_Link_ID GetRingID() const
	{
	  return linkID_;
	}

	/// <summary>
	/// Converts the DMA Packet to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DMA packet</returns>
	virtual std::string toPacketFormat();
	/// <summary>
	/// Convert the DMA Packet to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DMA packet</returns>
	virtual std::string toJSON();

	/// <summary>
	/// Stream the JSON representation of the DTC_DMAPacket to the given stream
	/// </summary>
	/// <param name="stream">Stream to write JSON data to</param>
	/// <param name="packet">Packet to stream</param>
	/// <returns>Stream reference for continued streaming</returns>
	friend std::ostream& operator<<(std::ostream& stream, DTC_DMAPacket& packet)
	{
	  stream << packet.toJSON();
	  return stream;
	}
  };

  /// <summary>
  /// Representation of a DCS Request Packet
  /// </summary>
  class DTC_DCSRequestPacket : public DTC_DMAPacket
  {
  public:
	  /// <summary>
	  /// Default Constructor, zeroes out header fields
	  /// </summary>
	DTC_DCSRequestPacket();
	/// <summary>
	/// DCSRequestPacket constructor, using given link and roc
	/// </summary>
	/// <param name="link">Link ID for packet</param>
	DTC_DCSRequestPacket(DTC_Link_ID link);
	/// <summary>
	/// Create a DTC_DCSRequestPacket instance with the given fields filled in
	/// </summary>
	/// <param name="link">Link ID for packet</param>
	/// <param name="type">OpCode of packet</param>
	/// <param name="address">Target address</param>
	/// <param name="data">Data (for write)</param>
	DTC_DCSRequestPacket(DTC_Link_ID link, DTC_DCSOperationType type, uint8_t address, uint16_t data);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="in">DTC_DCSRequestPacket to copy</param>
	DTC_DCSRequestPacket(const DTC_DCSRequestPacket& in) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="in">DTC_DCSRequestPacket rvalue</param>
	DTC_DCSRequestPacket(DTC_DCSRequestPacket&& in) = default;
	/// <summary>
	/// Construct a DTC_DCSRequestPacket using the data in the given DataPacket
	/// </summary>
	/// <param name="in">DataPacket to parse</param>
	explicit DTC_DCSRequestPacket(const DTC_DataPacket in);

	/// <summary>
	/// Default Copy Assignment Operator
	/// </summary>
	/// <param name="in">DTC_DCSRequestPacket to copy</param>
	/// <returns>DTC_DCSRequestPacket Reference</returns>
	DTC_DCSRequestPacket& operator=(const DTC_DCSRequestPacket& in) = default;
	/// <summary>
	/// Default Move Assignment Operator
	/// </summary>
	/// <param name="in">DTC_DCSRequestPacket rvalue</param>
	/// <returns>DTC_DCSRequestPacket Reference</returns>
	DTC_DCSRequestPacket& operator=(DTC_DCSRequestPacket&& in) = default;

	virtual ~DTC_DCSRequestPacket() = default;

	/// <summary>
	/// Gets the data word from the DCS Request Packet
	/// </summary>
	/// <returns>Value of the data word from the DCS Request</returns>
	uint16_t GetData() const
	{
	  return data_;
	}

	/// <summary>
	/// Sets the data word in the DCS Request Packet
	/// </summary>
	/// <param name="data">Value of the data word to set</param>
	void SetData(uint16_t data)
	{
	  data_ = data;
	}

	/// <summary>
	/// Gets the target address of the DCS Request Packet, in the ROC address space
	/// </summary>
	/// <returns>Target address of the DCS Request Packet</returns>
	uint8_t GetAddress() const
	{
	  return address_;
	}
	/// <summary>
	/// Sets the target address of the DCS Request Packet in the ROC address space
	/// </summary>
	/// <param name="address">New target address</param>
	void SetAddress(uint8_t address)
	{
	  address_ = address & 0x1F;
	}

	/// <summary>
	/// Gets the opcode of the DCS Request Packet
	/// </summary>
	/// <returns>Current Opcode of the DCS Request Packet</returns>
	DTC_DCSOperationType GetType() const
	{
	  return type_;
	}

	/// <summary>
	/// Sets the opcode of the DCS Request Packet
	/// </summary>
	/// <param name="type">Opcode to set</param>
	void SetType(DTC_DCSOperationType type)
	{
	  type_ = type;
	}

	/// <summary>
	/// Convert a DTC_DCSRequestPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DCS Request Packet contents set</returns>
	DTC_DataPacket ConvertToDataPacket() const override;
	/// <summary>
	/// Convert the DCS Request Packet to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DCS Request packet</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the DCS Request Packet to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DCS Request packet</returns>
	std::string toPacketFormat() override;
  private:
	DTC_DCSOperationType type_;
	uint8_t address_ : 5;
	uint16_t data_;
  };

  /// <summary>
  /// The DTC Heartbeat Packet (sometimes referred to as a "Readout Request" packet)
  /// </summary>
  class DTC_HeartbeatPacket : public DTC_DMAPacket
  {
  public:
	  /// <summary>
	  /// Construct a DTC_HeartbeatPacket
	  /// </summary>
	  /// <param name="link">Destination Link</param>
	explicit DTC_HeartbeatPacket(DTC_Link_ID link);
	/// <summary>
	/// Construct a DTC_HeartbeatPacket
	/// </summary>
	  /// <param name="link">Destination Link</param>
	/// <param name="timestamp">Timestamp of request</param>
	/// <param name="eventMode">Debug event mode bytes (Default: nullptr) If not null, must be 6 bytes long</param>
	DTC_HeartbeatPacket(DTC_Link_ID link, DTC_Timestamp timestamp, uint8_t* eventMode = nullptr);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="right">DTC_HeartbeatPacket to copy</param>
	DTC_HeartbeatPacket(const DTC_HeartbeatPacket& right) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="right">DTC_HeartbeatPacket to move</param>
	DTC_HeartbeatPacket(DTC_HeartbeatPacket&& right) = default;
	/// <summary>
	/// Construct a DTC_HeartbeatPacket from the given DTC_DataPacket
	/// </summary>
	/// <param name="in">DTC_DataPacket to overlay</param>
	explicit DTC_HeartbeatPacket(const DTC_DataPacket in);

	/// <summary>
	/// Default Destructor
	/// </summary>
	virtual ~DTC_HeartbeatPacket() = default;

	/// <summary>
	/// Get the DTC_Timestamp stored in the HeartbeatPacket
	/// </summary>
	/// <returns>Timestamp of Heartbeat</returns>
	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	/// <summary>
	/// Get the Mode bytes from the Heartbeat packet
	/// </summary>
	/// <returns>6-byte array containing mode bytes</returns>
	virtual uint8_t* GetData()
	{
	  return eventMode_;
	}

	/// <summary>
	/// Convert a DTC_HeartbeatPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DTC_HeartbeatPacket contents set</returns>
	DTC_DataPacket ConvertToDataPacket() const override;
	/// <summary>
	/// Convert the DTC_HeartbeatPacket to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DTC_HeartbeatPacket</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the DTC_HeartbeatPacket to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DTC_HeartbeatPacket</returns>
	std::string toPacketFormat() override;
  private:
	DTC_Timestamp timestamp_;
	uint8_t eventMode_[6];
  };

  /// <summary>
  /// The DTC Data Request Packet
  /// </summary>
  class DTC_DataRequestPacket : public DTC_DMAPacket
  {
  public:
	  /// <summary>
	  /// Construct a DTC_DataRequestPacket
	  /// </summary>
	  /// <param name="link">Destination Link</param>
	  /// <param name="debug">Debug Mode flag (Default: true)</param>
	  /// <param name="debugPacketCount">Debug Packet Count (Default: 0)</param>
	  /// <param name="type">Debug Type (Default: DTC_DebugType_SpecialSequence</param>
	DTC_DataRequestPacket(DTC_Link_ID link,  bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
	/// <summary>
	/// Construct a DTC_DataRequestPacket
	/// </summary>
	/// <param name="link">Destination Link</param>
	/// <param name="timestamp">Timestamp to request data for</param>
	/// <param name="debug">Debug Mode flag (Default: true)</param>
	/// <param name="debugPacketCount">Debug Packet Count (Default: 0)</param>
	/// <param name="type">Debug Type (Default: DTC_DebugType_SpecialSequence</param>
	DTC_DataRequestPacket(DTC_Link_ID link,  DTC_Timestamp timestamp, bool debug = true, uint16_t debugPacketCount = 0, DTC_DebugType type = DTC_DebugType_SpecialSequence);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="right">DTC_DataRequestPacket to copy</param>
	DTC_DataRequestPacket(const DTC_DataRequestPacket& right) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="right">DTC_DataRequestPacket to move</param>
	DTC_DataRequestPacket(DTC_DataRequestPacket&& right) = default;
	/// <summary>
	/// Construct a DTC_DataRequestPacket from the given DTC_DataPacket
	/// </summary>
	/// <param name="in">DTC_DataPacket to overlay</param>
	explicit DTC_DataRequestPacket(const DTC_DataPacket in);

	/// <summary>
	/// Get the value of the debug flag
	/// </summary>
	/// <returns>Debug Flag value</returns>
	bool GetDebug() const
	{
	  return debug_;
	}

	/// <summary>
	/// Get the Debug type
	/// </summary>
	/// <returns>DTC_DebugType enumeration value</returns>
	DTC_DebugType GetDebugType() const
	{
	  return type_;
	}

	/// <summary>
	/// Get the Debug Packet Count
	/// </summary>
	/// <returns>Number of packets requested by Data Request</returns>
	uint16_t GetDebugPacketCount() const
	{
	  return debugPacketCount_;
	}

	/// <summary>
	/// Set the Debug Packet Count
	/// </summary>
	/// <param name="count">Number of packets to request</param>
	void SetDebugPacketCount(uint16_t count);

	/// <summary>
	/// Get the timestamp of the request
	/// </summary>
	/// <returns>DTC_Timestamp of reqeust</returns>
	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	/// <summary>
	/// Convert a DTC_DataRequestPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DTC_DataRequestPacket contents set</returns>
	DTC_DataPacket ConvertToDataPacket() const override;
	/// <summary>
	/// Convert the DTC_DataRequestPacket to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DTC_DataRequestPacket</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the DTC_DataRequestPacket to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DTC_DataRequestPacket</returns>
	std::string toPacketFormat() override;
  private:
	DTC_Timestamp timestamp_;
	bool debug_;
	uint16_t debugPacketCount_;
	DTC_DebugType type_;
  };

  /// <summary>
  /// The DCS Reply Packet
  /// </summary>
  class DTC_DCSReplyPacket : public DTC_DMAPacket
  {
  public:
	  /// <summary>
	  /// Construct a DTC_DCSReplyPacket
	  /// </summary>
	  /// <param name="link">Value of the Link field</param>
	explicit DTC_DCSReplyPacket(DTC_Link_ID link);
	/// <summary>
	/// Construct a DTC_DCSReplyPacket
	/// </summary>
	/// <param name="link">Link of packet</param>
	/// <param name="counter">Counter value</param>
	/// <param name="type">DCS Operation Type</param>
	/// <param name="address">Address for operation</param>
	/// <param name="data">Data from operation</param>
	/// <param name="fifoEmpty">Whether the ROC is available for more DCS requests</param>
	DTC_DCSReplyPacket(DTC_Link_ID link, uint8_t counter, DTC_DCSOperationType type, uint8_t address, uint16_t data, bool fifoEmpty);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="right">DTC_DCSReplyPacket to copy</param>
	DTC_DCSReplyPacket(const DTC_DCSReplyPacket& right) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="right">DTC_DCSReplyPacket to move</param>
	DTC_DCSReplyPacket(DTC_DCSReplyPacket&& right) = default;
	/// <summary>
	/// Construct a DTC_DCSReplyPacket from the given DTC_DataPacket
	/// </summary>
	/// <param name="in">DTC_DataPacket to overlay</param>
	explicit DTC_DCSReplyPacket(const DTC_DataPacket in);

	/// <summary>
	/// Get the number of DCS Requests received by the ROC
	/// </summary>
	/// <returns></returns>
	uint8_t GetRequestCounter() const
	{
	  return requestCounter_;
	}

	/// <summary>
	/// Get the data from the DCS Reply packet
	/// </summary>
	/// <returns>DCS Reply packet data</returns>
	uint16_t GetData() const
	{
	  return data_;
	}

	/// <summary>
	/// Get the address from the DCS Reply packet
	/// </summary>
	/// <returns>Address in DCS Reply packet</returns>
	uint8_t GetAddress() const
	{
	  return address_;
	}

	/// <summary>
	/// Get the DCS Operation Type
	/// </summary>
	/// <returns>DTC_DCSOperationType enumeration value</returns>
	DTC_DCSOperationType GetType() const
	{
	  return type_;
	}

	/// <summary>
	/// Check if the DCS Receive FIFO is empty
	/// </summary>
	/// <returns>Value of DCS Receive FIFO Empty flag</returns>
	bool DCSReceiveFIFOEmpty() const
	{
	  return dcsReceiveFIFOEmpty_;
	}

	/// <summary>
	/// Convert a DTC_DCSReplyPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DTC_DCSReplyPacket contents set</returns>
	DTC_DataPacket ConvertToDataPacket() const override;
	/// <summary>
	/// Convert the DTC_DCSReplyPacket to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DTC_DCSReplyPacket</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the DTC_DCSReplyPacket to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DTC_DCSReplyPacket</returns>
	std::string toPacketFormat() override;
  private:
	uint8_t requestCounter_;
	DTC_DCSOperationType type_;
	bool dcsReceiveFIFOEmpty_;
	uint8_t address_ : 5;
	uint16_t data_;
  };

  /// <summary>
  /// The DTC Data Header Packet (A Data Header and its associated Data Packets forms a Data Block)
  /// </summary>
  class DTC_DataHeaderPacket : public DTC_DMAPacket
  {
  public:
	  /// <summary>
	  /// Construct a DTC_DataHeaderPacket
	  /// </summary>
	  /// <param name="link">Link from which packet came</param>
	  /// <param name="packetCount">Number of DTC_DataPackets in Data Block</param>
	  /// <param name="status">Status of Data Block</param>
	  /// <param name="dtcid">DTC ID from which packet came</param>
	  /// <param name="packetVersion">Version of data format</param>
	  /// <param name="timestamp">Timestamp of Data Packet (Default: DTC_Timetstamp())</param>
	  /// <param name="evbMode">EVB Mode byte (Default: 0)</param>
	DTC_DataHeaderPacket(DTC_Link_ID link,  uint16_t packetCount, DTC_DataStatus status, uint8_t dtcid, uint8_t packetVersion, DTC_Timestamp timestamp = DTC_Timestamp(), uint8_t evbMode = 0);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="right">DTC_DataHeaderPacket to copy</param>
	DTC_DataHeaderPacket(const DTC_DataHeaderPacket& right) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="right">DTC_DataHeaderPacket to move</param>
	DTC_DataHeaderPacket(DTC_DataHeaderPacket&& right) = default;
	/// <summary>
	/// Construct a DTC_DataHeaderPacket from the given DTC_DataPacket
	/// </summary>
	/// <param name="in">DTC_DataPacket to overlay</param>
	explicit DTC_DataHeaderPacket(const DTC_DataPacket in);

	/// <summary>
	/// Convert a DTC_DataHeaderPacket to DTC_DataPacket in "owner" mode
	/// </summary>
	/// <returns>DTC_DataPacket with DTC_DataHeaderPacket contents set</returns>
	DTC_DataPacket ConvertToDataPacket() const override;

	/// <summary>
	/// Get the EVB Mode word from the Data Header Packet
	/// </summary>
	/// <returns>EVB Mode of Data Block</returns>
	uint8_t GetEVBMode() const
	{
	  return evbMode_;
	}

	/// <summary>
	/// Get the Subsystem ID of the Data Block
	/// </summary>
	/// <returns>DTC_Subsystem enumeration value</returns>
	DTC_Subsystem GetSubsystem() const
	{
		return dtcId_.GetSubsystem();
	}

	/// <summary>
	/// Get the DTC ID of the Data Block
	/// </summary>
	/// <returns>DTC ID of Data Block</returns>
	uint8_t GetID() const
	{
		return dtcId_.GetID();
	}

	/// <summary>
	/// Get the number of Data Packets in the Data block
	/// </summary>
	/// <returns>The number of packets in the Data Block</returns>
	uint16_t GetPacketCount() const
	{
	  return packetCount_;
	}

	/// <summary>
	/// Get the Data Packet Version identifier from the Data Header
	/// </summary>
	/// <returns>Version number of Data Packets</returns>
	uint8_t GetVersion() const
	{
		return dataPacketVersion_;
	}

	/// <summary>
	/// Get the Timestamp of the Data Block
	/// </summary>
	/// <returns>timestamp of Data Block</returns>
	DTC_Timestamp GetTimestamp() const
	{
	  return timestamp_;
	}

	/// <summary>
	/// Get the Data Status of the Data Block
	/// </summary>
	/// <returns>DTC_DataStatus enumeration value</returns>
	DTC_DataStatus GetStatus() const
	{
	  return status_;
	}

	/// <summary>
	/// Convert the DTC_DataHeaderPacket to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DTC_DataHeaderPacket</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the DTC_DataHeaderPacket to "packet format" representation (See DTC_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DTC_DataHeaderPacket</returns>
	std::string toPacketFormat() override;

	/// <summary>
	/// Determine if two Data Header packets are equal (Evaluates DataPacket == DataPacket, see DTC_DataPacket::Equals)
	/// </summary>
	/// <param name="other">DataHeaderPacket to compare</param>
	/// <returns>True if Data Header packet contents are equal</returns>
	bool operator==(const DTC_DataHeaderPacket& other) const
	{
	  return Equals(other);
	}

	/// <summary>
	/// Determine if two Data Header packets are not equal (Returns !(dhp == dhp))
	/// </summary>
	/// <param name="other">DataHeaderPacket to compare</param>
	/// <returns>True if Data Header packet contents are not equal</returns>
	bool operator!=(const DTC_DataHeaderPacket& other) const
	{
	  return !Equals(other);
	}

	/// <summary>
	/// Determine if two Data Header packets are equal (Evaluates DataPacket == DataPacket, see DTC_DataPacket::Equals)
	/// </summary>
	/// <param name="other">DataHeaderPacket to compare</param>
	/// <returns>True if Data Header packet contents are equal</returns>
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
