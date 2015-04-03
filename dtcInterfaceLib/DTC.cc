#include "DTC.h"
#include <sstream> // Convert uint to hex string
#ifndef _WIN32
#include <unistd.h>
#include "trace.h"
#else
#include <chrono>
#include <thread>
#define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#define TRACE(...) 
#endif

DTCLib::DTC::DTC() : DTC_BUFFSIZE(sizeof(mu2e_databuff_t) / (16 * sizeof(uint8_t))), device_(),
daqbuffer_(nullptr), dcsbuffer_(nullptr),
lastReadPtr_(nullptr), nextReadPtr_(nullptr), dcsReadPtr_(nullptr)
{
#ifdef _WIN32
    simMode_ = true;
#else
    char* sim = getenv("DTCLIB_SIM_ENABLE");
    simMode_ = sim != NULL && sim[0] == '1';
#endif
    simMode_ = device_.init(simMode_) == 1;
}

//
// DMA Functions
//
std::vector<void*> DTCLib::DTC::GetData(DTC_Timestamp when, bool sendDReq, bool sendRReq)
{
    TRACE(19, "DTC::GetData before release_all");
    device_.release_all(0);
    std::vector<void*> output;
    if (sendRReq) {
        // Send a data request
        for (uint8_t ring = 0; ring < 6; ++ring){
            if (ReadRingEnabled((DTC_Ring_ID)ring)) {
                TRACE(19, "DTC::GetData before DTC_ReadoutRequestPacket req");
                DTC_ReadoutRequestPacket req((DTC_Ring_ID)ring, when, maxRocs_[ring]);
                TRACE(19, "DTC::GetData before WriteDMADAQPacket");
                WriteDMADAQPacket(req);
                TRACE(19, "DTC::GetData after  WriteDMADAQPacket");
            }
        }
    }
    if (sendDReq) {
        // Send a data request
        for (uint8_t ring = 0; ring < 6; ++ring){
            if (ReadRingEnabled((DTC_Ring_ID)ring)) {
                if (maxRocs_[ring] != DTC_ROC_Unused) {
                    for (uint8_t roc = 0; roc <= maxRocs_[ring]; ++roc) {
                        TRACE(19, "DTC::GetData before DTC_DataRequestPacket req");
                        DTC_DataRequestPacket req((DTC_Ring_ID)ring, (DTC_ROC_ID)roc, when);
                        TRACE(19, "DTC::GetData before WriteDMADAQPacket");
                        WriteDMADAQPacket(req);
                        TRACE(19, "DTC::GetData after  WriteDMADAQPacket");
                    }
                }
            }
        }
    }
    try{
        // Read the header packet
        DTC_DataHeaderPacket packet = ReadNextDAQPacket();
        TRACE(19, "DTC::GetData after  ReadDMADAQPacket");

        if (packet.GetTimestamp() != when && when.GetTimestamp(true) != 0)
        {
            TRACE(19, "DTC::GetData: Error: Lead packet has wrong timestamp!");
            return output;
        }
        else {
            when = packet.GetTimestamp();
        }

        TRACE(19, "DTC::GetData: Adding pointer %p to the list", (void*)lastReadPtr_);
        output.push_back(lastReadPtr_);

        bool done = false;
        while (!done) {
            try{
                DTC_DataHeaderPacket thispacket = ReadNextDAQPacket();
                if (thispacket.GetTimestamp() != when) {
                    done = true;
                    nextReadPtr_ = lastReadPtr_;
                    break;
                }

                TRACE(19, "DTC::GetData: Adding pointer %p to the list", (void*)lastReadPtr_);
                output.push_back(lastReadPtr_);
            }
            catch (DTC_WrongPacketTypeException ex)
            {
                TRACE(19, "DTC::GetData: End of data stream reached!");
                done = true;
            }
        }
    }
    catch (DTC_WrongPacketTypeException ex)
    {
        TRACE(19, "DTC::GetData: Bad omen: Wrong packet type at the current read position");
    }
    catch (DTC_IOErrorException ex)
    {
        nextReadPtr_ = nullptr;
        TRACE(19, "DTC::GetData: Timed out while trying to get next DMA");
    }

    TRACE(19, "DTC::GetData RETURN");
    return output;
}

