#ifndef DTC_TYPES_H
#define DTC_TYPES_H

#include <bitset> // std::bitset
#include <cstdint> // uint8_t, uint16_t
#include <vector> // std::vector
#include <iostream> //std::ostream
#ifndef _WIN32
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#else
#include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#endif

namespace DTCLib
{
    const std::string ExpectedDesignVersion = "v1.4_2015-07-01-00";

    enum DTC_Register : uint16_t {
        DTC_Register_DesignVersion = 0x9000,
        DTC_Register_DesignDate = 0x9004,
        DTC_Register_DTCControl = 0x9100,
        DTC_Register_DMATransferLength = 0x9104,
        DTC_Register_SERDESLoopbackEnable = 0x9108,
        DTC_Register_SERDESLoopbackEnable_Temp = 0x9168,
        DTC_Register_SERDESOscillatorStatus = 0x910C,
        DTC_Register_ROCEmulationEnable = 0x9110,
        DTC_Register_RingEnable = 0x9114,
        DTC_Register_SERDESReset = 0x9118,
        DTC_Register_SERDESRXDisparityError = 0x911C,
        DTC_Register_SERDESRXCharacterNotInTableError = 0x9120,
        DTC_Register_SERDESUnlockError = 0x9124,
        DTC_Register_SERDESPLLLocked = 0x9128,
        DTC_Register_SERDESTXBufferStatus = 0x912C,
        DTC_Register_SERDESRXBufferStatus = 0x9130,
        DTC_Register_SERDESRXStatus = 0x9134,
        DTC_Register_SERDESResetDone = 0x9138,
        DTC_Register_SERDESEyescanData = 0x913C,
        DTC_Register_SERDESRXCDRLock = 0x9140,
        DTC_Register_DMATimeoutPreset = 0x9144,
        DTC_Register_ROCReplyTimeout = 0x9148,
        DTC_Register_ROCTimeoutError = 0x914C,
        DTC_Register_ReceivePacketError = 0x9150,
        DTC_Register_TimestampPreset0 = 0x9180,
        DTC_Register_TimestampPreset1 = 0x9184,
        DTC_Register_DataPendingTimer = 0x9188,
        DTC_Register_NUMROCs = 0x918C,
        DTC_Register_FIFOFullErrorFlag0 = 0x9190,
        DTC_Register_FIFOFullErrorFlag1 = 0x9194,
        DTC_Register_FIFOFullErrorFlag2 = 0x9198,
        DTC_Register_PacketSize = 0x9204,
        DTC_Register_FPGAPROMProgramStatus = 0x9404,
        DTC_Register_FPGACoreAccess = 0x9408,
        DTC_Register_Invalid,
    };
    static const std::vector<DTC_Register> DTC_Registers = { DTC_Register_DesignVersion, DTC_Register_DesignDate,
        DTC_Register_DTCControl, DTC_Register_DMATransferLength, DTC_Register_SERDESLoopbackEnable,
        DTC_Register_SERDESLoopbackEnable_Temp, DTC_Register_SERDESOscillatorStatus, DTC_Register_ROCEmulationEnable,
        DTC_Register_RingEnable, DTC_Register_SERDESReset, DTC_Register_SERDESRXDisparityError,
        DTC_Register_SERDESRXCharacterNotInTableError, DTC_Register_SERDESUnlockError, DTC_Register_SERDESPLLLocked,
        DTC_Register_SERDESTXBufferStatus, DTC_Register_SERDESRXBufferStatus, DTC_Register_SERDESRXStatus,
        DTC_Register_SERDESResetDone, DTC_Register_SERDESEyescanData, DTC_Register_SERDESRXCDRLock,
        DTC_Register_DMATimeoutPreset, DTC_Register_ROCReplyTimeout, DTC_Register_ROCTimeoutError, 
        DTC_Register_ReceivePacketError, DTC_Register_TimestampPreset0, DTC_Register_TimestampPreset1,
        DTC_Register_DataPendingTimer, DTC_Register_NUMROCs, DTC_Register_FIFOFullErrorFlag0, 
        DTC_Register_FIFOFullErrorFlag1, DTC_Register_FIFOFullErrorFlag2, DTC_Register_PacketSize, 
        DTC_Register_FPGAPROMProgramStatus, DTC_Register_FPGACoreAccess };

