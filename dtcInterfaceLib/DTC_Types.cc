#include "DTC_Types.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#ifndef _WIN32
# include "trace.h"
#endif


DTCLib::DTC_SimMode DTCLib::DTC_SimModeConverter::ConvertToSimMode(std::string modeName) {
    if (modeName.find("racker") != std::string::npos) { return DTC_SimMode_Tracker; }
    if (modeName.find("alorimeter") != std::string::npos) { return DTC_SimMode_Calorimeter; }
    if (modeName.find("osmic") != std::string::npos) { return DTC_SimMode_CosmicVeto; }
    if (modeName.find("ardware") != std::string::npos) { return DTC_SimMode_Hardware; }
    if (modeName.find("erformance") != std::string::npos) { return DTC_SimMode_Performance; }

    return DTC_SimMode_Disabled;
}

DTCLib::DTC_Timestamp::DTC_Timestamp()
    : timestamp_(0) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint64_t timestamp)
    : timestamp_(timestamp) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    SetTimestamp(timestampLow, timestampHigh);
}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint8_t *timeArr, int offset)
{
    uint64_t* arr = (uint64_t*)(timeArr + offset);
    timestamp_ = *arr;
}

DTCLib::DTC_Timestamp::DTC_Timestamp(std::bitset<48> timestamp)
    : timestamp_(timestamp.to_ullong()) {}

void DTCLib::DTC_Timestamp::SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    timestamp_ = timestampLow + ((uint64_t)timestampHigh << 32);
}

void DTCLib::DTC_Timestamp::GetTimestamp(uint8_t* timeArr, int offset) const
{
    for (int i = 0; i < 6; i++)
    {
        timeArr[i + offset] = static_cast<uint8_t>(timestamp_ >> i * 8);
    }
}

std::string DTCLib::DTC_Timestamp::toJSON(bool arrayMode)
{
    std::stringstream ss;
    if (arrayMode) {
        uint8_t ts[6];
        GetTimestamp(ts, 0);
        ss << "\"timestamp\": [" << (int)ts[0] << ",";
        ss << (int)ts[1] << ",";
        ss << (int)ts[2] << ",";
        ss << (int)ts[3] << ",";
        ss << (int)ts[4] << ",";
        ss << (int)ts[5] << "]";
    }
    else {
        ss << "\"timestamp\": " << timestamp_;
    }
    return ss.str();
}

std::string DTCLib::DTC_Timestamp::toPacketFormat()
{
    uint8_t ts[6];
    GetTimestamp(ts, 0);
    std::stringstream ss;
    ss << std::setfill('0') << std::hex;
    ss << "0x" << std::setw(6) << (int)ts[1] << "\t" << "0x" << std::setw(6) << (int)ts[0] << "\n";
    ss << "0x" << std::setw(6) << (int)ts[3] << "\t" << "0x" << std::setw(6) << (int)ts[2] << "\n";
    ss << "0x" << std::setw(6) << (int)ts[5] << "\t" << "0x" << std::setw(6) << (int)ts[4] << "\n";
    return ss.str();
}

DTCLib::DTC_DataPacket::DTC_DataPacket()
{
    memPacket_ = false;
    dataPtr_ = new uint8_t[16]; // current min. dma length is 64 bytes
    dataSize_ = 16;
}

DTCLib::DTC_DataPacket::DTC_DataPacket(const DTC_DataPacket& in)
{
    dataSize_ = in.GetSize();
    memPacket_ = in.IsMemoryPacket();
    if (!memPacket_)
    {
        dataPtr_ = new uint8_t[dataSize_];
        memcpy(dataPtr_, in.GetData(), in.GetSize() * sizeof(uint8_t));
    }
    else
    {
        dataPtr_ = in.GetData();
    }
}

DTCLib::DTC_DataPacket::~DTC_DataPacket()
{
    if (!memPacket_ && dataPtr_ != nullptr) {
        delete[] dataPtr_;
        dataPtr_ = nullptr;
    }
}

