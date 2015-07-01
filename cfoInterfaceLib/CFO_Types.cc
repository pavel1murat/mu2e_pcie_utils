#include "CFO_Types.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#ifndef _WIN32
# include "trace.h"
#else
# ifndef TRACE
#  define TRACE(...)
# endif
#endif


CFOLib::CFO_SimMode CFOLib::CFO_SimModeConverter::ConvertToSimMode(std::string modeName) {
    if (modeName.find("nabled") != std::string::npos) { return CFO_SimMode_Enabled; }

    return CFO_SimMode_Disabled;
}


CFOLib::CFO_Timestamp::CFO_Timestamp()
    : timestamp_(0) {}

CFOLib::CFO_Timestamp::CFO_Timestamp(uint64_t timestamp)
    : timestamp_(timestamp) {}

CFOLib::CFO_Timestamp::CFO_Timestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    SetTimestamp(timestampLow, timestampHigh);
}

CFOLib::CFO_Timestamp::CFO_Timestamp(uint8_t *timeArr)
{
    timestamp_ = 0;
    for (int i = 0; i < 6; ++i)
    {
        uint64_t temp = (uint64_t)timeArr[i] << i * 8;
        timestamp_ += temp;
    }
}

CFOLib::CFO_Timestamp::CFO_Timestamp(std::bitset<48> timestamp)
    : timestamp_(timestamp.to_ullong()) {}


void CFOLib::CFO_Timestamp::SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
    timestamp_ = timestampLow + timestampHigh * 0x10000;
}

void CFOLib::CFO_Timestamp::GetTimestamp(uint8_t* timeArr, int offset) const
{
    for (int i = 0; i < 6; i++)
    {
        timeArr[i + offset] = static_cast<uint8_t>(timestamp_ >> i * 8);
    }
}

std::string CFOLib::CFO_Timestamp::toJSON(bool arrayMode)
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

std::string CFOLib::CFO_Timestamp::toPacketFormat()
{
    uint8_t ts[6];
    GetTimestamp(ts, 0);
    std::stringstream ss;
    ss << std::setfill('0') << std::hex;
    ss << "0x" << std::setw(6) << ts[1] << "\t" << "0x" << std::setw(6) << ts[0] << "\n";
    ss << "0x" << std::setw(6) << ts[3] << "\t" << "0x" << std::setw(6) << ts[2] << "\n";
    ss << "0x" << std::setw(6) << ts[5] << "\t" << "0x" << std::setw(6) << ts[4] << "\n";
    return ss.str();
}


CFOLib::CFO_ReadoutRequestPacket::CFO_ReadoutRequestPacket(CFO_Ring_ID ring, int hopCount, uint8_t* request, CFO_Timestamp timestamp, bool debug)
    : valid_(true), ring_(ring), hopCount_(hopCount), timestamp_(timestamp), debug_(debug)
{
    for (int i = 0; i < 4; ++i)
    {
        request_[i] = request[i];
    }
}

CFOLib::CFO_ReadoutRequestPacket::CFO_ReadoutRequestPacket(uint8_t* data)
{
    hopCount_ = data[2] & 0xF;
    packetType_ = (data[2] & 0xF0) >> 4;
    ring_ = (CFO_Ring_ID)(data[3] & 0xF);
    valid_ = ((data[3] & 0x80) == 1);
    if (packetType_ != 1) { throw CFO_WrongPacketTypeException(); }
    request_[0] = data[4];
    request_[1] = data[5];
    timestamp_ = CFO_Timestamp(&(data[6]));
    debug_ = ((data[12] & 0x1) == 1);
    request_[2] = data[14];
    request_[3] = data[15];
}


std::string CFOLib::CFO_ReadoutRequestPacket::toJSON()
{
    std::stringstream ss;
    ss << "\"ReadoutRequestPacket\": {";
    ss << "\"isValid\": " << valid_ << ",";
    ss << "\"ringID\": " << std::dec << ring_ << ",";
    ss << "\"packetType\": " << packetType_ << ",";
    ss << "\"hopCount\": " << hopCount_ << ",";
    ss << timestamp_.toJSON() << ",";
    ss << "\"request\": [" << std::hex << "0x" << (int)request_[0] << ",";
    ss << std::hex << "0x" << (int)request_[1] << ",";
    ss << std::hex << "0x" << (int)request_[2] << ",";
    ss << std::hex << "0x" << (int)request_[3] << "],";
    ss << "\"debug\": " << (debug_ ? "true" : "false");
    ss << "}";
    return ss.str();
}

