#pragma once
#include <vector>
#include "Utils.h"
#include <unordered_map>
#include <cassert>

class PageCollection
{
protected:
	std::vector<std::vector<uint8_t>> Pages;
	std::vector<uint8_t>* lastPage;
public:
	bool DoesPageRequirePadding = true;

	PageCollection()
	{
		std::vector<uint8_t> newPage;
		newPage.reserve(0x4000);
		Pages.push_back(std::move(newPage));
		lastPage = &Pages.back();
	}
	void AddNewPage()
	{
		lastPage->resize(0x4000, 0);//fill last page with zeros
		std::vector<uint8_t> newPage;
		newPage.reserve(0x4000);
		Pages.push_back(std::move(newPage));
		lastPage = &Pages.back();
	}
	size_t getPageCount()const { return Pages.size(); }

	size_t getPageCountIgnoreEmpty()const
	{
		return (lastPage->size() ? Pages.size() : Pages.size() - 1);
	}

	size_t getTotalSize()const
	{
		return ((Pages.size() - 1) << 14) + getLastPageSize();
	}
	size_t getPageSize(int PageIndex)const
	{
		return Pages[PageIndex].size();
	}
	size_t getLastPageSize()const
	{
		return lastPage->size();
	}
	size_t getLastPageSizeIgnoreEmpty()const
	{
		if (lastPage->size())
		{
			return lastPage->size();
		}
		if (getPageCount() > 1)
		{
			return 0x4000;
		}
		else
		{
			return 0;
		}
	}
	uint8_t* getPositionAddress(size_t position)
	{
		assert(position < getTotalSize() && "Code position out of range");

		if (DoesPageRequirePadding)
		{
			assert(position >> 14 < Pages.size() && "Code position out of page range");
			return  Pages[position >> 14].data() + (position & 0x3FFF);
		}
		else
			return Pages[0].data() + position;

	}
	uint8_t* getPageAddress(size_t pageIndex)
	{
		assert(pageIndex < getPageCount() && "Page index out of range");
		return Pages[pageIndex].data();
	}
	//Ensures there is enough space in the current code page for a specific opcode
	//Returns true if a new page needed to be created to fit the desired amount of bytes
	bool reserveBytes(size_t byteCount)
	{
		if (DoesPageRequirePadding && lastPage->size() + byteCount >= 0x4000)
		{
			AddNewPage();
			return true;
		}
		return false;
	}
	void AddPadding(const uint32_t paddingCount)
	{
		lastPage->resize(lastPage->size() + paddingCount);
	}
};

class CodePageCollection : public PageCollection
{
public:
	CodePageCollection() {}

	void AddInt8(const uint8_t value)
	{
		lastPage->push_back(value);
	}
	void changeInt8(const uint8_t newValue, const size_t position)
	{
		*getPositionAddress(position) = newValue;
	}
	virtual void AddInt16(const int16_t value) = 0;
	virtual void ChangeInt16(const int16_t newValue, const size_t position) = 0;

	virtual void AddInt24(const uint32_t value) = 0;
	virtual void ChangeInt24(const uint32_t newValue, const size_t position) = 0;

	virtual void AddInt32(const uint32_t value) = 0;
	virtual void ChangeInt32(const uint32_t newValue, const size_t position) = 0;

	virtual void AddFloat(const float value) = 0;

	void AddString(const std::string str, bool incNullTerminator = true)
	{
		auto curSize = lastPage->size();
		auto strSize = str.size() + (incNullTerminator ? 1 : 0);
		lastPage->resize(curSize + strSize);
		memcpy(lastPage->data() + curSize, str.data(), strSize);
	}

};
class CodePageCollectionBig : public CodePageCollection
{
public:
	void AddInt16(const int16_t value)override
	{
		lastPage->resize(lastPage->size() + 2, 0);
		*((int16_t*)(lastPage->data() + lastPage->size()) - 1) = Utils::Bitwise::SwapEndian(value);
	}
	void ChangeInt16(const int16_t newValue, const size_t position)override
	{
		*(int16_t*)getPositionAddress(position) = Utils::Bitwise::SwapEndian(newValue);
	}

