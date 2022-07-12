#pragma once
#include <unordered_map>

const int XboxPageSize = 16384;
const int XboxBaseSize = 8192;
const int PS3BaseSize = 4096;

namespace GameTarget
{
    typedef enum GameTarget
    {
        UNK,
        GTAIV,
        RDR,
        RDR_SCO,
        GTAV,
        RDR2,
        _Count
    } GameTarget;

    const std::string TargetName[GameTarget::_Count] =
    {
        "",
        "GTAIV",
        "RDR",
        "RDR_SCO",
        "GTAV",
        "RDR2"
    };

    const std::string TargetNameGeneric[GameTarget::_Count] =
    {
        "",
        "GTAIV",
        "RDR",
        "RDR",
        "GTAV",
        "RDR2"
    };
}

namespace Platform
{
    typedef enum Platform
    {
        UNK,
        XBOX,
        PSX,
        PC,
        _Count
    } Platform;

    const std::string PlatformName[Platform::_Count] =
    {
        "",
        "XBOX",
        "PSX",
        "PC"
    };

    const std::string PlatformNameGeneric[Platform::_Count] =
    {
        "",
        "Console",
        "Console",
        "PC"
    };
}

enum class Opcode : uint16_t
{
    Nop,
    Add,
    Sub,
    Mult,
    Div,
    Mod,
    Not,
    Neg,
    CmpEQ,
    CmpNE,
    CmpGT,
    CmpGE,
    CmpLT,
    CmpLE,
    fAdd,
    fSub,
    fMult,
    fDiv,
    fMod,
    fNeg,
    fCmpEQ,
    fCmpNE,
    fCmpGT,
    fCmpGE,
    fCmpLT,
    fCmpLE,
    vAdd,
    vSub,
    vMult,
    vDiv,
    vNeg,
    And,
    Or,
    Xor,
    ItoF,
    FtoI,
    FtoV,
    PushB,//rdr, gtav, rdr2
    PushB2,//rdr, gtav, rdr2
    PushB3,//rdr, gtav, rdr2
    Push,
    PushF,
    Dup,
    Drop,
    CallNative,
    Function,
    Return,
    pGet,
    pSet,
    pPeekSet,
    ToStack,
    FromStack,
    GetArrayPs,//gtaiv only
    GetArrayP1,//rdr and gtav only
    GetArray1,//rdr and gtav only
    SetArray1,//rdr and gtav only
    GetLocalPv0,//gtaiv only
    GetLocalPv1,//gtaiv only
    GetLocalPv2,//gtaiv only
    GetLocalPv3,//gtaiv only
    GetLocalPv4,//gtaiv only
    GetLocalPv5,//gtaiv only
    GetLocalPv6,//gtaiv only
    GetLocalPv7,//gtaiv only
    GetLocalPs,//gtaiv only
    GetLocalP1,//rdr and gtav only
    GetLocal1,//rdr and gtav only
    SetLocal1,//rdr and gtav only
    GetStaticPs,//gtaiv only
    GetStaticP1,//rdr and gtav only
    GetStatic1,//rdr and gtav only
    SetStatic1,//rdr and gtav only
    AddImm1,//rdr and gtav only
    MultImm1,//rdr and gtav only
    GetImmPs,//rdr and gtav only
    GetImmP1,//rdr and gtav only
    GetImm1,//rdr and gtav only
    SetImm1,//rdr and gtav only
    PushS,
    AddImm2,//rdr and gtav only
    MultImm2,//rdr and gtav only
    GetImmP2,//rdr and gtav only
    GetImm2,//rdr and gtav only
    SetImm2,//rdr and gtav only
    GetArrayP2,//rdr and gtav only
    GetArray2,//rdr and gtav only
    SetArray2,//rdr and gtav only
    GetLocalP2,//rdr and gtav only
    GetLocal2,//rdr and gtav only
    SetLocal2,//rdr and gtav only
    GetStaticP2,//rdr and gtav only
    GetStatic2,//rdr and gtav only
    SetStatic2,//rdr and gtav only
    GetGlobalPs,//gtaiv only
    GetGlobalP2,//rdr and gtav only
    GetGlobal2,//rdr and gtav only
    SetGlobal2,//rdr and gtav only
    Jump,
    JumpFalse,
    JumpTrue,//gtaiv / custom opcode
    JumpNE,
    JumpEQ,
    JumpLE,
    JumpLT,
    JumpGE,
    JumpGT,
    Call,//gtaiv and gtav only
    Call2,//rdr only
    Call2h1,//rdr only
    Call2h2,//rdr only
    Call2h3,//rdr only
    Call2h4,//rdr only
    Call2h5,//rdr only
    Call2h6,//rdr only
    Call2h7,//rdr only
    Call2h8,//rdr only
    Call2h9,//rdr only
    Call2hA,//rdr only
    Call2hB,//rdr only
    Call2hC,//rdr only
    Call2hD,//rdr only
    Call2hE,//rdr only
    Call2hF,//rdr only
    GetGlobalP3,//rdr and gtav only
    GetGlobal3,//rdr and gtav only
    SetGlobal3,//rdr and gtav only
    PushI24,
    Switch,
    PushString,
    PushStringS,
    PushArray,//rdr only
    PushStringNull,//gtaiv and rdr only
    GetHash,//gtav only
    StrCopy,
    ItoS,
    StrAdd,
    StrAddi,
    MemCopy,
    Catch,
    Throw,
    pCall,
    GetXProtect,//gtaiv PC only
    SetXProtect,//gtaiv PC only
    RefXProtect,//gtaiv PC only
    Exit,//gtaiv only
    ReturnP0R0,//rdr only
    ReturnP0R1,//rdr only
    ReturnP0R2,//rdr only
    ReturnP0R3,//rdr only
    ReturnP1R0,//rdr only
    ReturnP1R1,//rdr only
    ReturnP1R2,//rdr only
    ReturnP1R3,//rdr only
    ReturnP2R0,//rdr only
    ReturnP2R1,//rdr only
    ReturnP2R2,//rdr only
    ReturnP2R3,//rdr only
    ReturnP3R0,//rdr only
    ReturnP3R1,//rdr only
    ReturnP3R2,//rdr only
    ReturnP3R3,//rdr only
    Push_Neg16,//gtaiv only
    Push_Neg15,//gtaiv only
    Push_Neg14,//gtaiv only
    Push_Neg13,//gtaiv only
    Push_Neg12,//gtaiv only
    Push_Neg11,//gtaiv only
    Push_Neg10,//gtaiv only
    Push_Neg9,//gtaiv only
    Push_Neg8,//gtaiv only
    Push_Neg7,//gtaiv only
    Push_Neg6,//gtaiv only
    Push_Neg5,//gtaiv only
    Push_Neg4,//gtaiv only
    Push_Neg3,//gtaiv only
    Push_Neg2,//gtaiv only
    Push_Neg1,
    Push_0,//keep 0 to 7 together
    Push_1,
    Push_2,
    Push_3,
    Push_4,
    Push_5,
    Push_6,
    Push_7,
    PushF_Neg1,
    PushF_0,
    PushF_1,
    PushF_2,
    PushF_3,
    PushF_4,
    PushF_5,
    PushF_6,
    PushF_7,
    BitTest,    //gta5 pc only
    GetLocalS,   //rdr2 only
    SetLocalS,   //rdr2 only
    SetLocalSR,  //rdr2 only    what is R? find examples
    GetStaticS,  //rdr2 only
    SetStaticS,  //rdr2 only
    SetStaticSR, //rdr2 only
    pGetS,        //rdr2 only    name might be wrong 
    pSetS,        //rdr2 only    name might be wrong 
    pSetSR,       //rdr2 only    name might be wrong 
    GetGlobalS,  //rdr2 only
    SetGlobalS,  //rdr2 only
    SetGlobalSR, //rdr2 only
    GetStaticP3,//rdr2 only >= 1311
    GetStatic3,//rdr2 only  >= 1311
    SetStatic3,//rdr2 only  >= 1311


