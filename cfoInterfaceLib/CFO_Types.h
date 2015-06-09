#ifndef CFO_TYPES_H
#define CFO_TYPES_H

#include <bitset> // std::bitset
#include <cstdint> // uint8_t, uint16_t
#include <vector> // std::vector
#include <iostream> //std::ostream
#ifndef _WIN32
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#else
#include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#endif

namespace CFOLib
{
    const std::string ExpectedDesignVersion = "v1.0_2015-05-12-00";

    enum CFO_Register : uint16_t {
        CFO_Register_DesignVersion = 0x9000,
        CFO_Register_DesignDate = 0x9004,
        CFO_Register_CFOControl = 0x9100,
        CFO_Register_DMATransferLength = 0x9104,
        CFO_Register_SERDESLoopbackEnable = 0x9108,
        CFO_Register_SERDESLoopbackEnable_Temp = 0x9168,
        CFO_Register_RingEnable = 0x9114,
        CFO_Register_SERDESReset = 0x9118,
        CFO_Register_SERDESRXDisparityError = 0x911C,
        CFO_Register_SERDESRXCharacterNotInTableError = 0x9120,
        CFO_Register_SERDESUnlockError = 0x9124,
        CFO_Register_SERDESPLLLocked = 0x9128,
        CFO_Register_SERDESTXBufferStatus = 0x912C,
        CFO_Register_SERDESRXBufferStatus = 0x9130,
        CFO_Register_SERDESRXStatus = 0x9134,
        CFO_Register_SERDESResetDone = 0x9138,
        CFO_Register_SERDESEyescanData = 0x913C,
        CFO_Register_SERDESRXCDRLock = 0x9140,
        CFO_Register_DMATimeoutPreset = 0x9144,
        CFO_Register_TimestampPreset0 = 0x9180,
        CFO_Register_TimestampPreset1 = 0x9184,
        CFO_Register_DataPendingTimer = 0x9188,
        CFO_Register_NUMCFOs = 0x918C,
        CFO_Register_FIFOFullErrorFlag0 = 0x9190,
        CFO_Register_FIFOFullErrorFlag1 = 0x9194,
        CFO_Register_FIFOFullErrorFlag2 = 0x9198,
        CFO_Register_PacketSize = 0x9204,
        CFO_Register_RRInfoTableSize = 0x9304,
        CFO_Register_FPGAPROMProgramStatus = 0x9404,
        CFO_Register_FPGACoreAccess = 0x9408,
        CFO_Register_Invalid,
    };
    static const std::vector<CFO_Register> CFO_Registers = { CFO_Register_DesignVersion, CFO_Register_DesignDate,
        CFO_Register_CFOControl, CFO_Register_DMATransferLength, CFO_Register_SERDESLoopbackEnable,
        CFO_Register_SERDESLoopbackEnable_Temp, CFO_Register_RingEnable, CFO_Register_SERDESReset,
        CFO_Register_SERDESRXDisparityError, CFO_Register_SERDESRXCharacterNotInTableError,
        CFO_Register_SERDESUnlockError, CFO_Register_SERDESPLLLocked, CFO_Register_SERDESTXBufferStatus, 
        CFO_Register_SERDESRXBufferStatus, CFO_Register_SERDESRXStatus, CFO_Register_SERDESResetDone, 
        CFO_Register_SERDESEyescanData, CFO_Register_SERDESRXCDRLock, CFO_Register_DMATimeoutPreset, 
        CFO_Register_TimestampPreset0, CFO_Register_TimestampPreset1, CFO_Register_DataPendingTimer, 
        CFO_Register_NUMCFOs, CFO_Register_FIFOFullErrorFlag0, CFO_Register_FIFOFullErrorFlag1, 
        CFO_Register_FIFOFullErrorFlag2, CFO_Register_PacketSize, CFO_Register_RRInfoTableSize, 
        CFO_Register_FPGAPROMProgramStatus, CFO_Register_FPGACoreAccess };

    enum CFO_Ring_ID : uint8_t {
        CFO_Ring_0 = 0,
        CFO_Ring_1 = 1,
        CFO_Ring_2 = 2,
        CFO_Ring_3 = 3,
        CFO_Ring_4 = 4,
        CFO_Ring_5 = 5,
        CFO_Ring_6 = 6,
        CFO_Ring_7 = 7,
        CFO_Ring_Unused,
    };
    static const std::vector<CFO_Ring_ID> CFO_Rings = { CFO_Ring_0, CFO_Ring_1, CFO_Ring_2, CFO_Ring_3, CFO_Ring_4, CFO_Ring_5, CFO_Ring_6, CFO_Ring_7 };
    static const std::vector<std::string> CFO_RingNames = { "Ring0", "Ring1", "Ring2", "Ring3", "Ring4", "Ring5", "Ring6", "Ring7" };

