#include "Decompiler.h"
#include <sstream>
#include <fstream>
#include "Utils/types.h"

using namespace std;


#pragma region DecompileBase

void DecompileBase::CheckFunctionUnused(char* unusedFunctionName)
{
    static uint32_t unusedFunctionCounter = 0;
    static bool isFirstFunction = true;
    static auto functionInc = UsedFunctions.begin();

    int64_t pc = CurrentReadPos - CodeData.data();

    if (functionInc != UsedFunctions.end())
    {

        if (pc != *functionInc)
        {
            if (isFirstFunction)
            {
                if (unusedFunctionName == nullptr)
                    Out += ":EntryPoint//>\r\n";
                else
                {
                    Out += ":";
                    Out += string(unusedFunctionName);
                    Out += "//>\r\n";
                }
            }
            else
            {
                if (unusedFunctionName == nullptr)
                {
                    Out += "//<\r\n\r\n:UnusedFunction_";
                    Out += to_string(unusedFunctionCounter++);
                    Out += "//> pCall Location: ";
                    Out += to_string(pc);
                    Out += "\r\n";
                }
                else
                {
                    Out += "//<\r\n\r\n:";
                    Out += string(unusedFunctionName);
                    Out += "//> pCall Location: ";
                    Out += to_string(pc);
                    Out += "\r\n";
                }
            }
        }
        else
            functionInc++;

    }

    isFirstFunction = false;
}

bool DecompileBase::FindFunctionFromCallPos(uint8_t* position, uint8_t*& outPosition)
{
    while (position < CodeData.data() + CodeData.size())
    {
        switch (CommonOpsIndexedByTargetOps[*position])
        {
        case Opcode::Function: outPosition = position; return true;
        case Opcode::Nop: position++; break;
        case Opcode::Jump: position = ReadJumpForCallCheck(position); break;
        default: return false;
        }
    }
    return false;
}

string DecompileBase::GetOptionsDataName(string dataName)
{
    return GameTarget::TargetNameGeneric[(int)Options::GameTarget] + dataName + Platform::PlatformNameGeneric[(int)Options::Platform];
}

void DecompileBase::ParseStaticData()
{
    if (CommonHeader.StaticsCount)
    {
        OutHeader += "//> Default Static Information\r\n";
        OutHeader += "SetStaticsCount ";
        OutHeader += to_string(CommonHeader.StaticsCount);
        OutHeader += "\r\n";
    }

    for (uint32_t i = 0; i < CommonHeader.StaticsCount; i++)
    {
        int64_t value = Is64Bit ? 
            Reader->ReadInt32(reinterpret_cast<uint8_t*>((int64_t*)CommonHeader.StaticsOffset + i)) : 
            Reader->ReadInt32(reinterpret_cast<uint8_t*>(CommonHeader.StaticsOffset + i));
        if (value != 0)
        {
            auto j = CommonHeader.StaticsOffset + i;

            OutHeader += "SetDefaultStatic ";
            OutHeader += to_string(i);
            OutHeader += " ";
            OutHeader += to_string(value);
            OutHeader += "\r\n";
        }
    }

    if (CommonHeader.StaticsCount)
        OutHeader += "//<\r\n\r\n";
}

void DecompileBase::LoadCodeData()
{
    uint32_t lengthWritten = 0;

    assert(CommonHeader.CodeBlocksCount);
    CodeData.resize(CommonHeader.CodeLength);
    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        const uint32_t NextDataSegmentSize = min(MaxPageSize, CommonHeader.CodeLength - lengthWritten);
        memcpy(CodeData.data() + lengthWritten, CommonHeader.CodeBlockOffsets[i], NextDataSegmentSize);
        lengthWritten += NextDataSegmentSize;
    }
}

void DecompileBase::LoadStringData()
{
    uint32_t lengthWritten = 0;

    if (CommonHeader.StringBlocksCount)
    {
        StringData.resize(CommonHeader.TotalStringLength);
        for (uint32_t i = 0; i < CommonHeader.StringBlocksCount; i++)
        {
            const size_t NextDataSegmentSize = min(MaxPageSize, CommonHeader.TotalStringLength - lengthWritten);
            memcpy(StringData.data() + lengthWritten, CommonHeader.StringBlockOffsets[i], NextDataSegmentSize);
            lengthWritten += NextDataSegmentSize;


        }
    }
}

void DecompileBase::LoadLabels()
{
    CurrentReadPos = CodeData.data();

    for (uint32_t i = 0; i < CommonHeader.CodeLength; i += CurrentOpSize, CurrentReadPos += CurrentOpSize, CurrentOpSize = 1)
    {
        switch (CommonOpsIndexedByTargetOps[CodeData[i]])
        {
        case Opcode::PushB:
        case Opcode::GetArrayP1:
        case Opcode::GetArray1:
        case Opcode::SetArray1:
        case Opcode::GetLocalP1:
        case Opcode::GetLocal1:
        case Opcode::SetLocal1:
        case Opcode::GetStaticP1:
        case Opcode::GetStatic1:
        case Opcode::SetStatic1:
        case Opcode::AddImm1:
        case Opcode::MultImm1:
        case Opcode::GetImmP1:
        case Opcode::GetImm1:
        case Opcode::SetImm1:
        case Opcode::StrCopy:
        case Opcode::ItoS:
        case Opcode::StrAdd:
        case Opcode::StrAddi:
            CurrentOpSize = 2; break;

        case Opcode::Return:
        case Opcode::PushB2:
        case Opcode::PushS:
        case Opcode::AddImm2:
        case Opcode::MultImm2:
        case Opcode::GetImmP2:
        case Opcode::GetImm2:
        case Opcode::SetImm2:
        case Opcode::GetArrayP2:
        case Opcode::GetArray2:
        case Opcode::SetArray2:
        case Opcode::GetLocalP2:
        case Opcode::GetLocal2:
        case Opcode::SetLocal2:
        case Opcode::GetStaticP2:
        case Opcode::GetStatic2:
        case Opcode::SetStatic2:
        case Opcode::GetGlobalP2:
        case Opcode::GetGlobal2:
        case Opcode::SetGlobal2:
            CurrentOpSize = 3; break;

        case Opcode::PushB3:
        case Opcode::GetGlobalP3:
        case Opcode::GetGlobal3:
        case Opcode::SetGlobal3:
        case Opcode::PushI24:
        case Opcode::GetStaticP3://rdr2 only >= 1311
        case Opcode::GetStatic3://rdr2 only  >= 1311
        case Opcode::SetStatic3://rdr2 only  >= 1311
            CurrentOpSize = 4; break;

        case Opcode::Push:
        case Opcode::PushF:
            CurrentOpSize = 5; break;

        case Opcode::CallNative:	CurrentOpSize = GetCallNativeSize(); break;
        case Opcode::Function:		CurrentOpSize = GetFunctionSize(); break;
        case Opcode::PushString:	CurrentOpSize = GetPushStringSize(); break;
        case Opcode::PushArray:	    CurrentOpSize = GetPushArraySize(); break;

        case Opcode::Jump:
        case Opcode::JumpFalse:
        case Opcode::JumpTrue:
        case Opcode::JumpNE:
        case Opcode::JumpEQ:
        case Opcode::JumpLE:
        case Opcode::JumpLT:
        case Opcode::JumpGE:
        case Opcode::JumpGT:
            LogJumpLabel(); break;
        case Opcode::Switch:
            LogSwitchLabel(); break;

        case Opcode::Call2:		LogCallLabel(0); break;
        case Opcode::Call2h1:	LogCallLabel(1); break;
        case Opcode::Call2h2:	LogCallLabel(2); break;
        case Opcode::Call2h3:	LogCallLabel(3); break;
        case Opcode::Call2h4:	LogCallLabel(4); break;
        case Opcode::Call2h5:	LogCallLabel(5); break;
        case Opcode::Call2h6:	LogCallLabel(6); break;
        case Opcode::Call2h7:	LogCallLabel(7); break;
        case Opcode::Call2h8:	LogCallLabel(8); break;
        case Opcode::Call2h9:	LogCallLabel(9); break;
        case Opcode::Call2hA:	LogCallLabel(10); break;
        case Opcode::Call2hB:	LogCallLabel(11); break;
        case Opcode::Call2hC:	LogCallLabel(12); break;
        case Opcode::Call2hD:	LogCallLabel(13); break;
        case Opcode::Call2hE:	LogCallLabel(14); break;
        case Opcode::Call2hF:	LogCallLabel(15); break;
        case Opcode::Call:		LogCallLabel(); break;
        }
    }

}

