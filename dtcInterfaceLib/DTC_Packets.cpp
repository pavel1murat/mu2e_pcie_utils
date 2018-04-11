#include "DTC_Packets.h"
#include <sstream>
#include <iomanip>
#include <cstring>

DTCLib::DTC_DataPacket::DTC_DataPacket()
{
	memPacket_ = false;
	vals_ = std::vector<uint8_t>(16);
	dataPtr_ = &vals_[0];
	dataSize_ = 16;
}

DTCLib::DTC_DataPacket::DTC_DataPacket(const DTC_DataPacket& in)
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

DTCLib::DTC_DataPacket::~DTC_DataPacket()
{
	if (!memPacket_ && dataPtr_ != nullptr)
	{
		dataPtr_ = nullptr;
	}
}

void DTCLib::DTC_DataPacket::SetWord(uint16_t index, uint8_t data)
{
	if (!memPacket_ && index < dataSize_)
	{
	  const_cast<uint8_t*>(dataPtr_)[index] = data;
	}
}

uint8_t DTCLib::DTC_DataPacket::GetWord(uint16_t index) const
{
	if (index < dataSize_) return dataPtr_[index];
	return 0;
}

bool DTCLib::DTC_DataPacket::Resize(const uint16_t dmaSize)
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

std::string DTCLib::DTC_DataPacket::toJSON() const
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

std::string DTCLib::DTC_DataPacket::toPacketFormat() const
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

bool DTCLib::DTC_DataPacket::Equals(const DTC_DataPacket& other) const
{
	auto equal = true;
	for (uint16_t ii = 2; ii < 16; ++ii)
	{
		//TRACE(21, "DTC_DataPacket::Equals: Comparing %u to %u", GetWord(ii), other.GetWord(ii));
		if (other.GetWord(ii) != GetWord(ii))
		{
			equal = false;
			break;
		}
	}

	return equal;
}

DTCLib::DTC_DMAPacket::DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t byteCount, bool valid)
	: byteCount_(byteCount < 64 ? 64 : byteCount), valid_(valid), ringID_(ring), packetType_(type), rocID_(roc) {}

DTCLib::DTC_DataPacket DTCLib::DTC_DMAPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output;
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

DTCLib::DTC_DMAPacket::DTC_DMAPacket(const DTC_DataPacket in)
{
	auto word2 = in.GetData()[2];
	uint8_t roc = word2 & 0xF;
	uint8_t packetType = word2 >> 4;
	auto word3 = in.GetData()[3];
	uint8_t ringID = word3 & 0xF;
	valid_ = (word3 & 0x80) == 0x80;

	byteCount_ = in.GetData()[0] + (in.GetData()[1] << 8);
	ringID_ = static_cast<DTC_Ring_ID>(ringID);
	rocID_ = static_cast<DTC_ROC_ID>(roc);
	packetType_ = static_cast<DTC_PacketType>(packetType);
	TLOG_ARB(20, "DTC_DMAPacket") << headerJSON();
}

std::string DTCLib::DTC_DMAPacket::headerJSON() const
{
	std::stringstream ss;
	ss << "\"isValid\": " << valid_ << ",";
	ss << "\"byteCount\": " << std::hex << "0x" << byteCount_ << ",";
	ss << "\"ringID\": " << std::dec << ringID_ << ",";
	ss << "\"packetType\": " << packetType_ << ",";
	ss << "\"rocID\": " << rocID_;
	return ss.str();
}

std::string DTCLib::DTC_DMAPacket::headerPacketFormat() const
{
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	ss << "0x" << std::setw(6) << ((byteCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (byteCount_ & 0xFF) << std::endl;
	ss << std::setw(1) << static_cast<int>(valid_) << "   " << "0x" << std::setw(2) << ringID_ << "\t";
	ss << "0x" << std::setw(2) << packetType_ << "0x" << std::setw(2) << rocID_ << std::endl;
	return ss.str();
}

std::string DTCLib::DTC_DMAPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DMAPacket\": {";
	ss << headerJSON();
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DMAPacket::toPacketFormat()
{
	return headerPacketFormat();
}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket()
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, DTC_Ring_Unused, DTC_ROC_Unused), type_(), address_(0), data_(0) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc)
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, ring, roc), type_(), address_(0), data_(0) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_DCSOperationType type, uint8_t address, uint16_t data)
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, ring, roc)
	  , type_(type)
	  , address_(address & 0x1F)
	  , data_(data) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_DCSRequest)
	{
		throw DTC_WrongPacketTypeException(DTC_PacketType_DCSRequest, packetType_);
	}
	type_ = static_cast<DTC_DCSOperationType>(in.GetData()[4]);
	address_ = in.GetData()[6] & 0x1F;
	data_ = in.GetData()[10] + (in.GetData()[11] << 8);
}