void DTCLib::DTC_DataPacket::SetWord(uint16_t index, uint8_t data)
{
    if (!memPacket_ && index < dataSize_) {
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
    if (!memPacket_ && dmaSize > dataSize_) {
        uint8_t *data = new uint8_t[dmaSize];
        memset(data, 0, dmaSize * sizeof(uint8_t));
        memcpy(data, dataPtr_, dataSize_ * sizeof(uint8_t));
        delete[] dataPtr_;
        dataPtr_ = data;
        dataSize_ = dmaSize;
        return true;
    }

    //We can only grow, and only non-memory-mapped packets
    return false;
}

std::string DTCLib::DTC_DataPacket::toJSON()
{
    std::stringstream ss;
    ss << "\"DataPacket\": {";
    ss << "\"data\": [";
    ss << std::hex << std::setfill('0');
    for (uint16_t ii = 0; ii < dataSize_ - 1; ++ii) {
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


DTCLib::DTC_DMAPacket::DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t byteCount, bool valid)
    : valid_(valid), byteCount_(byteCount < 64 ? 64 : byteCount), ringID_(ring), packetType_(type), rocID_(roc) {}


DTCLib::DTC_DataPacket DTCLib::DTC_DMAPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output;
    uint8_t word0A = static_cast<uint8_t>(byteCount_);
    uint8_t word0B = static_cast<uint8_t>(byteCount_ >> 8);
    output.SetWord(0, word0A);
    output.SetWord(1, word0B);
    uint8_t word1A = (uint8_t)rocID_;
    word1A += (uint8_t)packetType_ << 4;
    uint8_t word1B = static_cast<uint8_t>(ringID_)+(valid_ ? 0x80 : 0x0);
    output.SetWord(2, word1A);
    output.SetWord(3, word1B);
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
    ss << "0x" << std::setw(6) << ((byteCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (byteCount_ & 0xFF) << "\n";
    ss << std::setw(1) << (int)valid_ << "   " << "0x" << std::setw(2) << ringID_ << "\t";
    ss << "0x" << std::setw(2) << packetType_ << "0x" << std::setw(2) << rocID_ << "\n";
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

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t* data)
    : DTC_DMAPacket(DTC_PacketType_DCSRequest, ring, roc)
{
    for (int i = 0; i < 12; ++i)
    {
        data_[i] = data[i];
    }
}

DTCLib::DTC_DCSRequestPacket::DTC_DCSRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    if (packetType_ != DTC_PacketType_DCSRequest) { throw DTC_WrongPacketTypeException(); }
    for (int i = 0; i < 12; ++i)
    {
        data_[i] = in.GetData()[i + 4];
    }
}

std::string DTCLib::DTC_DCSRequestPacket::toJSON()
{
    std::stringstream ss;
    ss << "\"DCSRequestPacket\": {";
    ss << headerJSON() << ",";
    ss << "\"data\": [" << std::hex << "0x" << (int)data_[0] << ",";
    ss << std::hex << "0x" << (int)data_[1] << ",";
    ss << std::hex << "0x" << (int)data_[2] << ",";
    ss << std::hex << "0x" << (int)data_[3] << ",";
    ss << std::hex << "0x" << (int)data_[4] << ",";
    ss << std::hex << "0x" << (int)data_[5] << ",";
    ss << std::hex << "0x" << (int)data_[6] << ",";
    ss << std::hex << "0x" << (int)data_[7] << ",";
    ss << std::hex << "0x" << (int)data_[8] << ",";
    ss << std::hex << "0x" << (int)data_[9] << ",";
    ss << std::hex << "0x" << (int)data_[10] << ",";
    ss << std::hex << "0x" << (int)data_[11] << "]";
    ss << "}";
    return ss.str();
}

std::string DTCLib::DTC_DCSRequestPacket::toPacketFormat()
{
    std::stringstream ss;
    ss << headerPacketFormat() << std::hex << std::setfill('0');
    for (int ii = 0; ii <= 10; ii += 2)
    {
        ss << "0x" << std::setw(6) << (int)data_[ii + 1] << "\t";
        ss << "0x" << std::setw(6) << (int)data_[ii] << "\n";
    }
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSRequestPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
    for (int i = 0; i < 12; ++i)
    {
        output.SetWord(i + 4, data_[i]);
    }
    return output;
}

DTCLib::DTC_ReadoutRequestPacket::DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC, bool debug)
    : DTC_DMAPacket(DTC_PacketType_ReadoutRequest, ring, maxROC), timestamp_(), debug_(debug), request_() {
    request_[0] = 0;
    request_[1] = 0;
    request_[2] = 0;
    request_[3] = 0;
}

DTCLib::DTC_ReadoutRequestPacket::DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC, bool debug, uint8_t* request)
    : DTC_DMAPacket(DTC_PacketType_ReadoutRequest, ring, maxROC), timestamp_(timestamp), debug_(debug), request_()
{
    if (request != nullptr) {
        for (int i = 0; i < 4; ++i)
        {
            request_[i] = request[i];
        }
    }
}