void DecompileBase::GetCode(const string& asmOutPath)
{
    OutHeader = "";
    Out = "";

    assert(CommonHeader.HeaderPtr != nullptr);
    assert(CommonHeader.StaticsOffset != nullptr);
    assert(CommonHeader.StaticsCount != 0xFFFFFFFF);
    assert(CommonHeader.CodeLength != 0xFFFFFFFF);


    OutHeader.reserve(1000000);
    Out.reserve(50000000);


    if (CommonHeader.Signature != Signature::Undefined)
    {
        OutHeader += "SetSignature 0x";
        OutHeader += Utils::DataConversion::IntToHex(CommonHeader.Signature);
        OutHeader += "\r\n";
    }


    if (CommonHeader.ParameterCount != 0 )
    {
        OutHeader += "SetParamCount ";
        OutHeader += to_string(CommonHeader.ParameterCount);
        OutHeader += "\r\n";
    }

    ParseStaticData();


    string nativeFile = Utils::IO::GetLastFileWithVersion(GetOptionsDataName("Natives"), Options::DecompileOptions::NativeVersion);

    if (nativeFile != "")
    {
        if (!Utils::IO::LoadCSVStringMap("Data/" + nativeFile, true, 16, NativeMap))
            Utils::System::Warn("Could not process natives file " + nativeFile + " ... using default natives");
    }
    else
        Utils::System::Warn("Natives file " + GetOptionsDataName("Natives") + " not found ... using default natives");


    string nativeTransFile = Utils::IO::GetLastFileWithVersion(GetOptionsDataName("NativesTranslation"), Options::DecompileOptions::NativeVersion);

    if (nativeTransFile != "")
    {
        if (!Utils::IO::LoadCSVMap("Data/" + nativeTransFile, true, 16, NativeTranslationMap))
            Utils::System::Warn("Could not process natives translation file " + nativeTransFile + " ... using default natives");

        cout << "Natives Translation file " << nativeFile << " applied" << endl;
    }


    LoadCodeData();
    LoadStringData();

    LoadLabels();

    CurrentReadPos = CodeData.data();

    map<uint32_t, uint32_t>::iterator JumpLabelInc = JumpLabelIndexes.begin();
    map<uint32_t, string>::iterator CallLabelInc = CallLabelIndexes.begin();

    for (uint32_t i = 0; i < CommonHeader.CodeLength; i += CurrentOpSize, CurrentReadPos += CurrentOpSize, CurrentOpSize = 1)
    {
        ParseJumpLabel(JumpLabelInc, i);
        ParseCallLabel(CallLabelInc, i);

        switch (CommonOpsIndexedByTargetOps[CodeData[i]])
        {
        case Opcode::Nop:			PrintNop(); break;
        case Opcode::Add:			PrintSingleOp("Add"); break;
        case Opcode::Sub:			PrintSingleOp("Sub"); break;
        case Opcode::Mult:			PrintSingleOp("Mult"); break;
        case Opcode::Div:			PrintSingleOp("Div"); break;
        case Opcode::Mod:			PrintSingleOp("Mod"); break;
        case Opcode::Not:			PrintSingleOp("Not"); break;
        case Opcode::Neg:			PrintSingleOp("Neg"); break;
        case Opcode::CmpEQ:			PrintSingleOp("CmpEQ"); break;
        case Opcode::CmpNE:			PrintSingleOp("CmpNE"); break;
        case Opcode::CmpGT:			PrintSingleOp("CmpGT"); break;
        case Opcode::CmpGE:			PrintSingleOp("CmpGE"); break;
        case Opcode::CmpLT:			PrintSingleOp("CmpLT"); break;
        case Opcode::CmpLE:			PrintSingleOp("CmpLE"); break;
        case Opcode::fAdd:			PrintSingleOp("fAdd"); break;
        case Opcode::fSub:			PrintSingleOp("fSub"); break;
        case Opcode::fMult:			PrintSingleOp("fMult"); break;
        case Opcode::fDiv:			PrintSingleOp("fDiv"); break;
        case Opcode::fMod:			PrintSingleOp("fMod"); break;
        case Opcode::fNeg:			PrintSingleOp("fNeg"); break;
        case Opcode::fCmpEQ:		PrintSingleOp("fCmpEQ"); break;
        case Opcode::fCmpNE:		PrintSingleOp("fCmpNE"); break;
        case Opcode::fCmpGT:		PrintSingleOp("fCmpGT"); break;
        case Opcode::fCmpGE:		PrintSingleOp("fCmpGE"); break;
        case Opcode::fCmpLT:		PrintSingleOp("fCmpLT"); break;
        case Opcode::fCmpLE:		PrintSingleOp("fCmpLE"); break;
        case Opcode::vAdd:			PrintSingleOp("vAdd"); break;
        case Opcode::vSub:			PrintSingleOp("vSub"); break;
        case Opcode::vMult:			PrintSingleOp("vMult"); break;
        case Opcode::vDiv:			PrintSingleOp("vDiv"); break;
        case Opcode::vNeg:			PrintSingleOp("vNeg"); break;
        case Opcode::And:			PrintSingleOp("And"); break;
        case Opcode::Or:			PrintSingleOp("Or"); break;
        case Opcode::Xor:			PrintSingleOp("Xor"); break;
        case Opcode::ItoF:			PrintSingleOp("ItoF"); break;
        case Opcode::FtoI:			PrintSingleOp("FtoI"); break;
        case Opcode::FtoV:			PrintSingleOp("FtoV"); break;
        case Opcode::PushB:			ReadPushB(); break;
        case Opcode::PushB2:		ReadPushB2(); break;
        case Opcode::PushB3:		ReadPushB3(); break;
        case Opcode::Push:			ReadPush(); break;
        case Opcode::PushF:			ReadPushF(); break;
        case Opcode::Dup:			PrintSingleOp("Dup"); break;
        case Opcode::Drop:			PrintSingleOp("Drop"); break;
        case Opcode::CallNative:	ReadCallNative(); break;
        case Opcode::Function:		ReadFunction(); break;
        case Opcode::Return:		ReadReturn(); break;
        case Opcode::pGet:			PrintSingleOp("pGet"); break;
        case Opcode::pSet:			PrintSingleOp("pSet"); break;
        case Opcode::pPeekSet:		PrintSingleOp("pPeekSet"); break;
        case Opcode::ToStack:		PrintSingleOp("ToStack"); break;
        case Opcode::FromStack:		PrintSingleOp("FromStack"); break;
        case Opcode::GetArrayPs:	PrintSingleOp("GetArrayPs"); break;
        case Opcode::GetArrayP1:	ReadUInt8("GetArrayP"); break;
        case Opcode::GetArray1:		ReadUInt8("GetArray"); break;
        case Opcode::SetArray1:		ReadUInt8("SetArray"); break;
        case Opcode::GetLocalPv0:	PrintSingleOp("GetLocalP 0"); break;
        case Opcode::GetLocalPv1:	PrintSingleOp("GetLocalP 1"); break;
        case Opcode::GetLocalPv2:	PrintSingleOp("GetLocalP 2"); break;
        case Opcode::GetLocalPv3:	PrintSingleOp("GetLocalP 3"); break;
        case Opcode::GetLocalPv4:	PrintSingleOp("GetLocalP 4"); break;
        case Opcode::GetLocalPv5:	PrintSingleOp("GetLocalP 5"); break;
        case Opcode::GetLocalPv6:	PrintSingleOp("GetLocalP 6"); break;
        case Opcode::GetLocalPv7:	PrintSingleOp("GetLocalP 7"); break;
        case Opcode::GetLocalPs:	PrintSingleOp("GetLocalPs"); break;
        case Opcode::GetLocalP1:	ReadUInt8("GetLocalP"); break;
        case Opcode::GetLocal1:		ReadUInt8("GetLocal"); break;
        case Opcode::SetLocal1:		ReadUInt8("SetLocal"); break;
        case Opcode::GetStaticPs:	PrintSingleOp("GetStaticPs"); break;
        case Opcode::GetStaticP1:	ReadUInt8("GetStaticP"); break;
        case Opcode::GetStatic1:	ReadUInt8("GetStatic"); break;
        case Opcode::SetStatic1:	ReadUInt8("SetStatic"); break;
        case Opcode::AddImm1:		ReadUInt8("AddImm"); break;
        case Opcode::MultImm1:		ReadUInt8("MultImm"); break;
        case Opcode::GetImmPs:		PrintSingleOp("GetImmPs"); break;
        case Opcode::GetImmP1:		ReadUInt8("GetImmP"); break;
        case Opcode::GetImm1:		ReadUInt8("GetImm"); break;
        case Opcode::SetImm1:		ReadUInt8("SetImm"); break;
        case Opcode::PushS:			ReadPushS(); break;
        case Opcode::AddImm2:		ReadInt16("AddImm"); break;
        case Opcode::MultImm2:		ReadInt16("MultImm"); break;
        case Opcode::GetImmP2:		ReadInt16("GetImmP"); break;
        case Opcode::GetImm2:		ReadInt16("GetImm"); break;
        case Opcode::SetImm2:		ReadInt16("SetImm"); break;
        case Opcode::GetArrayP2:	ReadUInt16("GetArrayP"); break;
        case Opcode::GetArray2:		ReadUInt16("GetArray"); break;
        case Opcode::SetArray2:		ReadUInt16("SetArray"); break;
        case Opcode::GetLocalP2:	ReadUInt16("GetLocalP"); break;
        case Opcode::GetLocal2:		ReadUInt16("GetLocal"); break;
        case Opcode::SetLocal2:		ReadUInt16("SetLocal"); break;
        case Opcode::GetStaticP2:	ReadUInt16("GetStaticP"); break;
        case Opcode::GetStatic2:	ReadUInt16("GetStatic"); break;
        case Opcode::SetStatic2:	ReadUInt16("SetStatic"); break;
        case Opcode::GetGlobalPs:	PrintSingleOp("GetGlobalPs"); break;
        case Opcode::GetGlobalP2:	ReadUInt16("GetGlobalP"); break;
        case Opcode::GetGlobal2:	ReadUInt16("GetGlobal"); break;
        case Opcode::SetGlobal2:	ReadUInt16("SetGlobal"); break;
        case Opcode::Jump:			ReadJump("Jump"); break;
        case Opcode::JumpFalse:		ReadJump("JumpFalse"); break;
        case Opcode::JumpTrue:		ReadJump("JumpTrue"); break;
        case Opcode::JumpNE:		ReadJump("JumpNE"); break;
        case Opcode::JumpEQ:		ReadJump("JumpEQ"); break;
        case Opcode::JumpLE:		ReadJump("JumpLE"); break;
        case Opcode::JumpLT:		ReadJump("JumpLT"); break;
        case Opcode::JumpGE:		ReadJump("JumpGE"); break;
        case Opcode::JumpGT:		ReadJump("JumpGT"); break;
        case Opcode::Call:			ReadCall(); break;
        case Opcode::Call2:			ReadCallCompact(0); break;
        case Opcode::Call2h1:		ReadCallCompact(1); break;
        case Opcode::Call2h2:		ReadCallCompact(2); break;
        case Opcode::Call2h3:		ReadCallCompact(3); break;
        case Opcode::Call2h4:		ReadCallCompact(4); break;
        case Opcode::Call2h5:		ReadCallCompact(5); break;
        case Opcode::Call2h6:		ReadCallCompact(6); break;
        case Opcode::Call2h7:		ReadCallCompact(7); break;
        case Opcode::Call2h8:		ReadCallCompact(8); break;
        case Opcode::Call2h9:		ReadCallCompact(9); break;
        case Opcode::Call2hA:		ReadCallCompact(10); break;
        case Opcode::Call2hB:		ReadCallCompact(11); break;
        case Opcode::Call2hC:		ReadCallCompact(12); break;
        case Opcode::Call2hD:		ReadCallCompact(13); break;
        case Opcode::Call2hE:		ReadCallCompact(14); break;
        case Opcode::Call2hF:		ReadCallCompact(15); break;
        case Opcode::GetGlobalP3:	ReadUInt24("GetGlobalP"); break;
        case Opcode::GetGlobal3:	ReadUInt24("GetGlobal"); break;
        case Opcode::SetGlobal3:	ReadUInt24("SetGlobal"); break;
        case Opcode::PushI24:		ReadPushI24(); break;
        case Opcode::Switch:		ReadSwitch(); break;
        case Opcode::PushString:	ReadPushString(); break;
        case Opcode::PushStringS:	PrintSingleOp("PushStringS"); break;
        case Opcode::PushArray: 	ReadPushArray(); break;
        case Opcode::PushStringNull:PrintSingleOp("PushString \"\""); break;
        case Opcode::GetHash:		PrintSingleOp("GetHash"); break;
        case Opcode::StrCopy:		ReadUInt8("StrCopy"); break;
        case Opcode::ItoS:			ReadUInt8("ItoS"); break;
        case Opcode::StrAdd:		ReadUInt8("StrAdd"); break;
        case Opcode::StrAddi:		ReadUInt8("StrAddi"); break;
        case Opcode::MemCopy:		PrintSingleOp("MemCopy"); break;
        case Opcode::Catch:			PrintSingleOp("Catch"); break;
        case Opcode::Throw:			PrintSingleOp("Throw"); break;
        case Opcode::pCall:			PrintSingleOp("pCall"); break;
        case Opcode::GetXProtect:	PrintSingleOp("GetXProtect"); break;
        case Opcode::SetXProtect:	PrintSingleOp("SetXProtect"); break;
        case Opcode::RefXProtect:	PrintSingleOp("RefXProtect"); break;
        case Opcode::Exit:			PrintSingleOp("Exit"); break;
        case Opcode::ReturnP0R0:	ReadReturnCompact(0); break;
        case Opcode::ReturnP0R1:	ReadReturnCompact(1); break;
        case Opcode::ReturnP0R2:	ReadReturnCompact(2); break;
        case Opcode::ReturnP0R3:	ReadReturnCompact(3); break;
        case Opcode::ReturnP1R0:	ReadReturnCompact(4); break;
        case Opcode::ReturnP1R1:	ReadReturnCompact(5); break;
        case Opcode::ReturnP1R2:	ReadReturnCompact(6); break;
        case Opcode::ReturnP1R3:	ReadReturnCompact(7); break;
        case Opcode::ReturnP2R0:	ReadReturnCompact(8); break;
        case Opcode::ReturnP2R1:	ReadReturnCompact(9); break;
        case Opcode::ReturnP2R2:	ReadReturnCompact(10); break;
        case Opcode::ReturnP2R3:	ReadReturnCompact(11); break;
        case Opcode::ReturnP3R0:	ReadReturnCompact(12); break;
        case Opcode::ReturnP3R1:	ReadReturnCompact(13); break;
        case Opcode::ReturnP3R2:	ReadReturnCompact(14); break;
        case Opcode::ReturnP3R3:	ReadReturnCompact(15); break;
        case Opcode::Push_Neg16:	ReadPushSingleOp(-16); break;
        case Opcode::Push_Neg15:	ReadPushSingleOp(-15); break;
        case Opcode::Push_Neg14:	ReadPushSingleOp(-14); break;
        case Opcode::Push_Neg13:	ReadPushSingleOp(-13); break;
        case Opcode::Push_Neg12:	ReadPushSingleOp(-12); break;
        case Opcode::Push_Neg11:	ReadPushSingleOp(-11); break;
        case Opcode::Push_Neg10:	ReadPushSingleOp(-10); break;
        case Opcode::Push_Neg9:		ReadPushSingleOp(-9); break;
        case Opcode::Push_Neg8:		ReadPushSingleOp(-8); break;
        case Opcode::Push_Neg7:		ReadPushSingleOp(-7); break;
        case Opcode::Push_Neg6:		ReadPushSingleOp(-6); break;
        case Opcode::Push_Neg5:		ReadPushSingleOp(-5); break;
        case Opcode::Push_Neg4:		ReadPushSingleOp(-4); break;
        case Opcode::Push_Neg3:		ReadPushSingleOp(-3); break;
        case Opcode::Push_Neg2:		ReadPushSingleOp(-2); break;
        case Opcode::Push_Neg1:		ReadPushSingleOp(-1); break;
        case Opcode::Push_0:		ReadPushSingleOp(0); break;
        case Opcode::Push_1:		ReadPushSingleOp(1); break;
        case Opcode::Push_2:		ReadPushSingleOp(2); break;
        case Opcode::Push_3:		ReadPushSingleOp(3); break;
        case Opcode::Push_4:		ReadPushSingleOp(4); break;
        case Opcode::Push_5:		ReadPushSingleOp(5); break;
        case Opcode::Push_6:		ReadPushSingleOp(6); break;
        case Opcode::Push_7:		ReadPushSingleOp(7); break;
        case Opcode::PushF_Neg1:	PrintSingleOp("PushF -1"); break;
        case Opcode::PushF_0:		PrintSingleOp("PushF 0"); break;
        case Opcode::PushF_1:		PrintSingleOp("PushF 1"); break;
        case Opcode::PushF_2:		PrintSingleOp("PushF 2"); break;
        case Opcode::PushF_3:		PrintSingleOp("PushF 3"); break;
        case Opcode::PushF_4:		PrintSingleOp("PushF 4"); break;
        case Opcode::PushF_5:		PrintSingleOp("PushF 5"); break;
        case Opcode::PushF_6:		PrintSingleOp("PushF 6"); break;
        case Opcode::PushF_7:		PrintSingleOp("PushF 7"); break;
        case Opcode::BitTest:		PrintSingleOp("BitTest"); break;


        case Opcode::GetLocalS:     PrintSingleOp("GetLocalS"); break;
        case Opcode::SetLocalS:     PrintSingleOp("SetLocalS"); break;
        case Opcode::SetLocalSR:    PrintSingleOp("SetLocalSR"); break;
        case Opcode::GetStaticS:    PrintSingleOp("GetStaticS"); break;
        case Opcode::SetStaticS:    PrintSingleOp("SetStaticS"); break;
        case Opcode::SetStaticSR:   PrintSingleOp("SetStaticSR"); break;
        case Opcode::pGetS:         PrintSingleOp("pGetS"); break;
        case Opcode::pSetS:         PrintSingleOp("pSetS"); break;
        case Opcode::pSetSR:        PrintSingleOp("pSetSR"); break;
        case Opcode::GetGlobalS:    PrintSingleOp("GetGlobalS"); break;
        case Opcode::SetGlobalS:    PrintSingleOp("SetGlobalS"); break;
        case Opcode::SetGlobalSR:   PrintSingleOp("SetGlobalSR"); break;


        case Opcode::GetStaticP3:   ReadUInt24("GetStaticP"); break;
        case Opcode::GetStatic3:    ReadUInt24("GetStatic"); break;
        case Opcode::SetStatic3:    ReadUInt24("SetStatic"); break;


        case Opcode::Uninitialized:
        default:
            if (DecompileTarget == GameTarget::GTAIV)
            {
                ReadPushSingleOp(CodeData[i] - 96);
            }
            else
            {
                Out += "//UNK_OP#";
                Out += to_string(CodeData[i]);
                Out += "\r\n";
            }
            break;
        }

    }

    //catches potential labels at the end of the script
    ParseJumpLabel(JumpLabelInc, CommonHeader.CodeLength);
    ParseCallLabel(CallLabelInc, CommonHeader.CodeLength);


    //final function closing bracket
    Out += "//<\r\n";


    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(asmOutPath.c_str(), file))
    {
        fwrite(OutHeader.data(), 1, OutHeader.size(), file);
        fwrite(Out.data(), 1, Out.size(), file);
        fclose(file);
    }
}

