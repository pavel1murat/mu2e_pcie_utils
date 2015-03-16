#include "DTC_Types.h"
#include <sstream>
#include <cstring>

DTCLib::DTC_Timestamp::DTC_Timestamp()
    : timestamp_(0) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint64_t timestamp)
    : timestamp_(timestamp) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    SetTimestamp(timestampLow, timestampHigh);
}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint8_t *timeArr)
{
    timestamp_ = 0;
    for (int i = 0; i < 6; ++i)
    {
        uint64_t temp = (uint64_t)timeArr[i] << i * 8;
        timestamp_ += temp;
    }
}

DTCLib::DTC_Timestamp::DTC_Timestamp(std::bitset<48> timestamp)
    : timestamp_(timestamp.to_ullong()) {}


void DTCLib::DTC_Timestamp::SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    timestamp_ = timestampLow + timestampHigh * 0x10000;
}

void DTCLib::DTC_Timestamp::GetTimestamp(uint8_t* timeArr, int offset) const
{
    for (int i = 0; i < 6; i++)
    {
        timeArr[i + offset] = static_cast<uint8_t>(timestamp_ >> i * 8);
    }
}

DTCLib::DTC_DataPacket::DTC_DataPacket() 
{
    memPacket_ = false;
    dataPtr_ = new uint8_t[16];
}

DTCLib::DTC_DataPacket::~DTC_DataPacket()
{
    if (!memPacket_) {
        delete[] dataPtr_;
    }
}

void DTCLib::DTC_DataPacket::SetWord(int index, uint8_t data)
{
    if (!memPacket_) {
        dataPtr_[index] = data;
    }
}

uint8_t DTCLib::DTC_DataPacket::GetWord(int index) const
{
    return dataPtr_[index];
}

std::string DTCLib::DTC_DataPacket::toJSON()
{
    std::stringstream ss;
    ss << "DataPacket: {";
    ss << "data: [" << (int)dataPtr_[0] << ",";
    ss << (int)dataPtr_[1] << ",";
    ss << (int)dataPtr_[2] << ",";
    ss << (int)dataPtr_[3] << ",";
    ss << (int)dataPtr_[4] << ",";
    ss << (int)dataPtr_[5] << ",";
    ss << (int)dataPtr_[6] << ",";
    ss << (int)dataPtr_[7] << ",";
    ss << (int)dataPtr_[8] << ",";
    ss << (int)dataPtr_[9] << ",";
    ss << (int)dataPtr_[10] << ",";
    ss << (int)dataPtr_[11] << ",";
    ss << (int)dataPtr_[12] << ",";
    ss << (int)dataPtr_[13] << ",";
    ss << (int)dataPtr_[14] << ",";
    ss << (int)dataPtr_[15] << "],";
    ss << "}";
    return ss.str();
}


DTCLib::DTC_DMAPacket::DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t byteCount, bool valid)
    : valid_(valid), byteCount_(byteCount), ringID_(ring), packetType_(type), rocID_(roc) {}


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
    for (int i = 0; i < 12; ++i)
    {
        output.SetWord(i + 4, 0);
    }
    return output;
}

DTCLib::DTC_DMAPacket::DTC_DMAPacket(const DTC_DataPacket in)
{
    uint8_t word0 = in.GetWord(0);
    uint8_t word1 = in.GetWord(1);
    std::bitset<16> byteCount = word0 + (word1 << 8);
    uint8_t word2 = in.GetWord(2);
    std::bitset<4> roc = word2;
    word2 >>= 4;
    std::bitset<4> packetType = word2;
    uint8_t word3 = in.GetWord(3);
    std::bitset<4> ringID = word3;
    valid_ = (word3 & 0x80) == 0x80;

    byteCount_ = static_cast<uint16_t>(byteCount.to_ulong());
    ringID_ = static_cast<DTC_Ring_ID>(ringID.to_ulong());
    rocID_ = static_cast<DTC_ROC_ID>(roc.to_ulong());
    packetType_ = static_cast<DTC_PacketType>(packetType.to_ulong());
}

