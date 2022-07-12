#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include <string>
#include <stdint.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/tokenizer.hpp>
#include "Utils/Constants.h"
#include "Utils/Utils.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>
#include "Parsing/ParseCMD.h"
#include "Utils/types.h"
#include "Utils/DataBuilder.h"
#include "Compiler/StringPageBuilder.h"
#include "Game/GameHeaders.h"

enum class IntTokenContext
{
    Default,
    Local,
    Static,
    Global

};

class CompileBase
{
private:

protected:
#pragma region vars
    std::unordered_map<Opcode, uint8_t>* CommonOpsToTargetOps = nullptr;
    bool IsLittleEndian;

    std::unique_ptr<DataBuilder> CodeBuilder;

    StringPageBuilder StringBuilder;

    std::vector<int32_t> Statics;
    //native, index
    std::unordered_map<uint64_t, uint16_t> NativeIndexes;
    //label, code loc of label (: prefix) aka (labels)
    std::unordered_map<std::string, uint32_t> LabelToCodePos;
    //label, code loc of needed label loc <code loc, isRelative (true->int16, false->int24)> (: prefix)  

    std::unordered_map<std::string, int32_t> Enums;
    std::unordered_map<std::string, int32_t> LocalNames;
    std::unordered_map<std::string, int32_t> StaticNames;
    std::unordered_map<std::string, int32_t> GlobalNames;


    typedef struct MissedLabel
    {
        uint32_t Position;
        bool IsRelative;
        uint32_t Line;
    } MissedLabel;

    std::unordered_map<std::string, std::vector<MissedLabel>> MissedLabels;


    std::string& Text;
    uint32_t size = 0;
    char* readP = 0;
    uint32_t readPSize = 0;
    bool reading = false;
    char lastDelim = 0;
    std::string keyStr = "";
    uint32_t line = 1;
    uint32_t lookaheadLine = 1;
    uint32_t index = 0;
    bool isFirstReadOfLine = true;
    char firstChar = 0;


    uint8_t* CurrentReadPos = nullptr;
    size_t CurrentOpSize = 1;

    GameTarget::GameTarget GameTarget;
    size_t MaxPageSize;
    bool Is64Bit = false;

    std::unordered_map<std::string, uint64_t> NativeMap;
    std::unordered_map<uint64_t, uint64_t> NativeTranslationMap;

    uint32_t ParameterCount = 0;
    Signature SignatureType = Signature::Undefined;

#pragma endregion

#pragma region CTOR
    CompileBase(GameTarget::GameTarget target, std::string& text, size_t maxPageSize = 16384, bool isLittleEndian = false);

    CompileBase(std::string& text, bool isLittleEndian = false, size_t maxPageSize = 16384);
#pragma endregion

    void SetEndian(bool isLittleEndian)
    {
        if (isLittleEndian)
            CodeBuilder = std::make_unique<DataBuilderLit>();
        else
            CodeBuilder = std::make_unique<DataBuilderBig>();

        IsLittleEndian = isLittleEndian;

        CodeBuilder->Data.reserve(10000000);
    }

    void FixCodePage(uint32_t size)
    {
        if (CodeBuilder->IsWriteOverPage(size))
            CodeBuilder->PadPage(CommonOpsToTargetOps->at(Opcode::Nop));
    }

    int StringToInt(std::string& str, IntTokenContext context = IntTokenContext::Default, uint32_t missedGetLabelLocPos = -1);

    Result<std::string> GetNextToken();

    std::string GetNextTokenInLine();

    int32_t GetNextTokenAsInt(IntTokenContext context = IntTokenContext::Default, uint32_t missedGetLabelLocPos = -1);

    float GetNextTokenAsFloat();

    uint64_t GetNextTokenAsNative();

    void AddLabel(std::string& label);

    virtual void FixMissedLabels();

    void AddSingleOp(Opcode op);

    void AddPad();

    virtual void AddPushB2();

    virtual void AddPushB3();

    virtual void AddPush(int val);

    virtual void AddPush();

    virtual void AddPushF(float val);

    virtual void AddPushF();

    virtual void AddCallNative() = 0;

    virtual void AddFunction();

    virtual void AddReturn();

    virtual void AddAddImm();

    virtual void AddMultImm();

    virtual void AddJump(Opcode jumpOp);

    virtual void AddJumpPos(std::string& label);

    //override gtaiv rdr
    virtual void AddCall();

    template<typename Func>
    void ParseSwitch(Func writeSwitch)
    {
        std::vector<std::pair<int, std::string>> casesAndLabels;

        int switchLine = line;
        int caseNum = 0;
        std::string label = "";
        Result<std::string> tok;

        while (true)
        {
            tok = GetNextToken();

            if (tok.Res == false)
                Utils::System::Throw("Could not find token on line " + std::to_string(line));

            if (switchLine == line)
            {
                try
                {
                    caseNum = StringToInt(tok.Data);
                    label = GetNextTokenInLine();
                    casesAndLabels.push_back({ caseNum, label });
                }
                catch (std::exception)
                {
                    Utils::System::Throw("Could not parse int token " + tok.Data + " on line " + std::to_string(line));
                }
            }
            else
                break;
        }

        //RDR2 requires sorted cases
        std::sort(casesAndLabels.begin(), casesAndLabels.end());
        
        writeSwitch(casesAndLabels);

        ParseNextLine(tok.Data);
    }

    //override rdr2
    virtual void AddSwitch();

    void ParseNextLine(std::string& tok);

    //override gtaiv and rdr
    virtual void AddPushString();
    virtual void AddPushArray();

    virtual void AddStrCopy();
    virtual void AddItoS();
    virtual void AddStrAdd();
    virtual void AddStrAddi();