void DecompileBase::GetString(const std::string& stringOutPath)
{
    OutHeader = "";
    Out = "";

    Out.reserve(5000000);

    LoadCodeData();
    CurrentReadPos = CodeData.data();

    for (uint32_t i = 0; i < CommonHeader.CodeLength; i += CurrentOpSize, CurrentReadPos += CurrentOpSize, CurrentOpSize = 1)
    {
        switch (CommonOpsIndexedByTargetOps[CodeData[i]])
        {
        case Opcode::PushB:
        case Opcode::GetArrayP1:
        case Opcode::GetArray1:
        case Opcode::SetArray1:
        case Opcode::GetLocalP1:
        case Opcode::GetLocal1:
        case Opcode::SetLocal1:
        case Opcode::GetStaticP1:
        case Opcode::GetStatic1:
        case Opcode::SetStatic1:
        case Opcode::AddImm1:
        case Opcode::MultImm1:
        case Opcode::GetImmP1:
        case Opcode::GetImm1:
        case Opcode::SetImm1:
        case Opcode::StrCopy:
        case Opcode::ItoS:
        case Opcode::StrAdd:
        case Opcode::StrAddi:
            CurrentOpSize = 2; break;

        case Opcode::Return:
        case Opcode::PushB2:
        case Opcode::PushS:
        case Opcode::AddImm2:
        case Opcode::MultImm2:
        case Opcode::GetImmP2:
        case Opcode::GetImm2:
        case Opcode::SetImm2:
        case Opcode::GetArrayP2:
        case Opcode::GetArray2:
        case Opcode::SetArray2:
        case Opcode::GetLocalP2:
        case Opcode::GetLocal2:
        case Opcode::SetLocal2:
        case Opcode::GetStaticP2:
        case Opcode::GetStatic2:
        case Opcode::SetStatic2:
        case Opcode::GetGlobalP2:
        case Opcode::GetGlobal2:
        case Opcode::SetGlobal2:
            CurrentOpSize = 3; break;

        case Opcode::PushB3:
        case Opcode::GetGlobalP3:
        case Opcode::GetGlobal3:
        case Opcode::SetGlobal3:
        case Opcode::PushI24:
            CurrentOpSize = 4; break;

        case Opcode::Push:
        case Opcode::PushF:
            CurrentOpSize = 5; break;

        case Opcode::CallNative:	CurrentOpSize = GetCallNativeSize(); break;
        case Opcode::Function:		CurrentOpSize = GetFunctionSize(); break;
        case Opcode::PushString:
        {
            uint8_t StringLength = *(CurrentReadPos + 1);
            CurrentOpSize = 2 + StringLength;
            for (uint32_t i = 0; i < StringLength && CurrentReadPos[2 + i] != 0; ++i)
            {
                switch (CurrentReadPos[2 + i])
                {
                case 0:
                    break;
                case '\r':
                    Out += "\\r";
                    continue;
                case '\n':
                    Out += "\\n";
                    continue;
                default:
                    Out += CurrentReadPos[2 + i];
                }
            }
            Out += ':';
            Out += Utils::DataConversion::IntToHex(Utils::Hashing::Joaat(reinterpret_cast<char*>(CurrentReadPos + 2)));
            Out += "\r\n";
        }
        break;
        case Opcode::PushArray:	CurrentOpSize = GetPushArraySize(); break;

        case Opcode::Jump:
        case Opcode::JumpFalse:
        case Opcode::JumpTrue:
        case Opcode::JumpNE:
        case Opcode::JumpEQ:
        case Opcode::JumpLE:
        case Opcode::JumpLT:
        case Opcode::JumpGE:
        case Opcode::JumpGT:
            CurrentOpSize = GetJumpSize(); break;
        case Opcode::Switch:
            CurrentOpSize = GetSwitchSize(); break;

        case Opcode::Call2:
        case Opcode::Call2h1:
        case Opcode::Call2h2:
        case Opcode::Call2h3:
        case Opcode::Call2h4:
        case Opcode::Call2h5:
        case Opcode::Call2h6:
        case Opcode::Call2h7:
        case Opcode::Call2h8:
        case Opcode::Call2h9:
        case Opcode::Call2hA:
        case Opcode::Call2hB:
        case Opcode::Call2hC:
        case Opcode::Call2hD:
        case Opcode::Call2hE:
        case Opcode::Call2hF:
        case Opcode::Call:
            CurrentOpSize = GetCallSize(); break;
        }
    }

    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(stringOutPath.c_str(), file))
    {
        fwrite(Out.data(), 1, Out.size(), file);
        fclose(file);
    }
}