    enum CFO_RXBufferStatus {
        CFO_RXBufferStatus_Nominal = 0,
        CFO_RXBufferStatus_BufferEmpty = 1,
        CFO_RXBufferStatus_BufferFull = 2,
        CFO_RXBufferStatus_Underflow = 5,
        CFO_RXBufferStatus_Overflow = 6,
        CFO_RXBufferStatus_Unknown = 0x10,
    };
    struct CFO_RXBufferStatusConverter {
    public:
        CFO_RXBufferStatus status_;
        CFO_RXBufferStatusConverter(CFO_RXBufferStatus status) : status_(status) {}
        std::string toString()
        {
            switch (status_)
            {
            case CFO_RXBufferStatus_Unknown:
            default:
                return "Unknown";
                break;
            case CFO_RXBufferStatus_Nominal:
                return "Nominal";
                break;
            case CFO_RXBufferStatus_BufferEmpty:
                return "BufferEmpty";
                break;
            case CFO_RXBufferStatus_BufferFull:
                return "BufferFull";
                break;
            case CFO_RXBufferStatus_Overflow:
                return "Overflow";
                break;
            case CFO_RXBufferStatus_Underflow:
                return "Underflow";
                break;
            }
        }
        friend std::ostream& operator<<(std::ostream& stream, const CFO_RXBufferStatusConverter& status) {
            switch (status.status_)
            {
            case CFO_RXBufferStatus_Unknown:
            default:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case CFO_RXBufferStatus_Nominal:
                stream << "{\"Nominal\":1,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case CFO_RXBufferStatus_BufferEmpty:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":1,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case CFO_RXBufferStatus_BufferFull:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":1,";
                stream << "\"Underflow\":0,";
                stream << "\"Overflow\":0}";
                break;
            case CFO_RXBufferStatus_Overflow:
                stream << "{\"Nominal\":0,";
                stream << "\"Empty\":0,";
                stream << "\"Full\":0,";
                stream << "\"Underflow\":1,";
                stream << "\"Overflow\":0}";
                break;
            case CFO_RXBufferStatus_Underflow:
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

    enum CFO_RXStatus {
        CFO_RXStatus_DataOK = 0,
        CFO_RXStatus_SKPAdded = 1,
        CFO_RXStatus_SKPRemoved = 2,
        CFO_RXStatus_ReceiverDetected = 3,
        CFO_RXStatus_DecodeError = 4,
        CFO_RXStatus_ElasticOverflow = 5,
        CFO_RXStatus_ElasticUnderflow = 6,
        CFO_RXStatus_RXDisparityError = 7,
    };
    struct CFO_RXStatusConverter {
    public:
        CFO_RXStatus status_;
        CFO_RXStatusConverter(CFO_RXStatus status) : status_(status) {}
        std::string toString() {
            switch (status_)
            {
            case CFO_RXStatus_DataOK:
                return "DataOK";
            case CFO_RXStatus_SKPAdded:
                return "SKPAdded";
            case CFO_RXStatus_SKPRemoved:
                return "SKPRemoved";
            case CFO_RXStatus_ReceiverDetected:
                return "ReceiverDetected";
            case CFO_RXStatus_DecodeError:
                return "DecodeErr";
            case CFO_RXStatus_ElasticOverflow:
                return "ElasticOF";
            case CFO_RXStatus_ElasticUnderflow:
                return "ElasticUF";
            case CFO_RXStatus_RXDisparityError:
                return "RXDisparity";
            }
            return "Unknown";
        }
        friend std::ostream& operator<<(std::ostream& stream, const CFO_RXStatusConverter& status) {
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
            case CFO_RXStatus_DataOK:
                stream << "{\"DataOK\":1,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_SKPAdded:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":1,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_SKPRemoved:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":1,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_ReceiverDetected:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":1,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_DecodeError:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":1,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_ElasticOverflow:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":1,";
                stream << "\"EUnderflow\":0,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_ElasticUnderflow:
                stream << "{\"DataOK\":0,";
                stream << "\"SKPAdded\":0,";
                stream << "\"SKPRemoved\":0,";
                stream << "\"ReceiverDetected\":0,";
                stream << "\"DecodeError\":0,";
                stream << "\"EOverflow\":0,";
                stream << "\"EUnderflow\":1,";
                stream << "\"DisparityError\":0}";
                break;
            case CFO_RXStatus_RXDisparityError:
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

    enum CFO_SERDESLoopbackMode {
        CFO_SERDESLoopbackMode_Disabled = 0,
        CFO_SERDESLoopbackMode_NearPCS = 1,
        CFO_SERDESLoopbackMode_NearPMA = 2,
        CFO_SERDESLoopbackMode_FarPMA = 4,
        CFO_SERDESLoopbackMode_FarPCS = 6,
    };
    struct CFO_SERDESLoopbackModeConverter {
    public:
        CFO_SERDESLoopbackMode mode_;
        CFO_SERDESLoopbackModeConverter(CFO_SERDESLoopbackMode mode) : mode_(mode) {}
        std::string toString() {
            switch (mode_)
            {
            case CFO_SERDESLoopbackMode_Disabled:
                return "Disabled";
            case CFO_SERDESLoopbackMode_NearPCS:
                return "NearPCS";
            case CFO_SERDESLoopbackMode_NearPMA:
                return "NearPMA";
            case CFO_SERDESLoopbackMode_FarPMA:
                return "FarPMA";
            case CFO_SERDESLoopbackMode_FarPCS:
                return "FarPCS";
            }
            return "Unknown";
        }
        friend std::ostream& operator<<(std::ostream& stream, const CFO_SERDESLoopbackModeConverter& mode) {
            switch (mode.mode_)
            {
            case CFO_SERDESLoopbackMode_Disabled:
            default:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case CFO_SERDESLoopbackMode_NearPCS:
                stream << "{\"NEPCS\":1,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case CFO_SERDESLoopbackMode_NearPMA:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":1,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":0}";
                break;
            case CFO_SERDESLoopbackMode_FarPMA:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":1,";
                stream << "\"FEPCS\":0}";
                break;
            case CFO_SERDESLoopbackMode_FarPCS:
                stream << "{\"NEPCS\":0,";
                stream << "\"NEMPA\":0,";
                stream << "\"FEPMA\":0,";
                stream << "\"FEPCS\":1}";
                break;
            }
            return stream;
        }
    };

    enum CFO_DataStatus {
        CFO_DataStatus_Valid = 0,
        CFO_DataStatus_NoValid = 1,
        CFO_DataStatus_Invalid = 2,
    };

    enum CFO_SimMode {
        CFO_SimMode_Disabled = 0,
        CFO_SimMode_Enabled = 1,
    };
    struct CFO_SimModeConverter {
    public:
        CFO_SimMode mode_;
        CFO_SimModeConverter(CFO_SimMode mode) : mode_(mode) {}
        static CFO_SimMode ConvertToSimMode(std::string);
        friend std::ostream& operator<<(std::ostream& stream, const CFO_SimModeConverter& mode) {
            switch (mode.mode_)
            {
            case CFO_SimMode_Disabled:
            default:
                stream << "false";
                break;
            case CFO_SimMode_Enabled:
                stream << "true";
                break;
            }
            return stream;
        }

    };

    class CFO_IOErrorException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "Unable to communicate with the CFO";
        }
    };
    class CFO_NotImplementedException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "I'm sorry, Dave, but I can't do that. (Because I don't know how)";
        }
    };
    class CFO_TimeoutOccurredException : public std::exception {
    public:
        virtual const char* what() const throw()
        {
            return "A Timeout occurred while communicating with the CFO";
        }
    };