std::string DTCLib::DTC_DMAPacket::headerJSON()
{
    std::stringstream ss;
    ss << "isValid: " << valid_ << ",";
    ss << "byteCount: " << byteCount_ << ",";
    ss << "ringID: " << ringID_ << ",";
    ss << "packetType: " << packetType_ << ",";
    ss << "rocID: " << rocID_ << ",";
    return ss.str();
}

std::string DTCLib::DTC_DMAPacket::toJSON()
{
    std::stringstream ss;
    ss << "DMAPacket: {";
    ss << headerJSON();
    ss << "}";
    return ss.str();
}

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
        data_[i] = in.GetWord(i + 4);
    }
}

std::string DTCLib::DTC_DCSRequestPacket::toJSON()
{
    std::stringstream ss;
    ss << "DCSRequestPacket: {";
    ss << headerJSON();
    ss << "data: [" << (int)data_[0] << ",";
    ss << (int)data_[1] << ",";
    ss << (int)data_[2] << ",";
    ss << (int)data_[3] << ",";
    ss << (int)data_[4] << ",";
    ss << (int)data_[5] << ",";
    ss << (int)data_[6] << ",";
    ss << (int)data_[7] << ",";
    ss << (int)data_[8] << ",";
    ss << (int)data_[9] << ",";
    ss << (int)data_[10] << ",";
    ss << (int)data_[11] << "],";
    ss << "}";
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
    : DTC_DMAPacket(DTC_PacketType_ReadoutRequest, ring, maxROC), timestamp_(), debug_(debug) {}

DTCLib::DTC_ReadoutRequestPacket::DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC, bool debug)
    : DTC_DMAPacket(DTC_PacketType_ReadoutRequest, ring, maxROC), timestamp_(timestamp), debug_(debug) {}

DTCLib::DTC_ReadoutRequestPacket::DTC_ReadoutRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    if (packetType_ != DTC_PacketType_ReadoutRequest) { throw DTC_WrongPacketTypeException(); }
    uint8_t arr[6];
    for (int i = 0; i < 6; ++i)
    {
        arr[i] = in.GetWord(i + 6);
    }
    timestamp_ = DTC_Timestamp(arr);
    debug_ = (in.GetWord(12) & 0x1) == 0x1;
}

std::string DTCLib::DTC_ReadoutRequestPacket::toJSON()
{
    uint8_t ts[6];
    timestamp_.GetTimestamp(ts, 0);
    std::stringstream ss;
    ss << "ReadoutRequestPacket: {";
    ss << headerJSON();
    ss << "timestamp: [" << (int)ts[0] << ",";
    ss << (int)ts[1] << ",";
    ss << (int)ts[2] << ",";
    ss << (int)ts[3] << ",";
    ss << (int)ts[4] << ",";
    ss << (int)ts[5] << "],";
    ss << "debug: " << (debug_?"true":"false") << ",";
    ss << "}";
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_ReadoutRequestPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
    timestamp_.GetTimestamp(output.GetData(), 6);
    output.SetWord(12, debug_ ? 1 : 0);
    return output;
}


DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, bool debug, uint16_t debugPacketCount)
    : DTC_DMAPacket(DTC_PacketType_DataRequest, ring, roc), timestamp_(), debug_(debug), debugPacketCount_(debugPacketCount) {}

DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp, bool debug, uint16_t debugPacketCount)
    : DTC_DMAPacket(DTC_PacketType_DataRequest, ring, roc), timestamp_(timestamp), debug_(debug), debugPacketCount_(debugPacketCount) {}

DTCLib::DTC_DataRequestPacket::DTC_DataRequestPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    if (packetType_ != DTC_PacketType_DataRequest) { throw DTC_WrongPacketTypeException(); }
    uint8_t arr[6];
    for (int i = 0; i < 6; ++i)
    {
        arr[i] = in.GetWord(i + 6);
    }
    timestamp_ = DTC_Timestamp(arr);
    debug_ = (in.GetWord(12) & 0x1) == 1;
    debugPacketCount_ = in.GetWord(14) + (in.GetWord(15) << 8);
}