    enum DTC_Ring_ID : uint8_t {
        DTC_Ring_0 = 0,
        DTC_Ring_1 = 1,
        DTC_Ring_2 = 2,
        DTC_Ring_3 = 3,
        DTC_Ring_4 = 4,
        DTC_Ring_5 = 5,
        DTC_Ring_CFO = 6,
        DTC_Ring_Unused,
    };
    static const std::vector<DTC_Ring_ID> DTC_Rings = { DTC_Ring_0, DTC_Ring_1, DTC_Ring_2, DTC_Ring_3, DTC_Ring_4, DTC_Ring_5 };

    enum DTC_PacketType : uint8_t {
        DTC_PacketType_DCSRequest = 0,
        DTC_PacketType_ReadoutRequest = 1,
        DTC_PacketType_DataRequest = 2,
        DTC_PacketType_DCSReply = 4,
        DTC_PacketType_DataHeader = 5,
        DTC_PacketType_Invalid = 0x10,
    };

    enum DTC_ROC_ID : uint8_t {
        DTC_ROC_0 = 0,
        DTC_ROC_1 = 1,
        DTC_ROC_2 = 2,
        DTC_ROC_3 = 3,
        DTC_ROC_4 = 4,
        DTC_ROC_5 = 5,
        DTC_ROC_Unused,
    };
    static const std::vector<DTC_ROC_ID> DTC_ROCS = { DTC_ROC_Unused, DTC_ROC_0, DTC_ROC_1, DTC_ROC_2, DTC_ROC_3, DTC_ROC_4, DTC_ROC_5 };

    enum DTC_RXBufferStatus {
        DTC_RXBufferStatus_Nominal = 0,
        DTC_RXBufferStatus_BufferEmpty = 1,
        DTC_RXBufferStatus_BufferFull = 2,
        DTC_RXBufferStatus_Underflow = 5,
        DTC_RXBufferStatus_Overflow = 6,
        DTC_RXBufferStatus_Unknown = 0x10,
    };
    struct DTC_RXBufferStatusConverter {
    public:
        DTC_RXBufferStatus status_;
        DTC_RXBufferStatusConverter(DTC_RXBufferStatus status) : status_(status) {}
        std::string toString()
        {
            switch (status_)
            {
            case DTC_RXBufferStatus_Unknown:
            default:
                return "Unknown";
                break;
            case DTC_RXBufferStatus_Nominal:
                return "Nominal";
                break;
            case DTC_RXBufferStatus_BufferEmpty:
                return "BufferEmpty";
                break;
            case DTC_RXBufferStatus_BufferFull:
                return "BufferFull";
                break;
            case DTC_RXBufferStatus_Overflow:
                return "Overflow";
                break;
            case DTC_RXBufferStatus_Underflow:
                return "Underflow";
                break;
            }
        }
        friend std::ostream& operator<<(std::ostream& stream, const DTC_RXBufferStatusConverter& status) {
            switch (status.status_)
            {
            case DTC_RXBufferStatus_Unknown:
            default:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case DTC_RXBufferStatus_Nominal:
                stream << "{\"Nominal\":1,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case DTC_RXBufferStatus_BufferEmpty:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":1,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case DTC_RXBufferStatus_BufferFull:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":1,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case DTC_RXBufferStatus_Overflow:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":1,";
                stream << "\"Overflow\":0}";
                break;
            case DTC_RXBufferStatus_Underflow:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":1}";
                break;
            }
            return stream;
        }
    };

