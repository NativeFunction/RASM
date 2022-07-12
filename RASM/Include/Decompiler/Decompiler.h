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
#include "Utils/DataReader.h"
#include "Utils/types.h"
#include "Game/GameHeaders.h"


class DecompileBase
{
private:
    Opcode CommonOpsIndexedByTargetOps[255];
    bool IsLittleEndian;

protected:

#pragma region Vars
    uint8_t* CurrentReadPos = nullptr;
    uint32_t CurrentOpSize = 1;

    CommonHeader CommonHeader;

    GameTarget::GameTarget DecompileTarget;
    uint32_t MaxPageSize;
    bool Is64Bit = false;

    std::string OutHeader = "";
    std::string Out = "";

    std::vector<uint8_t> CodeData;
    std::vector<char> StringData;

    std::unordered_map<uint64_t, std::string> NativeMap;
    std::unordered_map<uint64_t, uint64_t> NativeTranslationMap;

    std::unique_ptr<DataReader> Reader;

    //pc, call label index
    std::map<uint32_t, std::string> CallLabelIndexes;
    //pc, jump label index
    std::map<uint32_t, uint32_t> JumpLabelIndexes;

    //pc,
    std::set<uint32_t> UsedFunctions;

    //function vars
    uint32_t UnusedFunctionCounter = 0;
    bool IsFirstFunction = true;
    std::set<uint32_t>::iterator FunctionInc;

#pragma endregion

#pragma region CTOR
    DecompileBase(const Opcode commonOpsIndexedByTargetOps[255], GameTarget::GameTarget decompileTarget,
        uint32_t maxPageSize = 16384, bool isLittleEndian = false, bool is64Bit = false) :
        DecompileTarget(decompileTarget),
        MaxPageSize(maxPageSize),
        Is64Bit(is64Bit)
    {
        SetEndian(isLittleEndian);
        SetOps(commonOpsIndexedByTargetOps);
        FunctionInc = UsedFunctions.begin();
    }
#pragma endregion

#pragma region Utilities
    inline void VerboseComment(const std::string& s1, const std::string& s2)
    {
        Out += s1; Out += s2; Out += "\r\n";
    }
    inline void VerboseComment(const std::string& s1, const long long s2)
    {
        Out += s1; Out += std::to_string(s2); Out += "\r\n";
    }
    void ParseStaticData();
    virtual void LoadCodeData();
    virtual void LoadStringData();
    void LoadLabels();
    void CheckFunctionUnused(char* unusedFunctionName = nullptr, uint32_t len = 0);
    bool FindFunctionFromCallPos(uint8_t* position, uint8_t*& outPosition);
    void SetOps(const Opcode commonOpsIndexedByTargetOps[255])
    {
        memcpy_s((void*)CommonOpsIndexedByTargetOps, 255 * sizeof(Opcode), (void*)commonOpsIndexedByTargetOps, 255 * sizeof(Opcode));
    }
    Opcode* GetOps()
    {
        return CommonOpsIndexedByTargetOps;
    }
    std::string GetOptionsDataName(std::string dataName);
    void SetEndian(bool isLittleEndian)
    {
        if (isLittleEndian)
            Reader = std::make_unique<DataReaderLit>();
        else
            Reader = std::make_unique<DataReaderBig>();

        IsLittleEndian = isLittleEndian;
    }
#pragma endregion

#pragma region Labels
    //override gtaiv
    virtual void LogJumpLabel()
    {
        CurrentOpSize = 3;
        JumpLabelIndexes.insert({ CurrentReadPos + 3 - CodeData.data() + Reader->ReadInt16(CurrentReadPos + 1), JumpLabelIndexes.size() });
    }

    //override gtaiv rdr2
    virtual void LogSwitchLabel()
    {
        uint8_t caseCount = *(CurrentReadPos + 1);
        CurrentOpSize = 2 + caseCount * 6;

        for (uint32_t i = 0; i < caseCount; i++)
            JumpLabelIndexes.insert({ (CurrentReadPos + 2 + i * 6 + 6) - CodeData.data() + Reader->ReadInt16(CurrentReadPos + 2 + i * 6 + 4), JumpLabelIndexes.size() });
    }

    virtual void LogCallLabel(uint8_t callType = 0xFF) = 0;
#pragma endregion

#pragma region OpcodesSizes