std::string DTCLib::DTC::GetJSONData(DTC_Timestamp when, bool sendDReq, bool sendRReq)
{
    TRACE(19, "DTC::GetJSONData BEGIN");
    std::stringstream ss;
    TRACE(19, "DTC::GetJSONData before call to GetData");
    std::vector<void*> data = GetData(when, sendDReq, sendRReq);
    TRACE(19, "DTC::GetJSONData after call to GetData, data size %lu", data.size());

    for (size_t i = 0; i < data.size(); ++i)
    {
        TRACE(19, "DTC::GetJSONData constructing DataPacket:");
        DTC_DataPacket test = DTC_DataPacket(data[i]);
        TRACE(19, test.toJSON().c_str());
        TRACE(19, "DTC::GetJSONData constructing DataHeaderPacket");
        DTC_DataHeaderPacket theHeader = DTC_DataHeaderPacket(test);
        ss << "DataBlock: {";
        ss << theHeader.toJSON() << ",";
        ss << "DataPackets: [";
        for (int packet = 0; packet < theHeader.GetPacketCount(); ++packet)
        {
            void* packetPtr = (void*)(((char*)data[i]) + 16 * (1 + packet));
            ss << DTC_DataPacket(packetPtr).toJSON() << ",";
        }
        ss << "],";
        ss << "},";
    }

    TRACE(19, "DTC::GetJSONData RETURN");
    return ss.str();
}

std::vector<void*> DTCLib::DTC::GetData_OLD(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, const DTC_Timestamp& when, int* length)
{
    TRACE(19, "DTC::GetData before release_all");
    device_.release_all(0);
    std::vector<void*> output;
    // Send a data request
    TRACE(19, "DTC::GetData before DTC_DataRequestPacket req");
    DTC_DataRequestPacket req(ring, roc, when);
    TRACE(19, "DTC::GetData before WriteDMADAQPacket");
    WriteDMADAQPacket(req);
    TRACE(19, "DTC::GetData after  WriteDMADAQPacket");

    // Read the header packet
    DTC_DataHeaderPacket packet = ReadDMAPacket_OLD<DTC_DataHeaderPacket>(DTC_DMA_Engine_DAQ);
    TRACE(19, "DTC::GetData after  ReadDMADAQPacket");

    *length = packet.GetPacketCount() + 1;
    output.push_back(daqbuffer_);

    while (DTC_BUFFSIZE * output.size() < packet.GetPacketCount() + 1U)
    {
        device_.read_data(0, (void**)&daqbuffer_, 1000);
        output.push_back(daqbuffer_);
    }

    return output;
}

void DTCLib::DTC::DCSRequestReply(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t* dataIn)
{
    device_.release_all(1);
    DTC_DCSRequestPacket req(ring, roc, dataIn);
    WriteDMADCSPacket(req);
    DTC_DCSReplyPacket packet = ReadNextDCSPacket();

    TRACE(19, "DTC::DCSReqeuestReply after ReadNextDCSPacket");
    for (int ii = 0; ii < 12; ++ii) {
        dataIn[ii] = packet.GetData()[ii];
    }
}

void DTCLib::DTC::DCSRequestReply_OLD(const DTC_Ring_ID& ring, const DTC_ROC_ID& roc, uint8_t* dataIn)
{
    device_.release_all(1);
    DTC_DCSRequestPacket req(ring, roc, dataIn);
    WriteDMADCSPacket(req);
    DTC_DCSReplyPacket packet = ReadDMAPacket_OLD<DTC_DCSReplyPacket>(DTC_DMA_Engine_DCS);

    for (int ii = 0; ii < 12; ++ii) {
        dataIn[ii] = packet.GetData()[ii];
    }
}

