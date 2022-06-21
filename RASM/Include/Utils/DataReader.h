#pragma once
#include <stdint.h>
#include <intrin.h>

class DataReader
{
public:
	virtual int16_t ReadInt16(void* index) = 0;
	virtual uint16_t ReadUInt16(void* index) = 0;
	virtual int32_t ReadInt24(void* index) = 0;
	virtual uint32_t ReadUInt24(void* index) = 0;
	virtual int32_t ReadInt32(void* index) = 0;
	virtual uint32_t ReadUInt32(void* index) = 0;
	virtual int64_t ReadInt64(void* index) = 0;
	virtual uint64_t ReadUInt64(void* index) = 0;
	virtual float ReadFloat(void* index) = 0;
	virtual double ReadDouble(void* index) = 0;
};

class DataReaderLit : public DataReader
{
public:

	inline int16_t ReadInt16(void* index) override { return *reinterpret_cast<int16_t*>(index); }
	inline uint16_t ReadUInt16(void* index) override { return *reinterpret_cast<uint16_t*>(index); }
	inline int32_t ReadInt24(void* index) override { return *reinterpret_cast<int32_t*>(index) & 0x00FFFFFF; }
	inline uint32_t ReadUInt24(void* index) override { return *reinterpret_cast<uint32_t*>(index) & 0x00FFFFFF; }
	inline int32_t ReadInt32(void* index) override { return *reinterpret_cast<int32_t*>(index); }
	inline uint32_t ReadUInt32(void* index) override { return *reinterpret_cast<uint32_t*>(index); }
	inline int64_t ReadInt64(void* index) override { return *reinterpret_cast<int64_t*>(index); }
	inline uint64_t ReadUInt64(void* index) override { return *reinterpret_cast<uint64_t*>(index); }
	inline float ReadFloat(void* index) override { return *reinterpret_cast<float*>(index); }
	inline double ReadDouble(void* index) override { return *reinterpret_cast<double*>(index); }

};

class DataReaderBig : public DataReader
{
private:
	inline float ItoF(uint32_t x) { return *(float*)&x; }
	inline double ItoD(uint64_t x) { return *(double*)&x; }
public:

	inline int16_t ReadInt16(void* index) override { return _byteswap_ushort(*reinterpret_cast<uint16_t*>(index)); }
	inline uint16_t ReadUInt16(void* index) override { return _byteswap_ushort(*reinterpret_cast<uint16_t*>(index)); }
	inline int32_t ReadInt24(void* index) override { return (_byteswap_ulong(*reinterpret_cast<uint32_t*>(index)) & 0xFFFFFF00) >> 8; }
	inline uint32_t ReadUInt24(void* index) override { return (_byteswap_ulong(*reinterpret_cast<uint32_t*>(index)) & 0xFFFFFF00) >> 8; }
	inline int32_t ReadInt32(void* index) override { return _byteswap_ulong(*reinterpret_cast<uint32_t*>(index)); }
	inline uint32_t ReadUInt32(void* index) override { return _byteswap_ulong(*reinterpret_cast<uint32_t*>(index)); }
	inline int64_t ReadInt64(void* index) override { return _byteswap_uint64(*reinterpret_cast<int64_t*>(index)); }
	inline uint64_t ReadUInt64(void* index) override { return _byteswap_uint64(*reinterpret_cast<uint64_t*>(index)); }
	inline float ReadFloat(void* index) override { return ItoF(_byteswap_ulong(*reinterpret_cast<uint32_t*>(index))); }
	inline double ReadDouble(void* index) override { return ItoD(_byteswap_uint64(*reinterpret_cast<uint64_t*>(index))); }
};