    virtual inline size_t GetSwitchSize() const
    {
        return 2 + *(CurrentReadPos + 1) * 6;
    };
    virtual size_t GetCallSize() const = 0;
    virtual size_t GetJumpSize() const
    {
        return 3;
    }
    virtual size_t GetCallNativeSize() const = 0;
    virtual inline size_t GetFunctionSize() const
    {
        return 5 + *(CurrentReadPos + 4);
    }
    virtual inline size_t GetPushStringSize() const
    {
        return 2 + *(CurrentReadPos + 1);
    }
    virtual size_t GetPushArraySize() const
    {
        Utils::System::Throw("The PushArray opcode size is not available on this target"); return 0;
    }
#pragma endregion

#pragma region Opcodes


    void ParseCallLabel(std::map<uint32_t, std::string>::iterator& labelInc, uint32_t index);
    void ParseJumpLabel(std::map<uint32_t, uint32_t>::iterator& labelInc, uint32_t index);


#pragma region Read
    inline void ReadUInt8(const char* opName)
    {
        CurrentOpSize = 2;
        PrintSingleParamOp(opName, *(CurrentReadPos + 1));
    }
    inline void ReadInt16(const char* opName)
    {
        CurrentOpSize = 3;
        PrintSingleParamOp(opName, Reader->ReadInt16(CurrentReadPos + 1));
    }
    inline void ReadUInt16(const char* opName)
    {
        CurrentOpSize = 3;
        PrintSingleParamOp(opName, Reader->ReadUInt16(CurrentReadPos + 1));
    }
    inline void ReadUInt24(const char* opName)
    {
        CurrentOpSize = 4;
        PrintSingleParamOp(opName, Reader->ReadUInt24(CurrentReadPos + 1));
    }

    virtual inline void ReadPushSingleOp(int32_t value)
    {
        CurrentOpSize = 1;
        PrintPush(value);
    }
    virtual inline void ReadPush()
    {
        CurrentOpSize = 5;
        PrintPush(Reader->ReadInt32(CurrentReadPos + 1));
    }
    virtual inline void ReadPushI24()
    {
        CurrentOpSize = 4;
        PrintPush(Reader->ReadUInt24(CurrentReadPos + 1));
    }
    virtual inline void ReadPushB()
    {
        CurrentOpSize = 2;
        PrintPush(*(CurrentReadPos + 1));
    }
    virtual inline void ReadPushS()
    {
        CurrentOpSize = 3;
        PrintPush(Reader->ReadInt16(CurrentReadPos + 1));
    }
    virtual inline void ReadPushB2()
    {
        CurrentOpSize = 3;
        PrintPush(*(CurrentReadPos + 1), *(CurrentReadPos + 2));
    }
    virtual inline void ReadPushB3()
    {
        CurrentOpSize = 4;
        PrintPush(*(CurrentReadPos + 1), *(CurrentReadPos + 2), *(CurrentReadPos + 3));
    }

    inline void ReadPushF()
    {
        CurrentOpSize = 5;
        PrintPushF(Reader->ReadFloat(CurrentReadPos + 1));
    }

    inline void ReadReturn()
    {
        CurrentOpSize = 3;
        PrintReturn(*(CurrentReadPos + 1), *(CurrentReadPos + 2));
    }

    //used only by rdr and gta4
    inline void ReadPushString()
    {
        CurrentOpSize = 2 + *(CurrentReadPos + 1);
        int size = *(CurrentReadPos + 1);
        char* str = reinterpret_cast<char*>(CurrentReadPos + 2);

        if (str[size - 1] == '\0')
            size--;

        PrintPushString(str, size);
    }

    void PrintPushStringS(int index);

    //override gtaiv
    virtual inline void ReadFunction()
    {
        uint8_t FunctionNameLength = *(CurrentReadPos + 4);
        char* name = reinterpret_cast<char*>(CurrentReadPos + 5);

        CurrentOpSize = 5 + FunctionNameLength;
        PrintFunction(*(CurrentReadPos + 1), Reader->ReadUInt16(CurrentReadPos + 2), name, FunctionNameLength);

    }

    //callnative def| gta5 1 byte param/return, 2 byte call loc | rdr 2 byte call loc | gta4: 1 byte param, 1 byte return, 4 byte hash
    virtual void ReadCallNative() = 0;

    virtual uint8_t* ReadJumpForCallCheck(uint8_t* position)
    {
        return position + 3 + Reader->ReadInt16(position + 1);
    }

    //override gtaiv jump is 4 bytes
    virtual inline void ReadJump(const char* opName)
    {
        CurrentOpSize = 3;
        PrintJump(opName, CurrentReadPos + 3 - CodeData.data() + Reader->ReadInt16(CurrentReadPos + 1));
    }

    virtual void ReadSwitch();

    virtual void ReadCall()
    {
        Utils::System::Throw("The Call opcode is not available on this target");
    }

    virtual void ReadCallCompact(uint8_t callType)
    {
        Utils::System::Throw("The CallCompact opcode is not available on this target");
    }