    virtual void AddSetStaticsCount();
    virtual void AddSetDefaultStatic();

    virtual void AddSetStaticName();
    virtual void AddSetLocalName();
    virtual void AddSetGlobalName();
    virtual void AddSetEnum();
    virtual void AddSetParamCount();
    virtual void AddSetSignature();

    virtual void AddVarOp(IntTokenContext context, Opcode byteOp, Opcode shortOp, Opcode int24Op = Opcode::Uninitialized);

    virtual void WriteScript(const std::string& scriptOutPath) = 0;
public:

    void ParseOpcode(Opcode op);

    std::string GetOptionsDataName(std::string dataName);

    void Compile(const std::string& scriptOutPath);

};

class CompileGTAIV : public CompileBase
{

#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
        {Opcode::Nop,     0  },
        {Opcode::Add,     1  },
        {Opcode::Sub,     2  },
        {Opcode::Mult,     3  },
        {Opcode::Div,     4  },
        {Opcode::Mod,     5  },
        {Opcode::Not,     6  },
        {Opcode::Neg,     7  },
        {Opcode::CmpEQ,     8  },
        {Opcode::CmpNE,     9  },
        {Opcode::CmpGT,     10 },
        {Opcode::CmpGE,     11 },
        {Opcode::CmpLT,     12 },
        {Opcode::CmpLE,     13 },
        {Opcode::fAdd,     14 },
        {Opcode::fSub,     15 },
        {Opcode::fMult,     16 },
        {Opcode::fDiv,     17 },
        {Opcode::fMod,     18 },
        {Opcode::fNeg,     19 },
        {Opcode::fCmpEQ,     20 },
        {Opcode::fCmpNE,     21 },
        {Opcode::fCmpGT,     22 },
        {Opcode::fCmpGE,     23 },
        {Opcode::fCmpLT,     24 },
        {Opcode::fCmpLE,     25 },
        {Opcode::vAdd,     26 },
        {Opcode::vSub,     27 },
        {Opcode::vMult,     28 },
        {Opcode::vDiv,     29 },
        {Opcode::vNeg,     30 },
        {Opcode::And,     31 },
        {Opcode::Or,     32 },
        {Opcode::Xor,     33 },
        {Opcode::Jump,     34 },
        {Opcode::JumpFalse,     35 },
        {Opcode::JumpTrue,     36 },
        {Opcode::ItoF,     37 },
        {Opcode::FtoI,     38 },
        {Opcode::FtoV,     39 },
        {Opcode::PushS,     40 },
        {Opcode::Push,     41 },
        {Opcode::PushF,     42 },
        {Opcode::Dup,     43 },
        {Opcode::Drop,     44 },
        {Opcode::CallNative,     45 },
        {Opcode::Call,     46 },
        {Opcode::Function,     47 },
        {Opcode::Return,     48 },
        {Opcode::pGet,     49 },
        {Opcode::pSet,     50 },
        {Opcode::pPeekSet,     51 },
        {Opcode::ToStack,     52 },
        {Opcode::FromStack,     53 },
        {Opcode::GetLocalPv0,     54 },
        {Opcode::GetLocalPv1,     55 },
        {Opcode::GetLocalPv2,     56 },
        {Opcode::GetLocalPv3,     57 },
        {Opcode::GetLocalPv4,     58 },
        {Opcode::GetLocalPv5,     59 },
        {Opcode::GetLocalPv6,     60 },
        {Opcode::GetLocalPv7,     61 },
        {Opcode::GetLocalPs,     62 },
        {Opcode::GetStaticPs,     63 },
        {Opcode::GetGlobalPs,     64 },
        {Opcode::GetArrayPs,     65 },
        {Opcode::Switch,     66 },
        {Opcode::PushString,     67 },
        {Opcode::PushStringNull,     68 },
        {Opcode::StrCopy,     69 },
        {Opcode::ItoS,     70 },
        {Opcode::StrAdd,     71 },
        {Opcode::StrAddi,     72 },
        {Opcode::Catch,     73 },
        {Opcode::Throw,     74 },
        {Opcode::MemCopy,     75 },
        {Opcode::GetXProtect,     76 },
        {Opcode::SetXProtect,     77 },
        {Opcode::RefXProtect,     78 },
        {Opcode::Exit,     79 },
        {Opcode::Push_Neg16,     80 },
        {Opcode::Push_Neg15,     81 },
        {Opcode::Push_Neg14,     82 },
        {Opcode::Push_Neg13,     83 },
        {Opcode::Push_Neg12,     84 },
        {Opcode::Push_Neg11,     85 },
        {Opcode::Push_Neg10,     86 },
        {Opcode::Push_Neg9,     87 },
        {Opcode::Push_Neg8,     88 },
        {Opcode::Push_Neg7,     89 },
        {Opcode::Push_Neg6,     90 },
        {Opcode::Push_Neg5,     91 },
        {Opcode::Push_Neg4,     92 },
        {Opcode::Push_Neg3,     93 },
        {Opcode::Push_Neg2,     94 },
        {Opcode::Push_Neg1,     95 },
        {Opcode::Push_0,     96 },
        {Opcode::Push_1,     97 },
        {Opcode::Push_2,     98 },
        {Opcode::Push_3,     99 },
        {Opcode::Push_4,     100},
        {Opcode::Push_5,     101},
        {Opcode::Push_6,     102},
        {Opcode::Push_7,     103}
    };
#pragma endregion