    enum DTC_RXStatus {
        DTC_RXStatus_DataOK = 0,
        DTC_RXStatus_SKPAdded = 1,
        DTC_RXStatus_SKPRemoved = 2,
        DTC_RXStatus_ReceiverDetected = 3,
        DTC_RXStatus_DecodeError = 4,
        DTC_RXStatus_ElasticOverflow = 5,
        DTC_RXStatus_ElasticUnderflow = 6,
        DTC_RXStatus_RXDisparityError = 7,
    };
    struct DTC_RXStatusConverter {
    public:
        DTC_RXStatus status_;
        DTC_RXStatusConverter(DTC_RXStatus status) : status_(status) {}
        std::string toString() {
            switch (status_)
            {
            case DTC_RXStatus_DataOK:
                return "DataOK";
            case DTC_RXStatus_SKPAdded:
                return "SKPAdded";
            case DTC_RXStatus_SKPRemoved:
                return "SKPRemoved";
            case DTC_RXStatus_ReceiverDetected:
                return "ReceiverDetected";
            case DTC_RXStatus_DecodeError:
                return "DecodeErr";
            case DTC_RXStatus_ElasticOverflow:
                return "ElasticOF";
            case DTC_RXStatus_ElasticUnderflow:
                return "ElasticUF";
            case DTC_RXStatus_RXDisparityError:
                return "RXDisparity";
            }
            return "Unknown";
        }
        friend std::ostream& operator<<(std::ostream& stream, const DTC_RXStatusConverter& status) {
            switch (status.status_)
            {
            default:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_DataOK:
                stream << "{\"DataOK\":1,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_SKPAdded:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":1,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_SKPRemoved:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":1,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_ReceiverDetected:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":1,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_DecodeError:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":1,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_ElasticOverflow:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":1,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_ElasticUnderflow:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":1,";
                stream << "\"DisparityError\":0}";
                break;
            case DTC_RXStatus_RXDisparityError:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":1}";
                break;
            }
            return stream;
        }
    };

    enum DTC_SERDESLoopbackMode {
        DTC_SERDESLoopbackMode_Disabled = 0,
        DTC_SERDESLoopbackMode_NearPCS = 1,
        DTC_SERDESLoopbackMode_NearPMA = 2,
        DTC_SERDESLoopbackMode_FarPMA = 4,
        DTC_SERDESLoopbackMode_FarPCS = 6,
    };
    struct DTC_SERDESLoopbackModeConverter {
    public:
        DTC_SERDESLoopbackMode mode_;
        DTC_SERDESLoopbackModeConverter(DTC_SERDESLoopbackMode mode) : mode_(mode) {}
        std::string toString() {
            switch (mode_)
            {
            case DTC_SERDESLoopbackMode_Disabled:
                return "Disabled";
            case DTC_SERDESLoopbackMode_NearPCS:
                return "NearPCS";
            case DTC_SERDESLoopbackMode_NearPMA:
                return "NearPMA";
            case DTC_SERDESLoopbackMode_FarPMA:
                return "FarPMA";
            case DTC_SERDESLoopbackMode_FarPCS:
                return "FarPCS";
            }
            return "Unknown";
        }
        friend std::ostream& operator<<(std::ostream& stream, const DTC_SERDESLoopbackModeConverter& mode) {
            switch (mode.mode_)
            {
            case DTC_SERDESLoopbackMode_Disabled:
            default:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case DTC_SERDESLoopbackMode_NearPCS:
                stream << "{\"NEPCS\":1,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case DTC_SERDESLoopbackMode_NearPMA:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":1,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case DTC_SERDESLoopbackMode_FarPMA:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":1,";
                stream << "\"FEPCS\":0}";
                break;
            case DTC_SERDESLoopbackMode_FarPCS:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":1}";
                break;
            }
            return stream;
        }
    };

    enum DTC_DataStatus {
        DTC_DataStatus_Valid = 0,
        DTC_DataStatus_NoValid = 1,
        DTC_DataStatus_Invalid = 2,
    };

