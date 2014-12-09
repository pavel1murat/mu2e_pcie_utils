#ifndef DTC_TYPES_H
#define DTC_TYPES_H

#include <bitset> // std::bitset
#include <cstdint> // uint8_t, uint16_t

#define DTCControlRegister                        0x9100
#define SERDESLoopbackEnableRegister              0x9108
#define ROCEmulationEnableRegister                0x9110
#define RingEnableRegister                        0x9114
#define SERDESResetRegister                       0x9118
#define SERDESRXDisparityErrorRegister            0x911C
#define SERDESRXCharacterNotInTableErrorRegister  0x9120
#define SERDESUnlockErrorRegister                 0x9124
#define SERDESPLLLockedRegister                   0x9128
#define SERDESTXBufferStatusRegister              0x912C
#define SERDESRXBufferStatusRegister              0x9130
#define SERDESResetDoneRegister                   0x9138
#define TimestampPreset0Register                  0x9180
#define TimestampPreset1Register                  0x9184
#define FPGAPROMProgramDataRegister               0x91A0
#define FPGAPROMProgramStatusRegister             0x91A4

namespace DTC
{
	enum DTC_ErrorCode {
		DTC_ErrorCode_Success = 0,
		DTC_ErrorCode_WrongPacketType = -1,
		DTC_ErrorCode_NotImplemented = -99,
	};