protected:

    void FixMissedLabels() override;

    void AddCallNative() override;

    void AddPushString() override;

    void AddJump(Opcode jumpOp) override;

    void AddJumpPos(std::string& label) override;

    void AddCall() override;

    void AddFunction() override;

    void AddSwitch() override;

    void AddPush(int val) override;

    void AddPushF(float val) override;

    void AddGetLocalP(int val)
    {
        if (val <= 7)
            AddSingleOp((Opcode)((int)Opcode::GetLocalPv0 + val));
        else
        {
            AddPush(val);
            AddSingleOp(Opcode::GetLocalPs);
        }
    }

    void AddVarOp(IntTokenContext context, Opcode byteOp, Opcode shortOp, Opcode int24Op = Opcode::Uninitialized) override;

    void WriteScript(const std::string& scriptOutPath) override;


public:
    CompileGTAIV(std::string& text) : CompileBase(text)
    {
        Is64Bit = false;
        GameTarget = GameTarget::GTAIV;
        MaxPageSize = 16384;
        CommonOpsToTargetOps = &_CommonOpsToTargetOps;

        SetEndian(true);

        CodeBuilder->BlockSize = 16384;
    }

};

class CompileRDR : public CompileBase
{
#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
        {Opcode::Nop,    0  },
        {Opcode::Add,    1  },
        {Opcode::Sub,    2  },
        {Opcode::Mult,    3  },
        {Opcode::Div,    4  },
        {Opcode::Mod,    5  },
        {Opcode::Not,    6  },
        {Opcode::Neg,    7  },
        {Opcode::CmpEQ,    8  },
        {Opcode::CmpNE,    9  },
        {Opcode::CmpGT,    10 },
        {Opcode::CmpGE,    11 },
        {Opcode::CmpLT,    12 },
        {Opcode::CmpLE,    13 },
        {Opcode::fAdd,    14 },
        {Opcode::fSub,    15 },
        {Opcode::fMult,    16 },
        {Opcode::fDiv,    17 },
        {Opcode::fMod,    18 },
        {Opcode::fNeg,    19 },
        {Opcode::fCmpEQ,    20 },
        {Opcode::fCmpNE,    21 },
        {Opcode::fCmpGT,    22 },
        {Opcode::fCmpGE,    23 },
        {Opcode::fCmpLT,    24 },
        {Opcode::fCmpLE,    25 },
        {Opcode::vAdd,    26 },
        {Opcode::vSub,    27 },
        {Opcode::vMult,    28 },
        {Opcode::vDiv,    29 },
        {Opcode::vNeg,    30 },
        {Opcode::And,    31 },
        {Opcode::Or,    32 },
        {Opcode::Xor,    33 },
        {Opcode::ItoF,    34 },
        {Opcode::FtoI,    35 },
        {Opcode::FtoV,    36 },
        {Opcode::PushB,    37 },
        {Opcode::PushB2,    38 },
        {Opcode::PushB3,    39 },
        {Opcode::Push,    40 },
        {Opcode::PushF,    41 },
        {Opcode::Dup,    42 },
        {Opcode::Drop,    43 },
        {Opcode::CallNative,    44 },
        {Opcode::Function,    45 },
        {Opcode::Return,    46 },
        {Opcode::pGet,    47 },
        {Opcode::pSet,    48 },
        {Opcode::pPeekSet,    49 },
        {Opcode::ToStack,    50 },
        {Opcode::FromStack,    51 },
        {Opcode::GetArrayP1,    52 },
        {Opcode::GetArray1,    53 },
        {Opcode::SetArray1,    54 },
        {Opcode::GetLocalP1,    55 },
        {Opcode::GetLocal1,    56 },
        {Opcode::SetLocal1,    57 },
        {Opcode::GetStaticP1,    58 },
        {Opcode::GetStatic1,    59 },
        {Opcode::SetStatic1,    60 },
        {Opcode::AddImm1,    61 },
        {Opcode::GetImm1,    62 },
        {Opcode::SetImm1,    63 },
        {Opcode::MultImm1,    64 },
        {Opcode::PushS,    65 },
        {Opcode::AddImm2,    66 },
        {Opcode::GetImm2,    67 },
        {Opcode::SetImm2,    68 },
        {Opcode::MultImm2,    69 },
        {Opcode::GetArrayP2,    70 },
        {Opcode::GetArray2,    71 },
        {Opcode::SetArray2,    72 },
        {Opcode::GetLocalP2,    73 },
        {Opcode::GetLocal2,    74 },
        {Opcode::SetLocal2,    75 },
        {Opcode::GetStaticP2,    76 },
        {Opcode::GetStatic2,    77 },
        {Opcode::SetStatic2,    78 },
        {Opcode::GetGlobalP2,    79 },
        {Opcode::GetGlobal2,    80 },
        {Opcode::SetGlobal2,    81 },
        {Opcode::Call2,    82 },
        {Opcode::Call2h1,    83 },
        {Opcode::Call2h2,    84 },
        {Opcode::Call2h3,    85 },
        {Opcode::Call2h4,    86 },
        {Opcode::Call2h5,    87 },
        {Opcode::Call2h6,    88 },
        {Opcode::Call2h7,    89 },
        {Opcode::Call2h8,    90 },
        {Opcode::Call2h9,    91 },
        {Opcode::Call2hA,    92 },
        {Opcode::Call2hB,    93 },
        {Opcode::Call2hC,    94 },
        {Opcode::Call2hD,    95 },
        {Opcode::Call2hE,    96 },
        {Opcode::Call2hF,    97 },
        {Opcode::Jump,    98 },
        {Opcode::JumpFalse,    99 },
        {Opcode::JumpNE,    100},
        {Opcode::JumpEQ,    101},
        {Opcode::JumpLE,    102},
        {Opcode::JumpLT,    103},
        {Opcode::JumpGE,    104},
        {Opcode::JumpGT,    105},
        {Opcode::GetGlobalP3,    106},
        {Opcode::GetGlobal3,    107},
        {Opcode::SetGlobal3,    108},
        {Opcode::PushI24,    109},
        {Opcode::Switch,    110},
        {Opcode::PushString,    111},
        {Opcode::PushArray,    112},
        {Opcode::PushStringNull,    113},
        {Opcode::StrCopy,    114},
        {Opcode::ItoS,    115},
        {Opcode::StrAdd,    116},
        {Opcode::StrAddi,    117},
        {Opcode::MemCopy,    118},
        {Opcode::Catch,    119},
        {Opcode::Throw,    120},
        {Opcode::pCall,    121},
        {Opcode::ReturnP0R0,    122},
        {Opcode::ReturnP0R1,    123},
        {Opcode::ReturnP0R2,    124},
        {Opcode::ReturnP0R3,    125},
        {Opcode::ReturnP1R0,    126},
        {Opcode::ReturnP1R1,       127},
        {Opcode::ReturnP1R2,       128},
        {Opcode::ReturnP1R3,       129},
        {Opcode::ReturnP2R0,       130},
        {Opcode::ReturnP2R1,       131},
        {Opcode::ReturnP2R2,       132},
        {Opcode::ReturnP2R3,       133},
        {Opcode::ReturnP3R0,       134},
        {Opcode::ReturnP3R1,       135},
        {Opcode::ReturnP3R2,       136},
        {Opcode::ReturnP3R3,       137},
        {Opcode::Push_Neg1,        138},
        {Opcode::Push_0,           139},
        {Opcode::Push_1,           140},
        {Opcode::Push_2,           141},
        {Opcode::Push_3,           142},
        {Opcode::Push_4,           143},
        {Opcode::Push_5,           144},
        {Opcode::Push_6,           145},
        {Opcode::Push_7,           146},
        {Opcode::PushF_Neg1,       147},
        {Opcode::PushF_0,          148},
        {Opcode::PushF_1,          149},
        {Opcode::PushF_2,          150},
        {Opcode::PushF_3,          151},
        {Opcode::PushF_4,          152},
        {Opcode::PushF_5,          153},
        {Opcode::PushF_6,          154},
        {Opcode::PushF_7,          155},
    };