DTCLib::DTC_ReadoutRequestPacket::DTC_ReadoutRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    if (packetType_ != DTC_PacketType_ReadoutRequest) { throw DTC_WrongPacketTypeException(); }
    uint8_t* arr = in.GetData();
    request_[0] = arr[4];
    request_[1] = arr[5];
    request_[2] = arr[14];
    request_[3] = arr[15];
    timestamp_ = DTC_Timestamp(arr, 6);
    debug_ = (arr[12] & 0x1) == 0x1;
}

std::string DTCLib::DTC_ReadoutRequestPacket::toJSON()
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

std::string DTCLib::DTC_ReadoutRequestPacket::toPacketFormat()
{
    std::stringstream ss;
    ss << headerPacketFormat() << std::setfill('0') << std::hex;
    ss << "0x" << std::setw(6) << (int)request_[1] << "\t0x" << std::setw(6) << (int)request_[0] << "\n";
    ss << timestamp_.toPacketFormat();
    ss << "        \t       " << std::setw(1) << (int)debug_ << "\n";
    ss << "0x" << std::setw(6) << (int)request_[3] << "\t0x" << std::setw(6) << (int)request_[2] << "\n";
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_ReadoutRequestPacket::ConvertToDataPacket() const
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
    if (packetType_ != DTC_PacketType_DataRequest) { throw DTC_WrongPacketTypeException(); }
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
    ss << "        \t0x" << std::setw(2) << (int)type_  << "   " << std::setw(1) << (int)debug_ << "\n";
    ss << "0x" << std::setw(6) << ((debugPacketCount_ & 0xFF00) >> 8) << "\t" << "0x" << std::setw(6) << (debugPacketCount_ & 0xFF) << "\n";
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataRequestPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
    timestamp_.GetTimestamp(output.GetData(), 6);
    output.SetWord(12, ((int)type_ << 4) + (debug_ ? 1 : 0));
    output.SetWord(14, debugPacketCount_ & 0xFF);
    output.SetWord(15, (debugPacketCount_ >> 8) & 0xFF);
    return output;
}

void DTCLib::DTC_DataRequestPacket::SetDebugPacketCount(uint16_t count)
{
    if (count > 0) { debug_ = true; }
    else { debug_ = false; }
    debugPacketCount_ = count;
}


DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_Ring_ID ring)
    : DTC_DMAPacket(DTC_PacketType_DCSReply, ring, DTC_ROC_Unused) {}

DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t* data)
    : DTC_DMAPacket(DTC_PacketType_DCSReply, ring, DTC_ROC_Unused)
{
    for (int i = 0; i < 12; ++i)
    {
        data_[i] = data[i];
    }
}