#pragma region Opcodes
void DecompileBase::ParseCallLabel(map<uint32_t, string>::iterator& labelInc, uint32_t index)
{
    if (labelInc != reinterpret_cast<const map<uint32_t, string>*>(labelInc._Getcont())->end())
    {
        //assert(LabelInc->first >= Index);

        if (labelInc->first == index)
        {
            Out += "//<\r\n\r\n:";
            Out += labelInc->second;
            Out += "//>\r\n";
            labelInc++;
        }
    }
}

void DecompileBase::ParseJumpLabel(map<uint32_t, uint32_t>::iterator& labelInc, uint32_t index)
{
    if (labelInc != reinterpret_cast<const map<uint32_t, uint32_t>*>(labelInc._Getcont())->end())
    {
        //assert(LabelInc->first >= Index);
        if (labelInc->first == index)
        {
            Out += "\r\n:Label_";
            Out += to_string(labelInc->second);
            Out += "\r\n";
            labelInc++;
        }
    }
}

void DecompileBase::ReadSwitch()
{
    uint8_t caseCount = *(CurrentReadPos + 1);
    CurrentOpSize = 2 + caseCount * 6;

    vector<pair<int32_t, uint32_t>> caseAndIndex(caseCount);

    for (uint32_t i = 0; i < caseCount; i++)
    {
        uint8_t* index = CurrentReadPos + 2 + i * 6;
        caseAndIndex[i] = {
            Reader->ReadInt32(index),//caseNum
            (index + 6) - CodeData.data() + Reader->ReadInt16(index + 4)//labelIndex
        };
    }

    PrintSwitch(caseAndIndex);
}

#pragma region Print
inline void DecompileBase::PrintNop()
{
    if (Options::DecompileOptions::DecompileNops)
    {
        PrintSingleOp("Nop");
    }
}

inline void DecompileBase::PrintSingleOp(const char* opName)
{
    Out += opName;
    PrintVerbosePC();
    Out += "\r\n";
}

inline void DecompileBase::PrintSingleParamOp(const char* opName, int value)
{
    Out += opName;
    Out += " ";
    Out += std::to_string(value);

    PrintVerbosePC();
    Out += "\r\n";
}

inline void DecompileBase::PrintPush(int value, int value1, int value2)
{
    if (value1 == -1)
    {
        Out += "Push ";
        Out += std::to_string(value);
    }
    else if (value2 == -1)
    {
        Out += "PushB2 ";
        Out += std::to_string(value);
        Out += ", ";
        Out += std::to_string(value1);
    }
    else
    {
        Out += "PushB3 ";
        Out += std::to_string(value);
        Out += ", ";
        Out += std::to_string(value1);
        Out += ", ";
        Out += std::to_string(value2);
    }

    PrintVerbosePC();

    Out += "\r\n";
}

inline void DecompileBase::PrintPushF(float value)
{
    Out += "PushF ";
    Out += std::to_string(value);

    PrintVerbosePC();
    Out += "\r\n";
}

inline void DecompileBase::PrintReturn(int popCount, int returnCount)
{
    Out += "Return ";
    Out += std::to_string(popCount);
    Out += " ";
    Out += std::to_string(returnCount);

    PrintVerbosePC();
    Out += "\r\n";
}

//used only by rdr and gta4
inline void DecompileBase::PrintPushString(char* str, int size)
{
    Out += "PushString \"";
    //Out += Utils::DataConversion::StringToDataString(std::string(str, size));
    Out += Utils::DataConversion::StringToDataString(str);
    Out += "\"";


    PrintVerbosePC();
    Out += "\r\n";

}

inline void DecompileBase::PrintPushStringS(int index)
{

    Out += "PushString \"";
    Out += Utils::DataConversion::StringToDataString(reinterpret_cast<char*>(StringData.data() + index));
    Out += "\"";

    PrintVerbosePC();

    if (Options::DecompileOptions::Verbose)
    {
        Out += ", SI: ";
        Out += to_string(index);
    }

    Out += "\r\n";


}

inline void DecompileBase::PrintFunction(int paramCount, int varCount, char* name, int nameLength)
{
    if (nameLength)
        CheckFunctionUnused(name);
    else
        CheckFunctionUnused();

    Out += "Function ";
    Out += std::to_string(paramCount);
    Out += " ";
    Out += std::to_string(varCount);


    PrintVerbosePC();
    Out += "\r\n";
}

inline void DecompileBase::PrintCallNative(uint32_t nativeIndex, int32_t nativeParamCount, int32_t nativeRetCount, bool rawNative)
{
    Out += "CallNative ";

    if (nativeIndex >= CommonHeader.NativesCount)
    {
        //write unnamed native
        Out += "_unk_native_index_";
        Out += to_string(nativeIndex);
        Out += " ";
        Out += to_string(nativeParamCount);
        Out += " ";
        Out += to_string(nativeRetCount);

        Out += "\r\n";
        return;
    }

    uint64_t nativeValue = rawNative ?
        nativeIndex :
        (Is64Bit ?
            Reader->ReadUInt64(reinterpret_cast<uint8_t*>((uint64_t*)CommonHeader.NativesOffset + nativeIndex)) :
            Reader->ReadUInt32(reinterpret_cast<uint8_t*>((uint32_t*)CommonHeader.NativesOffset + nativeIndex)));

    if (NativeTranslationMap.size() > 0)
    {
        auto foundTransNative = NativeTranslationMap.find(nativeValue);

        if (foundTransNative != NativeTranslationMap.end())
        {
            nativeValue = foundTransNative->second;
        }
        else
            Utils::System::Warn("Native " + Utils::DataConversion::IntToHex(nativeValue) + " was not in translation table ... using default");
    }

    auto foundNative = NativeMap.find(nativeValue);


    if (foundNative != NativeMap.end())
    {
        string& NativeName = foundNative->second;

        //write named native
        Out += NativeName;
        Out += " ";
        Out += to_string(nativeParamCount);
        Out += " ";
        Out += to_string(nativeRetCount);

    }
    else
    {
        //write unnamed native
        Out += "_0x";
        if (Is64Bit)
            Out += Utils::DataConversion::IntToHex(Utils::Bitwise::SwapEndian(nativeValue));
        else
            Out += Utils::DataConversion::IntToHex(Utils::Bitwise::SwapEndian((uint32_t)nativeValue));

        Out += " ";
        Out += to_string(nativeParamCount);
        Out += " ";
        Out += to_string(nativeRetCount);
    }


    PrintVerbosePC();
    if (Options::DecompileOptions::Verbose)
    {
        if (!rawNative)
        {
            Out += ", NI: ";
            Out += to_string(nativeIndex);
        }

        if (foundNative != NativeMap.end())
        {
            Out += ", N: _0x";
            if (Is64Bit)
                Out += Utils::DataConversion::IntToHex(Utils::Bitwise::SwapEndian(nativeValue));
            else
                Out += Utils::DataConversion::IntToHex(Utils::Bitwise::SwapEndian((uint32_t)nativeValue));
        }
    }

    Out += "\r\n";
}

inline void DecompileBase::PrintJump(const char* opName, uint32_t index)
{
    Out += opName;
    Out += " @Label_";
    Out += std::to_string(JumpLabelIndexes[index]);


    PrintVerbosePC();
    if (Options::DecompileOptions::Verbose)
    {
        Out += ", JI: ";
        Out += to_string(index);
    }

    Out += "\r\n";
}

inline void DecompileBase::PrintSwitch(std::vector<std::pair<int32_t, uint32_t>> caseAndIndex)
{
    Out += "Switch ";

    for (auto i : caseAndIndex)
    {
        Out += "[";
        Out += to_string(i.first);
        Out += " @Label_";
        Out += to_string(JumpLabelIndexes[i.second]);
        Out += "]";
    }

    Out += "\r\n";

    PrintVerbosePC();
    if (Options::DecompileOptions::Verbose)
    {
        for (auto i : caseAndIndex)
        {
            Out += ", JI(";
            Out += to_string(i.first);
            Out += "): ";
            Out += to_string(i.second);
        }
        Out += "\r\n";
    }

}

inline void DecompileBase::PrintCall(uint32_t index)
{
    Out += "Call @";
    Out += CallLabelIndexes[index];

    PrintVerbosePC();
    if (Options::DecompileOptions::Verbose)
    {
        Out += ", CI: ";
        Out += to_string(index);
    }

    Out += "\r\n";
}

inline void DecompileBase::PrintPushArray(uint8_t* data, uint32_t size)
{
    Out += "PushArray ";
    Out += Utils::DataConversion::DataToHex(data, size);

    PrintVerbosePC();
    Out += "\r\n";
}

inline void DecompileBase::PrintVerbosePC()
{
    if (Options::DecompileOptions::Verbose)
    {
        Out += "//PC: ";
        Out += to_string(CurrentReadPos - CodeData.data());
    }
}

#pragma endregion


#pragma endregion

#pragma endregion