    class CFO_Timestamp {
    private:
        uint64_t timestamp_ : 48;
    public:
        CFO_Timestamp();
        CFO_Timestamp(uint64_t timestamp);
        CFO_Timestamp(uint32_t timestampLow, uint16_t timestampHigh);
        CFO_Timestamp(uint8_t* timeArr);
        CFO_Timestamp(std::bitset<48> timestamp);
        CFO_Timestamp(const CFO_Timestamp&) = default;
#ifndef _WIN32
        CFO_Timestamp(CFO_Timestamp&&) = default;
#endif

        virtual ~CFO_Timestamp() = default;
#ifndef _WIN32
        CFO_Timestamp& operator=(CFO_Timestamp&&) = default;
#endif
        CFO_Timestamp& operator=(const CFO_Timestamp&) = default;

        bool operator==(const CFO_Timestamp r) { return r.GetTimestamp(true) == timestamp_; }
        bool operator!=(const CFO_Timestamp r) { return r.GetTimestamp(true) != timestamp_; }
        bool operator< (const CFO_Timestamp r) { return r.GetTimestamp(true) < timestamp_; }

        void SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh);
        std::bitset<48> GetTimestamp() const { return timestamp_; }
        uint64_t GetTimestamp(bool dummy) const { if (dummy) { return timestamp_; } else return 0; }
        void GetTimestamp(uint8_t* timeArr, int offset = 0) const;
        std::string toJSON(bool arrayMode = false);
        std::string toPacketFormat();
    };