DTCLib::DTC_DCSReplyPacket::DTC_DCSReplyPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    TRACE(20, "DTC_DCSReplyPacket::DTC_DCSReplyPacket Before packetType test");
    if (packetType_ != DTC_PacketType_DCSReply) { throw DTC_WrongPacketTypeException(); }
    for (int i = 0; i < 12; ++i)
    {
        data_[i] = in.GetData()[i + 4];
    }
}

std::string DTCLib::DTC_DCSReplyPacket::toJSON()
{
    std::stringstream ss;
    ss << "\"DCSReplyPacket\": {";
    ss << headerJSON() << ",";
    ss << "\"data\": [" << std::hex << "0x" << (int)data_[0] << ",";
    ss << std::hex << "0x" << (int)data_[1] << ",";
    ss << std::hex << "0x" << (int)data_[2] << ",";
    ss << std::hex << "0x" << (int)data_[3] << ",";
    ss << std::hex << "0x" << (int)data_[4] << ",";
    ss << std::hex << "0x" << (int)data_[5] << ",";
    ss << std::hex << "0x" << (int)data_[6] << ",";
    ss << std::hex << "0x" << (int)data_[7] << ",";
    ss << std::hex << "0x" << (int)data_[8] << ",";
    ss << std::hex << "0x" << (int)data_[9] << ",";
    ss << std::hex << "0x" << (int)data_[10] << ",";
    ss << std::hex << "0x" << (int)data_[11] << "]";
    ss << "}";
    return ss.str();
}

std::string DTCLib::DTC_DCSReplyPacket::toPacketFormat()
{
    std::stringstream ss;
    ss << headerPacketFormat() << std::hex << std::setfill('0');
    for (int ii = 0; ii <= 10; ii += 2)
    {
        ss << "0x" << std::setw(6) << (int)data_[ii + 1] << "\t";
        ss << "0x" << std::setw(6) << (int)data_[ii] << "\n";
    }
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DCSReplyPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
    for (int i = 0; i < 12; ++i)
    {
        output.SetWord(i + 4, data_[i]);
    }
    return output;
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status)
    : DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), packetCount_(packetCount), timestamp_(), status_(status) {}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp)
    : DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), packetCount_(packetCount), timestamp_(timestamp), status_(status) 
{
  for(int i = 0; i < 3; ++i)
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
    timestamp_.GetTimestamp(output.GetData(), 6);
    output.SetWord(12, (uint8_t)status_);
    for (int i = 0; i < 3; ++i)
    {
        output.SetWord(i + 13, dataStart_[i]);
    }
    return output;
}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError() : data_(0) {}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError(std::bitset<2> data) : data_(data) {}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring)
{
    std::bitset<32> dataSet = data;
    uint32_t ringBase = (uint8_t)ring * 2;
    data_[0] = dataSet[ringBase];
    data_[1] = dataSet[ringBase + 1];
}


DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError() : data_(0) {}

DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError(std::bitset<2> data) : data_(data) {}

DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring)
{
    std::bitset<32> dataSet = data;
    uint32_t ringBase = (uint8_t)ring * 2;
    data_[0] = dataSet[ringBase];
    data_[1] = dataSet[ringBase + 1];
}

DTCLib::DTC_TestMode::DTC_TestMode() {}
DTCLib::DTC_TestMode::DTC_TestMode(bool state, bool loopback, bool txChecker, bool rxGenerator)
{
    txChecker = txChecker;
    loopbackEnabled = loopback;
    rxGenerator = rxGenerator;
    state_ = state;
}
DTCLib::DTC_TestMode::DTC_TestMode(uint32_t input)
{
    std::bitset<3> mode = (input & 0x700) >> 8;
    txChecker = mode[0];
    loopbackEnabled = mode[1];
    rxGenerator = mode[2];
    state_ = (input & 0xC000) != 0;
}
uint32_t DTCLib::DTC_TestMode::GetWord() const
{
    uint32_t output = 0;
    if (loopbackEnabled) {
        output += 0x200;
    }
    else
    {
        if (txChecker) {
            output += 0x100;
        }
        if (rxGenerator) {
            output += 0x400;
        }
    }
    if (state_)
    {
        output += 0x8000;
    }
    return output;
}
std::string DTCLib::DTC_TestMode::toString()
{
    std::string output = "";
    if (loopbackEnabled) {
        output += "L";
    }
    else
    {
        if (txChecker) {
            output += "T";
        }
        if (rxGenerator) {
            output += "R";
        }
    }
    if (state_)
    {
        output += "A";
    }
    return output;
}

