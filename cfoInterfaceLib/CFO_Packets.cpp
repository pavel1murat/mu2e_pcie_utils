#include "CFO_Packets.h"
#include <sstream>
#include <iomanip>
#include <cstring>

CFOLib::CFO_DataPacket::CFO_DataPacket()
{
	memPacket_ = false;
	vals_ = std::vector<uint8_t>(16);
	dataPtr_ = &vals_[0];
	dataSize_ = 16;
}

CFOLib::CFO_DataPacket::CFO_DataPacket(const CFO_DataPacket& in)
{
	dataSize_ = in.GetSize();
	memPacket_ = in.IsMemoryPacket();
	if (!memPacket_)
	{
		vals_ = std::vector<uint8_t>(dataSize_);
		dataPtr_ = &vals_[0];
		memcpy(const_cast<uint8_t*>(dataPtr_), in.GetData(), in.GetSize() * sizeof(uint8_t));
	}
	else
	{
		dataPtr_ = in.GetData();
	}
}

CFOLib::CFO_DataPacket::~CFO_DataPacket()
{
	if (!memPacket_ && dataPtr_ != nullptr)
	{
		dataPtr_ = nullptr;
	}
}

void CFOLib::CFO_DataPacket::SetWord(uint16_t index, uint8_t data)
{
	if (!memPacket_ && index < dataSize_)
	{
	  const_cast<uint8_t*>(dataPtr_)[index] = data;
	}
}

uint8_t CFOLib::CFO_DataPacket::GetWord(uint16_t index) const
{
	if (index < dataSize_) return dataPtr_[index];
	return 0;
}

bool CFOLib::CFO_DataPacket::Resize(const uint16_t dmaSize)
{
	if (!memPacket_ && dmaSize > dataSize_)
	{
		vals_.resize(dmaSize);
		dataPtr_ = &vals_[0];
		dataSize_ = dmaSize;
		return true;
	}

	//We can only grow, and only non-memory-mapped packets
	return false;
}

std::string CFOLib::CFO_DataPacket::toJSON() const
{
	std::stringstream ss;
	ss << "\"DataPacket\": {";
	ss << "\"data\": [";
	ss << std::hex << std::setfill('0');
	for (uint16_t ii = 0; ii < dataSize_ - 1; ++ii)
	{
		ss << "0x" << std::setw(2) << static_cast<int>(dataPtr_[ii]) << ",";
	}
	ss << "0x" << std::setw(2) << static_cast<int>(dataPtr_[dataSize_ - 1]) << "]";
	ss << "}";
	return ss.str();
}

std::string CFOLib::CFO_DataPacket::toPacketFormat() const
{
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	for (uint16_t ii = 0; ii < dataSize_ - 1; ii += 2)
	{
		ss << "0x" << std::setw(6) << static_cast<int>(dataPtr_[ii + 1]) << "\t";
		ss << "0x" << std::setw(6) << static_cast<int>(dataPtr_[ii]) << "\n";
	}
	return ss.str();
}

bool CFOLib::CFO_DataPacket::Equals(const CFO_DataPacket& other) const
{
	auto equal = true;
	for (uint16_t ii = 2; ii < 16; ++ii)
	{
		//TRACE(21, "CFO_DataPacket::Equals: Comparing %u to %u", GetWord(ii), other.GetWord(ii));
		if (other.GetWord(ii) != GetWord(ii))
		{
			equal = false;
			break;
		}
	}

	return equal;
}

CFOLib::CFO_DMAPacket::CFO_DMAPacket(CFO_PacketType type, CFO_Ring_ID ring, CFO_ROC_ID roc, uint16_t byteCount, bool valid)
	: byteCount_(byteCount < 64 ? 64 : byteCount), valid_(valid), ringID_(ring), packetType_(type), rocID_(roc) {}

CFOLib::CFO_DataPacket CFOLib::CFO_DMAPacket::ConvertToDataPacket() const
{
	CFO_DataPacket output;
	auto word0A = static_cast<uint8_t>(byteCount_);
	auto word0B = static_cast<uint8_t>(byteCount_ >> 8);
	output.SetWord(0, word0A);
	output.SetWord(1, word0B);
	auto word1A = static_cast<uint8_t>(rocID_);
	word1A += static_cast<uint8_t>(packetType_) << 4;
	uint8_t word1B = static_cast<uint8_t>(ringID_) + (valid_ ? 0x80 : 0x0);
	output.SetWord(2, word1A);
	output.SetWord(3, word1B);
	for (uint16_t i = 4; i < 16; ++i)
	{
		output.SetWord(i, 0);
	}
	return output;
}