    enum DTC_SimMode {
        DTC_SimMode_Disabled = 0,
        DTC_SimMode_Tracker = 1,
        DTC_SimMode_Calorimeter = 2,
        DTC_SimMode_CosmicVeto = 3,
        DTC_SimMode_Hardware = 4,
        DTC_SimMode_Performance = 5,
    };
    struct DTC_SimModeConverter {
    public:
        DTC_SimMode mode_;
        DTC_SimModeConverter(DTC_SimMode mode) : mode_(mode) {}
        static DTC_SimMode ConvertToSimMode(std::string);
        std::string toString() {
            switch (mode_)
            {
            case DTC_SimMode_Disabled:
            default:
                return "Disabled";
            case DTC_SimMode_Tracker:
                return "Tracker";
            case DTC_SimMode_Calorimeter:
                return "Calorimeter";
            case DTC_SimMode_CosmicVeto:
                return "CosmicVeto";
            case DTC_SimMode_Hardware:
                return "Hardware";
            case DTC_SimMode_Performance:
                return "Performance";
            }
        }
        friend std::ostream& operator<<(std::ostream& stream, const DTC_SimModeConverter& mode) {
            switch (mode.mode_)
            {
            case DTC_SimMode_Disabled:
            default:
                stream << "{\"Disabled\":1,";
                stream << "\"Tracker\":0,";
                stream << "\"Calorimeter\":0,";
                stream << "\"CosmicVeto\":0,";
                stream << "\"Hardware\":0,";
                stream << "\"Performance\":0}";
                break;
            case DTC_SimMode_Tracker:
                stream << "{\"Disabled\":0,";
                stream << "\"Tracker\":1,";
                stream << "\"Calorimeter\":0,";
                stream << "\"CosmicVeto\":0,";
                stream << "\"Hardware\":0,";
                stream << "\"Performance\":0}";
                break;
            case DTC_SimMode_Calorimeter:
                stream << "{\"Disabled\":0,";
                stream << "\"Tracker\":0,";
                stream << "\"Calorimeter\":1,";
                stream << "\"CosmicVeto\":0,";
                stream << "\"Hardware\":0,";
                stream << "\"Performance\":0}";
                break;
            case DTC_SimMode_CosmicVeto:
                stream << "{\"Disabled\":0,";
                stream << "\"Tracker\":0,";
                stream << "\"Calorimeter\":0,";
                stream << "\"CosmicVeto\":1,";
                stream << "\"Hardware\":0,";
                stream << "\"Performance\":0}";
                break;
            case DTC_SimMode_Hardware:
                stream << "{\"Disabled\":0,";
                stream << "\"Tracker\":0,";
                stream << "\"Calorimeter\":0,";
                stream << "\"CosmicVeto\":0,";
                stream << "\"Hardware\":1,";
                stream << "\"Performance\":0}";
                break;
            case DTC_SimMode_Performance:
                stream << "{\"Disabled\":0,";
                stream << "\"Tracker\":0,";
                stream << "\"Calorimeter\":0,";
                stream << "\"CosmicVeto\":0,";
                stream << "\"Hardware\":0,";
                stream << "\"Performance\":1}";
                break;
            }
            return stream;
        }

    };

    class DTC_WrongPacketTypeException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "Unexpected packet type encountered!";
        }
    };
    class DTC_IOErrorException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "Unable to communicate with the DTC";
        }
    };
    class DTC_DataCorruptionException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "Corruption detected in data stream from DTC";
        }
    };
    class DTC_TimeoutOccurredException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "A Timeout occurred while communicating with the DTC";
        }
    };

    class DTC_Timestamp {
    private:
        uint64_t timestamp_ : 48;
    public:
        DTC_Timestamp();
        DTC_Timestamp(uint64_t timestamp);
        DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh);
        DTC_Timestamp(uint8_t* timeArr, int offset = 0);
        DTC_Timestamp(std::bitset<48> timestamp);
        DTC_Timestamp(const DTC_Timestamp&) = default;
#ifndef _WIN32
        DTC_Timestamp(DTC_Timestamp&&) = default;
#endif

        virtual ~DTC_Timestamp() = default;
#ifndef _WIN32
        DTC_Timestamp& operator=(DTC_Timestamp&&) = default;
