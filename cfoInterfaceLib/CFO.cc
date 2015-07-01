#include "CFO.h"
#include <sstream> // Convert uint to hex string
#include <iomanip> // std::setw, std::setfill
#ifndef _WIN32
# include <unistd.h>
# include "trace.h"
#else
# include <chrono>
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# ifndef TRACE
#  define TRACE(...)
# endif
#endif

CFOLib::CFO::CFO(CFOLib::CFO_SimMode mode) : device_(),
rrqbuffer_(nullptr), simMode_(mode),
first_read_(true), lastReadPtr_(nullptr), nextReadPtr_(nullptr)
{
#ifdef _WIN32
    simMode_ = CFOLib::CFO_SimMode_Enabled;
#else
    char* sim = getenv("CFOLIB_SIM_ENABLE");
    if(sim != NULL) 
    {
        switch (sim[0])
        {
        case '1':
        case 'e':
        case 'E':
            simMode_ = CFOLib::CFO_SimMode_Enabled;
            break;
        case '0':
        default:
            simMode_ = CFOLib::CFO_SimMode_Disabled;
            break;
        }
    }
#endif
    SetSimMode(simMode_);

}

CFOLib::CFO_SimMode CFOLib::CFO::SetSimMode(CFO_SimMode mode)
{
    simMode_ = mode;
    device_.init(simMode_);

    if (simMode_ != CFOLib::CFO_SimMode_Disabled)
    {
        // Set up hardware simulation mode: Ring 0 Tx/Rx Enabled, Loopback Enabled, ROC Emulator Enabled. All other rings disabled.
        for (auto ring : CFO_Rings) {
            DisableRing(ring);
        }
        EnableRing(CFO_Ring_0, CFO_RingEnableMode(true, true), 1);
        SetSERDESLoopbackMode(CFO_Ring_0, CFO_SERDESLoopbackMode_NearPCS);
        SetInternalSystemClock();
    }
    return simMode_;
}

//
// DMA Functions
//
void CFOLib::CFO::WriteCFOTable(const CFO_ReadoutRequestTable& input)
{
    size_t size = input.size() * sizeof(CFO_ReadoutRequestTableItem) * 2;
    WriteReadoutRequestInfoTableSize(size);
    uint8_t* buf = new uint8_t[size];
    size_t index = 0;

    for (auto i : input)
    {
        for (int ring = 0; ring < CFO_RING_COUNT; ++ring) {
            for (int byte = 0; byte < 4; ++byte) {
                buf[index] = i.RequestBytes[ring][byte];
                index++;
            }
            index += 4;
        }
    }

    device_.write_data(buf, size);
    delete[] buf;
}

std::vector<CFOLib::CFO_ReadoutRequestPacket> CFOLib::CFO::ReadLoopbackData(int maxCount)
{
    std::vector<CFO_ReadoutRequestPacket> output;
    bool finished = false;

    while (((int)output.size() < maxCount || maxCount < 0) && !finished) 
    {
        try {
            output.push_back(ReadNextLoopbackPacket());
        }
        catch (CFO_WrongPacketTypeException ex) {
            finished = true;
        }
    }

    return output;
}
CFOLib::CFO_ReadoutRequestPacket&& CFOLib::CFO::ReadNextLoopbackPacket(int tmo_ms)
{
    TRACE(19, "CFO::ReadNextLoopbackPacket BEGIN");
    if (nextReadPtr_ != nullptr) {
        TRACE(19, "CFO::ReadNextLoopbackPacket BEFORE BUFFER CHECK nextReadPtr_=%p *nextReadPtr_=0x%08x"
            , (void*)nextReadPtr_, *(unsigned*)nextReadPtr_);
    }
    else {
        TRACE(19, "CFO::ReadNextLoopbackPacket BEFORE BUFFER CHECK nextReadPtr_=nullptr");
    }
    // Check if the nextReadPtr has been initialized, and if its pointing to a valid location
    if (nextReadPtr_ == nullptr || nextReadPtr_ >= (uint8_t*)rrqbuffer_ + sizeof(mu2e_databuff_t) || (*((uint16_t*)nextReadPtr_)) == 0) {
        if (first_read_) {
            TRACE(19, "CFO::ReadNextLoopbackPacket: calling device_.release_all");
            device_.release_all();
            lastReadPtr_ = nullptr;
        }
        TRACE(19, "CFO::ReadNextLoopbackPacket Obtaining new DAQ Buffer");
        ReadBuffer(tmo_ms); // does return val of type DTCLib::DTC_DataPacket
        // MUST BE ABLE TO HANDLE daqbuffer_==nullptr OR retry forever?
        nextReadPtr_ = &(rrqbuffer_[0]);
        TRACE(19, "CFO::ReadNextLoopbackPacket nextReadPtr_=%p *nextReadPtr_=0x%08x lastReadPtr_=%p"
            , (void*)nextReadPtr_, *(unsigned*)nextReadPtr_, (void*)lastReadPtr_);
        if (nextReadPtr_ == lastReadPtr_) {
            nextReadPtr_ = nullptr;
            //We didn't actually get a new buffer...this probably means there's no more data
            throw CFO_WrongPacketTypeException();
        }
    }
    first_read_ = false;
    //Read the next packet
    TRACE(19, "CFO::ReadNextLoopbackPacket reading next packet from buffer: nextReadPtr_=%p:", (void*)nextReadPtr_);
    CFO_ReadoutRequestPacket output = CFO_ReadoutRequestPacket((uint8_t*)nextReadPtr_);
    TRACE(19, output.toJSON().c_str());

    // Update the packet pointers

    // lastReadPtr_ is easy...
    lastReadPtr_ = nextReadPtr_;

    // Increment by the size of the packet
    nextReadPtr_ = (char*)nextReadPtr_ + 16;

    TRACE(19, "CFO::ReadNextLoopbackPacket RETURN");
    return std::move(output);
}