#pragma endregion

protected:

#pragma region RSC85Parsing
    std::vector<uint32_t> GetPageSizes(uint32_t& size);

    uint32_t GetHeaderPagePos(const std::vector<uint32_t>& DataSizePages);

    uint32_t ObjectStartPageSizeToFlag(size_t value)
    {
        //4096 = 1 << 12;
        //2048 = 1 << 11
        //The -12 should be platform dependent, if the PageShift is different on PS3 then it might be 11
        unsigned long r = 0;
        _BitScanReverse(&r, value);
        return r - 12;
    }
#pragma endregion

    uint16_t CreateRDRCallNative(const uint16_t index, const int parameterCount, const bool ret) const
    {
        return Utils::Bitwise::SwapEndian((uint16_t)(((index & 0xFF00) >> 2) | ((index & 0xFF) << 8) | (ret ? 1 : 0) | (parameterCount << 1)));
    }

    void FixMissedLabels() override;

    void AddCallNative() override;

    void AddPushString() override;

    void AddPushArray() override;

    void AddCall() override;

    void AddReturn() override;

    void WriteScript(const std::string& scriptOutPath) override;


public:
    CompileRDR(std::string& text) : CompileBase(text)
    {
        Is64Bit = false;
        GameTarget = GameTarget::RDR;
        MaxPageSize = 16384;
        CommonOpsToTargetOps = &_CommonOpsToTargetOps;

        SetEndian(false);

        CodeBuilder->BlockSize = 16384;
    }

};

