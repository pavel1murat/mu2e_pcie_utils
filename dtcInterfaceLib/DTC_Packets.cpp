#include "DTC_Packets.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#ifndef _WIN32
# include "trace.h"
#endif

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
		memcpy(dataPtr_, in.GetData(), in.GetSize() * sizeof(uint8_t));
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
		dataPtr_[index] = data;
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
		ss << "0x" << std::setw(2) << (int)dataPtr_[ii] << ",";
	}
	ss << "0x" << std::setw(2) << (int)dataPtr_[dataSize_ - 1] << "]";
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DataPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	for (uint16_t ii = 0; ii < dataSize_ - 1; ii += 2)
	{
		ss << "0x" << std::setw(6) << (int)dataPtr_[ii + 1] << "\t";
		ss << "0x" << std::setw(6) << (int)dataPtr_[ii] << "\n";
	}
	return ss.str();
}

bool DTCLib::DTC_DataPacket::Equals(const DTC_DataPacket& other)
{
	bool equal = true;
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
	uint8_t word0A = static_cast<uint8_t>(byteCount_);
	uint8_t word0B = static_cast<uint8_t>(byteCount_ >> 8);
	output.SetWord(0, word0A);
	output.SetWord(1, word0B);
	uint8_t word1A = (uint8_t)rocID_;
	word1A += (uint8_t)packetType_ << 4;
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
	uint8_t word2 = in.GetData()[2];
	uint8_t roc = word2 & 0xF;
	uint8_t packetType = word2 >> 4;
	uint8_t word3 = in.GetData()[3];
	uint8_t ringID = word3 & 0xF;
	valid_ = (word3 & 0x80) == 0x80;

	byteCount_ = in.GetData()[0] + (in.GetData()[1] << 8);
	ringID_ = static_cast<DTC_Ring_ID>(ringID);
	rocID_ = static_cast<DTC_ROC_ID>(roc);
	packetType_ = static_cast<DTC_PacketType>(packetType);
	TRACE(20, headerJSON().c_str());
}

std::string DTCLib::DTC_DMAPacket::headerJSON()
{
	std::stringstream ss;
	ss << "\"isValid\": " << valid_ << ",";
	ss << "\"byteCount\": " << std::hex << "0x" << byteCount_ << ",";
	ss << "\"ringID\": " << std::dec << ringID_ << ",";
	ss << "\"packetType\": " << packetType_ << ",";
	ss << "\"rocID\": " << rocID_;
	return ss.str();
}

std::string DTCLib::DTC_DMAPacket::headerPacketFormat()
{
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	ss << "0x" << std::setw(6) << ((byteCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (byteCount_ & 0xFF) << std::endl;
	ss << std::setw(1) << (int)valid_ << "   " << "0x" << std::setw(2) << ringID_ << "\t";
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
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, DTC_Ring_Unused, DTC_ROC_Unused) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc)
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, ring, roc) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_DCSOperationType type, uint8_t address, uint16_t data)
	: DTC_DMAPacket(DTC_PacketType_DCSRequest, ring, roc)
	, type_(type)
	, address_(address & 0x1F)
	, data_(data) {}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_DCSRequest)
	{
		throw DTC_WrongPacketTypeException();
	}
	type_ = (DTC_DCSOperationType)in.GetData()[4];
	address_ = in.GetData()[6] & 0x1F;
	data_ = in.GetData()[10] + (in.GetData()[11] << 8);
}

