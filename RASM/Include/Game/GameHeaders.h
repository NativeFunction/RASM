#pragma once
#include <cstdint>

typedef struct RelPtr
{
public:
    uint32_t Value = 0;

    template<typename T>
    T* GetPtr(const uint8_t* DataStart)
    {
        assert(DataStart != nullptr);
        return reinterpret_cast<T*>(const_cast<uint8_t*>(DataStart + (Value & 0x0FFFFFFF)));
    }

    uint32_t GetValue()
    {
        return Value & 0x0FFFFFFF;
    }

    RelPtr(uint32_t v) : Value(v)
    {
        Value |= 0x50000000;
    };

    RelPtr() : Value(0)
    {};

} RelPtr;//Relative Pointer

typedef struct RelPtr64
{

public:
    uint64_t Value = 0;

    template<typename T>
    T* GetPtr(const uint8_t* DataStart)
    {
        assert(DataStart != nullptr);
        return reinterpret_cast<T*>(const_cast<uint8_t*>(DataStart + (Value & 0x0FFFFFFF)));
    }

    uint64_t GetValue()
    {
        return Value & 0x0FFFFFFF;
    }


    RelPtr64(uint64_t v) : Value(v)
    {
        Value |= 0x50000000;
    };

    RelPtr64() : Value(0)
    {};

} RelPtr64;//Relative Pointer

enum class SCRFlag : uint32_t
{
    Standard = 0x5343520D,//SCR.
    Encrypted = 0x7363720E,//scr.
    CompressedEncrypted = 0x5363720E//Scr.
};

enum class Signature : uint32_t
{
    GTAIV = 0x0044207E,
    TLAD = 0x3FA5FA2D,
    TBOGT = 0x4AA44B39,

    RDR = 0x8A019D34,

    GTAV_XBOX = 0x2F56B3A0,
    GTAV_PC = 0xA46AF432,

    RDR2_SP = 0x03F176DE,
    RDR2_MP = 0xC9075085,

    Undefined = 0xFFFFFFFF
};

typedef struct CommonHeader
{
    uint8_t* HeaderPtr = nullptr;

    std::vector<uint8_t*> CodeBlockOffsets;
    std::vector<char*> StringBlockOffsets;

    uint32_t* StaticsOffset = nullptr;
    uint32_t* NativesOffset = nullptr;
    uint32_t* GlobalsOffset = nullptr;

    uint32_t CodeBlocksCount = 0xFFFFFFFF;
    uint32_t StringBlocksCount = 0xFFFFFFFF;

    uint32_t StaticsCount = 0xFFFFFFFF;
    uint32_t NativesCount = 0xFFFFFFFF;
    uint32_t GlobalsCount = 0xFFFFFFFF;

    uint32_t CodeLength = 0xFFFFFFFF;
    uint32_t TotalStringLength = 0xFFFFFFFF;
    uint32_t ParameterCount = 0;

    Signature Signature = Signature::Undefined;

    char* ScriptName = nullptr;

} CommonHeader;

const uint8_t GTAIVKey[32] = { 0x1A,0xB5,0x6F,0xED,0x7E,0xC3,0xFF,0x01,0x22,0x7B,0x69,0x15,0x33,0x97,0x5D,0xCE,0x47,0xD7,0x69,0x65,0x3F,0xF7,0x75,0x42,0x6A,0x96,0xCD,0x6D,0x53,0x07,0x56,0x5D };

typedef struct GTAIVHeader
{
    SCRFlag FormatType = (SCRFlag)0;
    uint32_t CodeLength = 0;
    uint32_t StaticsCount = 0;
    uint32_t GlobalsCount = 0;
    uint32_t ScriptFlags = 0;
    Signature Signature = Signature::Undefined;
} GTAIVHeader;

typedef union RSCFlag
{
    uint32_t Flag[2] = {0, 0};
    struct
    {
        uint32_t VPage2 : 8;
        uint32_t VPage1 : 6;
        uint32_t VPage0 : 2;
        uint32_t PPage2 : 8;
        uint32_t PPage1 : 4;
        uint32_t PPage0 : 3;
        uint32_t bResource : 1;//true


        uint32_t TotalVSize : 14; // TotalVSize (size = TotalVSize * 4096)
        uint32_t TotalPSize : 14; // TotalPSize (size = TotalPSize * 4096)
        uint32_t ObjectStartPage : 3; // resource starts at the 1st page with size = (0x1000 << ObjectStartPage)
        uint32_t bUseExtSize : 1; //true
    };

} RSCFlag;


typedef struct CSRHeader
{
    uint32_t ID = 0;
    uint32_t ResourceType = 0;
    RSCFlag Flags = {0};
} CSRHeader;