class CompileGTAVConsole : public CompileBase
{
#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
        {Opcode::Nop,              0  },
        {Opcode::Add,              1  },
        {Opcode::Sub,              2  },
        {Opcode::Mult,             3  },
        {Opcode::Div,              4  },
        {Opcode::Mod,              5  },
        {Opcode::Not,              6  },
        {Opcode::Neg,              7  },
        {Opcode::CmpEQ,            8  },
        {Opcode::CmpNE,            9  },
        {Opcode::CmpGT,            10 },
        {Opcode::CmpGE,            11 },
        {Opcode::CmpLT,            12 },
        {Opcode::CmpLE,            13 },
        {Opcode::fAdd,             14 },
        {Opcode::fSub,             15 },
        {Opcode::fMult,            16 },
        {Opcode::fDiv,             17 },
        {Opcode::fMod,             18 },
        {Opcode::fNeg,             19 },
        {Opcode::fCmpEQ,           20 },
        {Opcode::fCmpNE,           21 },
        {Opcode::fCmpGT,           22 },
        {Opcode::fCmpGE,           23 },
        {Opcode::fCmpLT,           24 },
        {Opcode::fCmpLE,           25 },
        {Opcode::vAdd,             26 },
        {Opcode::vSub,             27 },
        {Opcode::vMult,            28 },
        {Opcode::vDiv,             29 },
        {Opcode::vNeg,             30 },
        {Opcode::And,              31 },
        {Opcode::Or,               32 },
        {Opcode::Xor,              33 },
        {Opcode::ItoF,             34 },
        {Opcode::FtoI,             35 },
        {Opcode::FtoV,             36 },
        {Opcode::PushB,            37 },
        {Opcode::PushB2,           38 },
        {Opcode::PushB3,           39 },
        {Opcode::Push,             40 },
        {Opcode::PushF,            41 },
        {Opcode::Dup,              42 },
        {Opcode::Drop,             43 },
        {Opcode::CallNative,       44 },
        {Opcode::Function,         45 },
        {Opcode::Return,           46 },
        {Opcode::pGet,             47 },
        {Opcode::pSet,             48 },
        {Opcode::pPeekSet,         49 },
        {Opcode::ToStack,          50 },
        {Opcode::FromStack,        51 },
        {Opcode::GetArrayP1,       52 },
        {Opcode::GetArray1,        53 },
        {Opcode::SetArray1,        54 },
        {Opcode::GetLocalP1,       55 },
        {Opcode::GetLocal1,        56 },
        {Opcode::SetLocal1,        57 },
        {Opcode::GetStaticP1,      58 },
        {Opcode::GetStatic1,       59 },
        {Opcode::SetStatic1,       60 },
        {Opcode::AddImm1,          61 },
        {Opcode::MultImm1,         62 },
        {Opcode::GetImmPs,         63 },
        {Opcode::GetImmP1,         64 },
        {Opcode::GetImm1,          65 },
        {Opcode::SetImm1,          66 },
        {Opcode::PushS,            67 },
        {Opcode::AddImm2,          68 },
        {Opcode::MultImm2,         69 },
        {Opcode::GetImmP2,         70 },
        {Opcode::GetImm2,          71 },
        {Opcode::SetImm2,          72 },
        {Opcode::GetArrayP2,       73 },
        {Opcode::GetArray2,        74 },
        {Opcode::SetArray2,        75 },
        {Opcode::GetLocalP2,       76 },
        {Opcode::GetLocal2,        77 },
        {Opcode::SetLocal2,        78 },
        {Opcode::GetStaticP2,      79 },
        {Opcode::GetStatic2,       80 },
        {Opcode::SetStatic2,       81 },
        {Opcode::GetGlobalP2,      82 },
        {Opcode::GetGlobal2,       83 },
        {Opcode::SetGlobal2,       84 },
        {Opcode::Jump,             85 },
        {Opcode::JumpFalse,        86 },
        {Opcode::JumpNE,           87 },
        {Opcode::JumpEQ,           88 },
        {Opcode::JumpLE,           89 },
        {Opcode::JumpLT,           90 },
        {Opcode::JumpGE,           91 },
        {Opcode::JumpGT,           92 },
        {Opcode::Call,             93 },
        {Opcode::GetGlobalP3,      94 },
        {Opcode::GetGlobal3,       95 },
        {Opcode::SetGlobal3,       96 },
        {Opcode::PushI24,          97 },
        {Opcode::Switch,           98 },
        {Opcode::PushStringS,      99 },
        {Opcode::GetHash,          100},
        {Opcode::StrCopy,          101},
        {Opcode::ItoS,             102},
        {Opcode::StrAdd,           103},
        {Opcode::StrAddi,          104},
        {Opcode::MemCopy,          105},
        {Opcode::Catch,            106},
        {Opcode::Throw,            107},
        {Opcode::pCall,            108},
        {Opcode::Push_Neg1,        109},
        {Opcode::Push_0,           110},
        {Opcode::Push_1,           111},
        {Opcode::Push_2,           112},
        {Opcode::Push_3,           113},
        {Opcode::Push_4,           114},
        {Opcode::Push_5,           115},
        {Opcode::Push_6,           116},
        {Opcode::Push_7,           117},
        {Opcode::PushF_Neg1,       118},
        {Opcode::PushF_0,          119},
        {Opcode::PushF_1,          120},
        {Opcode::PushF_2,          121},
        {Opcode::PushF_3,          122},
        {Opcode::PushF_4,          123},
        {Opcode::PushF_5,          124},
        {Opcode::PushF_6,          125},
        {Opcode::PushF_7,          126}
    };

#pragma endregion

protected:

    void AddCallNative() override;

    void WriteScript(const std::string& scriptOutPath) override;


public:
    CompileGTAVConsole(std::string& text);

};