    //rdr only
    virtual void ReadReturnCompact(uint8_t returnType)
    {
        Utils::System::Throw("The ReturnCompact opcode is not available on this target");
    }

    //rdr only
    virtual void ReadPushArray()
    {
        Utils::System::Throw("The PushArray opcode is not available on this target");
    }


#pragma endregion

#pragma region Print
    void PrintNop();

    inline void PrintSingleOp(const char* opName);

    void PrintSingleParamOp(const char* opName, int value);

    void PrintPush(int value, int value1 = -1, int value2 = -1);

    void PrintPushF(float value);

    void PrintReturn(int popCount, int returnCount);

    //used only by rdr and gta4
    void PrintPushString(char* str, int size);

    void PrintFunction(int paramCount, int varCount, char* name, int nameLength);

    void PrintCallNative(uint32_t nativeIndex, int32_t nativeParamCount, int32_t nativeRetCount, bool rawNative = false);

    void PrintJump(const char* opName, uint32_t index);

    void PrintSwitch(std::vector<std::pair<int32_t, uint32_t>> caseAndLabel);

    void PrintCall(uint32_t index);

    void PrintPushArray(uint8_t* data, uint32_t size);

    void PrintVerbosePC();

#pragma endregion

#pragma endregion

public:
    virtual void OpenScript(std::vector<uint8_t>& data) = 0;

    virtual void CloseScript()
    {

        CurrentReadPos = nullptr;
        CurrentOpSize = 1;

        CommonHeader = { 0 };

        //DecompileTarget = GameTarget::UNK;
        //MaxPageSize = 16384;
        //Is64Bit = false;

        OutHeader = "";
        Out = "";

        CodeData.clear();
        StringData.clear();

        //NativeMap.clear();
        //NativeTranslationMap.clear();

        //Reader.release();

        //pc, call label index
        CallLabelIndexes.clear();
        //pc, jump label index
        JumpLabelIndexes.clear();

        //pc,
        UsedFunctions.clear();

        UnusedFunctionCounter = 0;
        IsFirstFunction = true;
        FunctionInc = UsedFunctions.begin();
    }

    virtual void GetString(const std::string& stringOutPath);

    void GetCode(const std::string& asmOutPath);
};

class DecompileGTAIV : public DecompileBase
{
#pragma region Vars
    std::vector<uint8_t> StaticsAndGlobals;
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpTrue, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushS, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Call, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetLocalPv0, Opcode::GetLocalPv1, Opcode::GetLocalPv2, Opcode::GetLocalPv3, Opcode::GetLocalPv4, Opcode::GetLocalPv5, Opcode::GetLocalPv6, Opcode::GetLocalPv7, Opcode::GetLocalPs, Opcode::GetStaticPs, Opcode::GetGlobalPs, Opcode::GetArrayPs, Opcode::Switch, Opcode::PushString, Opcode::PushStringNull, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::Catch, Opcode::Throw, Opcode::MemCopy, Opcode::GetXProtect, Opcode::SetXProtect, Opcode::RefXProtect, Opcode::Exit, Opcode::Push_Neg16, Opcode::Push_Neg15, Opcode::Push_Neg14, Opcode::Push_Neg13, Opcode::Push_Neg12, Opcode::Push_Neg11, Opcode::Push_Neg10, Opcode::Push_Neg9, Opcode::Push_Neg8, Opcode::Push_Neg7, Opcode::Push_Neg6, Opcode::Push_Neg5, Opcode::Push_Neg4, Opcode::Push_Neg3, Opcode::Push_Neg2, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };

#pragma endregion

protected:

#pragma region Utilities
    void LoadCodeData() override
    {}
    void LoadStringData() override
    {}
#pragma endregion

#pragma region Labels
    inline void LogJumpLabel() override
    {
        CurrentOpSize = 5;

        if (JumpLabelIndexes.find(Reader->ReadInt32(CurrentReadPos + 1)) == JumpLabelIndexes.end())
        {
            JumpLabelIndexes.insert({ Reader->ReadInt32(CurrentReadPos + 1), JumpLabelIndexes.size() });
        }

    }
    inline void LogCallLabel(uint8_t CallType = 0xFF) override
    {
        CurrentOpSize = 5;
        uint32_t FunctionIndex = Reader->ReadUInt32(CurrentReadPos + 1);

        if (CallLabelIndexes.find(FunctionIndex) == CallLabelIndexes.end())
        {
            uint8_t* FunctionPos = 0;
            if (FindFunctionFromCallPos(CodeData.data() + FunctionIndex, FunctionPos))
            {
                assert(FunctionPos >= CodeData.data());
                UsedFunctions.insert(FunctionPos - CodeData.data());

                CallLabelIndexes.insert({ FunctionIndex, "Function_" + std::to_string(CallLabelIndexes.size()) });
            }
        }
    }
    void LogSwitchLabel() override
    {
        uint8_t caseCount = *(CurrentReadPos + 1);
        CurrentOpSize = 2 + caseCount * 8;

        for (uint32_t i = 0; i < caseCount; i++)
        {
            uint8_t* index = CurrentReadPos + 2 + i * 8;
            JumpLabelIndexes.insert({ Reader->ReadInt32(index + 4), JumpLabelIndexes.size() });

        }
    }