typedef struct CompressedHeader
{
    uint32_t xCompressSignature = 0;
    uint32_t CompressedSize = 0;
} CompressedHeader;

typedef struct RDRHeader
{
    uint32_t PgBase = 0;
    RelPtr PageMapOffset = 0;
    RelPtr CodeBlocksOffset = 0;
    uint32_t CodeLength = 0;
    uint32_t ParameterCount = 0;
    uint32_t StaticsCount = 0;
    RelPtr StaticsOffset = 0;
    Signature Signature = Signature::Undefined;
    uint32_t NativesCount = 0;
    RelPtr NativesOffset = 0;
    uint32_t Padding[2] = {0, 0};
} RDRHeader;

const uint8_t RDRKey[32] = { 0xB7, 0x62, 0xDF, 0xB6, 0xE2, 0xB2, 0xC6, 0xDE, 0xAF, 0x72, 0x2A, 0x32, 0xD2, 0xFB, 0x6F, 0x0C, 0x98, 0xA3, 0x21, 0x74, 0x62, 0xC9, 0xC4, 0xED, 0xAD, 0xAA, 0x2E, 0xD0, 0xDD, 0xF9, 0x2F, 0x10 };

typedef struct GTAVHeader
{
    uint32_t PgBase = 0;//0
    RelPtr PageMapOffset = 0;//4
    RelPtr CodeBlocksOffset = 0;//8
    Signature Signature = Signature::Undefined;//12
    uint32_t CodeLength = 0;//16
    uint32_t ParameterCount = 0;//20
    uint32_t StaticsCount = 0;//24
    uint32_t GlobalsCount = 0;//28
    uint32_t NativesCount = 0;//32
    RelPtr StaticsOffset = 0;//36
    RelPtr GlobalsOffset = 0;//40
    RelPtr NativesOffset = 0;//44
    uint32_t Unk3 = 0;//48
    uint32_t Unk4 = 0;//52
    uint32_t NameHash = 0;//56
    uint32_t Unk5 = 0;//60 (typically 1)
    RelPtr ScriptNameOffset = 0;//64
    RelPtr StringBlocksOffset = 0;//68
    uint32_t TotalStringLength = 0;//72
    uint32_t Unk6 = 0;//76
} GTAVHeader;

typedef struct GTAVPCHeader
{
    uint64_t PgBase = 0;//0
    RelPtr64 PageMapOffset = 0;//8

    RelPtr64 CodeBlocksOffset = 0;//16
    Signature Signature = Signature::Undefined;//24
    uint32_t CodeLength = 0;//28
    uint32_t ParameterCount = 0;//32
    uint32_t StaticsCount = 0;//36
    uint32_t GlobalsCount = 0;//40
    uint32_t NativesCount = 0;//44
    RelPtr64 StaticsOffset = 0;//48
    RelPtr64 GlobalsOffset = 0;//56
    RelPtr64 NativesOffset = 0;//64
    uint64_t Unk3 = 0;//72
    uint64_t Unk4 = 0;//80
    uint32_t NameHash = 0;//88
    uint32_t Unk5 = 0;//92 (typically 1)
    RelPtr64 ScriptNameOffset = 0;//96
    RelPtr64 StringBlocksOffset = 0;//104
    uint32_t TotalStringLength = 0;//112
    uint32_t Unk6 = 0;//116
    uint64_t Unk7 = 0;//120
} GTAVPCHeader;

typedef struct RDR2Header
{
    uint64_t PgBase = 0;//0
    RelPtr64 PageMapOffset = 0;//8

    RelPtr64 CodeBlocksOffset = 0;//16
    Signature Signature = Signature::Undefined;//24
    uint32_t CodeLength = 0;//28
    uint32_t ParameterCount = 0;//32
    uint32_t StaticsCount = 0;//36
    uint32_t GlobalsCount = 0;//40
    uint32_t NativesCount = 0;//44
    RelPtr64 StaticsOffset = 0;//48
    RelPtr64 GlobalsOffset = 0;//56
    RelPtr64 NativesOffset = 0;//64
    uint64_t Unk3 = 0;//72
    uint64_t Unk4 = 0;//80
    uint32_t NameHash = 0;//88
    uint32_t Unk5 = 0;//92 (typically 1)
    RelPtr64 ScriptNameOffset = 0;//96
    RelPtr64 StringBlocksOffset = 0;//104
    uint32_t TotalStringLength = 0;//112
    uint32_t Unk6 = 0;//116
    uint64_t Unk7 = 0;//120

    uint64_t Unk8 = 0;//128
    RelPtr64 Unk8Ptr = 0;//136

    uint64_t Unk9 = 0;//144
    RelPtr64 Unk9Ptr = 0;//152

    uint64_t Unk10 = 0;//160
    RelPtr64 Unk10Ptr = 0;//168
} RDR2Header;