#pragma region DecompileGTAIV
void DecompileGTAIV::OpenScript(vector<uint8_t>& data)
{
    CurrentReadPos = data.data();

    SCRFlag format = (SCRFlag)Utils::Bitwise::SwapEndian(Reader->ReadUInt32(CurrentReadPos));

    CommonHeader.HeaderPtr = CurrentReadPos;


    GTAIVHeader* header = (GTAIVHeader*)CommonHeader.HeaderPtr;
    CommonHeader.StaticsCount = Reader->ReadUInt32(&(header->StaticsCount));
    CommonHeader.GlobalsCount = Reader->ReadUInt32(&(header->GlobalsCount));
    CommonHeader.CodeLength = Reader->ReadUInt32(&(header->CodeLength));
    CommonHeader.Signature = (Signature)Reader->ReadUInt32(&(header->Signature));

    uint32_t decompressedSize = CommonHeader.StaticsCount * 4 + CommonHeader.GlobalsCount * 4 + CommonHeader.CodeLength;

    CurrentReadPos += sizeof(GTAIVHeader);

    switch (format)
    {
    case SCRFlag::CompressedEncrypted: 
    {
        uint32_t CompressedSize = Reader->ReadUInt32(CurrentReadPos);
        CurrentReadPos += 4;

        if (!Utils::Crypt::AES_Decrypt(CurrentReadPos, data.size() - sizeof(GTAIVHeader) - 4, GTAIVKey))
            Utils::System::Throw("SCO Decryption Failed");

        CodeData.reserve(decompressedSize);
        Utils::Compression::ZLIB_DecompressNew(CurrentReadPos, CompressedSize, CodeData);

        if (CodeData.size() <= 0)
            Utils::System::Throw("SCO Decompressed Size Invalid");

        StaticsAndGlobals.resize(CommonHeader.StaticsCount * 4 + CommonHeader.GlobalsCount * 4);

        uint8_t* sngData = StaticsAndGlobals.data();
        memcpy_s(sngData, StaticsAndGlobals.size(), CodeData.data() + CommonHeader.CodeLength, CodeData.size() - CommonHeader.CodeLength);

        CommonHeader.StaticsOffset = (uint32_t*)sngData;
        CommonHeader.GlobalsOffset = (uint32_t*)(sngData + CommonHeader.StaticsCount * 4);

        CodeData.resize(CommonHeader.CodeLength);
    }
        break;
    case SCRFlag::Encrypted: 
        if (!Utils::Crypt::AES_Decrypt(CurrentReadPos, CommonHeader.CodeLength, GTAIVKey))
            Utils::System::Throw("SCO Decryption Failed");

        if (CommonHeader.StaticsCount > 0)
        {
            if (!Utils::Crypt::AES_Decrypt(CurrentReadPos + CommonHeader.CodeLength, CommonHeader.StaticsCount * 4, GTAIVKey))
                Utils::System::Throw("SCO Decryption Failed");
        }

        if (CommonHeader.GlobalsCount > 0)
        {
            if (!Utils::Crypt::AES_Decrypt(CurrentReadPos + CommonHeader.CodeLength + CommonHeader.StaticsCount * 4, CommonHeader.GlobalsCount * 4, GTAIVKey))
                Utils::System::Throw("SCO Decryption Failed");
        }
        //intentional no break
    case SCRFlag::Standard: 
    {
        
        CodeData.resize(CommonHeader.CodeLength);
        memcpy_s(CodeData.data(), CodeData.size(), CurrentReadPos, CommonHeader.CodeLength);

        StaticsAndGlobals.resize(CommonHeader.StaticsCount * 4 + CommonHeader.GlobalsCount * 4);
        

        memcpy_s(StaticsAndGlobals.data(), StaticsAndGlobals.size(), CurrentReadPos + CommonHeader.CodeLength, StaticsAndGlobals.size());

        CommonHeader.StaticsOffset = (uint32_t*)StaticsAndGlobals.data();
        CommonHeader.GlobalsOffset = (uint32_t*)(StaticsAndGlobals.data() + CommonHeader.StaticsCount * 4);

    }
        break;
    }


    if (Options::DecompileOptions::Verbose)
    {
        cout << CommonHeader.GlobalsCount << " GlobalsCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;

    }

    //
    //
    //switch (format)
    //{
    //    case SCRFlag::Standard:
    //    case SCRFlag::Encrypted:
    //    case SCRFlag::CompressedEncrypted:
    //    break;
    //
    //}
    //
    //RSCFlag Flags = { Reader->ReadUInt32(CurrentReadPos + offsetof(CSRHeader, Flag1)), Reader->ReadUInt32(CurrentReadPos + offsetof(CSRHeader, Flag2)) };
    //uint32_t DecompressedSize = GetSizeFromFlag(Flags.Flag[0], Flags.Flag[1]);//index 12 = xcompress decompile size
    //
    //CurrentReadPos += sizeof(CSRHeader);
    //
    //
    //if (!Utils::Crypt::AES_Decrypt(CurrentReadPos, data.size() - sizeof(CSRHeader), Key))
    //	Utils::System::Throw("Decryption Failed");
    //
    //if(header == compressed)
    //
    //uint32_t HeaderLocation = GetObjectStartPageOffset(Flags);
    //if (HeaderLocation != 0xFFFFFFFF)
    //{
    //	if (HeaderLocation < data.size())
    //		CurrentReadPos = data.data() + HeaderLocation;
    //	else
    //		Utils::System::Throw("Header Location Out Of Bounds");
    //}
    //else
    //	Utils::System::Throw("Invalid Header Location");
    //
    //
    //auto j = Reader->ReadUInt32(CurrentReadPos);
    //if (Reader->ReadUInt32(CurrentReadPos) != 0xA8D74300)
    //	Utils::System::Throw("Header Not Found");
    //
    //CommonHeader.HeaderPtr = CurrentReadPos;
    //
    //CommonHeader.TotalCodeLength = Reader->ReadUInt32(CurrentReadPos + offsetof(Header, TotalCodeLength));
    //CommonHeader.CodeBlocksListCount = (CommonHeader.TotalCodeLength + ((1 << 14) - 1)) >> 14;
    //CommonHeader.CodeBlocksListOffset.resize(CommonHeader.CodeBlocksListCount);
    //
    //uint32_t* CodeBlocksListOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(Header, CodeBlocksListOffset))).ToPtr<uint32_t>(data.data());
    //
    //for (uint32_t i = 0; i < CommonHeader.CodeBlocksListCount; i++)
    //	CommonHeader.CodeBlocksListOffset[i] = RelPtr(Reader->ReadUInt32(reinterpret_cast<uint8_t*>(CodeBlocksListOffset + i))).ToPtr<uint8_t>(data.data());
    //
    //CommonHeader.TotalStringLength = 0;
    //CommonHeader.StringBlocksListCount = 0;
    //
    //CommonHeader.NativesCount = Reader->ReadUInt32(CurrentReadPos + offsetof(Header, NativesCount));
    //CommonHeader.NativesOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(Header, NativesOffset))).ToPtr<uint32_t>(data.data());
    //
    //CommonHeader.StaticsCount = Reader->ReadUInt32(CurrentReadPos + offsetof(Header, StaticsCount));
    //CommonHeader.StaticsOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(Header, StaticsOffset))).ToPtr<uint32_t>(data.data());
    //
    //CommonHeader.ScriptName = "";
    //
    //CommonHeader.ParameterCount = Reader->ReadUInt32(CurrentReadPos + offsetof(Header, ParameterCount));

}


#pragma region Opcodes

#pragma endregion

#pragma endregion

#pragma region DecompileRDR

#pragma region Labels
void DecompileRDR::LogCallLabel(uint8_t callType)
{
    CurrentOpSize = 3;
    if (callType >= 0 && callType <= 15)
    {
        uint32_t FunctionIndex = GetCallOffset(Reader->ReadUInt16(CurrentReadPos + 1), callType);

        if (CallLabelIndexes.find(FunctionIndex) == CallLabelIndexes.end())
        {
            uint8_t* FunctionPos = 0;
            if (FindFunctionFromCallPos(CodeData.data() + FunctionIndex, FunctionPos))
            {


                uint8_t FunctionNameLength = *(FunctionPos + 4);

                assert(FunctionPos >= CodeData.data());
                UsedFunctions.insert(FunctionPos - CodeData.data());

                if (FunctionNameLength)
                    CallLabelIndexes.insert({ FunctionIndex, std::string(reinterpret_cast<char*>(CurrentReadPos + 5)) });
                else
                    CallLabelIndexes.insert({ FunctionIndex, "Function_" + std::to_string(CallLabelIndexes.size()) });
            }
        }
    }
    else
        Utils::System::Throw("Invalid call type " + std::to_string(callType) + " for CallCompact");
}


#pragma endregion

#pragma region Opcodes

void DecompileRDR::ReadCallNative()
{
    CurrentOpSize = 3;
    uint16_t nativeData = Utils::Bitwise::SwapEndian(Reader->ReadUInt16(CurrentReadPos + 1));

    PrintCallNative(GetCallIndex(nativeData), GetArgCountFromIndex(&nativeData), FunctionHasReturn(&nativeData));
}

void DecompileRDR::ReadCallCompact(uint8_t callType)
{
    CurrentOpSize = 3;
    if (callType >= 0 && callType <= 15)
    {
        int32_t callOffset = GetCallOffset(Reader->ReadUInt16(CurrentReadPos + 1), callType);

        PrintCall(callOffset);
    }
    else
        Utils::System::Throw("Invalid call type " + std::to_string(callType) + " for CallCompact");
}

void DecompileRDR::ReadReturnCompact(uint8_t returnType)
{
    if (returnType >= 0 && returnType <= 15)
    {
        PrintReturn(returnType / 4, returnType % 4);
    }
    else
        Utils::System::Throw("Invalid return type for ReturnCompact");
}

void DecompileRDR::ReadPushArray()
{
    const size_t arrayLength = Reader->ReadUInt32(CurrentReadPos + 1);
    CurrentOpSize = 5 + arrayLength;
    PrintPushArray(CurrentReadPos + 5, arrayLength);
}

#pragma endregion