#pragma endregion

#pragma region OpcodesSizes
    inline size_t GetSwitchSize() const override
    {
        return 2 + *(CurrentReadPos + 1) * 8;
    };
    inline size_t GetCallSize() const override
    {
        return 5;
    }
    inline size_t GetJumpSize() const override
    {
        return 5;
    }
    inline size_t GetFunctionSize() const override
    {
        return 4;
    }
    inline size_t GetCallNativeSize() const override
    {
        return 7;
    }
#pragma endregion

#pragma region Opcodes
    void ReadJump(const char* opName) override
    {
        CurrentOpSize = 5;
        PrintJump(opName, Reader->ReadInt32(CurrentReadPos + 1));
    }

    uint8_t* ReadJumpForCallCheck(uint8_t* Position) override
    {
        return CodeData.data() + Reader->ReadInt32(Position + 1);
    }

    inline void ReadFunction() override
    {
        CurrentOpSize = 4;
        PrintFunction(*(CurrentReadPos + 1), Reader->ReadUInt16(CurrentReadPos + 2), nullptr, 0);
    }

    inline void ReadCallNative() override
    {
        CurrentOpSize = 7;
        PrintCallNative(Reader->ReadInt32(CurrentReadPos + 3), *(CurrentReadPos + 1), *(CurrentReadPos + 2), true);
    }

    void ReadCall() override
    {
        CurrentOpSize = 5;
        PrintCall(Reader->ReadUInt32(CurrentReadPos + 1));
    }

    void ReadSwitch() override
    {
        uint8_t caseCount = *(CurrentReadPos + 1);
        CurrentOpSize = 2 + caseCount * 8;

        std::vector<std::pair<int32_t, uint32_t>> caseAndIndex(caseCount);

        for (uint32_t i = 0; i < caseCount; i++)
        {
            uint8_t* index = CurrentReadPos + 2 + i * 8;
            caseAndIndex[i] = {
                Reader->ReadInt32(index),//caseNum
                Reader->ReadInt32(index + 4)//labelIndex
            };
        }

        PrintSwitch(caseAndIndex);

    }

#pragma endregion

public:
    DecompileGTAIV(GameTarget::GameTarget GameTarget) :
        DecompileBase(_CommonOpsIndexedByTargetOps, GameTarget, 16384, true, false)
    {

    }

    void OpenScript(std::vector<uint8_t>& Data) override;

    void CloseScript() override
    {
        StaticsAndGlobals.clear();
        DecompileBase::CloseScript();
    }


};

class DecompileRDR : public DecompileBase
{
#pragma region Vars
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushB, Opcode::PushB2, Opcode::PushB3, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetArrayP1, Opcode::GetArray1, Opcode::SetArray1, Opcode::GetLocalP1, Opcode::GetLocal1, Opcode::SetLocal1, Opcode::GetStaticP1, Opcode::GetStatic1, Opcode::SetStatic1, Opcode::AddImm1, Opcode::GetImm1, Opcode::SetImm1, Opcode::MultImm1, Opcode::PushS, Opcode::AddImm2, Opcode::GetImm2, Opcode::SetImm2, Opcode::MultImm2, Opcode::GetArrayP2, Opcode::GetArray2, Opcode::SetArray2, Opcode::GetLocalP2, Opcode::GetLocal2, Opcode::SetLocal2, Opcode::GetStaticP2, Opcode::GetStatic2, Opcode::SetStatic2, Opcode::GetGlobalP2, Opcode::GetGlobal2, Opcode::SetGlobal2, Opcode::Call2, Opcode::Call2h1, Opcode::Call2h2, Opcode::Call2h3, Opcode::Call2h4, Opcode::Call2h5, Opcode::Call2h6, Opcode::Call2h7, Opcode::Call2h8, Opcode::Call2h9, Opcode::Call2hA, Opcode::Call2hB, Opcode::Call2hC, Opcode::Call2hD, Opcode::Call2hE, Opcode::Call2hF, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpNE, Opcode::JumpEQ, Opcode::JumpLE, Opcode::JumpLT, Opcode::JumpGE, Opcode::JumpGT, Opcode::GetGlobalP3, Opcode::GetGlobal3, Opcode::SetGlobal3, Opcode::PushI24, Opcode::Switch, Opcode::PushString, Opcode::PushArray, Opcode::PushStringNull, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::MemCopy, Opcode::Catch, Opcode::Throw, Opcode::pCall, Opcode::ReturnP0R0, Opcode::ReturnP0R1, Opcode::ReturnP0R2, Opcode::ReturnP0R3, Opcode::ReturnP1R0, Opcode::ReturnP1R1, Opcode::ReturnP1R2, Opcode::ReturnP1R3, Opcode::ReturnP2R0, Opcode::ReturnP2R1, Opcode::ReturnP2R2, Opcode::ReturnP2R3, Opcode::ReturnP3R0, Opcode::ReturnP3R1, Opcode::ReturnP3R2, Opcode::ReturnP3R3, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::PushF_Neg1, Opcode::PushF_0, Opcode::PushF_1, Opcode::PushF_2, Opcode::PushF_3, Opcode::PushF_4, Opcode::PushF_5, Opcode::PushF_6, Opcode::PushF_7, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };
#pragma endregion