DTCLib::DTC_TestCommand::DTC_TestCommand()
    : TestMode(), PacketSize(0), Engine(DTC_DMA_Engine_DAQ) {}
DTCLib::DTC_TestCommand::DTC_TestCommand(DTC_DMA_Engine dma, bool state,
    int PacketSize, bool loopback, bool txChecker, bool rxGenerator)
{
    Engine = dma;
    PacketSize = PacketSize;
    TestMode = DTC_TestMode(state, loopback, txChecker, rxGenerator);
}
DTCLib::DTC_TestCommand::DTC_TestCommand(m_ioc_cmd_t in)
{
    if (in.Engine == 0)
    {
        Engine = DTC_DMA_Engine_DAQ;
    }
    else if (in.Engine == 1)
    {
        Engine = DTC_DMA_Engine_DCS;
    }

    PacketSize = in.MinPktSize;
    TestMode = DTC_TestMode(in.TestMode);
}
m_ioc_cmd_t DTCLib::DTC_TestCommand::GetCommand() const
{
    m_ioc_cmd_t output;

    output.Engine = Engine;
    output.MinPktSize = output.MaxPktSize = PacketSize;
    output.TestMode = TestMode.GetWord();

    return output;
}

DTCLib::DTC_DMAState::DTC_DMAState(m_ioc_engstate_t in)
    : BDs(in.BDs), Buffers(in.Buffers), MinPktSize(in.MinPktSize),
    MaxPktSize(in.MaxPktSize), BDerrs(in.BDerrs), BDSerrs(in.BDSerrs),
    IntEnab(in.IntEnab), TestMode(in.TestMode)
{
    switch (in.Engine)
    {
    case 0:
        Direction = DTC_DMA_Direction_C2S;
        Engine = DTC_DMA_Engine_DAQ;
        break;
    case 0x20:
        Direction = DTC_DMA_Direction_S2C;
        Engine = DTC_DMA_Engine_DAQ;
        break;
    case 1:
        Direction = DTC_DMA_Direction_C2S;
        Engine = DTC_DMA_Engine_DCS;
        break;
    case 0x21:
        Direction = DTC_DMA_Direction_S2C;
        Engine = DTC_DMA_Engine_DCS;
        break;
    }
}
std::string DTCLib::DTC_DMAState::toString() {
    std::stringstream stream;
    stream << "{\"E\": " << Engine << ", \"D\": " << Direction;
    stream << ", \"BDs\": " << BDs << ", \"Buffers\": " << Buffers;
    stream << ", \"PktSize\": [" << MinPktSize << "," << MaxPktSize << "], ";
    stream << "\"BDerrs\": " << BDerrs << ", \"BDSerrs\": " << BDSerrs;
    stream << ", \"IntEnab\": " << IntEnab << ", \"TestMode\": " << TestMode.toString() << "}" << std::endl;
    return stream.str();
}