std::string CFOLib::CFO_ReadoutRequestPacket::toPacketFormat()
{
    std::stringstream ss;
    ss << std::setfill('0') << std::hex;
    ss << std::setw(1) << (int)valid_ << "   " << "0x" << std::setw(2) << ring_ << "\t";
    ss << "0x" << std::setw(2) << packetType_ << "0x" << std::setw(2) << hopCount_ << "\n";
    ss << "0x" << std::setw(6) << (int)request_[1] << "\t0x" << std::setw(6) << (int)request_[0] << "\n";
    ss << timestamp_.toPacketFormat();
    ss << "        \t       " << std::setw(1) << (int)debug_ << "\n";
    ss << "0x" << std::setw(6) << (int)request_[3] << "\t0x" << std::setw(6) << (int)request_[2] << "\n";
    return ss.str();
}

CFOLib::CFO_SERDESRXDisparityError::CFO_SERDESRXDisparityError() : data_(0) {}

CFOLib::CFO_SERDESRXDisparityError::CFO_SERDESRXDisparityError(std::bitset<2> data) : data_(data) {}

CFOLib::CFO_SERDESRXDisparityError::CFO_SERDESRXDisparityError(uint32_t data, CFO_Ring_ID ring)
{
    std::bitset<32> dataSet = data;
    uint32_t ringBase = (uint8_t)ring * 2;
    data_[0] = dataSet[ringBase];
    data_[1] = dataSet[ringBase + 1];
}


CFOLib::CFO_CharacterNotInTableError::CFO_CharacterNotInTableError() : data_(0) {}

CFOLib::CFO_CharacterNotInTableError::CFO_CharacterNotInTableError(std::bitset<2> data) : data_(data) {}

CFOLib::CFO_CharacterNotInTableError::CFO_CharacterNotInTableError(uint32_t data, CFO_Ring_ID ring)
{
    std::bitset<32> dataSet = data;
    uint32_t ringBase = (uint8_t)ring * 2;
    data_[0] = dataSet[ringBase];
    data_[1] = dataSet[ringBase + 1];
}

CFOLib::CFO_TestMode::CFO_TestMode() {}
CFOLib::CFO_TestMode::CFO_TestMode(bool state, bool loopback, bool txChecker, bool rxGenerator)
{
    txChecker = txChecker;
    loopbackEnabled = loopback;
    rxGenerator = rxGenerator;
    state_ = state;
}
CFOLib::CFO_TestMode::CFO_TestMode(uint32_t input)
{
    std::bitset<3> mode = (input & 0x700) >> 8;
    txChecker = mode[0];
    loopbackEnabled = mode[1];
    rxGenerator = mode[2];
    state_ = (input & 0xC000) != 0;
}
uint32_t CFOLib::CFO_TestMode::GetWord() const
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
std::string CFOLib::CFO_TestMode::toString()
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

CFOLib::CFO_TestCommand::CFO_TestCommand(bool state,
    int PacketSize, bool loopback, bool txChecker, bool rxGenerator)
{
    PacketSize = PacketSize;
    TestMode = CFO_TestMode(state, loopback, txChecker, rxGenerator);
}
CFOLib::CFO_TestCommand::CFO_TestCommand(m_ioc_cmd_t in)
{
    PacketSize = in.MinPktSize;
    TestMode = CFO_TestMode(in.TestMode);
}
m_ioc_cmd_t CFOLib::CFO_TestCommand::GetCommand() const
{
    m_ioc_cmd_t output;

    output.MinPktSize = output.MaxPktSize = PacketSize;
    output.TestMode = TestMode.GetWord();

    return output;
}