protected:

#pragma region Utilities
    inline int32_t GetCallIndex(uint16_t data)
    {
        return (((data & 0xFF) << 2) & 0x300) | ((data >> 8) & 0xFF);
    }
    inline int32_t GetArgCountFromIndex(uint16_t* indblock)
    {
        return (((uint8_t*)indblock)[0] & 0x3e) >> 1;
    }
    inline bool FunctionHasReturn(uint16_t* data)
    {
        return (((uint8_t*)data)[0] & 1) == 1 ? true : false;
    }
    inline int32_t GetCallOffset(uint16_t readOffset, int8_t callType)
    {
        return readOffset | (callType << 16);
    }
    inline uint32_t GetSizeFromFlag(uint32_t graphicsFlag, uint32_t systemFlag)
    {
        return (systemFlag & 0x80000000) == 0x80000000
            ? ((systemFlag & 0x3FFF) << 12) + ((_rotl(systemFlag, 30)) & 0x1FFF800)
            : ((0x100 << (_rotl(graphicsFlag, 21) & 0xF)) * (graphicsFlag & 0x7FF))
            + ((0x100 << (_rotl(graphicsFlag, 6) & 0xF)) * (_rotl(graphicsFlag, 17) & 0x7FF));
    }
    uint32_t GetObjectStartPageOffset(RSCFlag flags);

#pragma endregion

#pragma region Labels
    void LogCallLabel(uint8_t callType = 0xFF) override;
#pragma endregion

#pragma region OpcodeSizes
    inline size_t GetCallSize() const override
    {
        return 3;
    }
    inline size_t GetCallNativeSize() const override
    {
        return 3;
    }
    inline size_t GetPushArraySize() const override
    {
        return 5 + Reader->ReadUInt32(CurrentReadPos + 1);
    }
#pragma endregion

#pragma region Opcodes

    void ReadCallNative() override;
    void ReadCallCompact(uint8_t callType) override;
    void ReadReturnCompact(uint8_t returnType = 0xFF) override;
    void ReadPushArray() override;
#pragma endregion

public:
    DecompileRDR(GameTarget::GameTarget target) :
        DecompileBase(_CommonOpsIndexedByTargetOps, target, 16384, false, false)
    {}

    void OpenScript(std::vector<uint8_t>& data) override;

};

class DecompileGTAVConsole : public DecompileBase
{

#pragma region Vars
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushB, Opcode::PushB2, Opcode::PushB3, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetArrayP1, Opcode::GetArray1, Opcode::SetArray1, Opcode::GetLocalP1, Opcode::GetLocal1, Opcode::SetLocal1, Opcode::GetStaticP1, Opcode::GetStatic1, Opcode::SetStatic1, Opcode::AddImm1, Opcode::MultImm1, Opcode::GetImmPs, Opcode::GetImmP1, Opcode::GetImm1, Opcode::SetImm1, Opcode::PushS, Opcode::AddImm2, Opcode::MultImm2, Opcode::GetImmP2, Opcode::GetImm2, Opcode::SetImm2, Opcode::GetArrayP2, Opcode::GetArray2, Opcode::SetArray2, Opcode::GetLocalP2, Opcode::GetLocal2, Opcode::SetLocal2, Opcode::GetStaticP2, Opcode::GetStatic2, Opcode::SetStatic2, Opcode::GetGlobalP2, Opcode::GetGlobal2, Opcode::SetGlobal2, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpNE, Opcode::JumpEQ, Opcode::JumpLE, Opcode::JumpLT, Opcode::JumpGE, Opcode::JumpGT, Opcode::Call, Opcode::GetGlobalP3, Opcode::GetGlobal3, Opcode::SetGlobal3, Opcode::PushI24, Opcode::Switch, Opcode::PushStringS, Opcode::GetHash, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::MemCopy, Opcode::Catch, Opcode::Throw, Opcode::pCall, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::PushF_Neg1, Opcode::PushF_0, Opcode::PushF_1, Opcode::PushF_2, Opcode::PushF_3, Opcode::PushF_4, Opcode::PushF_5, Opcode::PushF_6, Opcode::PushF_7, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };
#pragma endregion