//
// Register IO Functions
//
std::string CFOLib::CFO::RegDump()
{
    std::ostringstream o;
    o.setf(std::ios_base::boolalpha);
    o << "{";
    o << "\"SimMode\":" << CFO_SimModeConverter(simMode_) << ",\n";
    o << "\"Version\":\"" << ReadDesignVersion() << "\",\n";
    o << "\"ResetCFO\":" << ReadResetCFO() << ",\n";
    o << "\"ResetSERDESOscillator\":" << ReadResetSERDESOscillator() << ",\n";
    o << "\"SERDESOscillatorClock\":" << ReadSERDESOscillatorClock() << ",\n";
    o << "\"ResetRRPTableStartAddress\":" << ReadResetRRPTableStartAddress() << ",\n";
    o << "\"SystemClock\":" << ReadSystemClock() << ",\n";
    o << "\"TriggerDMALength\":" << ReadTriggerDMATransferLength() << ",\n";
    o << "\"MinDMALength\":" << ReadMinDMATransferLength() << ",\n";
    o << "\"DMATimeout\":" << ReadDMATimeoutPreset() << ",\n";
    o << "\"Timestamp\":" << ReadTimestampPreset().GetTimestamp(true) << ",\n";
    o << "\"DataPendingTimer\":" << ReadDataPendingTimer() << ",\n";
    o << "\"PacketSize\":" << ReadPacketSize() << ",\n";
    o << "\"PROMFIFOFull\":" << ReadFPGAPROMProgramFIFOFull() << ",\n";
    o << "\"PROMReady\":" << ReadFPGAPROMReady() << ",\n";
    o << "\"FPGACoreFIFOFull\":" << ReadFPGACoreAccessFIFOFull() << ",\n";
    for (auto r : CFO_Rings) {
        o << RingRegDump(r, CFO_RingNames[(int)r]) << ",\n";
    }
    o << "}";

    return o.str();
}
std::string CFOLib::CFO::RingRegDump(const CFO_Ring_ID& ring, std::string id)
{
    std::ostringstream o;
    o.setf(std::ios_base::boolalpha);

    o << id << ":{\n";

    o << "\t\"Enabled\":" << ReadRingEnabled(ring) << ",\n";
    o << "\t\"CFOCount\":" << ReadRingDTCCount(ring) << ",\n";
    o << "\t\"ResetSERDES\":" << ReadResetSERDES(ring) << ",\n";
    o << "\t\"SERDESLoopback\":" << CFO_SERDESLoopbackModeConverter(ReadSERDESLoopback(ring)) << ",\n";
    o << "\t\"EyescanError\":" << ReadSERDESEyescanError(ring) << ",\n";
    o << "\t\"FIFOFullFlags\":" << ReadFIFOFullErrorFlags(ring) << ",\n";
    o << "\t\"FIFOHalfFull\":" << ReadSERDESBufferFIFOHalfFull(ring) << ",\n";
    o << "\t\"OverflowOrUnderflow\":" << ReadSERDESOverflowOrUnderflow(ring) << ",\n";
    o << "\t\"PLLLocked\":" << ReadSERDESPLLLocked(ring) << ",\n";
    o << "\t\"RXCDRLock\":" << ReadSERDESRXCDRLock(ring) << ",\n";
    o << "\t\"ResetDone\":" << ReadSERDESResetDone(ring) << ",\n";
    o << "\t\"UnlockError\":" << ReadSERDESUnlockError(ring) << ",\n";
    o << "\t\"RXBufferStatus\":" << CFO_RXBufferStatusConverter(ReadSERDESRXBufferStatus(ring)) << ",\n";
    o << "\t\"RXStatus\":" << CFO_RXStatusConverter(ReadSERDESRXStatus(ring)) << ",\n";
    o << "\t\"SERDESRXDisparity\":" << ReadSERDESRXDisparityError(ring) << ",\n";
    o << "\t\"CharacterError\":" << ReadSERDESRXCharacterNotInTableError(ring) << "\n";
    o << "}";

    return o.str();
}

std::string CFOLib::CFO::ConsoleFormatRegDump()
{
    std::ostringstream o;
    o << "Memory Map: " << std::endl;
    o << "    Address | Value      | Name                        | Translation" << std::endl;
    for (auto i : CFO_Registers)
    {
        o << "================================================================================" << std::endl;
        o << FormatRegister(i);
    }
    return o.str();
}