CFOLib::CFO_DMAState::CFO_DMAState(m_ioc_engstate_t in)
    : BDs(in.BDs), Buffers(in.Buffers), MinPktSize(in.MinPktSize),
    MaxPktSize(in.MaxPktSize), BDerrs(in.BDerrs), BDSerrs(in.BDSerrs),
    IntEnab(in.IntEnab), TestMode(in.TestMode)
{
    switch (in.Engine)
    {
    case 0:
        Direction = DTC_DMA_Direction_C2S;
        break;
    case 0x20:
        Direction = DTC_DMA_Direction_S2C;
        break;
    }
}
std::string CFOLib::CFO_DMAState::toString() {
    std::stringstream stream;
    stream << "{\"D\": " << Direction;
    stream << ", \"BDs\": " << BDs << ", \"Buffers\": " << Buffers;
    stream << ", \"PktSize\": [" << MinPktSize << "," << MaxPktSize << "], ";
    stream << "\"BDerrs\": " << BDerrs << ", \"BDSerrs\": " << BDSerrs;
    stream << ", \"IntEnab\": " << IntEnab << ", \"TestMode\": " << TestMode.toString() << "}" << std::endl;
    return stream.str();
}

CFOLib::CFO_DMAStat::CFO_DMAStat(DMAStatistics in) : LBR(in.LBR), LAT(in.LAT), LWT(in.LWT)
{
    switch (in.Engine)
    {
    case 0:
        Direction = DTC_DMA_Direction_C2S;
        break;
    case 0x20:
        Direction = DTC_DMA_Direction_S2C;
        break;
    }
}
std::string CFOLib::CFO_DMAStat::toString()
{
    std::stringstream stream;
    stream << "{\"D\": " << Direction;
    stream << ", \"LBR\": " << LBR << ", \"LAT\": " << LAT;
    stream << ", \"LWT\": " << LWT << "}" << std::endl;
    return stream.str();
}

CFOLib::CFO_DMAStats::CFO_DMAStats(m_ioc_engstats_t in)
{
    for (int i = 0; i < in.Count; ++i)
    {
        Stats.push_back(CFO_DMAStat(in.engptr[i]));
    }
}
CFOLib::CFO_DMAStats CFOLib::CFO_DMAStats::getData(DTC_DMA_Direction dir)
{
    CFO_DMAStats output;
    for (auto i : Stats)
    {
        if (i.Direction == dir)
        {
            output.addStat(i);
        }
    }

    if (output.size() == 0) {
        output.addStat(CFO_DMAStat());
    }
    return output;
}

CFOLib::CFO_PCIeState::CFO_PCIeState(m_ioc_pcistate_t in)
    : Version(in.Version), LinkState(in.LinkState == 1), LinkSpeed(in.LinkSpeed),
    LinkWidth(in.LinkWidth), VendorId(in.VendorId), DeviceId(in.DeviceId),
    IntMode(in.IntMode), MPS(in.MPS), MRRS(in.MRRS), InitFCCplD(in.InitFCCplD),
    InitFCCplH(in.InitFCCplH), InitFCNPD(in.InitFCNPD), InitFCNPH(in.InitFCNPH),
    InitFCPD(in.InitFCPD), InitFCPH(in.InitFCPH) {}

std::string CFOLib::CFO_PCIeState::toString()
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
    stream << "\t\"InitFCCplD\": " << "," << InitFCCplD << std::endl;             //* Initial FC Credits for Completion Data */
    stream << "\t\"InitFCCplH\": " << "," << InitFCCplH << std::endl;             //* Initial FC Credits for Completion Header */
    stream << "\t\"InitFCNPD\": " << "," << InitFCNPD << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "\t\"InitFCNPH\": " << "," << InitFCNPH << std::endl;              //* Initial FC Credits for Non-Posted Data */
    stream << "\t\"InitFCPD\": " << "," << InitFCPD << std::endl;               //* Initial FC Credits for Posted Data */
    stream << "\t\"InitFCPH\": " << "," << InitFCPH << std::endl;               //* Initial FC Credits for Posted Data */
    stream << "}" << std::endl;

    return stream.str();
}

CFOLib::CFO_PCIeStat::CFO_PCIeStat(TRNStatistics in)
    : LTX(in.LTX), LRX(in.LRX) {}
