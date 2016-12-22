#pragma once
#include <vector>

#define BLOCK_COUNT_MAX 10000

class fragmentTester
{
public:
	explicit fragmentTester(size_t bytes) : vals_(bytes), block_count(0) { }

	~fragmentTester() = default;

	size_t dataSize()
	{
		if (block_count == 0)
		{
			return 0;
		}
		return blocks[block_count - 1];
	}

	size_t fragSize() const
	{
		return vals_.size();
	}

	void addSpace(size_t bytes)
	{
		vals_.resize(vals_.size() + bytes);
	}

	uint8_t* dataBegin()
	{
		return reinterpret_cast<uint8_t*>(&vals_[0]);
	}

	void endSubEvt(size_t bytes)
	{
		if (block_count > 0)
		{
			blocks[block_count] = blocks[block_count - 1] + bytes;
		}
		else
		{
			blocks[0] = bytes;
		}
		block_count++;
	}

	size_t hdr_block_count() const
	{
		return block_count;
	}

private:
	std::vector<uint8_t> vals_;
	size_t blocks[BLOCK_COUNT_MAX];
	size_t block_count;
};