std::string DTCLib::DTC_DCSRequestPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DCSRequestPacket\": {";
	ss << headerJSON() << ", ";
	ss << "\"Operation Type\":" << DTC_DCSOperationTypeConverter(type_) << ", ";
	ss << "\"Address\": " << static_cast<int>(address_) << ", ";
	ss << "\"Data\": " << static_cast<int>(data_) << ", ";
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DCSRequestPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::hex << std::setfill('0');
	ss << "        \t" << std::setw(8) << static_cast<int>(type_) << std::endl;
	ss << "        \t    " << std::setw(4) << static_cast<int>(address_) << std::endl;
	ss << "        \t        " << std::endl;
	ss << std::setw(8) << ((data_ & 0xFF00) >> 8) << "\t" << (data_ & 0xFF) << std::endl;
	ss << "        \t        " << std::endl;
	ss << "        \t        " << std::endl;
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSRequestPacket::ConvertToDataPacket() const
{
	auto output = DTC_DMAPacket::ConvertToDataPacket();
	output.SetWord(4, static_cast<uint8_t>(type_));
	output.SetWord(6, static_cast<uint8_t>(address_) & 0x1F);
	output.SetWord(10, static_cast<uint8_t>(data_ & 0xFF));
	output.SetWord(11, static_cast<uint8_t>(((data_ & 0xFF00) >> 8)));
	return output;
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC)
	: DTC_DMAPacket(DTC_PacketType_Heartbeat, ring, maxROC), timestamp_(), eventMode_()
{
	eventMode_[0] = 0;
	eventMode_[1] = 0;
	eventMode_[2] = 0;
	eventMode_[3] = 0;
	eventMode_[4] = 0;
	eventMode_[5] = 0;
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC, uint8_t* eventMode)
	: DTC_DMAPacket(DTC_PacketType_Heartbeat, ring, maxROC), timestamp_(timestamp), eventMode_()
{
	if (eventMode != nullptr)
	{
		for (auto i = 0; i < 6; ++i)
		{
			eventMode_[i] = eventMode[i];
		}
	}
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(const DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_Heartbeat)
	{
		throw DTC_WrongPacketTypeException(DTC_PacketType_Heartbeat, packetType_);
	}
	auto arr = in.GetData();
	eventMode_[0] = arr[10];
	eventMode_[1] = arr[11];
	eventMode_[2] = arr[12];
	eventMode_[3] = arr[13];
	eventMode_[4] = arr[14];
	eventMode_[5] = arr[15];
	timestamp_ = DTC_Timestamp(arr, 4);
}

std::string DTCLib::DTC_HeartbeatPacket::toJSON()
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

std::string DTCLib::DTC_HeartbeatPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << timestamp_.toPacketFormat();
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[1]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[0]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[3]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[2]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(eventMode_[5]) << "\t0x" << std::setw(6) << static_cast<int>(eventMode_[4]) << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_HeartbeatPacket::ConvertToDataPacket() const
{
	auto output = DTC_DMAPacket::ConvertToDataPacket();
	timestamp_.GetTimestamp(output.GetData(), 4);
	for (auto i = 0; i < 6; ++i)
	{
	  output.SetWord(static_cast<uint16_t>(10 + i), eventMode_[i]);
	}
	return output;
}

DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, bool debug, uint16_t debugPacketCount, DTC_DebugType type)
	: DTC_DMAPacket(DTC_PacketType_DataRequest, ring, roc), timestamp_(), debug_(debug), debugPacketCount_(debugPacketCount), type_(type) {}

DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp, bool debug, uint16_t debugPacketCount, DTC_DebugType type)
	: DTC_DMAPacket(DTC_PacketType_DataRequest, ring, roc), timestamp_(timestamp), debug_(debug), debugPacketCount_(debugPacketCount), type_(type) {}

DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_DataRequest)
	{
	  throw DTC_WrongPacketTypeException(DTC_PacketType_DataRequest, packetType_);
	}
	timestamp_ = DTC_Timestamp(in.GetData(), 4);
	debug_ = (in.GetData()[12] & 0x1) == 1;
	type_ = DTC_DebugType((in.GetData()[12] & 0xF0) >> 4);
	debugPacketCount_ = in.GetData()[14] + (in.GetData()[15] << 8);
}

std::string DTCLib::DTC_DataRequestPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DataRequestPacket\": {";
	ss << headerJSON() << ",";
	ss << timestamp_.toJSON() << ",";
	ss << "\"debug\":" << (debug_ ? "true" : "false") << ",";
	ss << "\"debugPacketCount\": " << std::dec << static_cast<int>(debugPacketCount_) << ",";
	ss << DTC_DebugTypeConverter(type_);
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DataRequestPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << timestamp_.toPacketFormat();
	ss << "        \t        \n";
	ss << "        \t0x" << std::setw(2) << static_cast<int>(type_) << "   " << std::setw(1) << static_cast<int>(debug_) << "\n";
	ss << "0x" << std::setw(6) << ((debugPacketCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (debugPacketCount_ & 0xFF) << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataRequestPacket::ConvertToDataPacket() const
{
	auto output = DTC_DMAPacket::ConvertToDataPacket();
	timestamp_.GetTimestamp(output.GetData(), 4);
	output.SetWord(12, (static_cast<uint8_t>(type_) << 4) + (debug_ ? 1 : 0));
	output.SetWord(14, debugPacketCount_ & 0xFF);
	output.SetWord(15, (debugPacketCount_ >> 8) & 0xFF);
	return output;
}

void DTCLib::DTC_DataRequestPacket::SetDebugPacketCount(uint16_t count)
{
	if (count > 0)
	{
		debug_ = true;
	}
	else
	{
		debug_ = false;
	}
	debugPacketCount_ = count;
}

DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_Ring_ID ring)
	: DTC_DMAPacket(DTC_PacketType_DCSReply, ring, DTC_ROC_Unused), requestCounter_(0), type_(), dcsReceiveFIFOEmpty_(false), address_(0), data_(0) {}

DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t counter, DTC_DCSOperationType type, uint8_t address, uint16_t data, bool fifoEmpty)
	: DTC_DMAPacket(DTC_PacketType_DCSReply, ring, DTC_ROC_Unused)
	  , requestCounter_(counter)
	  , type_(type)
	  , dcsReceiveFIFOEmpty_(fifoEmpty)
	  , address_(address & 0x1F)
	  , data_(data) {}

DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	TRACE(20, "DTC_DCSReplyPacket::DTC_DCSReplyPacket Before packetType test");
	if (packetType_ != DTC_PacketType_DCSReply)
	{
	  throw DTC_WrongPacketTypeException(DTC_PacketType_DCSReply, packetType_);
	}

	type_ = static_cast<DTC_DCSOperationType>(in.GetData()[4]);
	requestCounter_ = in.GetData()[5];
	address_ = in.GetData()[6] & 0x1F;
	dcsReceiveFIFOEmpty_ = (in.GetData()[7] & 0x8) == 0x8;
	data_ = in.GetData()[10] + (in.GetData()[11] << 8);
}