	enum DTC_Ring_ID : uint8_t {
		DTC_Ring_0 = 0,
		DTC_Ring_1 = 1,
		DTC_Ring_2 = 2,
		DTC_Ring_3 = 3,
		DTC_Ring_4 = 4,
		DTC_Ring_5 = 5,
		DTC_Ring_Unused = 0x10,
	};

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
		DTC_ROC_Unused = 0x10,
	};
	enum DTC_RXBufferStatus {
		DTC_RXBufferStatus_Nominal = 0,
		DTC_RXBufferStatus_BufferEmpty = 1,
		DTC_RXBufferStatus_BufferFull = 2,
		DTC_RXBufferStatus_Underflow = 5,
		DTC_RXBufferStatus_Overflow = 6,
		DTC_RXBufferStatus_Unknown = 0x10,
	};
	class DTC_Timestamp {
	private:
		uint64_t timestamp_ : 48;
	public:
		DTC_Timestamp();
		DTC_Timestamp(uint64_t timestamp);
		DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh);
		DTC_Timestamp(uint8_t* timeArr);
		DTC_Timestamp(std::bitset<48> timestamp);
		DTC_Timestamp(const DTC_Timestamp&) = default;
		DTC_Timestamp(DTC_Timestamp&&) = default;

		virtual ~DTC_Timestamp() = default;

		DTC_Timestamp& operator=(DTC_Timestamp&&) = default;
		DTC_Timestamp& operator=(const DTC_Timestamp&) = default;


		void SetTimestamp(uint64_t timestamp) { timestamp_ = timestamp; }
		void SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh);
		void GetTimestamp(uint64_t *timestamp){ *timestamp = timestamp_; }
		void GetTimestamp(uint8_t* arr);
		uint64_t GetTimestamp(bool output) { if (output) return timestamp_; return 0; }
		std::bitset<48> GetTimestamp() { return timestamp_; }

	};

	class DTC_DataPacket {
	private:
		uint8_t dataWords_[16];

	public:
		DTC_DataPacket() {}
		DTC_DataPacket(uint8_t* data);
		DTC_DataPacket(const DTC_DataPacket&) = default;
		DTC_DataPacket(DTC_DataPacket&&) = default;

		virtual ~DTC_DataPacket() = default;

		DTC_DataPacket& operator=(const DTC_DataPacket&) = default;
		DTC_DataPacket& operator=(DTC_DataPacket&&) = default;

		void SetWord(int index, uint8_t data);
		uint8_t GetWord(int index);
	};

	class DTC_DMAPacket {
	protected:
		DTC_Ring_ID ringID_;
		DTC_ROC_ID rocID_;
		DTC_PacketType packetType_;
		uint8_t packetCount_;
		uint8_t data_[12];
	public:
		DTC_DMAPacket() : packetType_(DTC_PacketType_Invalid) {}
		DTC_DMAPacket(DTC_DataPacket in);
		DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t packetCount = 0);
		DTC_DMAPacket(DTC_PacketType type, DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t* data, uint8_t packetCount = 0);
		DTC_DMAPacket(const DTC_DMAPacket&) = default;
		DTC_DMAPacket(DTC_DMAPacket&&) = default;

		virtual ~DTC_DMAPacket() = default;

		DTC_DMAPacket& operator=(const DTC_DMAPacket&) = default;
		DTC_DMAPacket& operator=(DTC_DMAPacket&&) = default;

		virtual DTC_DataPacket ConvertToDataPacket();

		DTC_PacketType GetPacketType() { return packetType_; }
		virtual uint8_t* GetData() { return data_; }
	};

	class DTC_DCSRequestPacket : public DTC_DMAPacket {
	public:
		DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc);
		DTC_DCSRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, uint8_t* data);
		DTC_DCSRequestPacket(const DTC_DCSRequestPacket&) = default;
		DTC_DCSRequestPacket(DTC_DCSRequestPacket&&) = default;

		virtual ~DTC_DCSRequestPacket() = default;
	};

	class DTC_ReadoutRequestPacket : public DTC_DMAPacket {
	private:
		DTC_Timestamp timestamp_;
	public:
		DTC_ReadoutRequestPacket(DTC_DataPacket in);
		DTC_ReadoutRequestPacket(DTC_DMAPacket in);
		DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID maxROC = DTC_ROC_5);
		DTC_ReadoutRequestPacket(DTC_Ring_ID ring, DTC_Timestamp timestamp, DTC_ROC_ID maxROC = DTC_ROC_5);
		DTC_ReadoutRequestPacket(const DTC_ReadoutRequestPacket& right) = default;
		DTC_ReadoutRequestPacket(DTC_ReadoutRequestPacket&& right) = default;

		virtual ~DTC_ReadoutRequestPacket() = default;

		DTC_DMAPacket ConvertToDMAPacket();
		virtual DTC_DataPacket ConvertToDataPacket() { return ConvertToDMAPacket().ConvertToDataPacket(); }
	};

	class DTC_DataRequestPacket : public DTC_DMAPacket {
	private:
		DTC_Timestamp timestamp_;
	public:
		DTC_DataRequestPacket(DTC_DataPacket in);
		DTC_DataRequestPacket(DTC_DMAPacket in);
		DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc);
		DTC_DataRequestPacket(DTC_Ring_ID ring, DTC_ROC_ID roc, DTC_Timestamp timestamp);
		DTC_DataRequestPacket(const DTC_DataRequestPacket&) = default;
		DTC_DataRequestPacket(DTC_DataRequestPacket&&) = default;

		DTC_DMAPacket ConvertToDMAPacket();
		virtual DTC_DataPacket ConvertToDataPacket() { return ConvertToDMAPacket().ConvertToDataPacket(); }
	};

	class DTC_DCSReplyPacket : public DTC_DMAPacket {
	public:
		DTC_DCSReplyPacket(DTC_Ring_ID ring);
		DTC_DCSReplyPacket(DTC_Ring_ID ring, uint8_t* data);
		DTC_DCSReplyPacket(const DTC_DCSReplyPacket&) = default;
		DTC_DCSReplyPacket(DTC_DCSReplyPacket&&) = default;
		DTC_DCSReplyPacket(DTC_DMAPacket&& right);
	};

	class DTC_DataHeaderPacket : public DTC_DMAPacket {
	private:
		DTC_Timestamp timestamp_;
		uint8_t dataStart_[6];

	public:
		DTC_DataHeaderPacket(DTC_DataPacket in);
		DTC_DataHeaderPacket(DTC_DMAPacket in);
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint8_t packetCount);
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint8_t packetCount, DTC_Timestamp timestamp);
		DTC_DataHeaderPacket(DTC_Ring_ID ring, uint8_t packetCount, DTC_Timestamp timestamp, uint8_t* data);
		DTC_DataHeaderPacket(const DTC_DataHeaderPacket&) = default;
		DTC_DataHeaderPacket(DTC_DataHeaderPacket&&) = default;

		DTC_DMAPacket ConvertToDMAPacket();
		virtual DTC_DataPacket ConvertToDataPacket() { return ConvertToDMAPacket().ConvertToDataPacket(); }
		virtual uint8_t* GetData() { return dataStart_; }
		uint8_t GetPacketCount() { return packetCount_; }
	};

	class DTC_SERDESRXDisparityError {
	private:
		std::bitset<2> data_;

	public:
		DTC_SERDESRXDisparityError();
		DTC_SERDESRXDisparityError(std::bitset<2> data);
		DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring);
		DTC_SERDESRXDisparityError(const DTC_SERDESRXDisparityError&) = default;
		DTC_SERDESRXDisparityError(DTC_SERDESRXDisparityError&&) = default;

		DTC_SERDESRXDisparityError& operator=(const DTC_SERDESRXDisparityError&) = default;
		DTC_SERDESRXDisparityError& operator=(DTC_SERDESRXDisparityError&&) = default;

		void SetData(std::bitset<2> data) { data_ = data; }
		std::bitset<2> GetData() { return data_; }
		int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
	};

	class DTC_CharacterNotInTableError {
	private:
		std::bitset<2> data_;

	public:
		DTC_CharacterNotInTableError();
		DTC_CharacterNotInTableError(std::bitset<2> data);
		DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring);
		DTC_CharacterNotInTableError(const DTC_CharacterNotInTableError&) = default;
		DTC_CharacterNotInTableError(DTC_CharacterNotInTableError&&) = default;

		DTC_CharacterNotInTableError& operator=(const DTC_CharacterNotInTableError&) = default;
		DTC_CharacterNotInTableError& operator=(DTC_CharacterNotInTableError&&) = default;

		void SetData(std::bitset<2> data) { data_ = data; }
		std::bitset<2> GetData() { return data_; }
		int GetData(bool output) { if (output) return static_cast<int>(data_.to_ulong()); return 0; }
	};

	struct DTC_SERDESRXBufferStatus {
	private:
		std::bitset<3> data_;

	public:
		DTC_SERDESRXBufferStatus();
		DTC_SERDESRXBufferStatus(std::bitset<3> data);
		DTC_SERDESRXBufferStatus(uint32_t data, DTC_Ring_ID ring);
		DTC_SERDESRXBufferStatus(const DTC_SERDESRXBufferStatus&) = default;
		DTC_SERDESRXBufferStatus(DTC_SERDESRXBufferStatus&&) = default;

		DTC_SERDESRXBufferStatus& operator=(const DTC_SERDESRXBufferStatus&) = default;
		DTC_SERDESRXBufferStatus& operator=(DTC_SERDESRXBufferStatus&&) = default;

		void SetData(std::bitset<3> data) { data_ = data; }
		DTC_RXBufferStatus GetStatus();
	};
}

#endif //DTC_TYPES_H
