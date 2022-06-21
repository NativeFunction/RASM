#include <vector>
#include <memory>
#include <cassert>
#include <string>
#include <stdint.h>
#include "Utils/Utils.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>
#include "Parsing/ParseCMD.h"
#include "Utils/DataReader.h"
#include "Utils/types.h"


class DataBuilder
{
public:
    std::vector<uint8_t> Data;
    uint32_t BlockSize = 0;
    uint32_t LastEndWritePos = 0;

    bool SetInt8(int8_t value, uint32_t pos)
    {
        if (pos + sizeof(value) > Data.size()) return false;
        Data[pos] = value;
        return true;
    }
    bool SetUInt8(uint8_t value, uint32_t pos)
    {
        if (pos + sizeof(value) > Data.size()) return false;
        Data[pos] = value;
        return true;
    }
    bool SetString(const std::string& value, uint32_t pos)
    {
        if (pos + value.size() + 1 > Data.size()) return false;
        memcpy_s(Data.data() + pos, Data.size() - pos, value.data(), value.size());
        Data[pos + value.size()] = 0;
        return true;
    }
    bool SetData(void* ptr, uint32_t size, uint32_t pos)
    {
        if (pos + size > Data.size()) return false;
        memcpy_s(Data.data() + pos, Data.size() - pos, ptr, size);
        return true;
    }
    virtual bool SetInt16(int16_t value, uint32_t pos) = 0;
    virtual bool SetUInt16(uint16_t value, uint32_t pos) = 0;
    virtual bool SetInt24(int32_t value, uint32_t pos) = 0;
    virtual bool SetUInt24(uint32_t value, uint32_t pos) = 0;
    virtual bool SetInt32(int32_t value, uint32_t pos) = 0;
    virtual bool SetUInt32(uint32_t value, uint32_t pos) = 0;
    virtual bool SetInt64(int64_t value, uint32_t pos) = 0;
    virtual bool SetUInt64(uint64_t value, uint32_t pos) = 0;
    virtual bool SetFloat(float value, uint32_t pos) = 0;
    virtual bool SetDouble(double value, uint32_t pos) = 0;


    bool IsWriteOverPage(uint32_t writeSize)
    {
        if ((Data.size() + writeSize) % BlockSize == 0)
            return false;

        int lastPage = Data.size() / BlockSize;
        int currentPage = (Data.size() + writeSize) / BlockSize;

        if (lastPage != currentPage)
            return true;

        return false;

    }

    //void EndWrite(uint8_t val = 0)
    //{
    //    if (BlockSize != 0)
    //    {
    //        if (Data.size() - LastEndWritePos > BlockSize)
    //            Utils::System::Throw("Data size too large for block");
    //        
    //        int currentPage = Data.size() / BlockSize;
    //        int lastPage = LastEndWritePos / BlockSize;
    //        //if not perfect page && if crossed page
    //        if (Data.size() % BlockSize != 0 && (currentPage > lastPage))
    //        {
    //            int copySize = (lastPage + 1) * BlockSize - LastEndWritePos;
    //            Data.resize(copySize + Data.size());
    //
    //            memcpy_s(Data.data() + (lastPage + 1) * BlockSize, copySize, Data.data() + LastEndWritePos, copySize);
    //            memset(Data.data() + LastEndWritePos, val, copySize);
    //
    //        }
    //
    //        LastEndWritePos = Data.size();
    //    }
    //}

    void Pad(uint32_t amount, uint8_t val = 0)
    {
        if (Data.size() % amount != 0)
            Data.resize(Data.size() + amount - Data.size() % amount, val);
    }

    void PadDirect(uint32_t amount, uint8_t val = 0)
    {
        Data.resize(Data.size() + amount, val);
    }

    void PadPage(uint8_t val = 0)
    {
        int addSize = BlockSize - Data.size() % BlockSize;
        Data.resize(addSize + Data.size(), val);
    }


    void WriteInt8(int8_t value)
    {

        Data.push_back(value);

    }
    void WriteUInt8(uint8_t value)
    {

        Data.push_back(value);

    }
    void WriteString(const std::string& value, bool nullTerm = true)
    {
        Data.insert(Data.end(), value.begin(), value.end());
        if (nullTerm)
            Data.push_back(0);
    }
    void WriteData(void* ptr, uint32_t size)
    {
        Data.resize(Data.size() + size);
        memcpy_s(Data.data() + Data.size() - size, size, ptr, size);
    }
    virtual void WriteInt16(int16_t value) = 0;
    virtual void WriteUInt16(uint16_t value) = 0;
    virtual void WriteInt24(int32_t value) = 0;
    virtual void WriteUInt24(uint32_t value) = 0;
    virtual void WriteInt32(int32_t value) = 0;
    virtual void WriteUInt32(uint32_t value) = 0;
    virtual void WriteInt64(int64_t value) = 0;
    virtual void WriteUInt64(uint64_t value) = 0;
    virtual void WriteFloat(float value) = 0;
    virtual void WriteDouble(double value) = 0;

};