void DecompileRDR::OpenScript(vector<uint8_t>& data)
{
    CurrentReadPos = data.data();

    if (Reader->ReadUInt32(CurrentReadPos + offsetof(CSRHeader, ResourceType)) != 2)
        Utils::System::Throw("Invalid Resource Type");

    RSCFlag flags = {
        Reader->ReadUInt32(CurrentReadPos + offsetof(CSRHeader, Flags)),
        Reader->ReadUInt32(CurrentReadPos + offsetof(CSRHeader, Flags) + 4)
    };
    uint64_t decompressedSize = GetSizeFromFlag(flags.Flag[0], flags.Flag[1]);//index 12 = xcompress decompile size

    CurrentReadPos += sizeof(CSRHeader);


    if (!Utils::Crypt::AES_Decrypt(CurrentReadPos, data.size() - sizeof(CSRHeader), RDRKey))
        Utils::System::Throw("Decryption Failed");

    switch (Options::Platform)
    {
    case Platform::XBOX:
    {
        vector<uint8_t> decompressedData(decompressedSize);
        uint32_t compressedSize = Reader->ReadUInt32(CurrentReadPos + offsetof(CompressedHeader, CompressedSize));

        const uint32_t compressedDataStart = sizeof(CSRHeader) + sizeof(CompressedHeader);
        if (compressedSize != data.size() - compressedDataStart)
            Utils::System::Throw("Invalid Compression Size");

        CurrentReadPos += sizeof(CompressedHeader);

        Utils::Compression::XCompress_Decompress(CurrentReadPos, compressedSize, decompressedData.data(), &decompressedSize);

        data.resize(decompressedSize);
        memcpy(data.data(), decompressedData.data(), decompressedSize);
    }
    break;
    case Platform::PSX:
    {
        vector<uint8_t> decompressedData(decompressedSize);
        uint32_t dSize = 0;
        Utils::Compression::ZLIB_Decompress(CurrentReadPos, data.size() - sizeof(CSRHeader), decompressedData.data(), dSize);
        decompressedSize = dSize;

        data.resize(decompressedSize);
        memcpy(data.data(), decompressedData.data(), decompressedSize);
    }
    break;
    }

    uint32_t headerLocation = GetObjectStartPageOffset(flags);
    if (headerLocation != 0xFFFFFFFF)
    {
        if (headerLocation < data.size())
            CurrentReadPos = data.data() + headerLocation;
        else
            Utils::System::Throw("Header Location Out Of Bounds");
    }
    else
        Utils::System::Throw("Invalid Header Location");


    auto unk = Reader->ReadUInt32(CurrentReadPos);
    if (Reader->ReadUInt32(CurrentReadPos) != 0xA8D74300)
        Utils::System::Throw("Header Not Found");

    CommonHeader.HeaderPtr = CurrentReadPos;

    CommonHeader.CodeLength = Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, CodeLength));

    CommonHeader.TotalStringLength = 0;
    CommonHeader.StringBlocksCount = 0;

    CommonHeader.NativesCount = Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, NativesCount));
    CommonHeader.NativesOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, NativesOffset))).GetPtr<uint32_t>(data.data());

    CommonHeader.StaticsCount = Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, StaticsCount));
    CommonHeader.StaticsOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, StaticsOffset))).GetPtr<uint32_t>(data.data());

    CommonHeader.ScriptName = (char*)"";

    CommonHeader.ParameterCount = Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, ParameterCount));

    CommonHeader.Signature = (Signature)Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, Signature));

    if (Options::DecompileOptions::Verbose)
    {
        cout << CommonHeader.NativesCount << " NativesCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;

        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, PageMapOffset)) & 0x0FFFFFFF) << " PageMapOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, NativesOffset)) & 0x0FFFFFFF) << " NativesOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, StaticsOffset)) & 0x0FFFFFFF) << " StaticsOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, CodeBlocksOffset)) & 0x0FFFFFFF) << " CodeBlocksOffset " << endl;

    }


    CommonHeader.CodeBlocksCount = (CommonHeader.CodeLength + ((1 << 14) - 1)) >> 14;
    CommonHeader.CodeBlockOffsets.resize(CommonHeader.CodeBlocksCount);

    uint32_t* codeBlocksListOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(RDRHeader, CodeBlocksOffset))).GetPtr<uint32_t>(data.data());

    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        RelPtr offset = RelPtr(Reader->ReadUInt32(reinterpret_cast<uint8_t*>(codeBlocksListOffset + i)));
        CommonHeader.CodeBlockOffsets[i] = offset.GetPtr<uint8_t>(data.data());
        if (Options::DecompileOptions::Verbose)
            cout << offset.GetValue() << " Code Block " << endl;
    }

}

uint32_t DecompileRDR::GetObjectStartPageOffset(RSCFlag flags)
{
    uint32_t dwPageCounts[4] = { flags.VPage0, flags.VPage1, flags.VPage2, 0x7FFFFFFF };

    uint32_t dwStartOffset = 0xFFFFFFFF;
    uint32_t dwPageSize = 0x80000; // largest possible page size
    uint32_t dwTotalSize = 0;
    uint32_t dwLeft = flags.TotalVSize << 12;
    uint32_t dwStartSize = 0x1000 << flags.ObjectStartPage;

    for (uint32_t i = 0; i < 4; i++)
    {
        for (uint32_t j = 0; j < dwPageCounts[i] && dwLeft; j++)
        {
            while (dwPageSize > dwLeft)
                dwPageSize >>= 1;

            if (!dwPageSize)
                Utils::System::Throw("PageSize = 0");

            if (dwPageSize == dwStartSize && 0xFFFFFFFF == dwStartOffset)
                dwStartOffset = dwTotalSize;

            dwTotalSize += dwPageSize;
            dwLeft -= dwPageSize;
        }
        dwPageSize >>= 1;
    }


    if (dwTotalSize != (flags.TotalVSize << 12))
        Utils::System::Throw("Size mismatch: Total = 0x" + Utils::DataConversion::IntToHex(dwTotalSize) + ", Flags = 0x" + Utils::DataConversion::IntToHex(flags.TotalVSize << 12));
    else if (0xFFFFFFFF == dwStartOffset)
        Utils::System::Throw("Resource start not found");

    return dwStartOffset;
}

#pragma region Opcodes

#pragma endregion

#pragma endregion

#pragma region DecompileGTAVConsole
DecompileGTAVConsole::DecompileGTAVConsole() : DecompileBase()
{
    Is64Bit = false;
    DecompileTarget = GameTarget::GTAV;
    MaxPageSize = 16384;
    SetEndian(false);
    SetOps(_CommonOpsIndexedByTargetOps);
}

void DecompileGTAVConsole::OpenScript(vector<uint8_t>& data)
{
    CurrentReadPos = const_cast<uint8_t*>(data.data());

    if (Utils::Bitwise::SwapEndian(*reinterpret_cast<uint32_t*>(CurrentReadPos)) == 'RSC7')
        CurrentReadPos += 16;

    CommonHeader.HeaderPtr = CurrentReadPos;

    CommonHeader.CodeLength = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, CodeLength));
    CommonHeader.NativesCount = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, NativesCount));
    CommonHeader.NativesOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, NativesOffset))).GetPtr<uint32_t>(CurrentReadPos);

    CommonHeader.StaticsCount = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, StaticsCount));
    CommonHeader.StaticsOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, StaticsOffset))).GetPtr<uint32_t>(CurrentReadPos);

    CommonHeader.GlobalsCount = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, GlobalsCount));
    CommonHeader.GlobalsOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, GlobalsOffset))).GetPtr<uint32_t>(CurrentReadPos);

    CommonHeader.ScriptName = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, ScriptNameOffset))).GetPtr<char>(CurrentReadPos);


    CommonHeader.ParameterCount = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, ParameterCount));

    CommonHeader.Signature = (Signature)Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, Signature));
    CommonHeader.TotalStringLength = Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, TotalStringLength));

    if (Options::DecompileOptions::Verbose)
    {
        cout << CommonHeader.ScriptName << " ScriptName" << endl;
        cout << CommonHeader.NativesCount << " NativesCount" << endl;
        cout << CommonHeader.GlobalsCount << " GlobalsCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;
        cout << CommonHeader.TotalStringLength << " TotalStringLength" << endl;

        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, PageMapOffset)) & 0x0FFFFFFF) << " PageMapOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, ScriptNameOffset)) & 0x0FFFFFFF) << " ScriptNameOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, GlobalsOffset)) & 0x0FFFFFFF) << " GlobalsOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, NativesOffset)) & 0x0FFFFFFF) << " NativesOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, StaticsOffset)) & 0x0FFFFFFF) << " StaticsOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, CodeBlocksOffset)) & 0x0FFFFFFF) << " CodeBlocksOffset " << endl;
        cout << (Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, StringBlocksOffset)) & 0x0FFFFFFF) << " StringBlocksOffset " << endl;

    }


    CommonHeader.CodeBlocksCount = (CommonHeader.CodeLength + ((1 << 14) - 1)) >> 14;
    CommonHeader.CodeBlockOffsets.resize(CommonHeader.CodeBlocksCount);

    uint32_t* codeBlocksOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, CodeBlocksOffset))).GetPtr<uint32_t>(CurrentReadPos);

    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        RelPtr offset = RelPtr(Reader->ReadUInt32(reinterpret_cast<uint8_t*>(codeBlocksOffset + i)));
        CommonHeader.CodeBlockOffsets[i] = offset.GetPtr<uint8_t>(CurrentReadPos);
        if (Options::DecompileOptions::Verbose)
            cout << offset.GetValue() << " Code Block " << endl;
    }

    CommonHeader.StringBlocksCount = (CommonHeader.TotalStringLength + ((1 << 14) - 1)) >> 14;
    CommonHeader.StringBlockOffsets.resize(CommonHeader.StringBlocksCount);

    uint32_t* stringBlocksOffset = RelPtr(Reader->ReadUInt32(CurrentReadPos + offsetof(GTAVHeader, StringBlocksOffset))).GetPtr<uint32_t>(CurrentReadPos);

    for (uint32_t i = 0; i < CommonHeader.StringBlocksCount; i++)
    {
        RelPtr offset = RelPtr(Reader->ReadUInt32(reinterpret_cast<uint8_t*>(stringBlocksOffset + i)));
        CommonHeader.StringBlockOffsets[i] = offset.GetPtr<char>(CurrentReadPos);
        if (Options::DecompileOptions::Verbose)
            cout << offset.GetValue() << " String Block " << endl;
    }

}

#pragma region Labels
void DecompileGTAVConsole::LogCallLabel(uint8_t callType)
{
    CurrentOpSize = 4;
    uint32_t FunctionIndex = Reader->ReadUInt24(CurrentReadPos + 1);

    if (CallLabelIndexes.find(FunctionIndex) == CallLabelIndexes.end())
    {
        uint8_t* FunctionPos = 0;
        if (FindFunctionFromCallPos(CodeData.data() + FunctionIndex, FunctionPos))
        {


            uint8_t FunctionNameLength = *(FunctionPos + 4);

            assert(FunctionPos >= CodeData.data());

            UsedFunctions.insert(FunctionPos - CodeData.data());

            if (FunctionNameLength)
            {
                std::string name(FunctionNameLength, 0);

                strncpy(name.data(), reinterpret_cast<char*>(FunctionPos + 5), FunctionNameLength);


                if (!Utils::DataConversion::IsStringDataString(name))
                {
                    CallLabelIndexes.insert({ FunctionIndex, "Function_" + std::to_string(CallLabelIndexes.size()) + "(" + name + ")" });
                    return;
                }

            }

            CallLabelIndexes.insert({ FunctionIndex, "Function_" + std::to_string(CallLabelIndexes.size()) });
        }
    }
}