void DTCLib::DTC::SendReadoutRequestPacket(const DTC_Ring_ID& ring, const DTC_Timestamp& when)
{
    DTC_ReadoutRequestPacket req(ring, when, maxRocs_[ring]);
    WriteDMADAQPacket(req);
}

void DTCLib::DTC::SetMaxROCNumber(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
    maxRocs_[ring] = lastRoc;
}


void DTCLib::DTC::WriteDMADAQPacket(const DTC_DMAPacket& packet)
{
    return WriteDMAPacket(DTC_DMA_Engine_DAQ, packet);
}
void DTCLib::DTC::WriteDMADCSPacket(const DTC_DMAPacket& packet)
{
    return WriteDMAPacket(DTC_DMA_Engine_DCS, packet);
}
template<typename PacketType>
PacketType DTCLib::DTC::ReadDMAPacket_OLD(const DTC_DMA_Engine& channel)
{
    TRACE(19, "DTC::ReadDMAPacket before DTC_DMAPacket(ReadDataPacket(channel))");
    return PacketType(ReadBuffer(channel));
}

DTCLib::DTC_DataHeaderPacket DTCLib::DTC::ReadNextDAQPacket()
{
    TRACE(19, "DTC::ReadNextDAQPacket BEGIN");
    // Check if the nextReadPtr has been initialized, and if its pointing to a valid location
    if (nextReadPtr_ == nullptr || nextReadPtr_ >= daqbuffer_ + sizeof(*daqbuffer_) || *((uint16_t*)nextReadPtr_) == 0) {
        TRACE(19, "DTC::ReadNextDAQPacket Obtaining new DAQ Buffer");
        ReadBuffer(DTC_DMA_Engine_DAQ);
        nextReadPtr_ = &(daqbuffer_[0]);
        TRACE(19, "DTC::ReadNextDAQPacket nextReadPtr_=%p daqBuffer_=%p", (void*)nextReadPtr_, (void*)daqbuffer_);
    }
    //Read the next packet
    TRACE(19, "DTC::ReadNextDAQPacket reading next packet from buffer: nextReadPtr_=%p:",(void*)nextReadPtr_);
    DTC_DataPacket test = DTC_DataPacket(nextReadPtr_);
    TRACE(19, test.toJSON().c_str());
    DTC_DataHeaderPacket output = DTC_DataHeaderPacket(test);
    TRACE(19, output.toJSON().c_str());

    // Update the packet pointers

    // lastReadPtr_ is easy...
    lastReadPtr_ = nextReadPtr_;

    // Increment by the size of the data block
    nextReadPtr_ = (char*)nextReadPtr_ + 16 * (1 + output.GetPacketCount());
    
    TRACE(19, "DTC::ReadNextDAQPacket RETURN");
    return output;
}
DTCLib::DTC_DCSReplyPacket DTCLib::DTC::ReadNextDCSPacket()
{
    TRACE(19, "DTC::ReadNextDCSPacket BEGIN");
    if (dcsReadPtr_ == nullptr || dcsReadPtr_ >= dcsbuffer_ + sizeof(*dcsbuffer_) || *((uint16_t*)dcsReadPtr_) == 0) {
        TRACE(19, "DTC::ReadNextDCSPacket Obtaining new DCS Buffer");
        ReadBuffer(DTC_DMA_Engine_DCS);
        dcsReadPtr_ = &(dcsbuffer_[0]);
        TRACE(19, "DTC::ReadNextDCSPacket dcsReadPtr_=%p dcsBuffer_=%p", (void*)dcsReadPtr_, (void*)dcsbuffer_);
    }

    //Read the next packet
    TRACE(19, "DTC::ReadNextDCSPacket Reading packet from buffer: dcsReadPtr_=%p:", (void*)dcsReadPtr_);
    DTC_DCSReplyPacket output = DTC_DCSReplyPacket(DTC_DataPacket(dcsReadPtr_));
    TRACE(19, output.toJSON().c_str());

    // Update the packet pointer

    // Increment by the size of the data block
    dcsReadPtr_ = (char*)dcsReadPtr_ + 16;

    TRACE(19, "DTC::ReadNextDCSPacket RETURN");
    return output;
}