protected:


#pragma region Labels

    void LogCallLabel(uint8_t callType = 0xFF) override;

#pragma endregion

#pragma region OpcodeSizes
    inline size_t GetCallSize() const override
    {
        return 4;
    }
    inline size_t GetCallNativeSize() const override
    {
        return 4;
    }
    inline size_t GetPushStringSize() const override
    {
        return 1;
    }

#pragma endregion

    bool CheckNextOpcodeForStack(int32_t pushValue);

#pragma region Opcodes
    inline void ReadPushSingleOp(int32_t value) override
    {
        CurrentOpSize = 1;
        if (!CheckNextOpcodeForStack(value))
            DecompileBase::ReadPushSingleOp(value);
    }
    inline void ReadPush() override
    {
        CurrentOpSize = 5;
        int32_t LastPushValue = Reader->ReadInt32(CurrentReadPos + 1);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPush();
    }

    inline void ReadPushI24() override
    {
        CurrentOpSize = 4;
        int32_t LastPushValue = Reader->ReadUInt24(CurrentReadPos + 1);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPushI24();
    }
    inline void ReadPushB() override
    {
        CurrentOpSize = 2;
        int32_t LastPushValue = *(CurrentReadPos + 1);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPushB();
    }
    inline void ReadPushS() override
    {
        CurrentOpSize = 3;
        int32_t LastPushValue = Reader->ReadInt16(CurrentReadPos + 1);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPushS();
    }
    inline void ReadPushB2() override
    {
        CurrentOpSize = 3;
        int32_t LastPushValue = *(CurrentReadPos + 2);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPushB2();
    }
    inline void ReadPushB3() override
    {
        CurrentOpSize = 4;
        int32_t LastPushValue = *(CurrentReadPos + 3);
        if (!CheckNextOpcodeForStack(LastPushValue))
            DecompileBase::ReadPushB3();
    }

    inline void ReadCallNative() override
    {
        CurrentOpSize = 4;
        uint8_t NativeFlagHolder = *(CurrentReadPos + 1);
        PrintCallNative(Reader->ReadUInt16(CurrentReadPos + 2), NativeFlagHolder >> 2, NativeFlagHolder & 3);
    }

    inline void ReadCall() override
    {
        CurrentOpSize = 4;
        PrintCall(Reader->ReadUInt24(CurrentReadPos + 1));
    }

#pragma endregion

public:
    DecompileGTAVConsole() :
        DecompileBase(_CommonOpsIndexedByTargetOps, GameTarget::GTAV, 16384, false, false)
    {

    }

    DecompileGTAVConsole(const Opcode commonOpsIndexedByTargetOps[255], GameTarget::GameTarget decompileTarget,
        uint32_t maxPageSize = 16384, bool isLittleEndian = false, bool is64Bit = false) :
        DecompileBase(commonOpsIndexedByTargetOps, decompileTarget, maxPageSize, isLittleEndian, is64Bit)
    {

    }

    void OpenScript(std::vector<uint8_t>& data) override;

    void GetString(const std::string& stringOutPath) override;

};

class DecompileGTAVPC : public DecompileGTAVConsole
{
#pragma region Vars
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushB, Opcode::PushB2, Opcode::PushB3, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetArrayP1, Opcode::GetArray1, Opcode::SetArray1, Opcode::GetLocalP1, Opcode::GetLocal1, Opcode::SetLocal1, Opcode::GetStaticP1, Opcode::GetStatic1, Opcode::SetStatic1, Opcode::AddImm1, Opcode::MultImm1, Opcode::GetImmPs, Opcode::GetImmP1, Opcode::GetImm1, Opcode::SetImm1, Opcode::PushS, Opcode::AddImm2, Opcode::MultImm2, Opcode::GetImmP2, Opcode::GetImm2, Opcode::SetImm2, Opcode::GetArrayP2, Opcode::GetArray2, Opcode::SetArray2, Opcode::GetLocalP2, Opcode::GetLocal2, Opcode::SetLocal2, Opcode::GetStaticP2, Opcode::GetStatic2, Opcode::SetStatic2, Opcode::GetGlobalP2, Opcode::GetGlobal2, Opcode::SetGlobal2, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpNE, Opcode::JumpEQ, Opcode::JumpLE, Opcode::JumpLT, Opcode::JumpGE, Opcode::JumpGT, Opcode::Call, Opcode::GetGlobalP3, Opcode::GetGlobal3, Opcode::SetGlobal3, Opcode::PushI24, Opcode::Switch, Opcode::PushStringS, Opcode::GetHash, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::MemCopy, Opcode::Catch, Opcode::Throw, Opcode::pCall, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::PushF_Neg1, Opcode::PushF_0, Opcode::PushF_1, Opcode::PushF_2, Opcode::PushF_3, Opcode::PushF_4, Opcode::PushF_5, Opcode::PushF_6, Opcode::PushF_7, Opcode::BitTest, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };
#pragma endregion