DTCLib::DTC_DMAStat::DTC_DMAStat(DMAStatistics in) : LBR(in.LBR), LAT(in.LAT), LWT(in.LWT)
{
    switch (in.Engine)
    {
    case 0:
        Direction = DTC_DMA_Direction_C2S;
        Engine = DTC_DMA_Engine_DAQ;
        break;
    case 0x20:
        Direction = DTC_DMA_Direction_S2C;
        Engine = DTC_DMA_Engine_DAQ;
        break;
    case 1:
        Direction = DTC_DMA_Direction_C2S;
        Engine = DTC_DMA_Engine_DCS;
        break;
    case 0x21:
        Direction = DTC_DMA_Direction_S2C;
        Engine = DTC_DMA_Engine_DCS;
        break;
    }
}
std::string DTCLib::DTC_DMAStat::toString()
{
    std::stringstream stream;
    stream << "{\"E\": " << Engine << ", \"D\": " << Direction;
    stream << ", \"LBR\": " << LBR << ", \"LAT\": " << LAT;
    stream << ", \"LWT\": " << LWT << "}" << std::endl;
    return stream.str();
}

DTCLib::DTC_DMAStats::DTC_DMAStats(m_ioc_engstats_t in)
{
    for (int i = 0; i < in.Count; ++i)
    {
        Stats.push_back(DTC_DMAStat(in.engptr[i]));
    }
}
DTCLib::DTC_DMAStats DTCLib::DTC_DMAStats::getData(DTC_DMA_Engine dma, DTC_DMA_Direction dir)
{
    DTC_DMAStats output;
    for (auto i : Stats)
    {
        if (i.Engine == dma && i.Direction == dir)
        {
            output.addStat(i);
        }
    }

    if (output.size() == 0) {
        output.addStat(DTC_DMAStat());
    }
    return output;
}

DTCLib::DTC_PCIeState::DTC_PCIeState(m_ioc_pcistate_t in)
    : Version(in.Version), LinkState(in.LinkState == 1), LinkSpeed(in.LinkSpeed),
    LinkWidth(in.LinkWidth), VendorId(in.VendorId), DeviceId(in.DeviceId),
    IntMode(in.IntMode), MPS(in.MPS), MRRS(in.MRRS), InitFCCplD(in.InitFCCplD),
    InitFCCplH(in.InitFCCplH), InitFCNPD(in.InitFCNPD), InitFCNPH(in.InitFCNPH),
    InitFCPD(in.InitFCPD), InitFCPH(in.InitFCPH) {}

std::string DTCLib::DTC_PCIeState::toString()
{
    std::stringstream stream;

    stream << "{" << std::endl;
    stream << "\t\"Version\": " << Version << "," << std::endl;
    stream << "\t\"LinkState\": " << LinkState << "," << std::endl;
    stream << "\t\"LinkSpeed\": " << LinkSpeed << "," << std::endl;              //* Link Speed */
    stream << "\t\"LinkWidth\": " << LinkWidth << "," << std::endl;              //* Link Width */
    stream << "\t\"VendorId\": " << VendorId << "," << std::endl;     //* Vendor ID */
    stream << "\t\"DeviceId\": " << DeviceId << "," << std::endl;    //* Device ID */
    stream << "\t\"IntMode\": " << IntMode << "," << std::endl;                //* Legacy or MSI interrupts */
    stream << "\t\"MPS\": " << MPS << "," << std::endl;                    //* Max Payload Size */
    stream << "\t\"MRRS\": " << MRRS << "," << std::endl;                   //* Max Read Request Size */
    stream << "\t\"InitFCCplD\": " << InitFCCplD << "," << std::endl;             //* Initial FC Credits for Completion Data */
    stream << "\t\"InitFCCplH\": " << InitFCCplH << "," << std::endl;             //* Initial FC Credits for Completion Header */
    stream << "\t\"InitFCNPD\": "  << InitFCNPD << "," << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "\t\"InitFCNPH\": "  << InitFCNPH << "," << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "\t\"InitFCPD\": "  << InitFCPD << "," << std::endl;               //* Initial FC Credits for Posted Data */
    stream << "\t\"InitFCPH\": "  << InitFCPH << "," << std::endl;               //* Initial FC Credits for Posted Data */
    stream << "}" << std::endl;

    return stream.str();
}

DTCLib::DTC_PCIeStat::DTC_PCIeStat(TRNStatistics in)
    : LTX(in.LTX), LRX(in.LRX) {}