//
// Register IO Functions
//
std::string DTCLib::DTC::RegisterRead(const DTC_Register& address)
{
    uint32_t data = ReadRegister(address);
    std::stringstream stream;
    stream << std::hex << data;
    return std::string(stream.str());
}

std::string DTCLib::DTC::ReadDesignVersion()
{
    return RegisterRead(DTC_Register_DesignVersion);
}

void DTCLib::DTC::ResetDTC()
{
    std::bitset<32> data = ReadControlRegister();
    data[31] = 1; // DTC Reset bit
    WriteControlRegister(data.to_ulong());
}
bool DTCLib::DTC::ReadResetDTC()
{
    std::bitset<32> dataSet = ReadControlRegister();
    return dataSet[31];
}

void DTCLib::DTC::ResetSERDESOscillator(){
    std::bitset<32> data = ReadControlRegister();
    data[29] = 1; //SERDES Oscillator Reset bit
    WriteControlRegister(data.to_ulong());
    usleep(2);
    data[29] = 0;
    WriteControlRegister(data.to_ulong());
    for (uint8_t i = 0; i < 6; ++i)
    {
        ResetSERDES((DTC_Ring_ID)i);
    }
}
bool DTCLib::DTC::ReadResetSERDESOscillator()
{
    std::bitset<32> data = ReadControlRegister();
    return data[29];
}
void DTCLib::DTC::ToggleSERDESOscillatorClock()
{
    std::bitset<32> data = ReadControlRegister();
    data.flip(28);
    WriteControlRegister(data.to_ulong());

    ResetSERDESOscillator();
}
bool DTCLib::DTC::ReadSERDESOscillatorClock()
{
    std::bitset<32> data = ReadControlRegister();
    return data[28];
}

void DTCLib::DTC::ToggleSystemClockEnable()
{
    std::bitset<32> data = ReadControlRegister();
    data.flip(1);
    WriteControlRegister(data.to_ulong());
}
bool DTCLib::DTC::ReadSystemClock()
{
    std::bitset<32> data = ReadControlRegister();
    return data[1];
}
void DTCLib::DTC::ToggleTimingEnable()
{
    std::bitset<32> data = ReadControlRegister();
    data.flip(0);
    WriteControlRegister(data.to_ulong());
}
bool DTCLib::DTC::ReadTimingEnable()
{
    std::bitset<32> data = ReadControlRegister();
    return data[0];
}

int DTCLib::DTC::SetTriggerDMATransferLength(uint16_t length)
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = (data & 0x0000FFFF) + (length << 16);
    WriteDMATransferLengthRegister(data);
    return ReadTriggerDMATransferLength();
}
uint16_t DTCLib::DTC::ReadTriggerDMATransferLength()
{
    uint32_t data = ReadDMATransferLengthRegister();
    data >>= 16;
    return static_cast<uint16_t>(data);
}

int DTCLib::DTC::SetMinDMATransferLength(uint16_t length)
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = (data & 0xFFFF0000) + length;
    WriteDMATransferLengthRegister(data);
    return ReadMinDMATransferLength();
}
uint16_t DTCLib::DTC::ReadMinDMATransferLength()
{
    uint32_t data = ReadDMATransferLengthRegister();
    data = data & 0x0000FFFF;
    return static_cast<uint16_t>(data);
}

DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC::SetSERDESLoopbackMode(const DTC_Ring_ID& ring, const DTC_SERDESLoopbackMode& mode)
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
DTCLib::DTC_SERDESLoopbackMode DTCLib::DTC::ReadSERDESLoopback(const DTC_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESLoopbackEnableRegister() >> (3 * ring));
    if (dataSet == 0) {
        dataSet = (ReadSERDESLoopbackEnableTempRegister() >> (3 * ring));
    }
    return static_cast<DTC_SERDESLoopbackMode>(dataSet.to_ulong());
}

