#include "DTC_Types.h"
#include <sstream>
#include <iomanip>

DTCLib::DTC_RXStatusConverter::DTC_RXStatusConverter(DTC_RXStatus status): status_(status) { }

DTCLib::DTC_SimMode DTCLib::DTC_SimModeConverter::ConvertToSimMode(std::string modeName)
{
	if (modeName.find("isabled") != std::string::npos)
	{
		return DTC_SimMode_Disabled;
	}
	if (modeName.find("racker") != std::string::npos)
	{
		return DTC_SimMode_Tracker;
	}
	if (modeName.find("alorimeter") != std::string::npos)
	{
		return DTC_SimMode_Calorimeter;
	}
	if (modeName.find("osmic") != std::string::npos)
	{
		return DTC_SimMode_CosmicVeto;
	}
	if (modeName.find("oopback") != std::string::npos)
	{
		return DTC_SimMode_Loopback;
	}
	if (modeName.find("CFO") != std::string::npos || modeName.find("cfo") != std::string::npos)
	{
		return DTC_SimMode_NoCFO;
	}
	if (modeName.find("mulator") != std::string::npos)
	{
		return DTC_SimMode_ROCEmulator;
	}
	if (modeName.find("erformance") != std::string::npos)
	{
		return DTC_SimMode_Performance;
	}

	try
	{
		auto modeInt = static_cast<DTC_SimMode>(stoi(modeName, nullptr, 10));
		return modeInt != DTC_SimMode_Invalid ? modeInt : DTC_SimMode_Disabled;
	}
	catch (...)
	{
		return DTC_SimMode_Invalid;
	}
}

DTCLib::DTC_Timestamp::DTC_Timestamp()
	: timestamp_(0) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint64_t timestamp)
	: timestamp_(timestamp) {}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
	SetTimestamp(timestampLow, timestampHigh);
}

DTCLib::DTC_Timestamp::DTC_Timestamp(uint8_t* timeArr, int offset)
{
	auto arr = reinterpret_cast<uint64_t*>(timeArr + offset);
	timestamp_ = *arr;
}

DTCLib::DTC_Timestamp::DTC_Timestamp(std::bitset<48> timestamp)
	: timestamp_(timestamp.to_ullong()) {}

void DTCLib::DTC_Timestamp::SetTimestamp(uint32_t timestampLow, uint16_t timestampHigh)
{
	timestamp_ = timestampLow + (static_cast<uint64_t>(timestampHigh) << 32);
}

void DTCLib::DTC_Timestamp::GetTimestamp(uint8_t* timeArr, int offset) const
{
	for (auto i = 0; i < 6; i++)
	{
		timeArr[i + offset] = static_cast<uint8_t>(timestamp_ >> i * 8);
	}
}

std::string DTCLib::DTC_Timestamp::toJSON(bool arrayMode) const
{
	std::stringstream ss;
	if (arrayMode)
	{
		uint8_t ts[6];
		GetTimestamp(ts, 0);
		ss << "\"timestamp\": [" << static_cast<int>(ts[0]) << ",";
		ss << static_cast<int>(ts[1]) << ",";
		ss << static_cast<int>(ts[2]) << ",";
		ss << static_cast<int>(ts[3]) << ",";
		ss << static_cast<int>(ts[4]) << ",";
		ss << static_cast<int>(ts[5]) << "]";
	}
	else
	{
		ss << "\"timestamp\": " << timestamp_;
	}
	return ss.str();
}

std::string DTCLib::DTC_Timestamp::toPacketFormat() const
{
	uint8_t ts[6];
	GetTimestamp(ts, 0);
	std::stringstream ss;
	ss << std::setfill('0') << std::hex;
	ss << "0x" << std::setw(6) << static_cast<int>(ts[1]) << "\t" << "0x" << std::setw(6) << static_cast<int>(ts[0]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(ts[3]) << "\t" << "0x" << std::setw(6) << static_cast<int>(ts[2]) << "\n";
	ss << "0x" << std::setw(6) << static_cast<int>(ts[5]) << "\t" << "0x" << std::setw(6) << static_cast<int>(ts[4]) << "\n";
	return ss.str();
}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError() : data_(0) {}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError(std::bitset<2> data) : data_(data) {}

DTCLib::DTC_SERDESRXDisparityError::DTC_SERDESRXDisparityError(uint32_t data, DTC_Ring_ID ring)
{
	std::bitset<32> dataSet = data;
	uint32_t ringBase = static_cast<uint8_t>(ring) * 2;
	data_[0] = dataSet[ringBase];
	data_[1] = dataSet[ringBase + 1];
}

DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError() : data_(0) {}

DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError(std::bitset<2> data) : data_(data) {}

DTCLib::DTC_CharacterNotInTableError::DTC_CharacterNotInTableError(uint32_t data, DTC_Ring_ID ring)
{
	std::bitset<32> dataSet = data;
	uint32_t ringBase = static_cast<uint8_t>(ring) * 2;
	data_[0] = dataSet[ringBase];
	data_[1] = dataSet[ringBase + 1];
}


std::string DTCLib::Utilities::FormatByteString(double bytes)
{
	auto res = FormatBytes(bytes);
	std::stringstream s;
	s << std::setprecision(5) << res.first << " " << res.second;
	return s.str();
}

std::pair<double, std::string> DTCLib::Utilities::FormatBytes(double bytes)
{
	auto kb = bytes / 1024.0;
	auto mb = kb / 1024.0;
	auto gb = mb / 1024.0;
	auto tb = gb / 1024.0;
	auto val = bytes;
	auto unit = "bytes";

	if (tb > 1)
	{
		val = tb;
		unit = "TB";
	}
	else if (gb > 1)
	{
		val = gb;
		unit = "GB";
	}
	else if (mb > 1)
	{
		val = mb;
		unit = "MB";
	}
	else if (kb > 1)
	{
		val = kb;
		unit = "KB";
	}
	return std::make_pair(val, unit);
}