class CompileGTAVPC : public CompileGTAVConsole
{
#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
        {Opcode::Nop,              0  },
        {Opcode::Add,              1  },
        {Opcode::Sub,              2  },
        {Opcode::Mult,             3  },
        {Opcode::Div,              4  },
        {Opcode::Mod,              5  },
        {Opcode::Not,              6  },
        {Opcode::Neg,              7  },
        {Opcode::CmpEQ,            8  },
        {Opcode::CmpNE,            9  },
        {Opcode::CmpGT,            10 },
        {Opcode::CmpGE,            11 },
        {Opcode::CmpLT,            12 },
        {Opcode::CmpLE,            13 },
        {Opcode::fAdd,             14 },
        {Opcode::fSub,             15 },
        {Opcode::fMult,            16 },
        {Opcode::fDiv,             17 },
        {Opcode::fMod,             18 },
        {Opcode::fNeg,             19 },
        {Opcode::fCmpEQ,           20 },
        {Opcode::fCmpNE,           21 },
        {Opcode::fCmpGT,           22 },
        {Opcode::fCmpGE,           23 },
        {Opcode::fCmpLT,           24 },
        {Opcode::fCmpLE,           25 },
        {Opcode::vAdd,             26 },
        {Opcode::vSub,             27 },
        {Opcode::vMult,            28 },
        {Opcode::vDiv,             29 },
        {Opcode::vNeg,             30 },
        {Opcode::And,              31 },
        {Opcode::Or,               32 },
        {Opcode::Xor,              33 },
        {Opcode::ItoF,             34 },
        {Opcode::FtoI,             35 },
        {Opcode::FtoV,             36 },
        {Opcode::PushB,            37 },
        {Opcode::PushB2,           38 },
        {Opcode::PushB3,           39 },
        {Opcode::Push,             40 },
        {Opcode::PushF,            41 },
        {Opcode::Dup,              42 },
        {Opcode::Drop,             43 },
        {Opcode::CallNative,       44 },
        {Opcode::Function,         45 },
        {Opcode::Return,           46 },
        {Opcode::pGet,             47 },
        {Opcode::pSet,             48 },
        {Opcode::pPeekSet,         49 },
        {Opcode::ToStack,          50 },
        {Opcode::FromStack,        51 },
        {Opcode::GetArrayP1,       52 },
        {Opcode::GetArray1,        53 },
        {Opcode::SetArray1,        54 },
        {Opcode::GetLocalP1,       55 },
        {Opcode::GetLocal1,        56 },
        {Opcode::SetLocal1,        57 },
        {Opcode::GetStaticP1,      58 },
        {Opcode::GetStatic1,       59 },
        {Opcode::SetStatic1,       60 },
        {Opcode::AddImm1,          61 },
        {Opcode::MultImm1,         62 },
        {Opcode::GetImmPs,         63 },
        {Opcode::GetImmP1,         64 },
        {Opcode::GetImm1,          65 },
        {Opcode::SetImm1,          66 },
        {Opcode::PushS,            67 },
        {Opcode::AddImm2,          68 },
        {Opcode::MultImm2,         69 },
        {Opcode::GetImmP2,         70 },
        {Opcode::GetImm2,          71 },
        {Opcode::SetImm2,          72 },
        {Opcode::GetArrayP2,       73 },
        {Opcode::GetArray2,        74 },
        {Opcode::SetArray2,        75 },
        {Opcode::GetLocalP2,       76 },
        {Opcode::GetLocal2,        77 },
        {Opcode::SetLocal2,        78 },
        {Opcode::GetStaticP2,      79 },
        {Opcode::GetStatic2,       80 },
        {Opcode::SetStatic2,       81 },
        {Opcode::GetGlobalP2,      82 },
        {Opcode::GetGlobal2,       83 },
        {Opcode::SetGlobal2,       84 },
        {Opcode::Jump,             85 },
        {Opcode::JumpFalse,        86 },
        {Opcode::JumpNE,           87 },
        {Opcode::JumpEQ,           88 },
        {Opcode::JumpLE,           89 },
        {Opcode::JumpLT,           90 },
        {Opcode::JumpGE,           91 },
        {Opcode::JumpGT,           92 },
        {Opcode::Call,             93 },
        {Opcode::GetGlobalP3,      94 },
        {Opcode::GetGlobal3,       95 },
        {Opcode::SetGlobal3,       96 },
        {Opcode::PushI24,          97 },
        {Opcode::Switch,           98 },
        {Opcode::PushStringS,      99 },
        {Opcode::GetHash,          100},
        {Opcode::StrCopy,          101},
        {Opcode::ItoS,             102},
        {Opcode::StrAdd,           103},
        {Opcode::StrAddi,          104},
        {Opcode::MemCopy,          105},
        {Opcode::Catch,            106},
        {Opcode::Throw,            107},
        {Opcode::pCall,            108},
        {Opcode::Push_Neg1,        109},
        {Opcode::Push_0,           110},
        {Opcode::Push_1,           111},
        {Opcode::Push_2,           112},
        {Opcode::Push_3,           113},
        {Opcode::Push_4,           114},
        {Opcode::Push_5,           115},
        {Opcode::Push_6,           116},
        {Opcode::Push_7,           117},
        {Opcode::PushF_Neg1,       118},
        {Opcode::PushF_0,          119},
        {Opcode::PushF_1,          120},
        {Opcode::PushF_2,          121},
        {Opcode::PushF_3,          122},
        {Opcode::PushF_4,          123},
        {Opcode::PushF_5,          124},
        {Opcode::PushF_6,          125},
        {Opcode::PushF_7,          126},
        {Opcode::BitTest,          127}
    };
#pragma endregion

protected:

    void WriteScript(const std::string& scriptOutPath) override;
public:
    CompileGTAVPC(std::string& text);

};