bool DTCLib::DTC::ToggleROCEmulator()
{
    bool enabled = ReadROCEmulator();
    uint32_t value = enabled ? 0UL : 1UL;
    WriteROCEmulationEnableRegister(value);
    return ReadROCEmulator();
}
bool DTCLib::DTC::ReadROCEmulator()
{
    std::bitset<32> dataSet = ReadROCEmulationEnableRegister();
    return dataSet[0];
}

bool DTCLib::DTC::EnableRing(const DTC_Ring_ID& ring, const DTC_ROC_ID& lastRoc)
{
    if (lastRoc != DTC_ROC_Unused)
    {
        maxRocs_[ring] = lastRoc;
    }
    std::bitset<32> data = ReadRingEnableRegister();
    data[ring] = 1;
    WriteRingEnableRegister(data.to_ulong());
    return ReadRingEnabled(ring);
}
bool DTCLib::DTC::DisableRing(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadRingEnableRegister();
    data[ring] = 0;
    WriteRingEnableRegister(data.to_ulong());
    return ReadRingEnabled(ring);
}
bool DTCLib::DTC::ToggleRingEnabled(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadRingEnableRegister();
    data.flip((uint8_t)ring);
    WriteRingEnableRegister(data.to_ulong());
    return ReadRingEnabled(ring);
}
bool DTCLib::DTC::ReadRingEnabled(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadRingEnableRegister();
    return dataSet[ring];
}

bool DTCLib::DTC::ResetSERDES(const DTC_Ring_ID& ring, int interval)
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
bool DTCLib::DTC::ReadResetSERDES(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetRegister();
    return dataSet[ring];
}
bool DTCLib::DTC::ReadResetSERDESDone(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
    return dataSet[ring];
}

DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC::ReadSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
    return DTC_SERDESRXDisparityError(ReadSERDESRXDisparityErrorRegister(), ring);
}
DTCLib::DTC_SERDESRXDisparityError DTCLib::DTC::ClearSERDESRXDisparityError(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESRXDisparityErrorRegister();
    data[ring * 2] = 1;
    data[ring * 2 + 1] = 1;
    WriteSERDESRXDisparityErrorRegister(data.to_ulong());
    return ReadSERDESRXDisparityError(ring);
}
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC::ReadSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
    return DTC_CharacterNotInTableError(ReadSERDESRXCharacterNotInTableErrorRegister(), ring);
}
DTCLib::DTC_CharacterNotInTableError DTCLib::DTC::ClearSERDESRXCharacterNotInTableError(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESRXCharacterNotInTableErrorRegister();
    data[ring * 2] = 1;
    data[ring * 2 + 1] = 1;
    WriteSERDESRXCharacterNotInTableErrorRegister(data.to_ulong());

    return ReadSERDESRXCharacterNotInTableError(ring);
}

bool DTCLib::DTC::ReadSERDESUnlockError(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESUnlockErrorRegister();
    return dataSet[ring];
}
bool DTCLib::DTC::ClearSERDESUnlockError(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESUnlockErrorRegister();
    data[ring] = 1;
    WriteSERDESUnlockErrorRegister(data.to_ulong());
    return ReadSERDESUnlockError(ring);
}
bool DTCLib::DTC::ReadSERDESPLLLocked(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESPLLLockedRegister();
    return dataSet[ring];
}
bool DTCLib::DTC::ReadSERDESOverflowOrUnderflow(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
    return dataSet[ring * 2 + 1];
}
bool DTCLib::DTC::ReadSERDESBufferFIFOHalfFull(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESTXBufferStatusRegister();
    return dataSet[ring * 2];
}

DTCLib::DTC_RXBufferStatus DTCLib::DTC::ReadSERDESRXBufferStatus(const DTC_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESRXBufferStatusRegister() >> (3 * ring));
    return static_cast<DTC_RXBufferStatus>(dataSet.to_ulong());
}