CFOLib::CFO_DMAPacket::CFO_DMAPacket(const CFO_DataPacket in)
{
	auto word2 = in.GetData()[2];
	uint8_t roc = word2 & 0xF;
	uint8_t packetType = word2 >> 4;
	auto word3 = in.GetData()[3];
	uint8_t ringID = word3 & 0xF;
	valid_ = (word3 & 0x80) == 0x80;

	byteCount_ = in.GetData()[0] + (in.GetData()[1] << 8);
	ringID_ = static_cast<CFO_Ring_ID>(ringID);
	rocID_ = static_cast<CFO_ROC_ID>(roc);
	packetType_ = static_cast<CFO_PacketType>(packetType);
	TRACE(20, headerJSON().c_str());
}

std::string CFOLib::CFO_DMAPacket::headerJSON() const
{
	std::stringstream ss;
	ss << "\"isValid\": " << valid_ << ",";
	ss << "\"byteCount\": " << std::hex << "0x" << byteCount_ << ",";
	ss << "\"ringID\": " << std::dec << ringID_ << ",";
	ss << "\"packetType\": " << packetType_ << ",";
	ss << "\"rocID\": " << rocID_;
	return ss.str();
}

std::string CFOLib::CFO_DMAPacket::headerPacketFormat() const
{
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	ss << "0x" << std::setw(6) << ((byteCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (byteCount_ & 0xFF) << std::endl;
	ss << std::setw(1) << static_cast<int>(valid_) << "   " << "0x" << std::setw(2) << ringID_ << "\t";
	ss << "0x" << std::setw(2) << packetType_ << "0x" << std::setw(2) << rocID_ << std::endl;
	return ss.str();
}

std::string CFOLib::CFO_DMAPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DMAPacket\": {";
	ss << headerJSON();
	ss << "}";
	return ss.str();
}

std::string CFOLib::CFO_DMAPacket::toPacketFormat()
{
	return headerPacketFormat();
}

CFOLib::CFO_HeartbeatPacket::CFO_HeartbeatPacket(CFO_Ring_ID ring, CFO_ROC_ID maxROC)
	: CFO_DMAPacket(CFO_PacketType_Heartbeat, ring, maxROC), timestamp_(), eventMode_()
{
	eventMode_[0] = 0;
	eventMode_[1] = 0;
	eventMode_[2] = 0;
	eventMode_[3] = 0;
	eventMode_[4] = 0;
	eventMode_[5] = 0;
}

CFOLib::CFO_HeartbeatPacket::CFO_HeartbeatPacket(CFO_Ring_ID ring, CFO_Timestamp timestamp, CFO_ROC_ID maxROC, uint8_t* eventMode)
	: CFO_DMAPacket(CFO_PacketType_Heartbeat, ring, maxROC), timestamp_(timestamp), eventMode_()
{
	if (eventMode != nullptr)
	{
		for (auto i = 0; i < 6; ++i)
		{
			eventMode_[i] = eventMode[i];
		}
	}
}

CFOLib::CFO_HeartbeatPacket::CFO_HeartbeatPacket(const CFO_DataPacket in) : CFO_DMAPacket(in)
{
	if (packetType_ != CFO_PacketType_Heartbeat)
	{
		throw CFO_WrongPacketTypeException(CFO_PacketType_Heartbeat, packetType_);
	}
	auto arr = in.GetData();
	eventMode_[0] = arr[10];
	eventMode_[1] = arr[11];
	eventMode_[2] = arr[12];
	eventMode_[3] = arr[13];
	eventMode_[4] = arr[14];
	eventMode_[5] = arr[15];
	timestamp_ = CFO_Timestamp(arr, 4);
}

std::string CFOLib::CFO_HeartbeatPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"ReadoutRequestPacket\": {";
	ss << headerJSON() << ",";
	ss << timestamp_.toJSON() << ",";
	ss << "\"request\": [" << std::hex << "0x" << static_cast<int>(eventMode_[0]) << ",";
	ss << std::hex << "0x" << static_cast<int>(eventMode_[1]) << ",";
	ss << std::hex << "0x" << static_cast<int>(eventMode_[2]) << ",";
	ss << std::hex << "0x" << static_cast<int>(eventMode_[3]) << ",";
	ss << std::hex << "0x" << static_cast<int>(eventMode_[4]) << ",";
	ss << std::hex << "0x" << static_cast<int>(eventMode_[5]) << "],";
	ss << "}";
	return ss.str();
}

std::string CFOLib::CFO_HeartbeatPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << timestamp_.toPacketFormat();
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[1]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[0]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[3]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[2]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[5]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[4]) << "\n";
	return ss.str();
}

CFOLib::CFO_DataPacket CFOLib::CFO_HeartbeatPacket::ConvertToDataPacket() const
{
	auto output = CFO_DMAPacket::ConvertToDataPacket();
	timestamp_.GetTimestamp(output.GetData(), 4);
	for (auto i = 0; i < 6; ++i)
	{
	  output.SetWord(static_cast<uint16_t>(10 + i), eventMode_[i]);
	}
	return output;
}