class DataBuilderLit : public DataBuilder
{
public:
    inline bool SetInt16(int16_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int16_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetUInt16(uint16_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint16_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetInt24(int32_t value, uint32_t pos) override
    {
        uint8_t lastByte = Data[pos + 3];
        bool set = SetUInt32(value, pos);//0xFFFFFF00
        Data[pos + 3] = lastByte;
        return set;
    }
    inline bool SetUInt24(uint32_t value, uint32_t pos) override
    {
        uint8_t lastByte = Data[pos + 3];
        bool set = SetUInt32(value, pos);//0xFFFFFF00
        Data[pos + 3] = lastByte;
        return set;
    }
    inline bool SetInt32(int32_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int32_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetUInt32(uint32_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint32_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetInt64(int64_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int64_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetUInt64(uint64_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint64_t*)&Data[pos] = value;
        return true;
    }
    inline bool SetFloat(float value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(float*)&Data[pos] = value;
        return true;
    }
    inline bool SetDouble(double value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(double*)&Data[pos] = value;
        return true;
    }

    inline void WriteInt16(int16_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int16_t));
        *(int16_t*)(Data.data() + size) = value;
    }
    inline void WriteUInt16(uint16_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint16_t));
        *(uint16_t*)(Data.data() + size) = value;
    }
    inline void WriteInt24(int32_t value) override
    {
        WriteInt32(value);//0x00FFFFFF
        Data.resize(Data.size() - 1);
    }
    inline void WriteUInt24(uint32_t value) override
    {
        WriteUInt32(value);//0x00FFFFFF
        Data.resize(Data.size() - 1);
    }
    inline void WriteInt32(int32_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int32_t));
        *(int32_t*)(Data.data() + size) = value;
    }
    inline void WriteUInt32(uint32_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint32_t));
        *(uint32_t*)(Data.data() + size) = value;
    }
    inline void WriteInt64(int64_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int64_t));
        *(int64_t*)(Data.data() + size) = value;
    }
    inline void WriteUInt64(uint64_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint64_t));
        *(uint64_t*)(Data.data() + size) = value;
    }
    inline void WriteFloat(float value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(float));
        *(float*)(Data.data() + size) = value;
    }
    inline void WriteDouble(double value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(double));
        *(double*)(Data.data() + size) = value;
    }

};

class DataBuilderBig : public DataBuilder
{
public:
    inline bool SetInt16(int16_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int16_t*)&Data[pos] = _byteswap_ushort(value);
        return true;
    }
    inline bool SetUInt16(uint16_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint16_t*)&Data[pos] = _byteswap_ushort(value);
        return true;
    }
    inline bool SetInt24(int32_t value, uint32_t pos) override
    {
        uint8_t saved = Data[pos + 3];
        bool res = SetInt32(value << 8, pos);//0x00FFFFFF
        Data[pos + 3] = saved;

        return res;
    }
    inline bool SetUInt24(uint32_t value, uint32_t pos) override
    {
        uint8_t saved = Data[pos + 3];
        bool res = SetInt32(value << 8, pos);//0x00FFFFFF
        Data[pos + 3] = saved;

        return res;
    }
    inline bool SetInt32(int32_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int32_t*)&Data[pos] = _byteswap_ulong(value);
        return true;
    }
    inline bool SetUInt32(uint32_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint32_t*)&Data[pos] = _byteswap_ulong(value);
        return true;
    }
    inline bool SetInt64(int64_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(int64_t*)&Data[pos] = _byteswap_uint64(value);
        return true;
    }
    inline bool SetUInt64(uint64_t value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint64_t*)&Data[pos] = _byteswap_uint64(value);
        return true;
    }
    inline bool SetFloat(float value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint32_t*)&Data[pos] = _byteswap_ulong(*(uint32_t*)&value);
        return true;
    }
    inline bool SetDouble(double value, uint32_t pos) override
    {
        if (pos + sizeof(value) > Data.size()) return false;
        *(uint64_t*)&Data[pos] = _byteswap_uint64(*(uint64_t*)&value);
        return true;
    }

    inline void WriteInt16(int16_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int16_t));
        *(int16_t*)(Data.data() + size) = _byteswap_ushort(value);
    }
    inline void WriteUInt16(uint16_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint16_t));
        *(uint16_t*)(Data.data() + size) = _byteswap_ushort(value);
    }
    inline void WriteInt24(int32_t value) override
    {
        WriteInt32(value << 8);//0x00FFFFFF
        Data.resize(Data.size() - 1);
    }
    inline void WriteUInt24(uint32_t value) override
    {
        WriteUInt32(value << 8);//0x00FFFFFF
        Data.resize(Data.size() - 1);
    }
    inline void WriteInt32(int32_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int32_t));
        *(int32_t*)(Data.data() + size) = _byteswap_ulong(value);
    }
    inline void WriteUInt32(uint32_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint32_t));
        *(uint32_t*)(Data.data() + size) = _byteswap_ulong(value);
    }
    inline void WriteInt64(int64_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(int64_t));
        *(int64_t*)(Data.data() + size) = _byteswap_uint64(value);
    }
    inline void WriteUInt64(uint64_t value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint64_t));
        *(uint64_t*)(Data.data() + size) = _byteswap_uint64(value);
    }
    inline void WriteFloat(float value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint32_t));
        *(uint32_t*)(Data.data() + size) = _byteswap_ulong(*(uint32_t*)&value);
    }
    inline void WriteDouble(double value) override
    {
        uint32_t size = Data.size();
        Data.resize(size + sizeof(uint64_t));
        *(uint64_t*)(Data.data() + size) = _byteswap_uint64(*(uint64_t*)&value);
    }
};