#pragma endregion

void DecompileGTAVConsole::GetString(const std::string& stringOutPath)
{
    OutHeader = "";
    Out = "";

    Out.reserve(5000000);

    LoadStringData();

    char* currentReadPos = StringData.data();

    uint32_t currentHash = Utils::Hashing::Joaat(currentReadPos);

    for (uint32_t i = 0; i < StringData.size(); i++)
    {

        switch (currentReadPos[i])
        {
        case '\0':
            Out += ':';
            Out += Utils::DataConversion::IntToHex(currentHash);
            Out += "\r\n";
            currentHash = Utils::Hashing::Joaat(&currentReadPos[i + 1]);
            continue;
        case '\r':
            Out += "\\r";
            continue;
        case '\n':
            Out += "\\n";
            continue;
        default:
            Out += currentReadPos[i];
        }
    }


    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(stringOutPath.c_str(), file))
    {
        fwrite(Out.data(), 1, Out.size(), file);
        fclose(file);
    }
}

bool DecompileGTAVConsole::CheckNextOpcodeForStack(int32_t pushValue)
{
    uint32_t CurrentOpIndex = CurrentReadPos - CodeData.data();
    for (; CurrentOpIndex + CurrentOpSize < CodeData.size(); CurrentOpSize++)
    {
        switch (GetOps()[*(CurrentReadPos + CurrentOpSize)])
        {
            //could add decompilation of pcalls here
            //case Opcode::pCall:
                //return false;
        case Opcode::Nop:
            continue;
        case Opcode::PushStringS:
            //assert(pushValue >= 0);
            if (pushValue >= 0 && static_cast<uint32_t>(pushValue) < StringData.size())
            {
                CurrentOpSize++;

                PrintPushStringS(pushValue);

                return true;
            }

            return false;
        default:
            return false;
        }
    }
    return false;
}


#pragma endregion

#pragma region DecompileGTAVPC

DecompileGTAVPC::DecompileGTAVPC() : DecompileGTAVConsole()
{
    Is64Bit = true;
    DecompileTarget = GameTarget::GTAV;
    MaxPageSize = 16384;
    SetEndian(true);
    SetOps(_CommonOpsIndexedByTargetOps);
}

void DecompileGTAVPC::ReadCallNative()
{
    CurrentOpSize = 4;
    uint8_t NativeFlagHolder = *(CurrentReadPos + 1);
    PrintCallNative(Utils::Bitwise::SwapEndian(Reader->ReadUInt16(CurrentReadPos + 2)), NativeFlagHolder >> 2, NativeFlagHolder & 3);
}

void DecompileGTAVPC::OpenScript(vector<uint8_t>& data)
{
    CurrentReadPos = const_cast<uint8_t*>(data.data());

    if (Utils::Bitwise::SwapEndian(*reinterpret_cast<uint32_t*>(CurrentReadPos)) == 'RSC7')
        CurrentReadPos += 16;

    GTAVPCHeader* header = (GTAVPCHeader*)CurrentReadPos;

    int codeBlocksCount = (header->CodeLength + ((1 << 14) - 1)) >> 14;
    int stringBlocksCount = (header->TotalStringLength + ((1 << 14) - 1)) >> 14;

    CommonHeader.HeaderPtr = (uint8_t*)header;
    CommonHeader.CodeBlocksCount = codeBlocksCount;
    CommonHeader.StringBlocksCount = stringBlocksCount;
    CommonHeader.StaticsCount = header->StaticsCount;
    CommonHeader.NativesCount = header->NativesCount;
    CommonHeader.GlobalsCount = header->GlobalsCount;
    CommonHeader.CodeLength = header->CodeLength;
    CommonHeader.TotalStringLength = header->TotalStringLength;
    CommonHeader.ParameterCount = header->ParameterCount;
    CommonHeader.ScriptName = header->ScriptNameOffset.GetPtr<char>(CommonHeader.HeaderPtr);
    CommonHeader.GlobalsOffset = header->GlobalsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.NativesOffset = header->NativesOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.StaticsOffset = header->StaticsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.Signature = header->Signature;


    if (Options::DecompileOptions::Verbose)
    {
        cout << CommonHeader.ScriptName << " ScriptName" << endl;
        cout << CommonHeader.NativesCount << " NativesCount" << endl;
        cout << CommonHeader.GlobalsCount << " GlobalsCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;
        cout << CommonHeader.TotalStringLength << " TotalStringLength" << endl;

        cout << (header->PageMapOffset.Value & 0x0FFFFFFF) << " PageMapOffset " << endl;
        cout << (header->ScriptNameOffset.Value & 0x0FFFFFFF) << " ScriptNameOffset " << endl;
        cout << (header->GlobalsOffset.Value & 0x0FFFFFFF) << " GlobalsOffset " << endl;
        cout << (header->NativesOffset.Value & 0x0FFFFFFF) << " NativesOffset " << endl;
        cout << (header->StaticsOffset.Value & 0x0FFFFFFF) << " StaticsOffset " << endl;
        cout << (header->CodeBlocksOffset.Value & 0x0FFFFFFF) << " CodeBlocksOffset " << endl;
        cout << (header->StringBlocksOffset.Value & 0x0FFFFFFF) << " StringBlocksOffset " << endl;

    }

    uint64_t* codeBlocksOffset = header->CodeBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.CodeBlockOffsets.resize(CommonHeader.CodeBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        CommonHeader.CodeBlockOffsets[i] = RelPtr64(*(codeBlocksOffset + i)).GetPtr<uint8_t>(CommonHeader.HeaderPtr);
        if (Options::DecompileOptions::Verbose)
            cout << (*(codeBlocksOffset + i) & 0x0FFFFFFF) << " Code Block " << endl;
    }

    uint64_t* stringBlocksOffset = header->StringBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.StringBlockOffsets.resize(CommonHeader.StringBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.StringBlocksCount; i++)
    {
        CommonHeader.StringBlockOffsets[i] = RelPtr64(*(stringBlocksOffset + i)).GetPtr<char>(CommonHeader.HeaderPtr);
        if (Options::DecompileOptions::Verbose)
            cout << (*(stringBlocksOffset + i) & 0x0FFFFFFF) << " String Block " << endl;
    }


    for (uint32_t i = 0; i < CommonHeader.NativesCount; i++)
    {
        //GTA V PC natives arent stored sequentially in the table. 
        //Each native needs a bitwise rotate depending on its position and codetable size
        uint64_t native = *((uint64_t*)CommonHeader.NativesOffset + i);
        *((uint64_t*)CommonHeader.NativesOffset + i) = Utils::Bitwise::rotl(native, (CommonHeader.CodeLength + i) & 0x3F);
    }


}

#pragma endregion

#pragma region DecompileRDR2Console

DecompileRDR2Console::DecompileRDR2Console() : DecompileGTAVPC()
{
    Is64Bit = true;
    DecompileTarget = GameTarget::RDR2;
    MaxPageSize = 16384;
    SetEndian(true);
    SetOps(_CommonOpsIndexedByTargetOps);
}

void DecompileRDR2Console::LoadOpcodes()
{
    if (Options::DecompileOptions::OpcodeVersion == 0)
    {
        bool triggered = false;
        uint32_t size = lengthof(_CommonOpsIndexedByTargetOps);
        for (int i = 0; i < size; i++)
        {
            if (triggered || _CommonOpsIndexedByTargetOps[i] == Opcode::GetStaticP3)
            {
                if (i + 3 < size)
                {
                    _CommonOpsIndexedByTargetOps[i] = _CommonOpsIndexedByTargetOps[i + 3];
                    triggered = true;
                }
            }
        }
    }
}

void DecompileRDR2Console::OpenScript(vector<uint8_t>& data)
{
    LoadOpcodes();

    CurrentReadPos = const_cast<uint8_t*>(data.data());

    if (Utils::Bitwise::SwapEndian(*reinterpret_cast<uint32_t*>(CurrentReadPos)) == 'RSC8')
        CurrentReadPos += 16;

    RDR2Header* header = (RDR2Header*)CurrentReadPos;

    int codeBlocksCount = (header->CodeLength + ((1 << 14) - 1)) >> 14;
    int stringBlocksCount = (header->TotalStringLength + ((1 << 14) - 1)) >> 14;

    CommonHeader.HeaderPtr = (uint8_t*)header;
    CommonHeader.CodeBlocksCount = codeBlocksCount;
    CommonHeader.StringBlocksCount = stringBlocksCount;
    CommonHeader.StaticsCount = header->StaticsCount;
    CommonHeader.NativesCount = header->NativesCount;
    CommonHeader.GlobalsCount = header->GlobalsCount;
    CommonHeader.CodeLength = header->CodeLength;
    CommonHeader.TotalStringLength = header->TotalStringLength;
    CommonHeader.ParameterCount = header->ParameterCount;
    CommonHeader.ScriptName = header->ScriptNameOffset.GetPtr<char>(CommonHeader.HeaderPtr);
    CommonHeader.GlobalsOffset = header->GlobalsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.NativesOffset = header->NativesOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.StaticsOffset = header->StaticsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.Signature = header->Signature;

    if (Options::DecompileOptions::Verbose)
    {
        cout << CommonHeader.ScriptName << " ScriptName" << endl;
        cout << CommonHeader.NativesCount << " NativesCount" << endl;
        cout << CommonHeader.GlobalsCount << " GlobalsCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;
        cout << CommonHeader.TotalStringLength << " TotalStringLength" << endl;

        cout << (header->PageMapOffset.Value & 0x0FFFFFFF) << " PageMapOffset " << endl;
        cout << (header->ScriptNameOffset.Value & 0x0FFFFFFF) << " ScriptNameOffset " << endl;
        cout << (header->GlobalsOffset.Value & 0x0FFFFFFF) << " GlobalsOffset " << endl;
        cout << (header->NativesOffset.Value & 0x0FFFFFFF) << " NativesOffset " << endl;
        cout << (header->StaticsOffset.Value & 0x0FFFFFFF) << " StaticsOffset " << endl;
        cout << (header->CodeBlocksOffset.Value & 0x0FFFFFFF) << " CodeBlocksOffset " << endl;
        cout << (header->StringBlocksOffset.Value & 0x0FFFFFFF) << " StringBlocksOffset " << endl;

        cout << (header->Unk8Ptr.Value & 0x0FFFFFFF) << " unk8ptr " << header->Unk8 << endl;
        cout << (header->Unk9Ptr.Value & 0x0FFFFFFF) << " unk9ptr " << header->Unk9 << endl;
        cout << (header->Unk10Ptr.Value & 0x0FFFFFFF) << " unk10ptr " << header->Unk10 << endl;
    }

    uint64_t* codeBlocksOffset = header->CodeBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.CodeBlockOffsets.resize(CommonHeader.CodeBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        CommonHeader.CodeBlockOffsets[i] = RelPtr64(*(codeBlocksOffset + i)).GetPtr<uint8_t>(CommonHeader.HeaderPtr);
        if (Options::DecompileOptions::Verbose)
            cout << (*(codeBlocksOffset + i) & 0x0FFFFFFF) << " Code Block " << endl;
    }

    uint64_t* stringBlocksOffset = header->StringBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.StringBlockOffsets.resize(CommonHeader.StringBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.StringBlocksCount; i++)
    {
        CommonHeader.StringBlockOffsets[i] = RelPtr64(*(stringBlocksOffset + i)).GetPtr<char>(CommonHeader.HeaderPtr);

        if (Options::DecompileOptions::Verbose)
            cout << (*(stringBlocksOffset + i) & 0x0FFFFFFF) << " String Block " << endl;
    }

    for (int i = 0; i < CommonHeader.NativesCount; i++)
    {
        //RDR2 PC natives arent stored sequentially in the table. 
        //Each native needs a bitwise rotate depending on its position and codetable size
        uint64_t native = *((uint64_t*)CommonHeader.NativesOffset + i);
        *((uint64_t*)CommonHeader.NativesOffset + i) = Utils::Bitwise::rotl(native, (CommonHeader.CodeLength + i) & 0x3F);
    }

}