std::string DTCLib::DTC_DCSRequestPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DCSRequestPacket\": {";
	ss << headerJSON() << ", ";
	ss << "\"Operation Type\":" << DTC_DCSOperationTypeConverter(type_) << ", ";
	ss << "\"Address\": " << (int)address_ << ", ";
	ss << "\"Data\": " << (int)data_ << ", ";
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DCSRequestPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::hex << std::setfill('0');
	ss << "        \t" << std::setw(8) << (int)type_ << std::endl;
	ss << "        \t    " << std::setw(4) << (int)address_ << std::endl;
	ss << "        \t        " << std::endl;
	ss << std::setw(8) << ((data_ & 0xFF00) >> 8) << "\t" << (data_ & 0xFF) << std::endl;
	ss << "        \t        " << std::endl;
	ss << "        \t        " << std::endl;
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSRequestPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
	output.SetWord(4, (uint8_t)type_);
	output.SetWord(6, (uint8_t)address_);
	output.SetWord(10, static_cast<uint8_t>(data_ & 0xFF));
	output.SetWord(11, static_cast<uint8_t>(((data_ & 0xFF00) >> 8)));
	return output;
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC, bool debug)
	: DTC_DMAPacket(DTC_PacketType_Heartbeat, ring, maxROC), timestamp_(), debug_(debug), request_()
{
	request_[0] = 0;
	request_[1] = 0;
	request_[2] = 0;
	request_[3] = 0;
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC, bool debug, uint8_t* request)
	: DTC_DMAPacket(DTC_PacketType_Heartbeat, ring, maxROC), timestamp_(timestamp), debug_(debug), request_()
{
	if (request != nullptr)
	{
		for (int i = 0; i < 4; ++i)
		{
			request_[i] = request[i];
		}
	}
}

DTCLib::DTC_HeartbeatPacket::DTC_HeartbeatPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_Heartbeat)
	{
		throw DTC_WrongPacketTypeException();
	}
	uint8_t* arr = in.GetData();
	request_[0] = arr[4];
	request_[1] = arr[5];
	request_[2] = arr[14];
	request_[3] = arr[15];
	timestamp_ = DTC_Timestamp(arr, 6);
	debug_ = (arr[12] & 0x1) == 0x1;
}

std::string DTCLib::DTC_HeartbeatPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"ReadoutRequestPacket\": {";
	ss << headerJSON() << ",";
	ss << timestamp_.toJSON() << ",";
	ss << "\"request\": [" << std::hex << "0x" << (int)request_[0] << ",";
	ss << std::hex << "0x" << (int)request_[1] << ",";
	ss << std::hex << "0x" << (int)request_[2] << ",";
	ss << std::hex << "0x" << (int)request_[3] << "],";
	ss << "\"debug\": " << (debug_ ? "true" : "false");
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_HeartbeatPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << "0x" << std::setw(6) << (int)request_[1] << "\t0x" << std::setw(6) << (int)request_[0] << "\n";
	ss << timestamp_.toPacketFormat();
	ss << "        \t       " << std::setw(1) << (int)debug_ << "\n";
	ss << "0x" << std::setw(6) << (int)request_[3] << "\t0x" << std::setw(6) << (int)request_[2] << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_HeartbeatPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
	timestamp_.GetTimestamp(output.GetData(), 6);
	output.SetWord(12, debug_ ? 1 : 0);
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
		throw DTC_WrongPacketTypeException();
	}
	timestamp_ = DTC_Timestamp(in.GetData(), 6);
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
	ss << "\"debugPacketCount\": " << std::dec << (int)debugPacketCount_ << ",";
	ss << "\"debugType\": " << DTC_DebugTypeConverter(type_);
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DataRequestPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << "        \t        \n";
	ss << timestamp_.toPacketFormat();
	ss << "        \t0x" << std::setw(2) << (int)type_ << "   " << std::setw(1) << (int)debug_ << "\n";
	ss << "0x" << std::setw(6) << ((debugPacketCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (debugPacketCount_ & 0xFF) << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataRequestPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
	timestamp_.GetTimestamp(output.GetData(), 6);
	output.SetWord(12, ((uint8_t)type_ << 4) + (debug_ ? 1 : 0));
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
	: DTC_DMAPacket(DTC_PacketType_DCSReply, ring, DTC_ROC_Unused) {}

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
		throw DTC_WrongPacketTypeException();
	}

	type_ = (DTC_DCSOperationType)in.GetData()[4];
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
	ss << "\"Address\": " << (int)address_ << ",";
	ss << "\"Data\": " << (int)data_;
	ss << "\"Request Counter\": " << (int)requestCounter_ << ",";
	ss << "\"DCS Request FIFO Empty\": " << (dcsReceiveFIFOEmpty_ ? "\"true\"" : "\"false\"");
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DCSReplyPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::hex << std::setfill('0');
	ss << std::setw(8) << (int)requestCounter_ << "\t" << std::setw(8) << (int)type_ << std::endl;
	ss << (dcsReceiveFIFOEmpty_ ? "    E   " : "        ") << "\t    " << std::setw(4) << (int)address_ << std::endl;
	ss << "        \t        " << std::endl;
	ss << std::setw(8) << ((data_ & 0xFF00) >> 8) << "\t" << (data_ & 0xFF) << std::endl;
	ss << "        \t        " << std::endl;
	ss << "        \t        " << std::endl;
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSReplyPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
	output.SetWord(4, (uint8_t)type_);
	output.SetWord(5, requestCounter_);
	output.SetWord(6, (uint8_t)address_);
	output.SetWord(7, output.GetWord(7) & (dcsReceiveFIFOEmpty_ ? 0xFF : 0xF7));
	output.SetWord(10, static_cast<uint8_t>(data_ & 0xFF));
	output.SetWord(11, static_cast<uint8_t>((data_ & 0xFF00) >> 8));
	return output;
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status)
	: DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), packetCount_(packetCount), timestamp_(), status_(status) {}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp)
	: DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), packetCount_(packetCount), timestamp_(timestamp), status_(status)
{
	for (int i = 0; i < 3; ++i)
	{
		dataStart_[i] = 0;
	}
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp, uint8_t* data)
	: DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), packetCount_(packetCount), timestamp_(timestamp), status_(status)
{
	for (int i = 0; i < 3; ++i)
	{
		dataStart_[i] = data[i];
	}
	//delete[] data;
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
	if (packetType_ != DTC_PacketType_DataHeader)
	{
		throw DTC_WrongPacketTypeException();
	}
	uint8_t* arr = in.GetData();
	packetCount_ = arr[4] + (arr[5] << 8);
	timestamp_ = DTC_Timestamp(arr, 6);
	status_ = (DTC_DataStatus)arr[12];
	for (int i = 0; i < 3; i++)
	{
		dataStart_[i] = arr[i + 13];
	}
}