std::string DTCLib::DTC_DCSReplyPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DCSRequestPacket\": {";
	ss << headerJSON() << ",";
	ss << "\"Operation Type\":" << DTC_DCSOperationTypeConverter(type_) << ",";
	ss << "\"Address\": " << static_cast<int>(address_) << ",";
	ss << "\"Data\": " << static_cast<int>(data_);
	ss << "\"Request Counter\": " << static_cast<int>(requestCounter_) << ",";
	ss << "\"DCS Request FIFO Empty\": " << (dcsReceiveFIFOEmpty_ ? "\"true\"" : "\"false\"");
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DCSReplyPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::hex << std::setfill('0');
	ss << std::setw(8) << static_cast<int>(requestCounter_) << "\t" << std::setw(8) << static_cast<int>(type_) << std::endl;
	ss << (dcsReceiveFIFOEmpty_ ? "    E   " : "        ") << "\t    " << std::setw(4) << static_cast<int>(address_) << std::endl;
	ss << "        \t        " << std::endl;
	ss << std::setw(8) << ((data_ & 0xFF00) >> 8) << "\t" << (data_ & 0xFF) << std::endl;
	ss << "        \t        " << std::endl;
	ss << "        \t        " << std::endl;
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSReplyPacket::ConvertToDataPacket() const
{
	auto output = DTC_DMAPacket::ConvertToDataPacket();
	output.SetWord(4, static_cast<uint8_t>(type_));
	output.SetWord(5, requestCounter_);
	output.SetWord(6, static_cast<uint8_t>(address_));
	output.SetWord(7, output.GetWord(7) & (dcsReceiveFIFOEmpty_ ? 0xFF : 0xF7));
	output.SetWord(10, static_cast<uint8_t>(data_ & 0xFF));
	output.SetWord(11, static_cast<uint8_t>((data_ & 0xFF00) >> 8));
	return output;
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t packetCount, DTC_DataStatus status, uint8_t dtcid, 
	uint8_t packetVersion, DTC_Timestamp timestamp, uint8_t evbMode)
	: DTC_DMAPacket(DTC_PacketType_DataHeader, ring, roc, (1 + packetCount) * 16)
	, packetCount_(packetCount)
	, timestamp_(timestamp)
	, status_(status)
	, dataPacketVersion_(packetVersion)
	, dtcId_(dtcid)
	, evbMode_(evbMode)
{}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_DataHeader)
	{
		throw DTC_WrongPacketTypeException(DTC_PacketType_DataHeader, packetType_);
	}
	auto arr = in.GetData();
	packetCount_ = arr[4] + (arr[5] << 8);
	timestamp_ = DTC_Timestamp(arr, 6);
	status_ = DTC_DataStatus(arr[12]);
	dataPacketVersion_ = arr[13];
	dtcId_ = DTC_ID(arr[14]);
	evbMode_ = arr[15];
}

std::string DTCLib::DTC_DataHeaderPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DataHeaderPacket\": {";
	ss << headerJSON() << ",";
	ss << "\"packetCount\": " << std::dec << static_cast<int>(packetCount_) << ",";
	ss << timestamp_.toJSON() << ",";
	ss << "\"status\": " << std::dec << static_cast<int>(status_) << ",";
	ss << "\"packetVersion\": " << std::hex << static_cast<int>(dataPacketVersion_) << ",";
	ss << "\"DTC ID\": " << std::dec << static_cast<int>(dtcId_.GetID()) << ",";
	ss << "\"Subsystem\": " << std::dec << static_cast<int>(dtcId_.GetSubsystem()) << ",";
	ss << "\"evbMode\": " << std::hex << "0x" << static_cast<int>(evbMode_) << "}";
	return ss.str();
}

std::string DTCLib::DTC_DataHeaderPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << "     0x" << std::setw(1) << ((packetCount_ & 0x0700) >> 8) << "\t" << "0x" << std::setw(6) << (packetCount_ & 0xFF) << "\n";
	ss << timestamp_.toPacketFormat();
	ss << "0x" << std::setw(6) << static_cast<int>(dataPacketVersion_) << "\t" << "0x" << std::setw(6) << static_cast<int>(status_) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(evbMode_) << "\t" << std::dec << std::setw(2) << static_cast<int>(dtcId_.GetSubsystem()) << std::setw(6) << static_cast<int>(dtcId_.GetID()) << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataHeaderPacket::ConvertToDataPacket() const
{
	auto output = DTC_DMAPacket::ConvertToDataPacket();
	output.SetWord(4, static_cast<uint8_t>(packetCount_));
	output.SetWord(5, static_cast<uint8_t>((packetCount_ & 0x0700) >> 8));
	timestamp_.GetTimestamp(output.GetData(), 6);
	output.SetWord(12, static_cast<uint8_t>(status_));
	output.SetWord(13, static_cast<uint8_t>(dataPacketVersion_));
	output.SetWord(14, static_cast<uint8_t>(dtcId_.GetWord()));
	output.SetWord(15, evbMode_);
	return output;
}

bool DTCLib::DTC_DataHeaderPacket::Equals(const DTC_DataHeaderPacket& other) const
{
	return ConvertToDataPacket() == other.ConvertToDataPacket();
}