DTCLib::DTC_RXStatus DTCLib::DTC::ReadSERDESRXStatus(const DTC_Ring_ID& ring)
{
    std::bitset<3> dataSet = (ReadSERDESRXStatusRegister() >> (3 * ring));
    return static_cast<DTC_RXStatus>(dataSet.to_ulong());
}

bool DTCLib::DTC::ReadSERDESEyescanError(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESEyescanErrorRegister();
    return dataSet[ring];
}
bool DTCLib::DTC::ClearSERDESEyescanError(const DTC_Ring_ID& ring)
{
    std::bitset<32> data = ReadSERDESEyescanErrorRegister();
    data[ring] = 1;
    WriteSERDESEyescanErrorRegister(data.to_ulong());
    return ReadSERDESEyescanError(ring);
}
bool DTCLib::DTC::ReadSERDESRXCDRLock(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESRXCDRLockRegister();
    return dataSet[ring];
}

int DTCLib::DTC::WriteDMATimeoutPreset(uint32_t preset)
{
    WriteDMATimeoutPresetRegister(preset);
    return ReadDMATimeoutPreset();
}
uint32_t DTCLib::DTC::ReadDMATimeoutPreset()
{
    return ReadDMATimeoutPresetRegister();
}
int DTCLib::DTC::WriteDataPendingTimer(uint32_t timer)
{
    WriteDataPendingTimerRegister(timer);
    return ReadDataPendingTimer();
}
uint32_t DTCLib::DTC::ReadDataPendingTimer()
{
    return ReadDataPendingTimerRegister();
}
int DTCLib::DTC::SetPacketSize(uint16_t packetSize)
{
    WriteDMAPacketSizetRegister(0x00000000 + packetSize);
    return ReadPacketSize();
}
uint16_t DTCLib::DTC::ReadPacketSize()
{
    return static_cast<uint16_t>(ReadDMAPacketSizeRegister());
}

DTCLib::DTC_Timestamp DTCLib::DTC::WriteTimestampPreset(const DTC_Timestamp& preset)
{
    std::bitset<48> timestamp = preset.GetTimestamp();
    uint32_t timestampLow = static_cast<uint32_t>(timestamp.to_ulong());
    timestamp >>= 32;
    uint16_t timestampHigh = static_cast<uint16_t>(timestamp.to_ulong());

    WriteTimestampPreset0Register(timestampLow);
    WriteTimestampPreset1Register(timestampHigh);
    return ReadTimestampPreset();
}
DTCLib::DTC_Timestamp DTCLib::DTC::ReadTimestampPreset()
{
    uint32_t timestampLow = ReadTimestampPreset0Register();
    DTC_Timestamp output;
    output.SetTimestamp(timestampLow, static_cast<uint16_t>(ReadTimestampPreset1Register()));
    return output;
}

bool DTCLib::DTC::ReadFPGAPROMProgramFIFOFull()
{
    std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
    return dataSet[1];
}
bool DTCLib::DTC::ReadFPGAPROMReady()
{
    std::bitset<32> dataSet = ReadFPGAPROMProgramStatusRegister();
    return dataSet[0];
}

void DTCLib::DTC::ReloadFPGAFirmware()
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
bool DTCLib::DTC::ReadFPGACoreAccessFIFOFull()
{
    std::bitset<32> dataSet = ReadFPGACoreAccessRegister();
    return dataSet[0];
}

//
// PCIe/DMA Status and Performance
// DMA Testing Engine
//
DTCLib::DTC_TestMode DTCLib::DTC::StartTest(const DTC_DMA_Engine& dma, int packetSize, bool loopback, bool txChecker, bool rxGenerator)
{
    DTC_TestCommand testCommand(dma, true, packetSize, loopback, txChecker, rxGenerator);
    WriteTestCommand(testCommand, true);
    return ReadTestCommand().GetMode();
}
DTCLib::DTC_TestMode DTCLib::DTC::StopTest(const DTC_DMA_Engine& dma)
{
    WriteTestCommand(DTC_TestCommand(dma), false);
    return ReadTestCommand().GetMode();
}