	void AddInt24(const uint32_t value)override
	{
		lastPage->resize(lastPage->size() + 3, 0);
		*((uint32_t*)(lastPage->data() + lastPage->size()) - 1) |= Utils::Bitwise::SwapEndian(value & 0xFFFFFF);
	}
	void ChangeInt24(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)(getPositionAddress(position) - 1) |= Utils::Bitwise::SwapEndian(newValue & 0xFFFFFF);
	}

	void AddInt32(const uint32_t value)override
	{
		lastPage->resize(lastPage->size() + 4, 0);
		*((uint32_t*)(lastPage->data() + lastPage->size()) - 1) = Utils::Bitwise::SwapEndian(value);
	}
	void ChangeInt32(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)getPositionAddress(position) = Utils::Bitwise::SwapEndian(newValue);
	}

	void AddFloat(const float value)override
	{
		lastPage->resize(lastPage->size() + 4, 0);
		*((float*)(lastPage->data() + lastPage->size()) - 1) = Utils::Bitwise::SwapEndian(value);
	}
};
class CodePageCollectionLit : public CodePageCollection
{
public:
	void AddInt16(const int16_t value)override
	{
		lastPage->resize(lastPage->size() + 2, 0);
		*((int16_t*)(lastPage->data() + lastPage->size()) - 1) = value;
	}
	void ChangeInt16(const int16_t newValue, const size_t position)override
	{
		*(int16_t*)getPositionAddress(position) = newValue;
	}

	void AddInt24(const uint32_t value)override
	{
		lastPage->resize(lastPage->size() + 3, 0);
		*((uint32_t*)(lastPage->data() + lastPage->size()) - 1) |= (value & 0xFFFFFF) << 8;
	}
	void ChangeInt24(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)(getPositionAddress(position) - 1) |= (newValue & 0xFFFFFF) << 8;
	}

	void AddInt32(const uint32_t value)override
	{
		lastPage->resize(lastPage->size() + 4, 0);
		*((uint32_t*)(lastPage->data() + lastPage->size()) - 1) = value;
	}
	void ChangeInt32(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)getPositionAddress(position) = newValue;
	}
	void AddFloat(const float value)override
	{
		lastPage->resize(lastPage->size() + 4, 0);
		*((float*)(lastPage->data() + lastPage->size()) - 1) = value;
	}
};

class StringPageCollection : public PageCollection
{
private:
	std::unordered_map<std::string, size_t> stringLocationMap;
public:
	StringPageCollection() {}
	size_t AddString(const std::string& value)
	{
		auto it = stringLocationMap.find(value);
		if (it != stringLocationMap.end())
		{
			return it->second;
		}
		else
		{
			auto strSize = value.size() + 1;
			reserveBytes(strSize);
			auto index = getTotalSize();
			auto curSize = lastPage->size();
			lastPage->resize(curSize + strSize);
			memcpy(lastPage->data() + curSize, value.data(), strSize);
			stringLocationMap[value] = index;
			return index;
		}
	}
	virtual void ChangeInt32(const uint32_t newValue, const size_t position) = 0;
	size_t AddJumpTable(const uint32_t itemCount)
	{
		//lastPage->resize(lastPage->size() + 3 & ~3, 0);
		reserveBytes(itemCount * 4);
		size_t pageStartIndex = lastPage->size();
		size_t tableIndex = getTotalSize();
		lastPage->resize(pageStartIndex + itemCount * 4, 0);
		return tableIndex;
	}
	void padAlign(const uint32_t alignment)
	{
		lastPage->resize((lastPage->size() + alignment - 1) & ~(alignment - 1), 0);
	}
};
class StringPageCollectionBig : public StringPageCollection
{
public:
	void ChangeInt32(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)getPositionAddress(position) = Utils::Bitwise::SwapEndian(newValue);
	}
};
class StringPageCollectionLit : public StringPageCollection
{
public:
	void ChangeInt32(const uint32_t newValue, const size_t position)override
	{
		*(uint32_t*)getPositionAddress(position) = newValue;
	}
};
#pragma endregion