    class CFO_ReadoutRequestPacket
    {
    private:
        bool valid_;
        CFO_Ring_ID ring_;
        uint8_t packetType_ = 1;
        int hopCount_;
        uint8_t request_[4];
        CFO_Timestamp timestamp_;
        bool debug_;

    public:
        CFO_ReadoutRequestPacket() : valid_(true), ring_(CFO_Ring_Unused), hopCount_(0), timestamp_(), debug_(true) {}
        CFO_ReadoutRequestPacket(CFO_Ring_ID ring, int hopCount, uint8_t* request, CFO_Timestamp ts_ = CFO_Timestamp(), bool debug = false);

        void setDebug(bool debug) { debug_ = debug; }
        bool getDebug() { return debug_; };

        uint8_t getRequestByte(int byte) { if (byte < 4 && byte >= 0) { return request_[byte]; } return 0; }
        void setRequestByte(int byte, uint8_t req) { if (byte < 4 && byte >= 0) { request_[byte] = req; } }

        CFO_Timestamp getTimestamp() { return timestamp_; }
        void setTimestamp(CFO_Timestamp ts) { timestamp_ = ts; }

        std::string toJSON();
        std::string toPacketFormat();

    };

    class CFO_SERDESRXDisparityError {
    private:
        std::bitset<2> data_;

    public:
        CFO_SERDESRXDisparityError();
        CFO_SERDESRXDisparityError(std::bitset<2> data);
        CFO_SERDESRXDisparityError(uint32_t data, CFO_Ring_ID ring);
        CFO_SERDESRXDisparityError(const CFO_SERDESRXDisparityError&) = default;
#ifndef _WIN32
        CFO_SERDESRXDisparityError(CFO_SERDESRXDisparityError&&) = default;
#endif

        CFO_SERDESRXDisparityError& operator=(const CFO_SERDESRXDisparityError&) = default;
#ifndef _WIN32
        CFO_SERDESRXDisparityError& operator=(CFO_SERDESRXDisparityError&&) = default;
#endif

        void SetData(std::bitset<2> data) { data_ = data; }
        std::bitset<2> GetData() { return data_; }
        int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
        friend std::ostream& operator<<(std::ostream& stream, CFO_SERDESRXDisparityError error) {
            stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
            return stream;
    }
};

    class CFO_CharacterNotInTableError {
    private:
        std::bitset<2> data_;

    public:
        CFO_CharacterNotInTableError();
        CFO_CharacterNotInTableError(std::bitset<2> data);
        CFO_CharacterNotInTableError(uint32_t data, CFO_Ring_ID ring);
        CFO_CharacterNotInTableError(const CFO_CharacterNotInTableError&) = default;
#ifndef _WIN32
        CFO_CharacterNotInTableError(CFO_CharacterNotInTableError&&) = default;
#endif

        CFO_CharacterNotInTableError& operator=(const CFO_CharacterNotInTableError&) = default;
#ifndef _WIN32
        CFO_CharacterNotInTableError& operator=(CFO_CharacterNotInTableError&&) = default;
#endif

        void SetData(std::bitset<2> data) { data_ = data; }
        std::bitset<2> GetData() { return data_; }
        int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
        friend std::ostream& operator<<(std::ostream& stream, CFO_CharacterNotInTableError error) {
            stream << "{\"low\":" << error.GetData()[0] << ",\"high\":" << error.GetData()[1] << "}";
            return stream;
    }
    };

    struct CFO_TestMode {
    public:
        bool loopbackEnabled;
        bool txChecker;
        bool rxGenerator;
        bool state_;
    public:
        CFO_TestMode();
        CFO_TestMode(bool state, bool loopback, bool txChecker, bool rxGenerator);
        CFO_TestMode(uint32_t input);
        bool GetLoopbackState() { return loopbackEnabled; }
        bool GetTXCheckerState() { return txChecker; }
        bool GetRXGeneratorState() { return rxGenerator; }
        bool GetState() { return state_; }
        uint32_t GetWord() const;
        std::string toString();
    };

    struct CFO_TestCommand {
    public:
        CFO_TestMode TestMode;
        int PacketSize;
        CFO_TestCommand(bool state = false, int PacketSize = 0, bool loopback = false, bool txChecker = false, bool rxGenerator = false);
        CFO_TestCommand(m_ioc_cmd_t in);
        m_ioc_cmd_t GetCommand() const;
        CFO_TestMode GetMode() { return TestMode; }

    };