std::string CFOLib::CFO::FormatRegister(const CFO_Register& address)
{
    std::ostringstream o;
    o << std::hex << std::setfill('0');
    o << "    0x" << (int)address << "  | 0x" << std::setw(8) << (int)ReadRegister(address) << " ";

    switch (address) {
    case CFO_Register_DesignVersion:
        o << "| CFO Firmware Design Version | " << ReadDesignVersionNumber();
        break;
    case CFO_Register_DesignDate:
        o << "| CFO Firmware Design Date    | " << ReadDesignDate();
        break;
    case CFO_Register_CFOControl:
        o << "| CFO Control                 | ";
        o << "Reset: [" << (ReadResetCFO() ? "x" : " ") << "]," << std::endl;
        o << "                                                       | ";
        o << "SERDES Oscillator Reset: [" << (ReadResetSERDESOscillator() ? "x" : " ") << "]," << std::endl;
        o << "                                                       | ";
        o << "SERDES Oscillator Clock Select : [" << (ReadSERDESOscillatorClock() ? " 2.5Gbs" : "3.125Gbs") << "], " << std::endl;
        o << "                                                       | ";
        o << "Reset RRP Table Start Address : [" << (ReadResetRRPTableStartAddress() ? "x" : " ") << "], " << std::endl;
        o << "                                                       | ";
        o << "System Clock Select : [" << (ReadSystemClock() ? "Ext" : "Int") << "]" << std::endl;
        break;
    case CFO_Register_DMATransferLength:
        o << "| DMA Transfer Length         | ";
        o << "Trigger Length: 0x" << ReadTriggerDMATransferLength() << "," << std::endl;
        o << "                                                       | ";
        o << "Minimum Length : 0x" << ReadMinDMATransferLength();
        break;
    case CFO_Register_SERDESLoopbackEnable:
        o << "| SERDES Loopback Enable      | ";
        for (auto r : CFO_Rings) {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": " << CFO_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString();
        }
        break;
    case CFO_Register_SERDESLoopbackEnable_Temp:
        o << "| SERDES Loopback Enable 2    | ";
        for (auto r : CFO_Rings) {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": " << CFO_SERDESLoopbackModeConverter(ReadSERDESLoopback(r)).toString();
        }
        break;
    case CFO_Register_RingEnable:
        o << "| Ring Enable                 | ([TX,RX])" << std::endl;
        for (auto r : CFO_Rings) {
            CFO_RingEnableMode re = ReadRingEnabled(r);
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            o << "Ring " << (int)r << ": [";
            o << (re.TransmitEnable ? "x" : " ") << ",";
            o << (re.ReceiveEnable ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESReset:
        o << "| SERDES Reset                | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadResetSERDES(r) ? "x" : " ") << "]";
        }
        break;
    case CFO_Register_SERDESRXDisparityError:
        o << "| SERDES RX Disparity Error   | ([H,L])" << std::endl;
        for (auto r : CFO_Rings) {
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            CFO_SERDESRXDisparityError re = ReadSERDESRXDisparityError(r);
            o << "Ring " << (int)r << ": [";
            o << re.GetData()[1] << ",";
            o << re.GetData()[0] << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESRXCharacterNotInTableError:
        o << "| SERDES RX CNIT Error        | ([H,L])" << std::endl;
        for (auto r : CFO_Rings) {
            auto re = ReadSERDESRXCharacterNotInTableError(r);
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            o << "Ring " << (int)r << ": [";
            o << re.GetData()[1] << ",";
            o << re.GetData()[0] << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESUnlockError:
        o << "| SERDES Unlock Error         | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl  << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadSERDESUnlockError(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESPLLLocked:
        o << "| SERDES PLL Locked           | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadSERDESPLLLocked(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESTXBufferStatus:
        o << "| SERDES TX Buffer Status     | ([OF or UF, FIFO Half Full])" << std::endl;
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            o << "Ring " << (int)r << ": [";
            o << (ReadSERDESOverflowOrUnderflow(r) ? "x" : " ") << ",";
            o << (ReadSERDESBufferFIFOHalfFull(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESRXBufferStatus:
        o << "| SERDES RX Buffer Status     | ";
        for (auto r : CFO_Rings) {
            auto re = ReadSERDESRXBufferStatus(r);
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": " << CFO_RXBufferStatusConverter(re).toString() << "," << std::endl;
        }
        break;
    case CFO_Register_SERDESRXStatus:
        o << "| SERDES RX Status            | ";
        for (auto r : CFO_Rings) {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            auto re = ReadSERDESRXStatus(r);
            o << "Ring " << (int)r << ": " << CFO_RXStatusConverter(re).toString() << "," << std::endl;
        }
        break;
    case CFO_Register_SERDESResetDone:
        o << "| SERDES Reset Done           | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadResetSERDESDone(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESEyescanData:
        o << "| SERDES Eyescan Data Error   | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadSERDESEyescanError(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_SERDESRXCDRLock:
        o << "| SERDES RX CDR Lock          | ";
        for (auto r : CFO_Rings)
        {
            if ((int)r > 0) { o << "," << std::endl << "                                                       | "; }
            o << "Ring " << (int)r << ": [" << (ReadSERDESRXCDRLock(r) ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_DMATimeoutPreset:
        o << "| DMA Timeout                 | ";
        o << "0x" << ReadDMATimeoutPreset();
        break;
    case CFO_Register_TimestampPreset0:
        o << "| Timestamp Preset 0          | ";
        o << "0x" << ReadRegister(CFO_Register_TimestampPreset0);
        break;
    case CFO_Register_TimestampPreset1:
        o << "| Timestamp Preset 1          | ";
        o << "0x" << ReadRegister(CFO_Register_TimestampPreset1);
        break;
    case CFO_Register_DataPendingTimer:
        o << "| DMA Data Pending Timer      | ";
        o << "0x" << ReadDataPendingTimer();
        break;
    case CFO_Register_NUMCFOs:
        o << "| NUMDTCs                     | ";
        for (auto r : CFO_Rings) {
            if ((int)r > 0) {
                o << ", " << std::endl;
                o << "                                                       | ";
            }
            o << "Ring " << (int)r << ": " << ReadRingDTCCount(r);
        }
        break;
    case CFO_Register_FIFOFullErrorFlag0:
        o << "| FIFO Full Error Flags 0     | ([DataRequest, ReadoutRequest, OutputData])" << std::endl;
        for (auto r : CFO_Rings) {
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            auto re = ReadFIFOFullErrorFlags(r);
            o << "Ring " << (int)r << ": [";
            o << (re.DataRequestOutput ? "x" : " ") << ",";
            o << (re.ReadoutRequestOutput ? "x" : " ") << ",";
            o << (re.OutputData ? "x" : " ") << "]";
        }
        break;
    case CFO_Register_FIFOFullErrorFlag1:
        o << "| FIFO Full Error Flags 1     | ([DataInput, OutputDCSStage2, OutputDCS, OtherOutput]) " << std::endl;
        for (auto r : CFO_Rings) {
            auto re = ReadFIFOFullErrorFlags(r);
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            o << "Ring " << (int)r << ": [";
            o << (re.DataInput ? "x" : " ") << ",";
            o << (re.OutputDCSStage2 ? "x" : " ") << ",";
            o << (re.OutputDCS ? "x" : " ") << ",";
            o << (re.OtherOutput ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_FIFOFullErrorFlag2:
        o << "| FIFO Full Error Flags 2     | ([DCSStatusInput])" << std::endl;
        for (auto r : CFO_Rings) {
            auto re = ReadFIFOFullErrorFlags(r);
            if ((int)r > 0) { o << "," << std::endl; }
            o << "                                                       | ";
            o << "Ring " << (int)r << ": [" << (re.DCSStatusInput ? "x" : " ") << "]," << std::endl;
        }
        break;
    case CFO_Register_PacketSize:
        o << "| DMA Packet Size             | ";
        o << "0x" << ReadPacketSize();
        break;
    case CFO_Register_FPGAPROMProgramStatus:
        o << "| FPGA PROM Program Status    | ";
        o << "FPGA PROM Program FIFO Full: [" << (ReadFPGAPROMProgramFIFOFull() ? "x" : " ") << "]" << std::endl;
        o << "                                                       | ";
        o << "FPGA PROM Ready: [" << (ReadFPGAPROMReady() ? "x" : " ") << "]";
        break;
    case CFO_Register_FPGACoreAccess:
        o << "| FPGA Core Access            | ";
        o << "FPGA Core Access FIFO Full: [" << (ReadFPGACoreAccessFIFOFull() ? "x" : " ") << "]" << std::endl;
        o << "                                                       | ";
        o << "FPGA Core Access FIFO Empty: [" << (ReadFPGACoreAccessFIFOEmpty() ? "x" : " ") << "]";
        break;
    case CFO_Register_Invalid:
    default:
        o << "| Invalid Register            | !!!";
        break;
    }
    o << std::endl;
    return o.str();
}

std::string CFOLib::CFO::RegisterRead(const CFO_Register& address)
{
    uint32_t data = ReadRegister(address);
    std::stringstream stream;
    stream << std::hex << data;
    return std::string(stream.str());
}

std::string CFOLib::CFO::ReadDesignVersion()
{
    return ReadDesignVersionNumber() + "_" + ReadDesignDate();
}
std::string CFOLib::CFO::ReadDesignDate()
{
    uint32_t data = ReadDesignDateRegister();
    std::ostringstream o;
    int yearHex = (data & 0xFF000000) >> 24;
    int year = ((yearHex & 0xF0) >> 4) * 10 + (yearHex & 0xF);
    int monthHex = (data & 0xFF0000) >> 16;
    int month = ((monthHex & 0xF0) >> 4) * 10 + (monthHex & 0xF);
    int dayHex = (data & 0xFF00) >> 8;
    int day = ((dayHex & 0xF0) >> 4) * 10 + (dayHex & 0xF);
    int hour = ((data & 0xF0) >> 4) * 10 + (data & 0xF);
    o << "20" << std::setfill('0') << std::setw(2) << year << "-";
    o << std::setfill('0') << std::setw(2) << month << "-";
    o << std::setfill('0') << std::setw(2) << day << "-";
    o << std::setfill('0') << std::setw(2) << hour;
    //std::cout << o.str() << std::endl;
    return o.str();
}
std::string CFOLib::CFO::ReadDesignVersionNumber()
{
    uint32_t data = ReadDesignVersionNumberRegister();
    int minor = data & 0xFF;
    int major = (data & 0xFF00) >> 8;
    return "v" + std::to_string(major) + "." + std::to_string(minor);
}

void CFOLib::CFO::ResetCFO()
{
    std::bitset<32> data = ReadControlRegister();
    data[31] = 1; // CFO Reset bit
    WriteControlRegister(data.to_ulong());
}
bool CFOLib::CFO::ReadResetCFO()
{
    std::bitset<32> dataSet = ReadControlRegister();
    return dataSet[31];
}

void CFOLib::CFO::ResetSERDESOscillator(){
    std::bitset<32> data = ReadControlRegister();
    data[29] = 1; //SERDES Oscillator Reset bit
    WriteControlRegister(data.to_ulong());
    usleep(2);
    data[29] = 0;
    WriteControlRegister(data.to_ulong());
    for (auto ring : CFO_Rings)
    {
        ResetSERDES(ring);
    }
}
bool CFOLib::CFO::ReadResetSERDESOscillator()
{
    std::bitset<32> data = ReadControlRegister();
    return data[29];
}
void CFOLib::CFO::ToggleSERDESOscillatorClock()
{
    std::bitset<32> data = ReadControlRegister();
    data.flip(28);
    WriteControlRegister(data.to_ulong());

    ResetSERDESOscillator();
}
bool CFOLib::CFO::ReadSERDESOscillatorClock()
{
    std::bitset<32> data = ReadControlRegister();
    return data[28];
}

void  CFOLib::CFO::ToggleRRPTableStartAddress()
{
    std::bitset<32> data = ReadControlRegister();
    data[27].flip();
    WriteControlRegister(data.to_ulong());
}
bool  CFOLib::CFO::ReadResetRRPTableStartAddress()
{
    std::bitset<32> data = ReadControlRegister();
    return data[27];
}

bool CFOLib::CFO::SetExternalSystemClock()
{
    std::bitset<32> data = ReadControlRegister();
    data[1] = 1;
    WriteControlRegister(data.to_ulong());
    return ReadSystemClock();
}
bool CFOLib::CFO::SetInternalSystemClock()
{
    std::bitset<32> data = ReadControlRegister();
    data[1] = 0;
    WriteControlRegister(data.to_ulong());
    return ReadSystemClock();
}
bool CFOLib::CFO::ToggleSystemClockEnable()
{
    std::bitset<32> data = ReadControlRegister();
    data.flip(1);
    WriteControlRegister(data.to_ulong());
    return ReadSystemClock();
}
bool CFOLib::CFO::ReadSystemClock()
{
    std::bitset<32> data = ReadControlRegister();
    return data[1];
}

int CFOLib::CFO::SetTriggerDMATransferLength(uint16_t length)
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = (data & 0x0000FFFF) + (length << 16);
    WriteDMATransferLengthRegister(data);
    return ReadTriggerDMATransferLength();
}
uint16_t CFOLib::CFO::ReadTriggerDMATransferLength()
{
    uint32_t data = ReadDMATransferLengthRegister();
    data >>= 16;
    return static_cast<uint16_t>(data);
}

int CFOLib::CFO::SetMinDMATransferLength(uint16_t length)
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = (data & 0xFFFF0000) + length;
    WriteDMATransferLengthRegister(data);
    return ReadMinDMATransferLength();
}
uint16_t CFOLib::CFO::ReadMinDMATransferLength()
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = data & 0x0000FFFF;
    return static_cast<uint16_t>(data);
}

CFOLib::CFO_SERDESLoopbackMode CFOLib::CFO::SetSERDESLoopbackMode(const CFO_Ring_ID& ring, const CFO_SERDESLoopbackMode& mode)
{
    std::bitset<32> data = ReadSERDESLoopbackEnableRegister();
    std::bitset<3> modeSet = mode;
    data[3 * ring] = modeSet[0];
    data[3 * ring + 1] = modeSet[1];
    data[3 * ring + 2] = modeSet[2];
    WriteSERDESLoopbackEnableRegister(data.to_ulong());

    // Now do the temp register
    data = ReadSERDESLoopbackEnableTempRegister();
    modeSet = mode;
    data[3 * ring] = modeSet[0];
    data[3 * ring + 1] = modeSet[1];
    data[3 * ring + 2] = modeSet[2];
    WriteSERDESLoopbackEnableTempRegister(data.to_ulong());
    return ReadSERDESLoopback(ring);
}
CFOLib::CFO_SERDESLoopbackMode CFOLib::CFO::ReadSERDESLoopback(const CFO_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESLoopbackEnableRegister() >> (3 * ring));
    if (dataSet == 0) {
        dataSet = (ReadSERDESLoopbackEnableTempRegister() >> (3 * ring));
    }
    return static_cast<CFO_SERDESLoopbackMode>(dataSet.to_ulong());
}

CFOLib::CFO_RingEnableMode CFOLib::CFO::EnableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode, const int cfoCount)
{
    std::bitset<32> data = ReadRingEnableRegister();
    data[ring] = mode.TransmitEnable;
    data[ring + 8] = mode.ReceiveEnable;
    WriteRingEnableRegister(data.to_ulong());
    WriteRingDTCCount(ring, cfoCount);
    return ReadRingEnabled(ring);
}
CFOLib::CFO_RingEnableMode CFOLib::CFO::DisableRing(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode)
{
    std::bitset<32> data = ReadRingEnableRegister();
    data[ring] = data[ring] && !mode.TransmitEnable;
    data[ring + 8] = data[ring + 8] && !mode.ReceiveEnable;
    WriteRingEnableRegister(data.to_ulong());
    return ReadRingEnabled(ring);
}
CFOLib::CFO_RingEnableMode CFOLib::CFO::ToggleRingEnabled(const CFO_Ring_ID& ring, const CFO_RingEnableMode& mode)
{
    std::bitset<32> data = ReadRingEnableRegister();
    if (mode.TransmitEnable) { data.flip((uint8_t)ring); }
    if (mode.ReceiveEnable) { data.flip((uint8_t)ring + 8); }

    WriteRingEnableRegister(data.to_ulong());
    return ReadRingEnabled(ring);
}
CFOLib::CFO_RingEnableMode CFOLib::CFO::ReadRingEnabled(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadRingEnableRegister();
    return CFO_RingEnableMode(dataSet[ring], dataSet[ring + 8]);
}

bool CFOLib::CFO::ResetSERDES(const CFO_Ring_ID& ring, int interval)
{
    bool resetDone = false;
    while (!resetDone)
    {
        TRACE(0, "Entering SERDES Reset Loop");
        std::bitset<32> data = ReadSERDESResetRegister();
        data[ring] = 1;
        WriteSERDESResetRegister(data.to_ulong());

        usleep(interval);

        data = ReadSERDESResetRegister();
        data[ring] = 0;
        WriteSERDESResetRegister(data.to_ulong());

        resetDone = ReadSERDESResetDone(ring);
        TRACE(0, "End of SERDES Reset loop, done %d", resetDone);
    }
    return resetDone;
}
bool CFOLib::CFO::ReadResetSERDES(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetRegister();
    return dataSet[ring];
}
bool CFOLib::CFO::ReadResetSERDESDone(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
    return dataSet[ring];
}

CFOLib::CFO_SERDESRXDisparityError CFOLib::CFO::ReadSERDESRXDisparityError(const CFO_Ring_ID& ring)
{
    return CFO_SERDESRXDisparityError(ReadSERDESRXDisparityErrorRegister(), ring);
}
CFOLib::CFO_SERDESRXDisparityError CFOLib::CFO::ClearSERDESRXDisparityError(const CFO_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESRXDisparityErrorRegister();
    data[ring * 2] = 1;
    data[ring * 2 + 1] = 1;
    WriteSERDESRXDisparityErrorRegister(data.to_ulong());
    return ReadSERDESRXDisparityError(ring);
}
CFOLib::CFO_CharacterNotInTableError CFOLib::CFO::ReadSERDESRXCharacterNotInTableError(const CFO_Ring_ID& ring)
{
    return CFO_CharacterNotInTableError(ReadSERDESRXCharacterNotInTableErrorRegister(), ring);
}
CFOLib::CFO_CharacterNotInTableError CFOLib::CFO::ClearSERDESRXCharacterNotInTableError(const CFO_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESRXCharacterNotInTableErrorRegister();
    data[ring * 2] = 1;
    data[ring * 2 + 1] = 1;
    WriteSERDESRXCharacterNotInTableErrorRegister(data.to_ulong());

    return ReadSERDESRXCharacterNotInTableError(ring);
}

bool CFOLib::CFO::ReadSERDESUnlockError(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESUnlockErrorRegister();
    return dataSet[ring];
}
bool CFOLib::CFO::ClearSERDESUnlockError(const CFO_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESUnlockErrorRegister();
    data[ring] = 1;
    WriteSERDESUnlockErrorRegister(data.to_ulong());
    return ReadSERDESUnlockError(ring);
}
bool CFOLib::CFO::ReadSERDESPLLLocked(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESPLLLockedRegister();
    return dataSet[ring];
}
bool CFOLib::CFO::ReadSERDESOverflowOrUnderflow(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
    return dataSet[ring * 2 + 1];
}
bool CFOLib::CFO::ReadSERDESBufferFIFOHalfFull(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
    return dataSet[ring * 2];
}

CFOLib::CFO_RXBufferStatus CFOLib::CFO::ReadSERDESRXBufferStatus(const CFO_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESRXBufferStatusRegister() >> (3 * ring));
    return static_cast<CFO_RXBufferStatus>(dataSet.to_ulong());
}

CFOLib::CFO_RXStatus CFOLib::CFO::ReadSERDESRXStatus(const CFO_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESRXStatusRegister() >> (3 * ring));
    return static_cast<CFO_RXStatus>(dataSet.to_ulong());
}

bool CFOLib::CFO::ReadSERDESEyescanError(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESEyescanErrorRegister();
    return dataSet[ring];
}
bool CFOLib::CFO::ClearSERDESEyescanError(const CFO_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESEyescanErrorRegister();
    data[ring] = 1;
    WriteSERDESEyescanErrorRegister(data.to_ulong());
    return ReadSERDESEyescanError(ring);
}
bool CFOLib::CFO::ReadSERDESRXCDRLock(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESRXCDRLockRegister();
    return dataSet[ring];
}

int CFOLib::CFO::WriteDMATimeoutPreset(uint32_t preset)
{
    WriteDMATimeoutPresetRegister(preset);
    return ReadDMATimeoutPreset();
}
uint32_t CFOLib::CFO::ReadDMATimeoutPreset()
{
    return ReadDMATimeoutPresetRegister();
}
int CFOLib::CFO::WriteDataPendingTimer(uint32_t timer)
{
    WriteDataPendingTimerRegister(timer);
    return ReadDataPendingTimer();
}
uint32_t CFOLib::CFO::ReadDataPendingTimer()
{
    return ReadDataPendingTimerRegister();
}
int CFOLib::CFO::SetPacketSize(uint16_t packetSize)
{
    WriteDMAPacketSizeRegister(0x00000000 + packetSize);
    return ReadPacketSize();
}
uint16_t CFOLib::CFO::ReadPacketSize()
{
    return static_cast<uint16_t>(ReadDMAPacketSizeRegister());
}


uint32_t CFOLib::CFO::ReadReadoutRequestInfoTableSize()
{
    return ReadRRInfoTableSizeRegister();
}
uint32_t CFOLib::CFO::WriteReadoutRequestInfoTableSize(uint32_t size)
{
    WriteRRInfoTableSizeRegister(size);
    return ReadReadoutRequestInfoTableSize();
}

int CFOLib::CFO::WriteRingDTCCount(const CFO_Ring_ID& ring, const int count)
{
    std::bitset<32> ringDTCs = ReadNUMDTCsRegister();

    ringDTCs[ring * 3] = count & 1;
    ringDTCs[ring * 3 + 1] = ((count & 2) >> 1) & 1;
    ringDTCs[ring * 3 + 2] = ((count & 4) >> 2) & 1;

    WriteNUMDTCsRegister(ringDTCs.to_ulong());
    return ReadRingDTCCount(ring);
}
int CFOLib::CFO::ReadRingDTCCount(const CFO_Ring_ID& ring)
{
    std::bitset<32> ringDTCs = ReadNUMDTCsRegister();
    return ringDTCs[ring * 3] + (ringDTCs[ring * 3 + 1] << 1) + (ringDTCs[ring * 3 + 2] << 2);
}


CFOLib::CFO_FIFOFullErrorFlags CFOLib::CFO::WriteFIFOFullErrorFlags(const CFO_Ring_ID& ring, const CFO_FIFOFullErrorFlags& flags)
{
    std::bitset<32> data0 = ReadFIFOFullErrorFlag0Register();
    std::bitset<32> data1 = ReadFIFOFullErrorFlag1Register();
    std::bitset<32> data2 = ReadFIFOFullErrorFlag2Register();

    data0[ring] = flags.OutputData;
    data0[ring + 16] = flags.ReadoutRequestOutput;
    data0[ring + 24] = flags.DataRequestOutput;
    data1[ring] = flags.OtherOutput;
    data1[ring + 8] = flags.OutputDCS;
    data1[ring + 16] = flags.OutputDCSStage2;
    data1[ring + 24] = flags.DataInput;
    data2[ring] = flags.DCSStatusInput;

    WriteFIFOFullErrorFlag0Register(data0.to_ulong());
    WriteFIFOFullErrorFlag1Register(data1.to_ulong());
    WriteFIFOFullErrorFlag2Register(data2.to_ulong());

    return ReadFIFOFullErrorFlags(ring);
}
CFOLib::CFO_FIFOFullErrorFlags CFOLib::CFO::ToggleFIFOFullErrorFlags(const CFO_Ring_ID& ring, const CFO_FIFOFullErrorFlags& flags)
{
    std::bitset<32> data0 = ReadFIFOFullErrorFlag0Register();
    std::bitset<32> data1 = ReadFIFOFullErrorFlag1Register();
    std::bitset<32> data2 = ReadFIFOFullErrorFlag2Register();

    data0[ring] = flags.OutputData ? !data0[ring] : data0[ring];
    data0[ring + 16] = flags.ReadoutRequestOutput ? !data0[ring + 16] : data0[ring + 16];
    data0[ring + 24] = flags.DataRequestOutput ? !data0[ring + 24] : data0[ring + 24];
    data1[ring] = flags.OtherOutput ? !data1[ring] : data1[ring];
    data1[ring + 8] = flags.OutputDCS ? !data1[ring + 8] : data1[ring + 8];
    data1[ring + 16] = flags.OutputDCSStage2 ? !data1[ring + 16] : data1[ring + 16];
    data1[ring + 24] = flags.DataInput ? !data1[ring + 24] : data1[ring + 24];
    data2[ring] = flags.DCSStatusInput ? !data2[ring] : data2[ring];

    WriteFIFOFullErrorFlag0Register(data0.to_ulong());
    WriteFIFOFullErrorFlag1Register(data1.to_ulong());
    WriteFIFOFullErrorFlag2Register(data2.to_ulong());

    return ReadFIFOFullErrorFlags(ring);
}
CFOLib::CFO_FIFOFullErrorFlags CFOLib::CFO::ReadFIFOFullErrorFlags(const CFO_Ring_ID& ring)
{
    std::bitset<32> data0 = ReadFIFOFullErrorFlag0Register();
    std::bitset<32> data1 = ReadFIFOFullErrorFlag1Register();
    std::bitset<32> data2 = ReadFIFOFullErrorFlag2Register();
    CFO_FIFOFullErrorFlags flags;

    flags.OutputData = data0[ring];
    flags.ReadoutRequestOutput = data0[ring + 16];
    flags.DataRequestOutput = data0[ring + 24];
    flags.OtherOutput = data1[ring];
    flags.OutputDCS = data1[ring + 8];
    flags.OutputDCSStage2 = data1[ring + 16];
    flags.DataInput = data1[ring + 24];
    flags.DCSStatusInput = data2[ring];

    return flags;

}

CFOLib::CFO_Timestamp CFOLib::CFO::WriteTimestampPreset(const CFO_Timestamp& preset)
{
    std::bitset<48> timestamp = preset.GetTimestamp();
    uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
    timestamp >>= 32;
    uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

    WriteTimestampPreset0Register(timestampLow);
    WriteTimestampPreset1Register(timestampHigh);
    return ReadTimestampPreset();
}
CFOLib::CFO_Timestamp CFOLib::CFO::ReadTimestampPreset()
{
    uint32_t timestampLow = ReadTimestampPreset0Register();
    CFO_Timestamp output;
    output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadTimestampPreset1Register()));
    return output;
}