class CompileRDR2Console : public CompileGTAVPC
{
#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
        {Opcode::Nop,              0  },
        {Opcode::Add,              1  },
        {Opcode::Sub,              2  },
        {Opcode::Mult,             3  },
        {Opcode::Div,              4  },
        {Opcode::Mod,              5  },
        {Opcode::Not,              6  },
        {Opcode::Neg,              7  },
        {Opcode::CmpEQ,            8  },
        {Opcode::CmpNE,            9  },
        {Opcode::CmpGT,            10 },
        {Opcode::CmpGE,            11 },
        {Opcode::CmpLT,            12 },
        {Opcode::CmpLE,            13 },
        {Opcode::fAdd,             14 },
        {Opcode::fSub,             15 },
        {Opcode::fMult,            16 },
        {Opcode::fDiv,             17 },
        {Opcode::fMod,             18 },
        {Opcode::fNeg,             19 },
        {Opcode::fCmpEQ,           20 },
        {Opcode::fCmpNE,           21 },
        {Opcode::fCmpGT,           22 },
        {Opcode::fCmpGE,           23 },
        {Opcode::fCmpLT,           24 },
        {Opcode::fCmpLE,           25 },
        {Opcode::vAdd,             26 },
        {Opcode::vSub,             27 },
        {Opcode::vMult,            28 },
        {Opcode::vDiv,             29 },
        {Opcode::vNeg,             30 },
        {Opcode::And,              31 },
        {Opcode::Or,               32 },
        {Opcode::Xor,              33 },
        {Opcode::ItoF,             34 },
        {Opcode::FtoI,             35 },
        {Opcode::FtoV,             36 },
        {Opcode::PushB,            37 },
        {Opcode::PushB2,           38 },
        {Opcode::PushB3,           39 },
        {Opcode::Push,             40 },
        {Opcode::PushF,            41 },
        {Opcode::Dup,              42 },
        {Opcode::Drop,             43 },
        {Opcode::CallNative,       44 },
        {Opcode::Function,         45 },
        {Opcode::Return,           46 },
        {Opcode::pGet,             47 },
        {Opcode::pSet,             48 },
        {Opcode::pPeekSet,         49 },
        {Opcode::ToStack,          50 },
        {Opcode::FromStack,        51 },
        {Opcode::GetArrayP1,       52 },
        {Opcode::GetArray1,        53 },
        {Opcode::SetArray1,        54 },
        {Opcode::GetLocalP1,       55 },
        {Opcode::GetLocal1,        56 },
        {Opcode::SetLocal1,        57 },
        {Opcode::GetStaticP1,      58 },
        {Opcode::GetStatic1,       59 },
        {Opcode::SetStatic1,       60 },
        {Opcode::AddImm1,          61 },
        {Opcode::MultImm1,         62 },
        {Opcode::GetImmPs,         63 },
        {Opcode::GetImmP1,         64 },
        {Opcode::GetImm1,          65 },
        {Opcode::SetImm1,          66 },
        {Opcode::PushS,            67 },
        {Opcode::AddImm2,          68 },
        {Opcode::MultImm2,         69 },
        {Opcode::GetImmP2,         70 },
        {Opcode::GetImm2,          71 },
        {Opcode::SetImm2,          72 },
        {Opcode::GetArrayP2,       73 },
        {Opcode::GetArray2,        74 },
        {Opcode::SetArray2,        75 },
        {Opcode::GetLocalP2,       76 },
        {Opcode::GetLocal2,        77 },
        {Opcode::SetLocal2,        78 },
        {Opcode::GetStaticP2,      79 },
        {Opcode::GetStatic2,       80 },
        {Opcode::SetStatic2,       81 },
        {Opcode::GetGlobalP2,      82 },
        {Opcode::GetGlobal2,       83 },
        {Opcode::SetGlobal2,       84 },
        {Opcode::Jump,             85 },
        {Opcode::JumpFalse,        86 },
        {Opcode::JumpNE,           87 },
        {Opcode::JumpEQ,           88 },
        {Opcode::JumpLE,           89 },
        {Opcode::JumpLT,           90 },
        {Opcode::JumpGE,           91 },
        {Opcode::JumpGT,           92 },
        {Opcode::Call,             93 },
        {Opcode::GetStaticP3,      94 },
        {Opcode::GetStatic3,       95 },
        {Opcode::SetStatic3,       96 },
        {Opcode::GetGlobalP3,      97 },
        {Opcode::GetGlobal3,       98 },
        {Opcode::SetGlobal3,       99 },
        {Opcode::PushI24,          100},
        {Opcode::Switch,           101},
        {Opcode::PushStringS,      102},
        {Opcode::GetHash,          103},
        {Opcode::StrCopy,          104},
        {Opcode::ItoS,             105},
        {Opcode::StrAdd,           106},
        {Opcode::StrAddi,          107},
        {Opcode::MemCopy,          108},
        {Opcode::Catch,            109},
        {Opcode::Throw,            110},
        {Opcode::pCall,            111},
        {Opcode::Push_Neg1,        112},
        {Opcode::Push_0,           113},
        {Opcode::Push_1,           114},
        {Opcode::Push_2,           115},
        {Opcode::Push_3,           116},
        {Opcode::Push_4,           117},
        {Opcode::Push_5,           118},
        {Opcode::Push_6,           119},
        {Opcode::Push_7,           120},
        {Opcode::PushF_Neg1,       121},
        {Opcode::PushF_0,          122},
        {Opcode::PushF_1,          123},
        {Opcode::PushF_2,          124},
        {Opcode::PushF_3,          125},
        {Opcode::PushF_4,          126},
        {Opcode::PushF_5,          127},
        {Opcode::PushF_6,          128},
        {Opcode::PushF_7,          129},
        {Opcode::GetLocalS,        130},
        {Opcode::SetLocalS,        131},
        {Opcode::SetLocalSR,       132},
        {Opcode::GetStaticS,       133},
        {Opcode::SetStaticS,       134},
        {Opcode::SetStaticSR,      135},
        {Opcode::pGetS,            136},
        {Opcode::pSetS,            137},
        {Opcode::pSetSR,           138},
        {Opcode::GetGlobalS,       139},
        {Opcode::SetGlobalS,       140},
        {Opcode::SetGlobalSR,      141}
    };
#pragma endregion

protected:

    void AddSwitch() override;
    virtual void LoadOpcodes();

    void WriteScript(const std::string& scriptOutPath) override;

public:

    CompileRDR2Console(std::string& text);

};