protected:

#pragma region Opcodes

    void ReadCallNative() override;

#pragma endregion

public:
    DecompileGTAVPC() :
        DecompileGTAVConsole(_CommonOpsIndexedByTargetOps, GameTarget::GTAV, 16384, true, true)
    {}

    DecompileGTAVPC(const Opcode commonOpsIndexedByTargetOps[255], GameTarget::GameTarget decompileTarget,
        uint32_t maxPageSize = 16384, bool isLittleEndian = false, bool is64Bit = false) :
        DecompileGTAVConsole(commonOpsIndexedByTargetOps, decompileTarget, maxPageSize, isLittleEndian, is64Bit)
    {

    }

    void OpenScript(std::vector<uint8_t>& data) override;

};

class DecompileRDR2Console : public DecompileGTAVPC
{
#pragma region Vars

    //default ops
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushB, Opcode::PushB2, Opcode::PushB3, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetArrayP1, Opcode::GetArray1, Opcode::SetArray1, Opcode::GetLocalP1, Opcode::GetLocal1, Opcode::SetLocal1, Opcode::GetStaticP1, Opcode::GetStatic1, Opcode::SetStatic1, Opcode::AddImm1, Opcode::MultImm1, Opcode::GetImmPs, Opcode::GetImmP1, Opcode::GetImm1, Opcode::SetImm1, Opcode::PushS, Opcode::AddImm2, Opcode::MultImm2, Opcode::GetImmP2, Opcode::GetImm2, Opcode::SetImm2, Opcode::GetArrayP2, Opcode::GetArray2, Opcode::SetArray2, Opcode::GetLocalP2, Opcode::GetLocal2, Opcode::SetLocal2, Opcode::GetStaticP2, Opcode::GetStatic2, Opcode::SetStatic2, Opcode::GetGlobalP2, Opcode::GetGlobal2, Opcode::SetGlobal2, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpNE, Opcode::JumpEQ, Opcode::JumpLE, Opcode::JumpLT, Opcode::JumpGE, Opcode::JumpGT, Opcode::Call, Opcode::GetStaticP3, Opcode::GetStatic3, Opcode::SetStatic3, Opcode::GetGlobalP3, Opcode::GetGlobal3, Opcode::SetGlobal3, Opcode::PushI24, Opcode::Switch, Opcode::PushStringS, Opcode::GetHash, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::MemCopy, Opcode::Catch, Opcode::Throw, Opcode::pCall, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::PushF_Neg1, Opcode::PushF_0, Opcode::PushF_1, Opcode::PushF_2, Opcode::PushF_3, Opcode::PushF_4, Opcode::PushF_5, Opcode::PushF_6, Opcode::PushF_7, Opcode::GetLocalS, Opcode::SetLocalS, Opcode::SetLocalSR, Opcode::GetStaticS, Opcode::SetStaticS, Opcode::SetStaticSR, Opcode::pGetS, Opcode::pSetS, Opcode::pSetSR, Opcode::GetGlobalS, Opcode::SetGlobalS, Opcode::SetGlobalSR, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };

#pragma endregion

protected:

    virtual void LoadOpcodes();

#pragma region Labels

    void LogSwitchLabel() override;

#pragma endregion

#pragma region OpcodeSizes

    inline size_t GetSwitchSize() const override
    {
        return 3 + Reader->ReadUInt16(CurrentReadPos + 1) * 6;
    }

#pragma endregion

#pragma region Opcodes

    void ReadSwitch() override;

#pragma endregion

