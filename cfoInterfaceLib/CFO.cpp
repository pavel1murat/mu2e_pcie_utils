#include "CFO.h"

#include <sstream> // Convert uint to hex string


#include <iostream>
#include <fstream>

#include <unistd.h>
#include "trace.h"

CFOLib::CFO::CFO(std::string expectedDesignVersion, CFO_SimMode mode, unsigned rocMask) : CFO_Registers(expectedDesignVersion, mode, rocMask),
bufferIndex_(0), first_read_(true), daqDMAByteCount_(0), dcsDMAByteCount_(0)
{
	//ELF, 05/18/2016: Rick reports that 3.125 Gbp
	//SetSERDESOscillatorClock(CFO_SerdesClockSpeed_25Gbps); // We're going to 2.5Gbps for now
}

CFOLib::CFO::~CFO()
{
}

//
// DMA Functions
//

void CFOLib::CFO::WriteSimFileToCFO(std::string file, bool /*goForever*/, bool overwriteEnvironment, std::string outputFileName, bool skipVerify)
{
	bool success = false;
	int retryCount = 0;
	while (!success && retryCount < 5)
	{

		TRACE(4, "CFO::WriteSimFileToCFO BEGIN");
		auto writeOutput = outputFileName != "";
		std::ofstream outputStream;
		if (writeOutput)
		{
			outputStream.open(outputFileName, std::ios::out | std::ios::binary);
		}
		auto sim = getenv("CFOLIB_SIM_FILE");
		if (!overwriteEnvironment && sim != nullptr)
		{
			file = std::string(sim);
		}


		TRACE(4, "CFO::WriteSimFileToCFO file is " + file + ", Setting up CFO");
		DisableDetectorEmulator();
		DisableDetectorEmulatorMode();
		//ResetDDR();  // this can take about a second
		ResetDDRWriteAddress();
		ResetDDRReadAddress();
		SetDDRDataLocalStartAddress(0x0);
		SetDDRDataLocalEndAddress(0xFFFFFFFF);
		EnableDetectorEmulatorMode();
		SetDetectorEmulationDMACount(1);
		SetDetectorEmulationDMADelayCount(250); // 1 microseconds
		uint64_t totalSize = 0;
		auto n = 0;

		auto sizeCheck = true;
		TRACE(4, "CFO::WriteSimFileToCFO Opening file");
		std::ifstream is(file, std::ifstream::binary);
		TRACE(4, "CFO::WriteSimFileToCFO Reading file");
		while (is && is.good() && sizeCheck)
		{
			TRACE(5, "CFO::WriteSimFileToCFO Reading a DMA from file...%s", file.c_str());
			auto buf = reinterpret_cast<cfo_databuff_t*>(new char[0x10000]);
			is.read(reinterpret_cast<char*>(buf), sizeof(uint64_t));
			if (is.eof())
			{
				TRACE(5, "CFO::WriteSimFileToCFO End of file reached.");
				delete[] buf;
				break;
			}
			auto sz = *reinterpret_cast<uint64_t*>(buf);
			//TRACE(5, "Size is %llu, writing to device", (long long unsigned)sz);
			is.read(reinterpret_cast<char*>(buf) + 8, sz - sizeof(uint64_t));
			if (sz < 80 && sz > 0)
			{
				auto oldSize = sz;
				sz = 80;
				memcpy(buf, &sz, sizeof(uint64_t));
				uint64_t sixtyFour = 64;
				memcpy(reinterpret_cast<uint64_t*>(buf) + 1, &sixtyFour, sizeof(uint64_t));
				bzero(reinterpret_cast<uint64_t*>(buf) + 2, sz - oldSize);
			}
			//is.read((char*)buf + 8, sz - sizeof(uint64_t));
			if (sz > 0 && (sz + totalSize < 0xFFFFFFFF || simMode_ == CFO_SimMode_LargeFile))
			{
				TRACE(5, "CFO::WriteSimFileToCFO Size is %zu, writing to device", sz);
				if (writeOutput)
				{
					TRACE(11, "CFO::WriteSimFileToCFO: Stripping off DMA header words and writing to binary file");
					outputStream.write(reinterpret_cast<char*>(buf) + 16, sz - 16);
				}

				auto exclusiveByteCount = *(reinterpret_cast<uint64_t*>(buf) + 1);
				TRACE(11, "CFO::WriteSimFileToCFO: Inclusive byte count: %llu, Exclusive byte count: %llu", (long long unsigned)sz, (long long unsigned)exclusiveByteCount);
				if (sz - 16 != exclusiveByteCount)
				{
					TRACE(0, "CFO::WriteSimFileToCFO: ERROR: Inclusive Byte count %llu is inconsistent with exclusive byte count %llu for DMA at 0x%llx (%llu != %llu)",
						(long long unsigned)sz, (long long unsigned)exclusiveByteCount, (long long unsigned) totalSize, (long long unsigned)sz - 16, (long long unsigned)exclusiveByteCount);
					sizeCheck = false;
				}

				totalSize += sz - 8;
				n++;
				TRACE(10, "CFO::WriteSimFileToCFO: totalSize is now %lu, n is now %lu", static_cast<unsigned long>(totalSize), static_cast<unsigned long>(n));
				WriteDetectorEmulatorData(buf, static_cast<size_t>(sz));
			}
			else if (sz > 0)
			{
				TRACE(5, "CFO::WriteSimFileToCFO CFO memory is now full. Closing file.");
				sizeCheck = false;
			}
			delete[] buf;
		}

		TRACE(4, "CFO::WriteSimFileToCFO Closing file. sizecheck=%i, eof=%i, fail=%i, bad=%i", sizeCheck, is.eof(), is.fail(), is.bad());
		is.close();
		if (writeOutput) outputStream.close();
		SetDDRDataLocalEndAddress(static_cast<uint32_t>(totalSize - 1));
		success = skipVerify || VerifySimFileInCFO(file, outputFileName);
		retryCount++;
	}

	if (retryCount == 5)
	{
		TRACE(0, "CFO::WriteSimFileToCFO FAILED after 5 attempts! ABORTING!");
		exit(4);
	}
	else
	{
		TRACE(2, "CFO::WriteSimFileToCFO Took %d attempts to write file", retryCount);
	}

	SetDetectorEmulatorInUse();
	TRACE(4, "CFO::WriteSimFileToCFO END");
}

