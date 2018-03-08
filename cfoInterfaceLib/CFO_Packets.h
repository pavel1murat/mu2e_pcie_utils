#ifndef CFO_PACKETS_H
#define CFO_PACKETS_H

#include <bitset>
#include <cstdint> // uint8_t, uint16_t
#include <vector>

#include "CFO_Types.h"

#include "cfo_driver/cfo_mmap_ioctl.h"


namespace CFOLib
{

  /// <summary>
  /// Defined Packet Types for the CFO DMA Protocol
  /// </summary>
  enum CFO_PacketType : uint8_t
	{
		CFO_PacketType_Heartbeat = 1,
		CFO_PacketType_Invalid = 0x10,
		};

  /// <summary>
  /// The CFO_DataPacket class represents the 16 bytes of raw data for all CFO packets.
  /// The class works in two modes: "overlay" mode, where the data is in a fixed location in memory and modification is restricted, and
  /// "owner" mode, where the DataPacket is a concrete instance.
  /// </summary>
  class CFO_DataPacket
  {
  public:
	  /// <summary>
	  /// Construct a CFO_DataPacket in owner mode
	  /// </summary>
	CFO_DataPacket();

	/// <summary>
	/// Construct a CFO_DataPacket in "overlay" mode using the given DMA buffer pointer. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer data</param>
	explicit CFO_DataPacket(const cfo_databuff_t* data) : dataPtr_(*data), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Construct a CFO_DataPacket using a pointer to data. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer to data</param>
	explicit CFO_DataPacket(const void* data) : dataPtr_(static_cast<const uint8_t*>(data)), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Construct a CFO_DataPacket in "overlay" mode using the given byte pointer. Flag will be set that the packet is read-only.
	/// </summary>
	/// <param name="data">Pointer to data</param>
	explicit CFO_DataPacket(const uint8_t* data) : dataPtr_(data), dataSize_(16), memPacket_(true) { }

	/// <summary>
	/// Creates a copy of the CFO_DataPacket. Mode is preserved, if the existing DataPacket was in "owner" mode, a deep copy is made, otherwise
	/// the reference to the read-only memory will be copied.
	/// </summary>
	/// <param name="in">Input CFO_DataPacket</param>
	CFO_DataPacket(const CFO_DataPacket& in);
	/// <summary>
	/// Default move constructor
	/// </summary>
	/// <param name="in">CFO_DataPacket rvalue</param>
	CFO_DataPacket(CFO_DataPacket&& in) = default;

	virtual ~CFO_DataPacket();

	/// <summary>
	/// Default copy-assignment operator
	/// </summary>
	/// <param name="in">CFO_DataPacket lvalue</param>
	/// <returns>CFO_DataPacket reference</returns>
	CFO_DataPacket& operator=(const CFO_DataPacket& in) = default;
	/// <summary>
	/// Default move-assignment operator
	/// </summary>
	/// <param name="in">CFO_DataPacket rvalue</param>
	/// <returns>CFO_DataPacket reference</returns>
	CFO_DataPacket& operator=(CFO_DataPacket&& in) = default;

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
	/// Creates a JSON represenation of the CFO_DataPacket
	/// </summary>
	/// <returns>JSON-formatted string representation of the CFO_DataPacket</returns>
	std::string toJSON() const;
	/// <summary>
	/// Create a "packet format" representation of the CFO_DataPacket. See "CFO Hardware User's Guide" for "packet format" representation.
	/// </summary>
	/// <returns>"packet format" string representation of the CFO_DataPacket</returns>
	std::string toPacketFormat() const;
	/// <summary>
	/// Resize a CFO_DataPacket in "owner" mode. New size must be larger than current.
	/// </summary>
	/// <param name="dmaSize">Size in bytes of the new packet</param>
	/// <returns>If the resize operation was successful</returns>
	bool Resize(const uint16_t dmaSize);

	/// <summary>
	/// Get the current size, in bytes, of the CFO_DataPacket (default: 16)
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
	/// Add a CFO_DataPacket's data to this DataPacket. DataPacket must be large enough to accomodate data and in "owner" mode.
	/// </summary>
	/// <param name="other">Packet to add</param>
	/// <param name="offset">Where to copy packet in this DataPacket</param>
	/// <returns>True if successful</returns>
	bool CramIn(CFO_DataPacket& other, int offset)
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
	/// <returns>Pointer to CFO_DataPacket data. Use GetSize() to determine the valid range of this pointer</returns>
	const uint8_t* GetData() const
	{
	  return dataPtr_;
	}