std::string DTCLib::DTC_DataHeaderPacket::toJSON()
{
	std::stringstream ss;
	ss << "\"DataHeaderPacket\": {";
	ss << headerJSON() << ",";
	ss << "\"packetCount\": " << std::dec << (int)packetCount_ << ",";
	ss << timestamp_.toJSON() << ",";
	ss << "\"status\": " << std::dec << (int)status_ << ",";
	ss << "\"data\": [" << std::hex << "0x" << (int)dataStart_[0] << ",";
	ss << std::hex << "0x" << (int)dataStart_[1] << ",";
	ss << std::hex << "0x" << (int)dataStart_[2] << "]";
	ss << "}";
	return ss.str();
}

std::string DTCLib::DTC_DataHeaderPacket::toPacketFormat()
{
	std::stringstream ss;
	ss << headerPacketFormat() << std::setfill('0') << std::hex;
	ss << "     0x" << std::setw(1) << ((packetCount_ & 0x0700) >> 8) << "\t" << "0x" << std::setw(6) << (packetCount_ & 0xFF) << "\n";
	ss << timestamp_.toPacketFormat();
	ss << "0x" << std::setw(6) << (int)dataStart_[0] << "\t" << "0x" << std::setw(6) << (int)status_ << "\n";
	ss << "0x" << std::setw(6) << (int)dataStart_[2] << "\t" << "0x" << std::setw(6) << (int)dataStart_[1] << "\n";
	return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataHeaderPacket::ConvertToDataPacket() const
{
	DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
        output.SetWord(4, static_cast<uint8_t>(packetCount_));
        output.SetWord(5, static_cast<uint8_t>((packetCount_ & 0x0700) >> 8));
	timestamp_.GetTimestamp(output.GetData(), 6);
	output.SetWord(12, (uint8_t)status_);
	for (uint16_t i = 0; i < 3; ++i)
	{
		output.SetWord(i + 13, dataStart_[i]);
	}
	return output;
}

bool DTCLib::DTC_DataHeaderPacket::Equals(const DTC_DataHeaderPacket& other)
{
	return ConvertToDataPacket() == other.ConvertToDataPacket();
}