bool CFOLib::CFO::VerifySimFileInCFO(std::string file, std::string rawOutputFilename)
{
	uint64_t totalSize = 0;
	auto n = 0;
	auto sizeCheck = true;

	auto writeOutput = rawOutputFilename != "";
	std::ofstream outputStream;
	if (writeOutput)
	{
		outputStream.open(rawOutputFilename + ".verify", std::ios::out | std::ios::binary);
	}

	auto sim = getenv("CFOLIB_SIM_FILE");
	if (file.size() == 0 && sim != nullptr)
	{
		file = std::string(sim);
	}

	ResetDDRReadAddress();
	TRACE(4, "CFO::VerifySimFileInCFO Opening file");
	std::ifstream is(file, std::ifstream::binary);
	if (!is || !is.good())
	{
		TRACE(0, "CFO::VerifySimFileInCFO Failed to open file " + file + "!");
	}

	TRACE(4, "CFO::VerifySimFileInCFO Reading file");
	while (is && is.good() && sizeCheck)
	{
		TRACE(5, "CFO::VerifySimFileInCFO Reading a DMA from file...%s", file.c_str());
		auto buf = reinterpret_cast<cfo_databuff_t*>(new char[0x10000]);
		is.read(reinterpret_cast<char*>(buf), sizeof(uint64_t));
		if (is.eof())
		{
			TRACE(5, "CFO::VerifySimFileInCFO End of file reached.");
			delete[] buf;
			break;
		}
		auto sz = *reinterpret_cast<uint64_t*>(buf);
		//TRACE(5, "Size is %llu, writing to device", (long long unsigned)sz);
		is.read(reinterpret_cast<char*>(buf) + 8, sz - sizeof(uint64_t));
		if (sz < 80 && sz > 0)
		{
			auto oldSize = sz;
			sz = 80;
			memcpy(buf, &sz, sizeof(uint64_t));
			uint64_t sixtyFour = 64;
			memcpy(reinterpret_cast<uint64_t*>(buf) + 1, &sixtyFour, sizeof(uint64_t));
			bzero(reinterpret_cast<uint64_t*>(buf) + 2, sz - oldSize);
		}

		if (sz > 0 && (sz + totalSize < 0xFFFFFFFF || simMode_ == CFO_SimMode_LargeFile))
		{
			TRACE(5, "CFO::VerifySimFileInCFO Expected Size is %zu, reading from device", sz);
			auto exclusiveByteCount = *(reinterpret_cast<uint64_t*>(buf) + 1);
			TRACE(11, "CFO::VerifySimFileInCFO: Inclusive byte count: %llu, Exclusive byte count: %llu", (long long unsigned)sz, (long long unsigned)exclusiveByteCount);
			if (sz - 16 != exclusiveByteCount)
			{
				TRACE(0, "CFO::VerifySimFileInCFO: ERROR: Inclusive Byte count %llu is inconsistent with exclusive byte count %llu for DMA at 0x%llx (%llu != %llu)",
					(long long unsigned)sz, (long long unsigned)exclusiveByteCount, (long long unsigned) totalSize, (long long unsigned)sz - 16, (long long unsigned)exclusiveByteCount);
				sizeCheck = false;
			}

			totalSize += sz;
			n++;
			TRACE(10, "CFO::VerifySimFileInCFO: totalSize is now %lu, n is now %lu", static_cast<unsigned long>(totalSize), static_cast<unsigned long>(n));
			//WriteDetectorEmulatorData(buf, static_cast<size_t>(sz));
			DisableDetectorEmulator();
			SetDetectorEmulationDMACount(1);
			EnableDetectorEmulator();

			cfo_databuff_t* buffer;
			auto tmo_ms = 1500;
			TRACE(4, "CFO::VerifySimFileInCFO - before read for DAQ ");
			auto sts = device_.read_data(CFO_DMA_Engine_EventTable, reinterpret_cast<void**>(&buffer), tmo_ms);
			if (writeOutput && sts > 8)
			{
				TRACE(11, "CFO::VerifySimFileInCFO: Writing to binary file");
				outputStream.write(reinterpret_cast<char*>(*buffer + 8), sts - 8);
			}
			size_t readSz = *(reinterpret_cast<uint64_t*>(buffer));
			TRACE(4, "CFO::VerifySimFileInCFO - after read, sz=%zu sts=%d rdSz=%zu", sz, sts, readSz);

			// DMA engine strips off leading 64-bit word
			TRACE(6, "CFO::VerifySimFileInCFO - Checking buffer size");
			if (static_cast<size_t>(sts) != sz - sizeof(uint64_t))
			{
				TRACE(0, "CFO::VerifySimFileInCFO Buffer %d has size 0x%zx but the input file has size 0x%zx for that buffer!", n, static_cast<size_t>(sts), sz - sizeof(uint64_t));

				device_.read_release(CFO_DMA_Engine_EventTable, 1);
				delete[] buf;
				is.close();
				if (writeOutput) outputStream.close();
				return false;
			}

			TRACE(6, "CFO::VerifySimFileInCFO - Checking buffer contents");
			size_t cnt = sts % sizeof(uint64_t) == 0 ? sts / sizeof(uint64_t) : 1 + (sts / sizeof(uint64_t));

			for (size_t ii = 0; ii < cnt; ++ii)
			{
				auto l = *(reinterpret_cast<uint64_t*>(*buffer) + ii);
				auto r = *(reinterpret_cast<uint64_t*>(*buf) + ii + 1);
				if (l != r)
				{
					size_t address = totalSize - sz + ((ii + 1) * sizeof(uint64_t));
					TRACE(0, "CFO::VerifySimFileInCFO Buffer %d word %zu (Address in file 0x%zx): Expected 0x%llx, but got 0x%llx. Returning False!", n, ii, address,
						  static_cast<unsigned long long>(r), static_cast<unsigned long long>(l));
					delete[] buf;
					is.close();
					if (writeOutput) outputStream.close();
					device_.read_release(CFO_DMA_Engine_EventTable, 1);
					return false;
				}
			}
			device_.read_release(CFO_DMA_Engine_EventTable, 1);
		}
		else if (sz > 0)
		{
			TRACE(5, "CFO::VerifySimFileInCFO CFO memory is now full. Closing file.");
			sizeCheck = false;
		}
		delete[] buf;
	}

	TRACE(4, "CFO::VerifySimFileInCFO Closing file. sizecheck=%i, eof=%i, fail=%i, bad=%i", sizeCheck, is.eof(), is.fail(), is.bad());
	TRACE(1, "CFO::VerifySimFileInCFO: The Detector Emulation file was written correctly");
	is.close();
	if (writeOutput) outputStream.close();
	return true;
}

void CFOLib::CFO::WriteDetectorEmulatorData(cfo_databuff_t* buf, size_t sz)
{
	if (sz < dmaSize_)
	{
		sz = dmaSize_;
	}
	auto retry = 3;
	int errorCode;
	do
	{
		TRACE(21, "CFO::WriteDetectorEmulatorData: Writing buffer of size %zu", sz);
		errorCode = device_.write_data(CFO_DMA_Engine_EventTable, buf, sz);
		retry--;
	} while (retry > 0 && errorCode != 0);
	if (errorCode != 0)
	{
		throw CFO_IOErrorException();
	}
}