#endif
        DTC_Timestamp& operator=(const DTC_Timestamp&) = default;

        bool operator==(const DTC_Timestamp r) { return r.GetTimestamp(true) == timestamp_; }
        bool operator!=(const DTC_Timestamp r) { return r.GetTimestamp(true) != timestamp_; }
        bool operator< (const DTC_Timestamp r) { return r.GetTimestamp(true) > timestamp_; }
        DTC_Timestamp operator+(const int r) { return DTC_Timestamp(r + timestamp_); }

        void SetTimestamp(uint64_t timestamp) { timestamp_ = timestamp & 0x0000FFFFFFFFFFFF; }
        void SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh);
        std::bitset<48> GetTimestamp() const { return timestamp_; }
        uint64_t GetTimestamp(bool dummy) const { if (dummy) { return timestamp_; } else return 0; }
        void GetTimestamp(uint8_t* timeArr, int offset = 0) const;
        std::string toJSON(bool arrayMode = false);
        std::string toPacketFormat();
    };

    class DTC_DataPacket {
        friend class DTC;
        friend class DTC_DMAPacket;
        friend class DTC_ReadoutRequestPacket;
        friend class DTC_DataRequestPacket;
        friend class DTC_DCSRequestPacket;
        friend class DTC_DCSReplyPacket;
        friend class DTC_DataHeaderPacket;
    private:
        uint8_t* dataPtr_;
        uint16_t dataSize_;
        bool memPacket_;

    public:
        DTC_DataPacket();
        DTC_DataPacket(mu2e_databuff_t* data) : dataPtr_(*data), dataSize_(16), memPacket_(true){}
        DTC_DataPacket(void* data) : dataPtr_((uint8_t*)data), dataSize_(16), memPacket_(true){}
        DTC_DataPacket(uint8_t* data) : dataPtr_(data), dataSize_(16), memPacket_(true){}
        DTC_DataPacket(const DTC_DataPacket&);
#ifndef _WIN32
        DTC_DataPacket(DTC_DataPacket&&) = default;
#endif

        virtual ~DTC_DataPacket();

        DTC_DataPacket& operator=(const DTC_DataPacket&) = default;
#ifndef _WIN32
        DTC_DataPacket& operator=(DTC_DataPacket&&) = default;
#endif

        void SetWord(uint16_t index, uint8_t data);
        uint8_t GetWord(uint16_t index) const;
        std::string toJSON();
        std::string toPacketFormat();
        bool Resize(const uint16_t dmaSize);
        uint16_t GetSize() const { return dataSize_; }
        bool IsMemoryPacket() const { return memPacket_; }

    protected:
        uint8_t* GetData() const { return dataPtr_; }
    };

    class DTC_DMAPacket {
    protected:
        bool           valid_;
        uint16_t       byteCount_;
        DTC_Ring_ID    ringID_;
        DTC_PacketType packetType_;
        DTC_ROC_ID     rocID_;

    public:
        DTC_DMAPacket() : packetType_(DTC_PacketType_Invalid) {}
        DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint16_t byteCount = 64, bool valid = true);

        DTC_DMAPacket(const DTC_DataPacket in);
        DTC_DMAPacket(const DTC_DMAPacket&) = default;
#ifndef _WIN32
        DTC_DMAPacket(DTC_DMAPacket&&) = default;
#endif

        virtual ~DTC_DMAPacket() = default;

        DTC_DMAPacket& operator=(const DTC_DMAPacket&) = default;
#ifndef _WIN32
        DTC_DMAPacket& operator=(DTC_DMAPacket&&) = default;
#endif

        virtual DTC_DataPacket ConvertToDataPacket() const;

        DTC_PacketType GetPacketType() { return packetType_; }

        std::string headerJSON();
        std::string headerPacketFormat();
        uint16_t GetByteCount() { return byteCount_; }
        DTC_Ring_ID GetRingID() { return ringID_; }
        virtual std::string toPacketFormat();
        virtual std::string toJSON();
        friend std::ostream& operator<<(std::ostream& stream, DTC_DMAPacket& packet) {
            stream << packet.toJSON();
            return stream;
        }
    };

    class DTC_DCSRequestPacket : public DTC_DMAPacket {
    private:
        uint8_t data_[12];
    public:
        DTC_DCSRequestPacket();
        DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc);
        DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t* data);
        DTC_DCSRequestPacket(const DTC_DCSRequestPacket&) = default;
#ifndef _WIN32
        DTC_DCSRequestPacket(DTC_DCSRequestPacket&&) = default;
#endif
        DTC_DCSRequestPacket(DTC_DataPacket in);

        DTC_DCSRequestPacket& operator=(const DTC_DCSRequestPacket&) = default;
#ifndef _WIN32
        DTC_DCSRequestPacket& operator=(DTC_DCSRequestPacket&&) = default;
#endif

        virtual ~DTC_DCSRequestPacket() = default;

        uint8_t* GetData() { return data_; }
        DTC_DataPacket ConvertToDataPacket() const;
        std::string toJSON();
        std::string toPacketFormat();
    };

    class DTC_ReadoutRequestPacket : public DTC_DMAPacket {
    private:
        DTC_Timestamp timestamp_;
        bool debug_;
        uint8_t request_[4];
    public:
        DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC = DTC_ROC_5, bool debug = true);
        DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC = DTC_ROC_5, bool debug = true, uint8_t* request = nullptr);
        DTC_ReadoutRequestPacket(const DTC_ReadoutRequestPacket& right) = default;
#ifndef _WIN32
        DTC_ReadoutRequestPacket(DTC_ReadoutRequestPacket&& right) = default;