DTCLib::DTC_DMAState DTCLib::DTC::ReadDMAState(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir)
{
    m_ioc_engstate_t state;
    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_dma_state(dma, dir, &state);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw DTC_IOErrorException();
    }

    return DTC_DMAState(state);
}
DTCLib::DTC_DMAStats DTCLib::DTC::ReadDMAStats(const DTC_DMA_Engine& dma, const DTC_DMA_Direction& dir)
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
        throw DTC_IOErrorException();
    }

    return DTC_DMAStats(stats).getData(dma, dir);
}

DTCLib::DTC_PCIeState DTCLib::DTC::ReadPCIeState()
{
    m_ioc_pcistate_t state;
    int errorCode = 0;
    int retry = 3;
    do {
        errorCode = device_.read_pcie_state(&state);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0) { throw DTC_IOErrorException(); }
    return DTC_PCIeState(state);
}
DTCLib::DTC_PCIeStat DTCLib::DTC::ReadPCIeStats()
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
    if (errorCode != 0) { throw DTC_IOErrorException(); }
    return DTC_PCIeStat(statData[0]);
}

//
// Private Functions.
//
DTCLib::DTC_DataPacket DTCLib::DTC::ReadBuffer(const DTC_DMA_Engine& channel)
{
    mu2e_databuff_t* buffer_ = channel == DTC_DMA_Engine_DAQ ? daqbuffer_ : dcsbuffer_;
    int retry = 3;
    int errorCode = 0;
    do {
        TRACE(19, "DTC::ReadBuffer before device_.read_data");
        errorCode = device_.read_data(channel, (void**)&buffer_, 1000);
        retry--;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw DTC_IOErrorException();
    }
    TRACE(16, "DTC::ReadDataPacket buffer_=%p errorCode=%d", (void*)buffer_, errorCode);
    if (channel == DTC_DMA_Engine_DAQ) { daqbuffer_ = buffer_; }
    else if (channel == DTC_DMA_Engine_DCS) { dcsbuffer_ = buffer_; }
    return DTC_DataPacket(buffer_);
}
void DTCLib::DTC::WriteDataPacket(const DTC_DMA_Engine& channel, const DTC_DataPacket& packet)
{
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.write_loopback_data(channel, packet.GetData(), 16 * sizeof(uint8_t));
        retry--;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw DTC_IOErrorException();
    }
}
void DTCLib::DTC::WriteDMAPacket(const DTC_DMA_Engine& channel, const DTC_DMAPacket& packet)
{
    return WriteDataPacket(channel, packet.ConvertToDataPacket());
}

void DTCLib::DTC::WriteRegister(uint32_t data, const DTC_Register& address)
{
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.write_register(address, 100, data);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new DTC_IOErrorException();
    }
}
uint32_t DTCLib::DTC::ReadRegister(const DTC_Register& address)
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
        throw new DTC_IOErrorException();
    }

    return data;
}

bool DTCLib::DTC::ReadSERDESResetDone(const DTC_Ring_ID& ring)
{
    std::bitset<32> dataSet = ReadSERDESResetDoneRegister();
    return dataSet[ring];
}


void DTCLib::DTC::WriteTestCommand(const DTC_TestCommand& comm, bool start)
{
    int retry = 3;
    int errorCode = 0;
    do {
        errorCode = device_.write_test_command(comm.GetCommand(), start);
        --retry;
    } while (retry > 0 && errorCode != 0);
    if (errorCode != 0)
    {
        throw new DTC_IOErrorException();
    }
}
DTCLib::DTC_TestCommand DTCLib::DTC::ReadTestCommand()
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
        throw new DTC_IOErrorException;
    }
    return DTC_TestCommand(comm);
}