bool CFOLib::CFO::ReadFPGAPROMProgramFIFOFull()
{
    std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
    return dataSet[1];
}
bool CFOLib::CFO::ReadFPGAPROMReady()
{
    std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
    return dataSet[0];
}

void CFOLib::CFO::ReloadFPGAFirmware()
{
    WriteFPGACoreAccessRegister(0xFFFFFFFF);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0xAA995566);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x20000000);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x30020001);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x00000000);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x30008001);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x0000000F);
    while (ReadFPGACoreAccessFIFOFull()) { usleep(10); }
    WriteFPGACoreAccessRegister(0x20000000);
}
bool CFOLib::CFO::ReadFPGACoreAccessFIFOFull()
{
    std::bitset<32> dataSet = ReadFPGACoreAccessRegister();
    return dataSet[1];
}
bool CFOLib::CFO::ReadFPGACoreAccessFIFOEmpty()
{
    std::bitset<32> dataSet = ReadFPGACoreAccessRegister();
    return dataSet[0];
}

//
// PCIe/DMA Status and Performance
// DMA Testing Engine
//
CFOLib::CFO_TestMode CFOLib::CFO::StartTest(int packetSize, bool loopback, bool txChecker, bool rxGenerator)
{
    CFO_TestCommand testCommand(true, packetSize, loopback, txChecker, rxGenerator);
    WriteTestCommand(testCommand, true);
    return ReadTestCommand().GetMode();
}
CFOLib::CFO_TestMode CFOLib::CFO::StopTest()
{
    WriteTestCommand(CFO_TestCommand(), false);
    return ReadTestCommand().GetMode();
}