public:
    DecompileRDR2Console() :
        DecompileGTAVPC(_CommonOpsIndexedByTargetOps, GameTarget::RDR2, 16384, true, true)
    {}

    DecompileRDR2Console(const Opcode commonOpsIndexedByTargetOps[255], GameTarget::GameTarget decompileTarget,
        uint32_t maxPageSize = 16384, bool isLittleEndian = false, bool is64Bit = false) :
        DecompileGTAVPC(commonOpsIndexedByTargetOps, decompileTarget, maxPageSize, isLittleEndian, is64Bit)
    {}

    void OpenScript(std::vector<uint8_t>& data) override;

};

class DecompileRDR2PC : public DecompileRDR2Console
{
#pragma region Vars

    //default ops
    const Opcode _CommonOpsIndexedByTargetOps[255] = { Opcode::Nop, Opcode::Add, Opcode::Sub, Opcode::Mult, Opcode::Div, Opcode::Mod, Opcode::Not, Opcode::Neg, Opcode::CmpEQ, Opcode::CmpNE, Opcode::CmpGT, Opcode::CmpGE, Opcode::CmpLT, Opcode::CmpLE, Opcode::fAdd, Opcode::fSub, Opcode::fMult, Opcode::fDiv, Opcode::fMod, Opcode::fNeg, Opcode::fCmpEQ, Opcode::fCmpNE, Opcode::fCmpGT, Opcode::fCmpGE, Opcode::fCmpLT, Opcode::fCmpLE, Opcode::vAdd, Opcode::vSub, Opcode::vMult, Opcode::vDiv, Opcode::vNeg, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::ItoF, Opcode::FtoI, Opcode::FtoV, Opcode::PushB, Opcode::PushB2, Opcode::PushB3, Opcode::Push, Opcode::PushF, Opcode::Dup, Opcode::Drop, Opcode::CallNative, Opcode::Function, Opcode::Return, Opcode::pGet, Opcode::pSet, Opcode::pPeekSet, Opcode::ToStack, Opcode::FromStack, Opcode::GetArrayP1, Opcode::GetArray1, Opcode::SetArray1, Opcode::GetLocalP1, Opcode::GetLocal1, Opcode::SetLocal1, Opcode::GetStaticP1, Opcode::GetStatic1, Opcode::SetStatic1, Opcode::AddImm1, Opcode::MultImm1, Opcode::GetImmPs, Opcode::GetImmP1, Opcode::GetImm1, Opcode::SetImm1, Opcode::PushS, Opcode::AddImm2, Opcode::MultImm2, Opcode::GetImmP2, Opcode::GetImm2, Opcode::SetImm2, Opcode::GetArrayP2, Opcode::GetArray2, Opcode::SetArray2, Opcode::GetLocalP2, Opcode::GetLocal2, Opcode::SetLocal2, Opcode::GetStaticP2, Opcode::GetStatic2, Opcode::SetStatic2, Opcode::GetGlobalP2, Opcode::GetGlobal2, Opcode::SetGlobal2, Opcode::Jump, Opcode::JumpFalse, Opcode::JumpNE, Opcode::JumpEQ, Opcode::JumpLE, Opcode::JumpLT, Opcode::JumpGE, Opcode::JumpGT, Opcode::Call, Opcode::GetGlobalP3, Opcode::GetGlobal3, Opcode::SetGlobal3, Opcode::PushI24, Opcode::Switch, Opcode::PushStringS, Opcode::GetHash, Opcode::StrCopy, Opcode::ItoS, Opcode::StrAdd, Opcode::StrAddi, Opcode::MemCopy, Opcode::Catch, Opcode::Throw, Opcode::pCall, Opcode::Push_Neg1, Opcode::Push_0, Opcode::Push_1, Opcode::Push_2, Opcode::Push_3, Opcode::Push_4, Opcode::Push_5, Opcode::Push_6, Opcode::Push_7, Opcode::PushF_Neg1, Opcode::PushF_0, Opcode::PushF_1, Opcode::PushF_2, Opcode::PushF_3, Opcode::PushF_4, Opcode::PushF_5, Opcode::PushF_6, Opcode::PushF_7, Opcode::GetLocalS, Opcode::SetLocalS, Opcode::SetLocalSR, Opcode::GetStaticS, Opcode::SetStaticS, Opcode::SetStaticSR, Opcode::pGetS, Opcode::pSetS, Opcode::pSetSR, Opcode::GetGlobalS, Opcode::SetGlobalS, Opcode::SetGlobalSR, Opcode::GetStaticP3, Opcode::GetStatic3, Opcode::SetStatic3, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized, Opcode::Uninitialized };

#pragma endregion


protected:
    void LoadOpcodes() override;


public:
    DecompileRDR2PC() :
        DecompileRDR2Console(_CommonOpsIndexedByTargetOps, GameTarget::RDR2, 16384, true, true)
    {}

    void OpenScript(std::vector<uint8_t>& data) override;

};