#endif
        DTC_ReadoutRequestPacket(DTC_DataPacket in);

        virtual ~DTC_ReadoutRequestPacket() = default;

        bool GetDebug() { return debug_; }
        DTC_Timestamp GetTimestamp() { return timestamp_; }
        virtual uint8_t* GetData() { return request_; }
        DTC_DataPacket ConvertToDataPacket() const;
        std::string toJSON();
        std::string toPacketFormat();
    };

    class DTC_DataRequestPacket : public DTC_DMAPacket {
    private:
        DTC_Timestamp timestamp_;
        bool debug_;
        uint16_t debugPacketCount_;
    public:
        DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, bool debug = true, uint16_t debugPacketCount = 0);
        DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp, bool debug = true, uint16_t debugPacketCount = 0);
        DTC_DataRequestPacket(const DTC_DataRequestPacket&) = default;
#ifndef _WIN32
        DTC_DataRequestPacket(DTC_DataRequestPacket&&) = default;
#endif
        DTC_DataRequestPacket(DTC_DataPacket in);

        bool GetDebug() { return debug_; }
        uint16_t GetDebugPacketCount() { return debugPacketCount_; }
        void SetDebugPacketCount(uint16_t count);
        DTC_Timestamp GetTimestamp() { return timestamp_; }
        DTC_DataPacket ConvertToDataPacket() const;
        std::string toJSON();
        std::string toPacketFormat();
    };

    class DTC_DCSReplyPacket : public DTC_DMAPacket {
    private:
        uint8_t data_[12];
    public:
        DTC_DCSReplyPacket(DTC_Ring_ID ring);
        DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t* data);
        DTC_DCSReplyPacket(const DTC_DCSReplyPacket&) = default;
#ifndef _WIN32
        DTC_DCSReplyPacket(DTC_DCSReplyPacket&&) = default;
#endif
        DTC_DCSReplyPacket(DTC_DataPacket in);

        uint8_t* GetData() { return data_; }
        DTC_DataPacket ConvertToDataPacket() const;
        std::string toJSON();
        std::string toPacketFormat();
    };

    class DTC_DataHeaderPacket : public DTC_DMAPacket {
    private:
        uint16_t packetCount_;
        DTC_Timestamp timestamp_;
        uint8_t dataStart_[3];
        DTC_DataStatus status_;

    public:
        DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status);
        DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp);
        DTC_DataHeaderPacket(DTC_Ring_ID ring, uint16_t packetCount, DTC_DataStatus status, DTC_Timestamp timestamp, uint8_t* data);
        DTC_DataHeaderPacket(const DTC_DataHeaderPacket&) = default;
#ifndef _WIN32
        DTC_DataHeaderPacket(DTC_DataHeaderPacket&&) = default;
#endif
        DTC_DataHeaderPacket(DTC_DataPacket in);

        DTC_DataPacket ConvertToDataPacket() const;
        virtual uint8_t* GetData() { return dataStart_; }
        uint16_t GetPacketCount() { return packetCount_; }
        DTC_Timestamp GetTimestamp() { return timestamp_; }
        DTC_DataStatus GetStatus() { return status_; }
        std::string toJSON();
        std::string toPacketFormat();
    };

    class DTC_SERDESRXDisparityError {
    private:
        std::bitset<2> data_;

    public:
        DTC_SERDESRXDisparityError();
        DTC_SERDESRXDisparityError(std::bitset<2> data);
        DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring);
        DTC_SERDESRXDisparityError(const DTC_SERDESRXDisparityError&) = default;
#ifndef _WIN32
        DTC_SERDESRXDisparityError(DTC_SERDESRXDisparityError&&) = default;
#endif

        DTC_SERDESRXDisparityError& operator=(const DTC_SERDESRXDisparityError&) = default;
#ifndef _WIN32
        DTC_SERDESRXDisparityError& operator=(DTC_SERDESRXDisparityError&&) = default;
#endif

        void SetData(std::bitset<2> data) { data_ = data; }
        std::bitset<2> GetData() { return data_; }
        int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
        friend std::ostream& operator<<(std::ostream& stream, DTC_SERDESRXDisparityError error) {
            stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
            return stream;
        }
    };

    class DTC_CharacterNotInTableError {
    private:
        std::bitset<2> data_;

    public:
        DTC_CharacterNotInTableError();
        DTC_CharacterNotInTableError(std::bitset<2> data);
        DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring);
        DTC_CharacterNotInTableError(const DTC_CharacterNotInTableError&) = default;