	/// <summary>
	/// Comparison operator. Returns this.Equals(other)
	/// </summary>
	/// <param name="other">DataPacket to compare</param>
	/// <returns>this.Equals(other)</returns>
	bool operator==(const CFO_DataPacket& other) const
	{
	  return Equals(other);
	}

	/// <summary>
	/// Comparison operator. Returns !this.Equals(other)
	/// </summary>
	/// <param name="other">DataPacket to compare</param>
	/// <returns>!this.Equals(other)</returns>
	bool operator!=(const CFO_DataPacket& other) const
	{
	  return !Equals(other);
	}

	/// <summary>
	/// Compare the contents of two DataPackets. Ignores the first two bytes as they may differ (reserved words in most DMA packets).
	/// </summary>
	/// <param name="other">Data packet to compare</param>
	/// <returns>Whether the two DataPackets contents are equal</returns>
	bool Equals(const CFO_DataPacket& other) const;

	/// <summary>
	/// Serialize a CFO_DataPacket to the given ostream
	/// </summary>
	/// <param name="s">Stream to write to</param>
	/// <param name="p">DataPacket to stream, in binary</param>
	/// <returns>Stream reference for continued streaming</returns>
	friend std::ostream& operator<<(std::ostream& s, CFO_DataPacket& p)
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
  /// Header information common to all CFO Packets (except Data Packets)
  /// </summary>
  class CFO_DMAPacket
  {
  protected:
	uint16_t byteCount_; ///< Byte count of current block
	bool valid_; ///< Whether the CFO believes the packet to be valid
	CFO_Ring_ID ringID_; ///< Ring identifier of packet
	CFO_PacketType packetType_; ///< Packet type
	CFO_ROC_ID rocID_; ///< ROC identifier of the packet

  public:
	  /// <summary>
	  /// CFO_DMAPacket default constructor. Fills in header fields with default (invalid) values.
	  /// </summary>
	CFO_DMAPacket() : byteCount_(0), valid_(false), ringID_(CFO_Ring_Unused), packetType_(CFO_PacketType_Invalid), rocID_(CFO_ROC_Unused) { }

	/// <summary>
	/// Create a CFO_DMAPacket with the given parameters
	/// </summary>
	/// <param name="type">Packet Type</param>
	/// <param name="ring">Ring ID</param>
	/// <param name="roc">ROC ID</param>
	/// <param name="byteCount">Block byte count. Default is the minimum value, 64 bytes</param>
	/// <param name="valid">Valid flag for packet, default true</param>
	CFO_DMAPacket(CFO_PacketType type, CFO_Ring_ID ring, CFO_ROC_ID roc, uint16_t byteCount = 64, bool valid = true);

	/// <summary>
	/// Construct a CFO_DMAPacket using the data in the given DataPacket
	/// </summary>
	/// <param name="in">CFO_DataPacket to interpret</param>
	explicit CFO_DMAPacket(const CFO_DataPacket in);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="in">CFO_DMAPacket to copy</param>
	CFO_DMAPacket(const CFO_DMAPacket& in) = default;
	/// <summary>
	/// Default move Constructor
	/// </summary>
	/// <param name="in">CFO_DMAPacket rvalue</param>
	CFO_DMAPacket(CFO_DMAPacket&& in) = default;

	virtual ~CFO_DMAPacket() = default;

	/// <summary>
	/// Default Copy Assignment Operator
	/// </summary>
	/// <param name="in">CFO_DMAPacket to copy</param>
	/// <returns>CFO_DMAPacket reference</returns>
	CFO_DMAPacket& operator=(const CFO_DMAPacket& in) = default;
	/// <summary>
	/// Default Move Assignment Operator
	/// </summary>
	/// <param name="in">CFO_DMAPacket rvalue</param>
	/// <returns>CFO_DMAPacket reference</returns>
	CFO_DMAPacket& operator=(CFO_DMAPacket&& in) = default;

	/// <summary>
	/// Convert a CFO_DMAPacket to CFO_DataPacket in "owner" mode
	/// </summary>
	/// <returns>CFO_DataPacket with DMA Header bytes set</returns>
	virtual CFO_DataPacket ConvertToDataPacket() const;

	/// <summary>
	/// Packet Type accessor
	/// </summary>
	/// <returns>Packet Type of DMA Packet</returns>
	CFO_PacketType GetPacketType() const
	{
	  return packetType_;
	}