CFOLib::CFO_DMAState CFOLib::CFO::ReadDMAState(const DTC_DMA_Direction& dir)
{
    m_ioc_engstate_t state;
    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_dma_state(dir, &state);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw CFO_IOErrorException();
    }

    return CFO_DMAState(state);
}
CFOLib::CFO_DMAStats CFOLib::CFO::ReadDMAStats(const DTC_DMA_Direction& dir)
{
    DMAStatistics statData[100];
    m_ioc_engstats_t stats;
    stats.Count = 100;
    stats.engptr = statData;

    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_dma_stats(&stats);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw CFO_IOErrorException();
    }

    return CFO_DMAStats(stats).getData(dir);
}

CFOLib::CFO_PCIeState CFOLib::CFO::ReadPCIeState()
{
    m_ioc_pcistate_t state;
    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_pcie_state(&state);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0) { throw CFO_IOErrorException(); }
    return CFO_PCIeState(state);
}
CFOLib::CFO_PCIeStat CFOLib::CFO::ReadPCIeStats()
{
    TRNStatistics statData[1];
    TRNStatsArray stats;
    stats.Count = 1;
    stats.trnptr = statData;
    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_trn_stats(&stats);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0) { throw CFO_IOErrorException(); }
    return CFO_PCIeStat(statData[0]);
}