void DecompileRDR2Console::LogSwitchLabel()
{
    uint16_t caseCount = Reader->ReadUInt16(CurrentReadPos + 1);
    CurrentOpSize = 3 + caseCount * 6;

    for (uint32_t i = 0; i < caseCount; i++)
    {
        uint8_t* index = CurrentReadPos + 3 + i * 6;

        JumpLabelIndexes.insert({ (index + 6) - CodeData.data() + Reader->ReadInt16(index + 4), JumpLabelIndexes.size() });

    }
}
#pragma region Opcodes

void DecompileRDR2Console::ReadSwitch()
{

    uint16_t caseCount = Reader->ReadUInt16(CurrentReadPos + 1);
    CurrentOpSize = 3 + caseCount * 6;

    vector<pair<int32_t, uint32_t>> caseAndIndex(caseCount);

    for (uint32_t i = 0; i < caseCount; i++)
    {
        uint8_t* index = CurrentReadPos + 3 + i * 6;
        caseAndIndex[i] = {
            Reader->ReadInt32(index),//caseNum
            (index + 6) - CodeData.data() + Reader->ReadInt16(index + 4)//labelIndex
        };
    }

    PrintSwitch(caseAndIndex);

}

#pragma endregion


#pragma endregion

#pragma region DecompileRDR2PC

DecompileRDR2PC::DecompileRDR2PC() : DecompileRDR2Console()
{
    Is64Bit = true;
    DecompileTarget = GameTarget::RDR2;
    MaxPageSize = 16384;
    SetEndian(true);
    SetOps(_CommonOpsIndexedByTargetOps);
}

void DecompileRDR2PC::LoadOpcodes()
{
    if (Options::DecompileOptions::OpcodeVersion == 0)
    {
        bool triggered = false;
        uint32_t size = lengthof(_CommonOpsIndexedByTargetOps);
        for (int i = 0; i < size; i++)
        {
            if (triggered || _CommonOpsIndexedByTargetOps[i] == Opcode::GetStaticP3)
            {
                _CommonOpsIndexedByTargetOps[i] = Opcode::Uninitialized;
                triggered = true;
            }
        }

        return;
    }


    string opcodeFile = Utils::IO::GetLastFileWithVersion(GetOptionsDataName("Opcodes"), Options::DecompileOptions::OpcodeVersion);

    if (opcodeFile != "")
    {
        std::unordered_map<uint64_t, uint64_t> map;
        if (Utils::IO::LoadCSVMap("Data/" + opcodeFile, true, 10, map))
        {
            Opcode temp[255];
            memset(temp, (uint8_t)Opcode::Uninitialized, 255 * sizeof(Opcode));

            for (auto& i : map)
            {
                uint64_t newOpcode = i.first;
                uint64_t baseOpcode = i.second;

                if (newOpcode >= 255 || baseOpcode >= 255)
                {
                    Utils::System::Warn("Opcode File " + opcodeFile + " at Opcode " + std::to_string(baseOpcode) + ": NewOpcode or BaseOpcode > 255 on opcode parsing ... skipping further parsing");
                    break;
                }

                temp[newOpcode] = _CommonOpsIndexedByTargetOps[baseOpcode];

            }

            memcpy_s(_CommonOpsIndexedByTargetOps, 255 * sizeof(Opcode), temp, 255 * sizeof(Opcode));
            SetOps(_CommonOpsIndexedByTargetOps);
        }
        else
            Utils::System::Warn("Could not process opcode file " + opcodeFile + " ... using default ops");
    }
    else
        Utils::System::Warn("Opcode file " + opcodeFile + " not found ... using default ops");

}

void DecompileRDR2PC::OpenScript(vector<uint8_t>& data)
{
    LoadOpcodes();

    CurrentReadPos = const_cast<uint8_t*>(data.data());

    if (Utils::Bitwise::SwapEndian(*reinterpret_cast<uint32_t*>(CurrentReadPos)) == 'RSC8')
        CurrentReadPos += 16;

    RDR2Header* header = (RDR2Header*)CurrentReadPos;

    int codeBlocksCount = (header->CodeLength + ((1 << 14) - 1)) >> 14;
    int stringBlocksCount = (header->TotalStringLength + ((1 << 14) - 1)) >> 14;


    CommonHeader.HeaderPtr = (uint8_t*)header;
    CommonHeader.CodeBlocksCount = codeBlocksCount;
    CommonHeader.StringBlocksCount = stringBlocksCount;
    CommonHeader.StaticsCount = header->StaticsCount;
    CommonHeader.NativesCount = header->NativesCount;
    CommonHeader.GlobalsCount = header->GlobalsCount;
    CommonHeader.CodeLength = header->CodeLength;
    CommonHeader.TotalStringLength = header->TotalStringLength;
    CommonHeader.ParameterCount = header->ParameterCount;
    CommonHeader.ScriptName = header->ScriptNameOffset.GetPtr<char>(CommonHeader.HeaderPtr);
    CommonHeader.GlobalsOffset = header->GlobalsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.NativesOffset = header->NativesOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.StaticsOffset = header->StaticsOffset.GetPtr<uint32_t>(CommonHeader.HeaderPtr);
    CommonHeader.Signature = header->Signature;

    if (Options::DecompileOptions::Verbose)
    {


        cout << CommonHeader.ScriptName << " ScriptName" << endl;
        cout << CommonHeader.NativesCount << " NativesCount" << endl;
        cout << CommonHeader.GlobalsCount << " GlobalsCount" << endl;
        cout << CommonHeader.CodeLength << " CodeLength" << endl;
        cout << CommonHeader.TotalStringLength << " TotalStringLength" << endl;

        cout << (header->PageMapOffset.Value & 0x0FFFFFFF) << " PageMapOffset " << endl;
        cout << (header->ScriptNameOffset.Value & 0x0FFFFFFF) << " ScriptNameOffset " << endl;
        cout << (header->GlobalsOffset.Value & 0x0FFFFFFF) << " GlobalsOffset " << endl;
        cout << (header->NativesOffset.Value & 0x0FFFFFFF) << " NativesOffset " << endl;
        cout << (header->StaticsOffset.Value & 0x0FFFFFFF) << " StaticsOffset " << endl;
        cout << (header->CodeBlocksOffset.Value & 0x0FFFFFFF) << " CodeBlocksOffset " << endl;
        cout << (header->StringBlocksOffset.Value & 0x0FFFFFFF) << " StringBlocksOffset " << endl;

        cout << (header->Unk8Ptr.Value & 0x0FFFFFFF) << " unk8ptr " << header->Unk8 << endl;
        cout << (header->Unk9Ptr.Value & 0x0FFFFFFF) << " unk9ptr " << header->Unk9 << endl;
        cout << (header->Unk10Ptr.Value & 0x0FFFFFFF) << " unk10ptr " << header->Unk10 << endl;
    }

    uint64_t* CodeBlocksOffset = header->CodeBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.CodeBlockOffsets.resize(CommonHeader.CodeBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.CodeBlocksCount; i++)
    {
        CommonHeader.CodeBlockOffsets[i] = RelPtr64(*(CodeBlocksOffset + i)).GetPtr<uint8_t>(CommonHeader.HeaderPtr);
        if (Options::DecompileOptions::Verbose)
            cout << (*(CodeBlocksOffset + i) & 0x0FFFFFFF) << " Code Block " << endl;
    }

    uint64_t* StringBlocksOffset = header->StringBlocksOffset.GetPtr<uint64_t>(CommonHeader.HeaderPtr);
    CommonHeader.StringBlockOffsets.resize(CommonHeader.StringBlocksCount);
    for (uint32_t i = 0; i < CommonHeader.StringBlocksCount; i++)
    {
        CommonHeader.StringBlockOffsets[i] = RelPtr64(*(StringBlocksOffset + i)).GetPtr<char>(CommonHeader.HeaderPtr);
        if (Options::DecompileOptions::Verbose)
            cout << (*(StringBlocksOffset + i) & 0x0FFFFFFF) << " String Block " << endl;
    }


    //decryption
    uint8_t carry = CommonHeader.CodeLength;
    for (int i = 0; i < CommonHeader.NativesCount * 8; i++)
    {
        uint8_t* bPtr = (uint8_t*)CommonHeader.NativesOffset + i;
        uint8_t b = *bPtr;

        *bPtr ^= carry;
        carry = b;
    }

    for (int i = 0; i < CommonHeader.NativesCount; i++)
    {
        //RDR2 PC natives arent stored sequentially in the table. 
        //Each native needs a bitwise rotate depending on its position and codetable size
        uint64_t native = *((uint64_t*)CommonHeader.NativesOffset + i);
        *((uint64_t*)CommonHeader.NativesOffset + i) = Utils::Bitwise::rotl(native, (CommonHeader.CodeLength + i) & 0x3F);
    }



}

#pragma endregion