    //custom opcodes
    SetStaticsCount,
    SetDefaultStatic,
    SetStaticName,
    SetLocalName,
    SetGlobalName,
    SetEnum,
    SetParamCount,
    SetSignature,
    Pad,


    //custom multi ops
    //GetArrayP,
    //GetArray,
    //SetArray,
    //GetLocalP,
    //GetLocal,
    //SetLocal,
    //GetStaticP,
    //GetStatic,
    //SetStatic,
    //GetImm,
    //GetGlobalP,
    //GetGlobal,
    //SetGlobal,
    //AddImm,
    //MultImm,

    //not opcodes
    _Count,
    Uninitialized = 0xFFFF,
};


const std::unordered_map<std::string, Opcode> OpcodeNamesUpper =
{
    {"NOP", Opcode::Nop},
    {"ADD", Opcode::Add},
    {"SUB", Opcode::Sub},
    {"MULT", Opcode::Mult},
    {"DIV", Opcode::Div},
    {"MOD", Opcode::Mod},
    {"NOT", Opcode::Not},
    {"NEG", Opcode::Neg},
    {"CMPEQ", Opcode::CmpEQ},
    {"CMPNE", Opcode::CmpNE},
    {"CMPGT", Opcode::CmpGT},
    {"CMPGE", Opcode::CmpGE},
    {"CMPLT", Opcode::CmpLT},
    {"CMPLE", Opcode::CmpLE},
    {"FADD", Opcode::fAdd},
    {"FSUB", Opcode::fSub},
    {"FMULT", Opcode::fMult},
    {"FDIV", Opcode::fDiv},
    {"FMOD", Opcode::fMod},
    {"FNEG", Opcode::fNeg},
    {"FCMPEQ", Opcode::fCmpEQ},
    {"FCMPNE", Opcode::fCmpNE},
    {"FCMPGT", Opcode::fCmpGT},
    {"FCMPGE", Opcode::fCmpGE},
    {"FCMPLT", Opcode::fCmpLT},
    {"FCMPLE", Opcode::fCmpLE},
    {"VADD", Opcode::vAdd},
    {"VSUB", Opcode::vSub},
    {"VMULT", Opcode::vMult},
    {"VDIV", Opcode::vDiv},
    {"VNEG", Opcode::vNeg},
    {"AND", Opcode::And},
    {"OR", Opcode::Or},
    {"XOR", Opcode::Xor},
    {"ITOF", Opcode::ItoF},
    {"FTOI", Opcode::FtoI},
    {"FTOV", Opcode::FtoV},
    {"PUSHB", Opcode::PushB},
    {"PUSHB2", Opcode::PushB2},
    {"PUSHB3", Opcode::PushB3},
    {"PUSH", Opcode::Push},
    {"PUSHF", Opcode::PushF},
    {"DUP", Opcode::Dup},
    {"DROP", Opcode::Drop},
    {"CALLNATIVE", Opcode::CallNative},
    {"FUNCTION", Opcode::Function},
    {"RETURN", Opcode::Return},
    {"PGET", Opcode::pGet},
    {"PSET", Opcode::pSet},
    {"PPEEKSET", Opcode::pPeekSet},
    {"TOSTACK", Opcode::ToStack},
    {"FROMSTACK", Opcode::FromStack},
    {"GETARRAYPS", Opcode::GetArrayPs},
    //{"GETARRAYP1", Opcode::GetArrayP1},
    //{"GETARRAY1", Opcode::GetArray1},
    //{"SETARRAY1", Opcode::SetArray1},
    //{"GETFRAMEPV0", Opcode::GetLocalPv0},
    //{"GETFRAMEPV1", Opcode::GetLocalPv1},
    //{"GETFRAMEPV2", Opcode::GetLocalPv2},
    //{"GETFRAMEPV3", Opcode::GetLocalPv3},
    //{"GETFRAMEPV4", Opcode::GetLocalPv4},
    //{"GETFRAMEPV5", Opcode::GetLocalPv5},
    //{"GETFRAMEPV6", Opcode::GetLocalPv6},
    //{"GETFRAMEPV7", Opcode::GetLocalPv7},
    {"GETLOCALPS", Opcode::GetLocalPs},
    //{"GETFRAMEP1", Opcode::GetLocalP1},
    //{"GETFRAME1", Opcode::GetLocal1},
    //{"SETFRAME1", Opcode::SetLocal1},
    {"GETSTATICPS", Opcode::GetStaticPs},
    //{"GETSTATICP1", Opcode::GetStaticP1},
    //{"GETSTATIC1", Opcode::GetStatic1},
    //{"SETSTATIC1", Opcode::SetStatic1},
    //{"ADD1", Opcode::AddImm1},
    //{"MULT1", Opcode::MultImm1},
    {"GETIMMPS", Opcode::GetImmPs},
    //{"GETIMMP1", Opcode::GetImmP1},
    //{"GETIMM1", Opcode::GetImm1},
    //{"SETIMM1", Opcode::SetImm1},
    //{"PUSHS", Opcode::PushS},
    //{"ADD2", Opcode::AddImm2},
    //{"MULT2", Opcode::MultImm2},
    //{"GETIMMP2", Opcode::GetImmP2},
    //{"GETIMM2", Opcode::GetImm2},
    //{"SETIMM2", Opcode::SetImm2},
    //{"GETARRAYP2", Opcode::GetArrayP2},
    //{"GETARRAY2", Opcode::GetArray2},
    //{"SETARRAY2", Opcode::SetArray2},
    //{"GETFRAMEP2", Opcode::GetLocalP2},
    //{"GETFRAME2", Opcode::GetLocal2},
    //{"SETFRAME2", Opcode::SetLocal2},
    //{"GETSTATICP2", Opcode::GetStaticP2},
    //{"GETSTATIC2", Opcode::GetStatic2},
    //{"SETSTATIC2", Opcode::SetStatic2},
    {"GETGLOBALPS", Opcode::GetGlobalPs},
    //{"GETGLOBALP2", Opcode::GetGlobalP2},
    //{"GETGLOBAL2", Opcode::GetGlobal2},
    //{"SETGLOBAL2", Opcode::SetGlobal2},
    {"JUMP", Opcode::Jump},
    {"JUMPFALSE", Opcode::JumpFalse},
    {"JUMPTRUE", Opcode::JumpTrue},
    {"JUMPNE", Opcode::JumpNE},
    {"JUMPEQ", Opcode::JumpEQ},
    {"JUMPLE", Opcode::JumpLE},
    {"JUMPLT", Opcode::JumpLT},
    {"JUMPGE", Opcode::JumpGE},
    {"JUMPGT", Opcode::JumpGT},
    {"CALL", Opcode::Call},
    //{"CALL2", Opcode::Call2},
    //{"CALL2H1", Opcode::Call2h1},
    //{"CALL2H2", Opcode::Call2h2},
    //{"CALL2H3", Opcode::Call2h3},
    //{"CALL2H4", Opcode::Call2h4},
    //{"CALL2H5", Opcode::Call2h5},
    //{"CALL2H6", Opcode::Call2h6},
    //{"CALL2H7", Opcode::Call2h7},
    //{"CALL2H8", Opcode::Call2h8},
    //{"CALL2H9", Opcode::Call2h9},
    //{"CALL2HA", Opcode::Call2hA},
    //{"CALL2HB", Opcode::Call2hB},
    //{"CALL2HC", Opcode::Call2hC},
    //{"CALL2HD", Opcode::Call2hD},
    //{"CALL2HE", Opcode::Call2hE},
    //{"CALL2HF", Opcode::Call2hF},
    //{"GETGLOBALP3", Opcode::GetGlobalP3},
    //{"GETGLOBAL3", Opcode::GetGlobal3},
    //{"SETGLOBAL3", Opcode::SetGlobal3},
    //{"PUSHI24", Opcode::PushI24},
    {"SWITCH", Opcode::Switch},
    {"PUSHSTRING", Opcode::PushString},
    {"PUSHSTRINGS", Opcode::PushStringS},
    {"PUSHARRAY", Opcode::PushArray},
    //{"PUSHSTRINGNULL", Opcode::PushStringNull},
    {"GETHASH", Opcode::GetHash},
    {"STRCOPY", Opcode::StrCopy},
    {"ITOS", Opcode::ItoS},
    {"STRADD", Opcode::StrAdd},
    {"STRADDI", Opcode::StrAddi},
    {"MEMCOPY", Opcode::MemCopy},
    {"CATCH", Opcode::Catch},
    {"THROW", Opcode::Throw},
    {"PCALL", Opcode::pCall},
    {"GETXPROTECT", Opcode::GetXProtect},
    {"SETXPROTECT", Opcode::SetXProtect},
    {"REFXPROTECT", Opcode::RefXProtect},
    {"EXIT", Opcode::Exit},
    //{"RETURNP0R0", Opcode::ReturnP0R0},
    //{"RETURNP0R1", Opcode::ReturnP0R1},
    //{"RETURNP0R2", Opcode::ReturnP0R2},
    //{"RETURNP0R3", Opcode::ReturnP0R3},
    //{"RETURNP1R0", Opcode::ReturnP1R0},
    //{"RETURNP1R1", Opcode::ReturnP1R1},
    //{"RETURNP1R2", Opcode::ReturnP1R2},
    //{"RETURNP1R3", Opcode::ReturnP1R3},
    //{"RETURNP2R0", Opcode::ReturnP2R0},
    //{"RETURNP2R1", Opcode::ReturnP2R1},
    //{"RETURNP2R2", Opcode::ReturnP2R2},
    //{"RETURNP2R3", Opcode::ReturnP2R3},
    //{"RETURNP3R0", Opcode::ReturnP3R0},
    //{"RETURNP3R1", Opcode::ReturnP3R1},
    //{"RETURNP3R2", Opcode::ReturnP3R2},
    //{"RETURNP3R3", Opcode::ReturnP3R3},
    //{"PUSH_-16", Opcode::Push_Neg16},
    //{"PUSH_-15", Opcode::Push_Neg15},
    //{"PUSH_-14", Opcode::Push_Neg14},
    //{"PUSH_-13", Opcode::Push_Neg13},
    //{"PUSH_-12", Opcode::Push_Neg12},
    //{"PUSH_-11", Opcode::Push_Neg11},
    //{"PUSH_-10", Opcode::Push_Neg10},
    //{"PUSH_-9", Opcode::Push_Neg9},
    //{"PUSH_-8", Opcode::Push_Neg8},
    //{"PUSH_-7", Opcode::Push_Neg7},
    //{"PUSH_-6", Opcode::Push_Neg6},
    //{"PUSH_-5", Opcode::Push_Neg5},
    //{"PUSH_-4", Opcode::Push_Neg4},
    //{"PUSH_-3", Opcode::Push_Neg3},
    //{"PUSH_-2", Opcode::Push_Neg2},
    //{"PUSH_-1", Opcode::Push_Neg1},
    //{"PUSH_0", Opcode::Push_0},
    //{"PUSH_1", Opcode::Push_1},
    //{"PUSH_2", Opcode::Push_2},
    //{"PUSH_3", Opcode::Push_3},
    //{"PUSH_4", Opcode::Push_4},
    //{"PUSH_5", Opcode::Push_5},
    //{"PUSH_6", Opcode::Push_6},
    //{"PUSH_7", Opcode::Push_7},
    //{"PUSHF_-1", Opcode::PushF_Neg1},
    //{"PUSHF_0", Opcode::PushF_0},
    //{"PUSHF_1", Opcode::PushF_1},
    //{"PUSHF_2", Opcode::PushF_2},
    //{"PUSHF_3", Opcode::PushF_3},
    //{"PUSHF_4", Opcode::PushF_4},
    //{"PUSHF_5", Opcode::PushF_5},
    //{"PUSHF_6", Opcode::PushF_6},
    //{"PUSHF_7", Opcode::PushF_7},
    {"BITTEST", Opcode::BitTest},
    {"GETLOCALS", Opcode::GetLocalS},
    {"SETLOCALS", Opcode::SetLocalS},
    {"SETLOCALSR", Opcode::SetLocalSR},
    {"GETSTATICS", Opcode::GetStaticS},
    {"SETSTATICS", Opcode::SetStaticS},
    {"SETSTATICSR", Opcode::SetStaticSR},
    {"PGETS", Opcode::pGetS},
    {"PSETS", Opcode::pSetS},
    {"PSETSR", Opcode::pSetSR},
    {"GETGLOBALS", Opcode::GetGlobalS},
    {"SETGLOBALS", Opcode::SetGlobalS},
    {"SETGLOBALSR", Opcode::SetGlobalSR},
    //{"GETSTATICP3", Opcode::GetStaticP3},
    //{"GETSTATIC3", Opcode::GetStatic3},
    //{"SETSTATIC3", Opcode::SetStatic3},
    //custom opcodes
    {"JUMPTRUE", Opcode::JumpTrue},
    {"SETSTATICSCOUNT", Opcode::SetStaticsCount},
    {"SETDEFAULTSTATIC", Opcode::SetDefaultStatic},
    {"SETSTATICNAME", Opcode::SetStaticName},
    {"SETLOCALNAME", Opcode::SetLocalName},
    {"SETGLOBALNAME", Opcode::SetLocalName},
    {"SETENUM", Opcode::SetEnum},
    {"SETPARAMCOUNT", Opcode::SetParamCount},
    {"SETSIGNATURE", Opcode::SetSignature},
    {"PAD", Opcode::Pad},

    { "ADDIMM", Opcode::AddImm1 },
    { "MULTIMM", Opcode::MultImm1 },
    { "GETARRAYP", Opcode::GetArrayP1 },
    { "GETARRAY", Opcode::GetArray1 },
    { "SETARRAY", Opcode::SetArray1 },
    { "GETLOCALP", Opcode::GetLocalP1 },
    { "GETLOCAL", Opcode::GetLocal1 },
    { "SETLOCAL", Opcode::SetLocal1 },
    { "GETSTATICP", Opcode::GetStaticP1 },
    { "GETSTATIC", Opcode::GetStatic1 },
    { "SETSTATIC", Opcode::SetStatic1 },
    { "GETIMMP", Opcode::GetImmP1 },
    { "GETIMM", Opcode::GetImm1 },
    { "SETIMM", Opcode::SetImm1 },
    { "GETGLOBALP", Opcode::GetGlobalP2 },
    { "GETGLOBAL", Opcode::GetGlobal2 },
    { "SETGLOBAL", Opcode::SetGlobal2 }

};

enum PushFunctions : uint16_t
{
    GetHash,
    GetEnum
};

enum DataTableType
{
    CODE,
    STRING
};

enum EnumType
{
    Int,
    Float,
    String
};