std::string DTCLib::DTC_DataRequestPacket::toJSON()
{
    uint8_t ts[6];
    timestamp_.GetTimestamp(ts, 0);
    std::stringstream ss;
    ss << "DataRequestPacket: {";
    ss << headerJSON();
    ss << "timestamp: [" << (int)ts[0] << ",";
    ss << (int)ts[1] << ",";
    ss << (int)ts[2] << ",";
    ss << (int)ts[3] << ",";
    ss << (int)ts[4] << ",";
    ss << (int)ts[5] << "],";
    ss << "debug:" << (debug_?"true":"false") << ",";
    ss << "debugPacketCount: " << (int)debugPacketCount_ << ",";
    ss << "}";
    return ss.str();
}

DTCLib::DTC_DataPacket DTCLib::DTC_DataRequestPacket::ConvertToDataPacket() const
{
    DTC_DataPacket output = DTC_DMAPacket::ConvertToDataPacket();
    timestamp_.GetTimestamp(output.GetData(), 6);
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
    if (packetType_ != DTC_PacketType_DCSReply) { throw DTC_WrongPacketTypeException(); }
    for (int i = 0; i < 12; ++i)
    {
        data_[i] = in.GetWord(i + 4);
    }
}

std::string DTCLib::DTC_DCSReplyPacket::toJSON()
{
    std::stringstream ss;
    ss << "DCSReplyPacket: {";
    ss << headerJSON();
    ss << "data: [" << (int)data_[0] << ",";
    ss << (int)data_[1] << ",";
    ss << (int)data_[2] << ",";
    ss << (int)data_[3] << ",";
    ss << (int)data_[4] << ",";
    ss << (int)data_[5] << ",";
    ss << (int)data_[6] << ",";
    ss << (int)data_[7] << ",";
    ss << (int)data_[8] << ",";
    ss << (int)data_[9] << ",";
    ss << (int)data_[10] << ",";
    ss << (int)data_[11] << "],";
    ss << "}";
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

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_Data_Status status)
    : DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1+packetCount)*16), status_(status) {}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_Data_Status status, DTC_Timestamp timestamp)
    : DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), timestamp_(timestamp), status_(status) {}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_Data_Status status, DTC_Timestamp timestamp, uint8_t* data)
    : DTC_DMAPacket(DTC_PacketType_DataHeader, ring, DTC_ROC_Unused, (1 + packetCount) * 16), timestamp_(timestamp), status_(status)
{
    for (int i = 0; i < 3; ++i)
    {
        dataStart_[i] = data[i];
    }
}

DTCLib::DTC_DataHeaderPacket::DTC_DataHeaderPacket(DTC_DataPacket in) : DTC_DMAPacket(in)
{
    if (packetType_ != DTC_PacketType_DataHeader) { throw DTC_WrongPacketTypeException(); }
    packetCount_ = in.GetWord(4) + (in.GetWord(5) << 8);
    uint8_t arr[6];
    for (int i = 0; i < 6; ++i)
    {
        arr[i] = in.GetWord(i + 6);
    }
    timestamp_ = DTC_Timestamp(arr);
    status_ = (DTC_Data_Status)in.GetWord(12);
    for (int i = 0; i < 3; i++)
    {
        dataStart_[i] = in.GetWord(i + 13);
    }
}