#ifndef _WIN32
        DTC_CharacterNotInTableError(DTC_CharacterNotInTableError&&) = default;
#endif

        DTC_CharacterNotInTableError& operator=(const DTC_CharacterNotInTableError&) = default;
#ifndef _WIN32
        DTC_CharacterNotInTableError& operator=(DTC_CharacterNotInTableError&&) = default;
#endif

        void SetData(std::bitset<2> data) { data_ = data; }
        std::bitset<2> GetData() { return data_; }
        int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
        friend std::ostream& operator<<(std::ostream& stream, DTC_CharacterNotInTableError error) {
            stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
            return stream;
        }
    };

    struct DTC_TestMode {
    public:
        bool loopbackEnabled;
        bool txChecker;
        bool rxGenerator;
        bool state_;
    public:
        DTC_TestMode();
        DTC_TestMode(bool state, bool loopback, bool txChecker, bool rxGenerator);
        DTC_TestMode(uint32_t input);
        bool GetLoopbackState() { return loopbackEnabled; }
        bool GetTXCheckerState() { return txChecker; }
        bool GetRXGeneratorState() { return rxGenerator; }
        bool GetState() { return state_; }
        uint32_t GetWord() const;
        std::string toString();
    };

    struct DTC_TestCommand {
    public:
        DTC_TestMode TestMode;
        int PacketSize;
        DTC_DMA_Engine Engine;
        DTC_TestCommand();
        DTC_TestCommand(DTC_DMA_Engine dma, bool state = false, int PacketSize = 0, bool loopback = false, bool txChecker = false, bool rxGenerator = false);
        DTC_TestCommand(m_ioc_cmd_t in);
        m_ioc_cmd_t GetCommand() const;
        DTC_TestMode GetMode() { return TestMode; }

    };

    struct DTC_DMAState {
    public:
        DTC_DMA_Engine Engine;
        DTC_DMA_Direction Direction;
        int BDs;                    /**< Total Number of BDs */
        int Buffers;                /**< Total Number of buffers */
        uint32_t MinPktSize;        /**< Minimum packet size */
        uint32_t MaxPktSize;        /**< Maximum packet size */
        int BDerrs;                 /**< Total BD errors */
        int BDSerrs;                /**< Total BD short errors - only TX BDs */
        int IntEnab;                /**< Interrupts enabled or not */
        DTC_TestMode TestMode;
        DTC_DMAState() {}
        DTC_DMAState(m_ioc_engstate_t in);
        std::string toString();
    };

    struct DTC_DMAStat
    {
    public:
        DTC_DMA_Engine Engine;
        DTC_DMA_Direction Direction;
        uint32_t LBR;           /**< Last Byte Rate */
        uint32_t LAT;           /**< Last Active Time */
        uint32_t LWT;           /**< Last Wait Time */
        DTC_DMAStat() : Engine(DTC_DMA_Engine_Invalid), Direction(DTC_DMA_Direction_Invalid), LBR(0), LAT(0), LWT(0) {}
        DTC_DMAStat(DMAStatistics in);
        std::string toString();
    };
    struct DTC_DMAStats {
    public:
        std::vector<DTC_DMAStat> Stats;
        DTC_DMAStats() {}
        DTC_DMAStats(m_ioc_engstats_t in);
        DTC_DMAStats getData(DTC_DMA_Engine dma, DTC_DMA_Direction dir);
        void addStat(DTC_DMAStat in) { Stats.push_back(in); }
        size_t size() { return Stats.size(); }
        DTC_DMAStat at(int index) { return Stats.at(index); }
    };

    struct DTC_PCIeState {
    public:
        uint32_t Version;       /**< Hardware design version info */
        bool LinkState;              /**< Link State - up or down */
        int LinkSpeed;              /**< Link Speed */
        int LinkWidth;              /**< Link Width */
        uint32_t VendorId;     /**< Vendor ID */
        uint32_t DeviceId;    /**< Device ID */
        int IntMode;                /**< Legacy or MSI interrupts */
        int MPS;                    /**< Max Payload Size */
        int MRRS;                   /**< Max Read Request Size */
        int InitFCCplD;             /**< Initial FC Credits for Completion Data */
        int InitFCCplH;             /**< Initial FC Credits for Completion Header */
        int InitFCNPD;              /**< Initial FC Credits for Non-Posted Data */
        int InitFCNPH;              /**< Initial FC Credits for Non-Posted Data */
        int InitFCPD;               /**< Initial FC Credits for Posted Data */
        int InitFCPH;               /**< Initial FC Credits for Posted Data */
        DTC_PCIeState() {}
        DTC_PCIeState(m_ioc_pcistate_t in);
        std::string toString();
    };

    struct DTC_PCIeStat {
    public:
        uint32_t LTX;           /**< Last TX Byte Rate */
        uint32_t LRX;           /**< Last RX Byte Rate */
        DTC_PCIeStat() {}
        DTC_PCIeStat(TRNStatistics in);
    };

    struct DTC_RingEnableMode {
    public:
        bool TransmitEnable;
        bool ReceiveEnable;
        bool TimingEnable;
        DTC_RingEnableMode() : TransmitEnable(true), ReceiveEnable(true), TimingEnable(true) {}
        DTC_RingEnableMode(bool transmit, bool receive, bool timing) : TransmitEnable(transmit), ReceiveEnable(receive), TimingEnable(timing) {}
        friend std::ostream& operator<<(std::ostream& stream, const DTC_RingEnableMode& mode) {
            bool formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
            stream.setf(std::ios_base::boolalpha);
            stream << "{\"TransmitEnable\":" << mode.TransmitEnable << ",\"ReceiveEnable\":" << mode.ReceiveEnable << ",\"TimingEnable\":" << mode.TimingEnable << "}";
            if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
            return stream;
        }
        friend bool operator==(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right){ return (left.TransmitEnable == right.TransmitEnable) && (left.ReceiveEnable == right.ReceiveEnable) && (left.TimingEnable == right.TimingEnable); }
        friend bool operator!=(const DTC_RingEnableMode& left, const DTC_RingEnableMode& right){ return !(left == right); }
    };

    struct DTC_FIFOFullErrorFlags {
    public:
        bool OutputData;
        bool CFOLinkInput;
        bool ReadoutRequestOutput;
        bool DataRequestOutput;
        bool OtherOutput;
        bool OutputDCS;
        bool OutputDCSStage2;
        bool DataInput;
        bool DCSStatusInput;
        DTC_FIFOFullErrorFlags()
            : OutputData(false)
            , CFOLinkInput(false)
            , ReadoutRequestOutput(false)
            , DataRequestOutput(false)
            , OtherOutput(false)
            , OutputDCS(false)
            , OutputDCSStage2(false)
            , DataInput(false)
            , DCSStatusInput(false)
        {}
        DTC_FIFOFullErrorFlags(bool outputData, bool cfoLinkInput, bool readoutRequest, bool dataRequest,
            bool otherOutput, bool outputDCS, bool outputDCS2, bool dataInput, bool dcsInput)
            : OutputData(outputData)
            , CFOLinkInput(cfoLinkInput)
            , ReadoutRequestOutput(readoutRequest)
            , DataRequestOutput(dataRequest)
            , OtherOutput(otherOutput)
            , OutputDCS(outputDCS)
            , OutputDCSStage2(outputDCS2)
            , DataInput(dataInput)
            , DCSStatusInput(dcsInput)
        {}
        friend std::ostream& operator<<(std::ostream& stream, const DTC_FIFOFullErrorFlags& flags) {
            bool formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
            stream.setf(std::ios_base::boolalpha);
            stream << "{\"OutputData\":" << flags.OutputData
                << ",\"CFOLinkInput\":" << flags.CFOLinkInput
                << ",\"ReadoutRequestOutput\":" << flags.ReadoutRequestOutput
                << ",\"DataRequestOutput\":" << flags.DataRequestOutput
                << ",\"OtherOutput\":" << flags.OtherOutput
                << ",\"OutputDCS\":" << flags.OutputDCS
                << ",\"OutputDCSStage2\":" << flags.OutputDCSStage2
                << ",\"DataInput\":" << flags.DataInput
                << ",\"DCSStatusInput\":" << flags.DCSStatusInput << "}";
            if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
            return stream;
        }
    };

}

#endif //DTC_TYPES_H