class CompileRDR2PC : public CompileRDR2Console
{
#pragma region vars
    std::unordered_map<Opcode, uint8_t> _CommonOpsToTargetOps =
    {
         {Opcode::Nop,              0  },
         {Opcode::Add,              1  },
         {Opcode::Sub,              2  },
         {Opcode::Mult,             3  },
         {Opcode::Div,              4  },
         {Opcode::Mod,              5  },
         {Opcode::Not,              6  },
         {Opcode::Neg,              7  },
         {Opcode::CmpEQ,            8  },
         {Opcode::CmpNE,            9  },
         {Opcode::CmpGT,            10 },
         {Opcode::CmpGE,            11 },
         {Opcode::CmpLT,            12 },
         {Opcode::CmpLE,            13 },
         {Opcode::fAdd,             14 },
         {Opcode::fSub,             15 },
         {Opcode::fMult,            16 },
         {Opcode::fDiv,             17 },
         {Opcode::fMod,             18 },
         {Opcode::fNeg,             19 },
         {Opcode::fCmpEQ,           20 },
         {Opcode::fCmpNE,           21 },
         {Opcode::fCmpGT,           22 },
         {Opcode::fCmpGE,           23 },
         {Opcode::fCmpLT,           24 },
         {Opcode::fCmpLE,           25 },
         {Opcode::vAdd,             26 },
         {Opcode::vSub,             27 },
         {Opcode::vMult,            28 },
         {Opcode::vDiv,             29 },
         {Opcode::vNeg,             30 },
         {Opcode::And,              31 },
         {Opcode::Or,               32 },
         {Opcode::Xor,              33 },
         {Opcode::ItoF,             34 },
         {Opcode::FtoI,             35 },
         {Opcode::FtoV,             36 },
         {Opcode::PushB,            37 },
         {Opcode::PushB2,           38 },
         {Opcode::PushB3,           39 },
         {Opcode::Push,             40 },
         {Opcode::PushF,            41 },
         {Opcode::Dup,              42 },
         {Opcode::Drop,             43 },
         {Opcode::CallNative,       44 },
         {Opcode::Function,         45 },
         {Opcode::Return,           46 },
         {Opcode::pGet,             47 },
         {Opcode::pSet,             48 },
         {Opcode::pPeekSet,         49 },
         {Opcode::ToStack,          50 },
         {Opcode::FromStack,        51 },
         {Opcode::GetArrayP1,       52 },
         {Opcode::GetArray1,        53 },
         {Opcode::SetArray1,        54 },
         {Opcode::GetLocalP1,       55 },
         {Opcode::GetLocal1,        56 },
         {Opcode::SetLocal1,        57 },
         {Opcode::GetStaticP1,      58 },
         {Opcode::GetStatic1,       59 },
         {Opcode::SetStatic1,       60 },
         {Opcode::AddImm1,          61 },
         {Opcode::MultImm1,         62 },
         {Opcode::GetImmPs,         63 },
         {Opcode::GetImmP1,         64 },
         {Opcode::GetImm1,          65 },
         {Opcode::SetImm1,          66 },
         {Opcode::PushS,            67 },
         {Opcode::AddImm2,          68 },
         {Opcode::MultImm2,         69 },
         {Opcode::GetImmP2,         70 },
         {Opcode::GetImm2,          71 },
         {Opcode::SetImm2,          72 },
         {Opcode::GetArrayP2,       73 },
         {Opcode::GetArray2,        74 },
         {Opcode::SetArray2,        75 },
         {Opcode::GetLocalP2,       76 },
         {Opcode::GetLocal2,        77 },
         {Opcode::SetLocal2,        78 },
         {Opcode::GetStaticP2,      79 },
         {Opcode::GetStatic2,       80 },
         {Opcode::SetStatic2,       81 },
         {Opcode::GetGlobalP2,      82 },
         {Opcode::GetGlobal2,       83 },
         {Opcode::SetGlobal2,       84 },
         {Opcode::Jump,             85 },
         {Opcode::JumpFalse,        86 },
         {Opcode::JumpNE,           87 },
         {Opcode::JumpEQ,           88 },
         {Opcode::JumpLE,           89 },
         {Opcode::JumpLT,           90 },
         {Opcode::JumpGE,           91 },
         {Opcode::JumpGT,           92 },
         {Opcode::Call,             93 },
         {Opcode::GetGlobalP3,      94 },
         {Opcode::GetGlobal3,       95 },
         {Opcode::SetGlobal3,       96 },
         {Opcode::PushI24,          97 },
         {Opcode::Switch,           98 },
         {Opcode::PushStringS,      99 },
         {Opcode::GetHash,          100},
         {Opcode::StrCopy,          101},
         {Opcode::ItoS,             102},
         {Opcode::StrAdd,           103},
         {Opcode::StrAddi,          104},
         {Opcode::MemCopy,          105},
         {Opcode::Catch,            106},
         {Opcode::Throw,            107},
         {Opcode::pCall,            108},
         {Opcode::Push_Neg1,        109},
         {Opcode::Push_0,           110},
         {Opcode::Push_1,           111},
         {Opcode::Push_2,           112},
         {Opcode::Push_3,           113},
         {Opcode::Push_4,           114},
         {Opcode::Push_5,           115},
         {Opcode::Push_6,           116},
         {Opcode::Push_7,           117},
         {Opcode::PushF_Neg1,       118},
         {Opcode::PushF_0,          119},
         {Opcode::PushF_1,          120},
         {Opcode::PushF_2,          121},
         {Opcode::PushF_3,          122},
         {Opcode::PushF_4,          123},
         {Opcode::PushF_5,          124},
         {Opcode::PushF_6,          125},
         {Opcode::PushF_7,          126},
         {Opcode::GetLocalS,        127},
         {Opcode::SetLocalS,        128},
         {Opcode::SetLocalSR,       129},
         {Opcode::GetStaticS,       130},
         {Opcode::SetStaticS,       131},
         {Opcode::SetStaticSR,      132},
         {Opcode::pGetS,            133},
         {Opcode::pSetS,            134},
         {Opcode::pSetSR,           135},
         {Opcode::GetGlobalS,       136},
         {Opcode::SetGlobalS,       137},
         {Opcode::SetGlobalSR,      138},
         {Opcode::GetStaticP3,      139},
         {Opcode::GetStatic3,       140},
         {Opcode::SetStatic3,       141}
    };
#pragma endregion

protected:

    void LoadOpcodes() override;

public:

    CompileRDR2PC(std::string& text);

};