	/// <summary>
	/// Gets the DMA Header in JSON
	/// </summary>
	/// <returns>JSON-formatted string representation of DMA Header information</returns>
	std::string headerJSON() const;
	/// <summary>
	/// Gets the DMA header in "packet format" (See CFO_DataPacket::toPacketFormat())
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
	/// Gets the Ring ID of the packet
	/// </summary>
	/// <returns>The Ring ID of the packet</returns>
	CFO_Ring_ID GetRingID() const
	{
	  return ringID_;
	}

	/// <summary>
	/// Converts the DMA Packet to "packet format" representation (See CFO_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of DMA packet</returns>
	virtual std::string toPacketFormat();
	/// <summary>
	/// Convert the DMA Packet to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of DMA packet</returns>
	virtual std::string toJSON();

	/// <summary>
	/// Stream the JSON representation of the CFO_DMAPacket to the given stream
	/// </summary>
	/// <param name="stream">Stream to write JSON data to</param>
	/// <param name="packet">Packet to stream</param>
	/// <returns>Stream reference for continued streaming</returns>
	friend std::ostream& operator<<(std::ostream& stream, CFO_DMAPacket& packet)
	{
	  stream << packet.toJSON();
	  return stream;
	}
  };
  
  /// <summary>
  /// The CFO Heartbeat Packet (sometimes referred to as a "Readout Request" packet)
  /// </summary>
  class CFO_HeartbeatPacket : public CFO_DMAPacket
  {
  public:
	  /// <summary>
	  /// Construct a CFO_HeartbeatPacket
	  /// </summary>
	  /// <param name="ring">Destination Ring</param>
	  /// <param name="maxROC">Number of "hops" along the ring the packet will travel (Default: CFO_ROC_5)</param>
	explicit CFO_HeartbeatPacket(CFO_Ring_ID ring, CFO_ROC_ID maxROC = CFO_ROC_5);
	/// <summary>
	/// Construct a CFO_HeartbeatPacket
	/// </summary>
	  /// <param name="ring">Destination Ring</param>
	/// <param name="timestamp">Timestamp of request</param>
	  /// <param name="maxROC">Number of "hops" along the ring the packet will travel (Default: CFO_ROC_5)</param>
	/// <param name="eventMode">Debug event mode bytes (Default: nullptr) If not null, must be 6 bytes long</param>
	CFO_HeartbeatPacket(CFO_Ring_ID ring, CFO_Timestamp timestamp, CFO_ROC_ID maxROC = CFO_ROC_5, uint8_t* eventMode = nullptr);
	/// <summary>
	/// Default Copy Constructor
	/// </summary>
	/// <param name="right">CFO_HeartbeatPacket to copy</param>
	CFO_HeartbeatPacket(const CFO_HeartbeatPacket& right) = default;
	/// <summary>
	/// Default Move Constructor
	/// </summary>
	/// <param name="right">CFO_HeartbeatPacket to move</param>
	CFO_HeartbeatPacket(CFO_HeartbeatPacket&& right) = default;
	/// <summary>
	/// Construct a CFO_HeartbeatPacket from the given CFO_DataPacket
	/// </summary>
	/// <param name="in">CFO_DataPacket to overlay</param>
	explicit CFO_HeartbeatPacket(const CFO_DataPacket in);

	/// <summary>
	/// Default Destructor
	/// </summary>
	virtual ~CFO_HeartbeatPacket() = default;

	/// <summary>
	/// Get the CFO_Timestamp stored in the HeartbeatPacket
	/// </summary>
	/// <returns>Timestamp of Heartbeat</returns>
	CFO_Timestamp GetTimestamp() const
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
	/// Convert a CFO_HeartbeatPacket to CFO_DataPacket in "owner" mode
	/// </summary>
	/// <returns>CFO_DataPacket with CFO_HeartbeatPacket contents set</returns>
	CFO_DataPacket ConvertToDataPacket() const override;
	/// <summary>
	/// Convert the CFO_HeartbeatPacket to JSON representation
	/// </summary>
	/// <returns>JSON-formatted string representation of CFO_HeartbeatPacket</returns>
	std::string toJSON() override;
	/// <summary>
	/// Converts the CFO_HeartbeatPacket to "packet format" representation (See CFO_DataPacket::toPacketFormat())
	/// </summary>
	/// <returns>"packet format" string representation of CFO_HeartbeatPacket</returns>
	std::string toPacketFormat() override;
  private:
	CFO_Timestamp timestamp_;
	uint8_t eventMode_[6];
  };

}

#endif //CFO_PACKETS_H