std::string DTCLib::DTC_DataHeaderPacket::toJSON()
{
    uint8_t ts[6];
    timestamp_.GetTimestamp(ts, 0);
    std::stringstream ss;
    ss << "DataHeaderPacket: {";
    ss << headerJSON();
    ss << "packetCount: " << (int)packetCount_ << ",";
    ss << "timestamp: [" << (int)ts[0] << ",";
    ss << (int)ts[1] << ",";
    ss << (int)ts[2] << ",";
    ss << (int)ts[3] << ",";
    ss << (int)ts[4] << ",";
    ss << (int)ts[5] << "],";
    ss << "status: " << (int)status_ << ",";
    ss << "data: [" << (int)dataStart_[0] << ",";
    ss << (int)dataStart_[1] << ",";
    ss << (int)dataStart_[2] << ",";
    ss << (int)dataStart_[3] << "],";
    ss << "}";
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

DTCLib::DTC_ClockFanoutPacket::DTC_ClockFanoutPacket(uint8_t partition)
    : partition_(partition) {}

DTCLib::DTC_ClockFanoutPacket::DTC_ClockFanoutPacket(uint8_t partition, DTC_Timestamp timestamp)
    : partition_(partition), timestamp_(timestamp) {}

DTCLib::DTC_ClockFanoutPacket::DTC_ClockFanoutPacket(uint8_t partition, DTC_Timestamp timestamp, uint8_t* data)
    : partition_(partition), timestamp_(timestamp)
{
    for (int i = 0; i < 4; ++i)
    {
        dataStart_[i] = data[i];
    }
}

DTCLib::DTC_ClockFanoutPacket::DTC_ClockFanoutPacket(DTC_DataPacket in) :
partition_(in.GetWord(3) & 0x0F)
{
    uint8_t timestampProto[6];
    for (int i = 0; i < 6; i++)
    {
        timestampProto[i] = in.GetWord(6 + i);
    }
    timestamp_ = DTC_Timestamp(timestampProto);
    for (int i = 0; i < 4; i++)
    {
        dataStart_[i] = in.GetWord(i + 12);
    }
}


DTCLib::DTC_DataPacket DTCLib::DTC_ClockFanoutPacket::ConvertToDataPacket() const
{
    uint8_t data[16];
    data[0] = static_cast<uint8_t>(byteCount_);
    data[1] = static_cast<uint8_t>(byteCount_ >> 8);
    data[2] = 0x00;
    data[3] = (valid_ ? 0x80 : 0x00) + (partition_ & 0xF);
    timestamp_.GetTimestamp(data, 6);
    for (int ii = 0; ii < 4; ++ii)
    {
        data[ii + 12] = dataStart_[ii];
    }
    return DTC_DataPacket(data);
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
    stream << "E: " << Engine << ", D: " << Direction;
    stream << ", BDs: " << BDs << ", Buffers: " << Buffers;
    stream << ", PktSize: (" << MinPktSize << "," << MaxPktSize << "), ";
    stream << "BDerrs: " << BDerrs << ", BDSerrs: " << BDSerrs;
    stream << ", IntEnab: " << IntEnab << ", TestMode: " << TestMode.toString() << std::endl;
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
    stream << "E: " << Engine << ", D: " << Direction;
    stream << ", LBR: " << LBR << ", LAT: " << LAT;
    stream << ", LWT: " << LWT << std::endl;
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

    stream << "Version: " << Version << std::endl;
    stream << "LinkState: " << LinkState << std::endl;
    stream << "LinkSpeed: " << LinkSpeed << std::endl;              //* Link Speed */
    stream << "LinkWidth: " << LinkWidth << std::endl;              //* Link Width */
    stream << "VendorId: " << VendorId << std::endl;     //* Vendor ID */
    stream << "DeviceId: " << DeviceId << std::endl;    //* Device ID */
    stream << "IntMode: " << IntMode << std::endl;                //* Legacy or MSI interrupts */
    stream << "MPS: " << MPS << std::endl;                    //* Max Payload Size */
    stream << "MRRS: " << MRRS << std::endl;                   //* Max Read Request Size */
    stream << "InitFCCplD: " << InitFCCplD << std::endl;             //* Initial FC Credits for Completion Data */
    stream << "InitFCCplH: " << InitFCCplH << std::endl;             //* Initial FC Credits for Completion Header */
    stream << "InitFCNPD: " << InitFCNPD << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "InitFCNPH: " << InitFCNPH << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "InitFCPD: " << InitFCPD << std::endl;               //* Initial FC Credits for Posted Data */
    stream << "InitFCPH: " << InitFCPH << std::endl;               //* Initial FC Credits for Posted Data */

    return stream.str();
}

DTCLib::DTC_PCIeStat::DTC_PCIeStat(TRNStatistics in)
    : LTX(in.LTX), LRX(in.LRX) {}