//
// Private Functions.
//
mu2e_databuff_t* CFOLib::CFO::ReadBuffer(int tmo_ms)
{
    mu2e_databuff_t* buffer;
    int retry = 2;
    int errorCode;
    do {
        TRACE(19, "CFO::ReadBuffer before device_.read_data");
        errorCode = device_.read_data((void**)&buffer, tmo_ms);
        retry--;
    } while (retry > 0 && errorCode == 0);
    if (errorCode == 0) // timeout
        throw CFO_TimeoutOccurredException();
    else if (errorCode < 0)
        throw CFO_IOErrorException();
    TRACE(16, "CFO::ReadDataPacket buffer_=%p errorCode=%d *buffer_=0x%08x"
        , (void*)buffer, errorCode, *(unsigned*)buffer);
    rrqbuffer_ = buffer;
    return buffer;
}

void CFOLib::CFO::WriteRegister(uint32_t data, const CFO_Register& address)
{
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.write_register(address, 100, data);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new CFO_IOErrorException();
    }
}
uint32_t CFOLib::CFO::ReadRegister(const CFO_Register& address)
{
    int retry = 3;
    int errorCode = 0;
    uint32_t data;
    do {
        errorCode = device_.read_register(address, 100, &data);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new CFO_IOErrorException();
    }

    return data;
}

bool CFOLib::CFO::ReadSERDESResetDone(const CFO_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
    return dataSet[ring];
}


void CFOLib::CFO::WriteTestCommand(const CFO_TestCommand& comm, bool start)
{
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.write_test_command(comm.GetCommand(), start);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new CFO_IOErrorException();
    }
}
CFOLib::CFO_TestCommand CFOLib::CFO::ReadTestCommand()
{
    m_ioc_cmd_t comm;
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.read_test_command(&comm);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new CFO_IOErrorException;
    }
    return CFO_TestCommand(comm);
}