    struct CFO_DMAState {
    public:
        DTC_DMA_Direction Direction;
        int BDs;                    /**< Total Number of BDs */
        int Buffers;                /**< Total Number of buffers */
        uint32_t MinPktSize;        /**< Minimum packet size */
        uint32_t MaxPktSize;        /**< Maximum packet size */
        int BDerrs;                 /**< Total BD errors */
        int BDSerrs;                /**< Total BD short errors - only TX BDs */
        int IntEnab;                /**< Interrupts enabled or not */
        CFO_TestMode TestMode;
        CFO_DMAState() {}
        CFO_DMAState(m_ioc_engstate_t in);
        std::string toString();
    };

    struct CFO_DMAStat
    {
    public:
        DTC_DMA_Direction Direction;
        uint32_t LBR;           /**< Last Byte Rate */
        uint32_t LAT;           /**< Last Active Time */
        uint32_t LWT;           /**< Last Wait Time */
        CFO_DMAStat() : Direction(DTC_DMA_Direction_Invalid), LBR(0), LAT(0), LWT(0) {}
        CFO_DMAStat(DMAStatistics in);
        std::string toString();
    };
    struct CFO_DMAStats {
    public:
        std::vector<CFO_DMAStat> Stats;
        CFO_DMAStats() {}
        CFO_DMAStats(m_ioc_engstats_t in);
        CFO_DMAStats getData(DTC_DMA_Direction dir);
        void addStat(CFO_DMAStat in) { Stats.push_back(in); }
        size_t size() { return Stats.size(); }
        CFO_DMAStat at(int index) { return Stats.at(index); }
    };

    struct CFO_PCIeState {
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
        CFO_PCIeState() {}
        CFO_PCIeState(m_ioc_pcistate_t in);
        std::string toString();
    };

    struct CFO_PCIeStat {
    public:
        uint32_t LTX;           /**< Last TX Byte Rate */
        uint32_t LRX;           /**< Last RX Byte Rate */
        CFO_PCIeStat() {}
        CFO_PCIeStat(TRNStatistics in);
    };

    struct CFO_RingEnableMode {
    public:
        bool TransmitEnable;
        bool ReceiveEnable;
        CFO_RingEnableMode() : TransmitEnable(true), ReceiveEnable(true) {}
        CFO_RingEnableMode(bool transmit, bool receive) : TransmitEnable(transmit), ReceiveEnable(receive) {}
        friend std::ostream& operator<<(std::ostream& stream, const CFO_RingEnableMode& mode) {
            bool formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
            stream.setf(std::ios_base::boolalpha);
            stream << "{\"TransmitEnable\":" << mode.TransmitEnable << ",\"ReceiveEnable\":" << mode.ReceiveEnable << "}";
            if (!formatSet) stream.unsetf(std::ios_base::boolalpha);
            return stream;
        }
        friend bool operator!=(const CFO_RingEnableMode& left, const CFO_RingEnableMode& right){ return (left.TransmitEnable == right.TransmitEnable) && (left.ReceiveEnable == right.ReceiveEnable); }
    };

    struct CFO_FIFOFullErrorFlags {
    public:
        bool OutputData;
        bool ReadoutRequestOutput;
        bool DataRequestOutput;
        bool OtherOutput;
        bool OutputDCS;
        bool OutputDCSStage2;
        bool DataInput;
        bool DCSStatusInput;
        CFO_FIFOFullErrorFlags()
            : OutputData(false)
            , ReadoutRequestOutput(false)
            , DataRequestOutput(false)
            , OtherOutput(false)
            , OutputDCS(false)
            , OutputDCSStage2(false)
            , DataInput(false)
            , DCSStatusInput(false)
        {}
        CFO_FIFOFullErrorFlags(bool outputData, bool readoutRequest, bool dataRequest,
            bool otherOutput, bool outputDCS, bool outputDCS2, bool dataInput, bool dcsInput)
            : OutputData(outputData)
            , ReadoutRequestOutput(readoutRequest)
            , DataRequestOutput(dataRequest)
            , OtherOutput(otherOutput)
            , OutputDCS(outputDCS)
            , OutputDCSStage2(outputDCS2)
            , DataInput(dataInput)
            , DCSStatusInput(dcsInput)
        {}
        friend std::ostream& operator<<(std::ostream& stream, const CFO_FIFOFullErrorFlags& flags) {
            bool formatSet = (stream.flags() & std::ios_base::boolalpha) != 0;
            stream.setf(std::ios_base::boolalpha);
            stream << "{\"OutputData\":" << flags.OutputData
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

#endif //CFO_TYPES_H
