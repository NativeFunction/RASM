#include "Compiler.h"
#include <sstream>
#include <fstream>
#include "Utils/types.h"
#include <string>

using namespace std;

#pragma region CompileBase

string CompileBase::GetOptionsDataName(string dataName)
{
    return GameTarget::TargetNameGeneric[(int)Options::GameTarget] + dataName + Platform::PlatformNameGeneric[(int)Options::Platform];
}

CompileBase::CompileBase(GameTarget::GameTarget target, std::string& text, size_t maxPageSize, bool isLittleEndian) :

    GameTarget(target),
    MaxPageSize(maxPageSize),
    IsLittleEndian(isLittleEndian),
    Text(text),
    StringBuilder(StringPageBuilder(maxPageSize))
{
    CodeBuilder->Data.reserve(1000000);
    StringBuilder.Reserve(1000);
    Text += "\n";
    size = Text.size();
}

CompileBase::CompileBase(std::string& text, bool isLittleEndian, size_t maxPageSize) :
    Text(text), MaxPageSize(maxPageSize), StringBuilder(StringPageBuilder(maxPageSize))
{
    StringBuilder.Reserve(1000);
    Text += "\n";
    size = Text.size();

    SetEndian(isLittleEndian);

}


int CompileBase::StringToInt(std::string& str)
{
    if (str.size() > 2 && str[0] == '0' && tolower(str[1]) == 'x')
        return Utils::Bitwise::SwapEndian((int32_t)stoll(str, nullptr, 16));//big endian
    else
        return stoll(str, nullptr, 10);
}

Result<string> CompileBase::GetNextToken()
{
    line = lookaheadLine;

    for (; index < size; index++)
    {
        char c = tolower(Text[index]);

        switch (c)
        {
        case '\n':
            isFirstReadOfLine = true;
            lookaheadLine++;
            reading = false;

            Text[index] = 0;
            if (readP != 0)
                readPSize = &Text[index] - readP;
            break;
        case ' ':
        case '\r':
        case ']':
        case '(':
        case ')':
        case '[':
            isFirstReadOfLine = false;
            reading = false;

            Text[index] = 0;
            if (readP != 0)
                readPSize = &Text[index] - readP;
            break;
        case '/'://comment
        {
            if (index + 1 < size && Text[index + 1] == '/')
            {
                reading = false;
                Text[index] = 0;
                if (readP != 0)
                    readPSize = &Text[index] - readP;


                index++;
                for (; index < size && Text[index] != '\n'; index++);

                lookaheadLine++;
                isFirstReadOfLine = true;
                break;
            }
        }
        break;
        //case ':':


        //continue;
        case '"':
            if ((Text[index - 1] == ' ' || Text[index - 1] == '\0') && index + 1 < size)
            {
                line = lookaheadLine;
                if (++index >= size) continue;

                readP = &Text[index];
                char* readPTerm = readP;

                for (; index < size && Text[index] != '"' && Text[index] != '\n'; index++)
                {
                    if (Text[index] == '\\')//special chars
                    {
                        if (++index >= size) continue;

                        switch (Text[index])
                        {
                        case 'r':
                            *readPTerm = '\r';
                            readPTerm++;
                            continue;
                        case 'n':
                            *readPTerm = '\n';
                            readPTerm++;
                            continue;
                        case 't':
                            *readPTerm = '\t';
                            readPTerm++;
                            continue;
                        case 'v':
                            *readPTerm = '\v';
                            readPTerm++;
                            continue;
                        case 'a':
                            *readPTerm = '\a';
                            readPTerm++;
                            continue;
                        case '\\':
                            *readPTerm = '\\';
                            readPTerm++;
                            continue;
                        case 'x':
                        {
                            char hex = 0;
                            char c;

                            if (++index >= size) continue;
                            c = tolower(Text[index]);

                            if (c >= '0' && c <= '9')
                                hex = c - '0';
                            else if (c >= 'a' && c <= 'f')
                                hex = c - 'a' + 10;
                            else
                            {
                                index--;
                                break;
                            }


                            if (++index >= size) continue;
                            c = tolower(Text[index]);

                            if (c >= '0' && c <= '9')
                            {
                                hex <<= 4;
                                hex |= c - '0';
                            }
                            else if (c >= 'a' && c <= 'f')
                            {
                                hex <<= 4;
                                hex |= c - 'a' + 10;
                            }
                            else
                            {
                                index--;
                            }

                            *readPTerm = hex;
                            readPTerm++;
                            continue;

                        }
                        break;
                        case '"':
                            *readPTerm = '"';
                            readPTerm++;
                            continue;
                        default:
                        {
                            //octal
                            if (Text[index] >= '0' && Text[index] <= '7')
                            {
                                unsigned char octal = Text[index] - '0';

                                if (++index >= size) continue;
                                if (Text[index] >= '0' && Text[index] <= '7')
                                {
                                    octal = (octal << 3) | (Text[index] - '0');

                                    if (++index >= size) continue;
                                    if (Text[index] >= '0' && Text[index] <= '7')
                                        octal = (octal << 3) | (Text[index] - '0');
                                    else
                                        index--;
                                }
                                else
                                    index--;

                                *readPTerm = octal;
                                readPTerm++;
                                continue;
                            }
                        }

                        index--;
                        }

                    }
                    else
                    {
                        *readPTerm = Text[index];
                        readPTerm++;
                    }
                }

                if (Text[index] == '\n')
                    Utils::System::Throw("String end qoute not found on line " + to_string(line));

                *readPTerm = 0;
                reading = false;

                readPSize = readPTerm - readP;

                break;
            }
            continue;
        default:
        {
            //opcode


            bool isChar = (c >= 'a' && c <= 'z');
            if ((isChar || (c >= '0' && c <= '9') || c == '-' || c == ':' || c == '@' || c == '_') && readP == 0)
            {
                line = lookaheadLine;
                readP = &Text[index];
                if (!reading)
                    firstChar = c;
            }

            reading = true;

            //only opcodes to uppercase, skip label and calls
            if (isChar && isFirstReadOfLine && firstChar != ':' && firstChar != '@')
            {
                Text[index] = toupper(Text[index]);
            }

            continue;
        }

        }


        if (!reading && readP != 0)
        {
            string token = string(readP, readPSize);

            readP = 0;
            readPSize = 0;
            index++;
            return Result(token);

        }


    }

    return Result<string>();
}

string CompileBase::GetNextTokenInLine()
{
    int currentLine = line;
    Result<string> tok = GetNextToken();

    if (line != currentLine || tok.Res == false)
        Utils::System::Throw("Could not find token on line " + to_string(currentLine));

    return tok.Data;
}

int32_t CompileBase::GetNextTokenAsInt()
{
    string tok = GetNextTokenInLine();
    try
    {
        return StringToInt(tok);
    }
    catch (exception)
    {
    }

    Utils::System::Throw("Could not parse int token " + tok + " on line " + to_string(line));
    return 0;
}

float CompileBase::GetNextTokenAsFloat()
{
    string tok = GetNextTokenInLine();
    try
    {
        return stof(tok);
    }
    catch (exception)
    {
    }

    Utils::System::Throw("Could not parse float token " + tok + " on line " + to_string(line));
    return 0;
}

uint64_t CompileBase::GetNextTokenAsNative()
{
    string tok = GetNextTokenInLine();

    try
    {
        auto foundNative = NativeMap.find(tok);
        uint64_t nativeValue = 0;
        if (foundNative != NativeMap.end())
            nativeValue = foundNative->second;
        else
        {
            if (tok.size() > 3 && tok[0] == '_' && tok[1] == '0' && tolower(tok[2]) == 'x')
            {
                nativeValue = stoull(tok.substr(1, tok.size() - 1), nullptr, 16);
            }
            else
                Utils::System::Throw("Native " + tok + " on line " + to_string(line) + " was not found");

        }

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



        return nativeValue;

    }
    catch (exception)
    {
    }

    Utils::System::Throw("Could not parse native token " + tok + " on line " + to_string(line));
    return 0;
}

void CompileBase::AddLabel(std::string& label)
{
    auto res = LabelToCodePos.find(label);

    if (res != LabelToCodePos.end())
        Utils::System::Throw("Redefinition of label " + label + " on line " + to_string(line));

    LabelToCodePos.insert({ label, CodeBuilder->Data.size() });
}

void CompileBase::FixMissedLabels()
{
    for (auto i : MissedLabels)
    {
        auto res = LabelToCodePos.find(i.first);
        if (res != LabelToCodePos.end())
        {
            for (auto j : i.second)//indexes
            {
                //is relative
                if (j.IsRelative)
                    //codePos relative, codePosToPlace
                    CodeBuilder->SetInt16(res->second - (j.Position + 2), j.Position);
                else
                    //codePos, codePosToPlace
                    CodeBuilder->SetUInt24(res->second, j.Position);

            }
        }
        else
        {
            Utils::System::Throw("Could not find label " + i.first + " on line " + to_string(i.second[0].Line));
        }
    }
}

void CompileBase::AddSingleOp(Opcode op)
{

    auto res = CommonOpsToTargetOps->find(op);

    if (res == CommonOpsToTargetOps->end())
        Utils::System::Throw("Invalid Opcode on line " + to_string(line));

    uint8_t targetOp = res->second;
    CodeBuilder->WriteUInt8(targetOp);
}

void CompileBase::AddPad()
{
    int amount = GetNextTokenAsInt();

    for (int i = 0; i < amount; i++)
    {
        AddSingleOp(Opcode::Nop);
    }
}

void CompileBase::AddPushB2()
{
    FixCodePage(3);
    AddSingleOp(Opcode::PushB2);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());

}

void CompileBase::AddPushB3()
{
    FixCodePage(4);
    AddSingleOp(Opcode::PushB3);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
}

void CompileBase::AddPush(int val)
{
    if (val >= 0)
    {
        if (val <= 7)
        {
            /*Push_0,
            Push_1,
            Push_2,
            Push_3,
            Push_4,
            Push_5,
            Push_6,
            Push_7*/
            AddSingleOp((Opcode)((int)Opcode::Push_0 + val));
        }
        else if (val <= 255)
        {
            FixCodePage(2);
            AddSingleOp(Opcode::PushB);
            CodeBuilder->WriteUInt8(val);
        }
        else if (val <= 0x7FFF)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::PushS);
            CodeBuilder->WriteInt16(val);
        }
        else if (val <= 0xFFFFFF)
        {
            FixCodePage(4);
            AddSingleOp(Opcode::PushI24);
            CodeBuilder->WriteUInt24(val);
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::Push);
            CodeBuilder->WriteInt32(val);
        }
    }
    else//neg
    {
        if (val == -1)
        {
            AddSingleOp(Opcode::Push_Neg1);
        }
        else if (val >= -32768)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::PushS);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::Push);
            CodeBuilder->WriteInt32(val);
        }
    }


}

void CompileBase::AddPush()
{
    AddPush(GetNextTokenAsInt());
}

void CompileBase::AddPushF(float val)
{
    if (val >= 0)
    {
        //is float a int
        if (val - (int)val == 0 && val <= 7)
        {
            /*
            PushF_0,
            PushF_1,
            PushF_2,
            PushF_3,
            PushF_4,
            PushF_5,
            PushF_6,
            PushF_7*/
            AddSingleOp((Opcode)((int)Opcode::PushF_0 + val));
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::PushF);
            CodeBuilder->WriteFloat(val);
        }
    }
    else//neg
    {
        if (val == -1)
        {
            AddSingleOp(Opcode::PushF_Neg1);
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::PushF);
            CodeBuilder->WriteFloat(val);
        }
    }
}

void CompileBase::AddPushF()
{
    AddPushF(GetNextTokenAsFloat());
}


void CompileBase::AddFunction()
{
    int paramCount = GetNextTokenAsInt();
    int varCount = GetNextTokenAsInt();

    FixCodePage(5);
    AddSingleOp(Opcode::Function);
    CodeBuilder->WriteUInt8(paramCount);
    CodeBuilder->WriteUInt16(varCount);
    CodeBuilder->WriteUInt8(0);//function name length
}

void CompileBase::AddReturn()
{
    int paramCount = GetNextTokenAsInt();
    int varCount = GetNextTokenAsInt();

    FixCodePage(3);
    AddSingleOp(Opcode::Return);
    CodeBuilder->WriteUInt8(paramCount);
    CodeBuilder->WriteUInt8(varCount);
}

void CompileBase::AddAddImm()
{
    int val = GetNextTokenAsInt();

    if (val >= 0)
    {
        if (val <= 255)
        {
            FixCodePage(2);
            AddSingleOp(Opcode::AddImm1);
            CodeBuilder->WriteUInt8(val);
        }
        else if (val <= 0x7FFF)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::AddImm2);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            AddPush(val);
            AddSingleOp(Opcode::Add);
        }
    }
    else//neg
    {
        if (val >= -32768)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::AddImm2);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            AddPush(val);
            AddSingleOp(Opcode::Add);
        }
    }



}

void CompileBase::AddMultImm()
{
    int val = GetNextTokenAsInt();

    if (val >= 0)
    {
        if (val <= 255)
        {
            FixCodePage(2);
            AddSingleOp(Opcode::MultImm1);
            CodeBuilder->WriteUInt8(val);
        }
        else if (val <= 0x7FFF)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::MultImm2);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            AddPush(val);
            AddSingleOp(Opcode::Mult);
        }
    }
    else//neg
    {
        if (val >= -32768)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::MultImm2);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            AddPush(val);
            AddSingleOp(Opcode::Mult);
        }
    }
}

void CompileBase::AddJump(Opcode jumpOp)
{
    string label = GetNextTokenInLine();

    if (jumpOp == Opcode::JumpTrue)
    {
        AddSingleOp(Opcode::Push_0);//single ops dont need page fixed
        FixCodePage(3);
        AddSingleOp(Opcode::JumpNE);
    }
    else
    {
        FixCodePage(3);
        AddSingleOp(jumpOp);
    }

    AddJumpPos(label);

}

void CompileBase::AddJumpPos(std::string& label)
{
    if (label[0] == '@')
    {
        label[0] = ':';
        auto res = LabelToCodePos.find(label);

        if (res != LabelToCodePos.end())
        {
            //backwards jump
            int offset = res->second - (CodeBuilder->Data.size() + 2);

            if (offset >= -32768)
                CodeBuilder->WriteInt16(offset);
            else
                Utils::System::Throw("Unsupported data size of " + to_string(offset) + " for op on line " + to_string(line));
        }
        else
        {
            MissedLabels[label].push_back({ (uint32_t)CodeBuilder->Data.size(), true, line });
            CodeBuilder->WriteInt16(0);
        }
    }
}

void CompileBase::AddCall()
{
    string label = GetNextTokenInLine();
    if (label[0] == '@')
    {
        label[0] = ':';
        auto res = LabelToCodePos.find(label);

        FixCodePage(4);

        if (res != LabelToCodePos.end())
        {

            AddSingleOp(Opcode::Call);

            if (res->second <= 0xFFFFFF)
                CodeBuilder->WriteUInt24(res->second);
            else
                Utils::System::Throw("Unsupported data size for op on line " + to_string(line));
        }
        else
        {
            AddSingleOp(Opcode::Call);
            MissedLabels[label].push_back({ (uint32_t)CodeBuilder->Data.size(), false, line });
            CodeBuilder->WriteUInt24(0);
        }
    }
}

//override rdr2
void CompileBase::AddSwitch()
{
    //CurrentOpSize = 2 + caseCount * 6;
    ParseSwitch(
        [=](const vector<pair<int, string>>& casesAndLabels)
        {
            if (casesAndLabels.size() == 0 || casesAndLabels.size() > 0xFF)
                Utils::System::Throw("Unsupported case count for switch on line " + to_string(line - 1));


            FixCodePage(2 + casesAndLabels.size() * 6);

            AddSingleOp(Opcode::Switch);
            CodeBuilder->WriteUInt8(casesAndLabels.size());//caseCount


            for (auto i : casesAndLabels)
            {
                CodeBuilder->WriteInt32(i.first);
                AddJumpPos(i.second);
            }
        }
    );
}

void CompileBase::ParseNextLine(std::string& tok)
{
    if (tok != "")
    {
        if (tok[0] == ':')
            AddLabel(tok);
        else
        {

            auto res = OpcodeNamesUpper.find(tok);
            if (res != OpcodeNamesUpper.end())
            {
                ParseOpcode(res->second);
            }
            else
                Utils::System::Throw("Invalid Opcode " + tok + " on line " + to_string(line));
        }
    }
}

//override gtaiv and rdr

void CompileBase::AddPushString()
{
    string str = GetNextTokenInLine();

    AddPush(StringBuilder.GetOrAdd(str));
    AddSingleOp(Opcode::PushStringS);
}

void CompileBase::AddPushArray()
{
    Utils::System::Throw("The PushArray opcode is not available on this target");
}

void CompileBase::AddStrCopy()
{
    FixCodePage(2);
    AddSingleOp(Opcode::StrCopy);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
}

void CompileBase::AddItoS()
{
    FixCodePage(2);
    AddSingleOp(Opcode::ItoS);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
}

void CompileBase::AddStrAdd()
{
    FixCodePage(2);
    AddSingleOp(Opcode::StrAdd);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
}

void CompileBase::AddStrAddi()
{
    FixCodePage(2);
    AddSingleOp(Opcode::StrAddi);
    CodeBuilder->WriteUInt8(GetNextTokenAsInt());
}

void CompileBase::AddSetStaticsCount()
{
    Statics.resize(GetNextTokenAsInt());
}

void CompileBase::AddSetDefaultStatic()
{
    int index = GetNextTokenAsInt();

    if (index >= 0 && index <= Statics.size())
        Statics[index] = GetNextTokenAsInt();
    else
        Utils::System::Throw("Static index out of bounds on line " + to_string(line));

}

void CompileBase::AddSetStaticName()
{}

void CompileBase::AddSetLocalName()
{}

void CompileBase::AddSetEnum()
{}

inline void CompileBase::AddSetParamCount()
{
    ParameterCount = GetNextTokenAsInt();
}

inline void CompileBase::AddSetSignature()
{
    SignatureType = (Signature)GetNextTokenAsInt();
}

void CompileBase::AddVarOp(Opcode byteOp, Opcode shortOp, Opcode int24Op)
{
    int val = GetNextTokenAsInt();

    if (val >= 0)
    {
        if (byteOp != Opcode::Uninitialized && val <= 255)
        {
            FixCodePage(2);
            AddSingleOp(byteOp);
            CodeBuilder->WriteUInt8(val);
            return;
        }

        if (shortOp != Opcode::Uninitialized && val <= 0xFFFF)
        {
            FixCodePage(3);
            AddSingleOp(shortOp);
            CodeBuilder->WriteUInt16(val);
            return;
        }

        if (int24Op != Opcode::Uninitialized && val <= 0xFFFFFF)
        {
            FixCodePage(4);
            AddSingleOp(int24Op);
            CodeBuilder->WriteUInt24(val);
            return;
        }
    }
    Utils::System::Throw("Unsupported data size for op on line " + to_string(line));
}

void CompileBase::ParseOpcode(Opcode op)
{

    switch (op)
    {
    case Opcode::Nop:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mult:
    case Opcode::Div:
    case Opcode::Mod:
    case Opcode::Not:
    case Opcode::Neg:
    case Opcode::CmpEQ:
    case Opcode::CmpNE:
    case Opcode::CmpGT:
    case Opcode::CmpGE:
    case Opcode::CmpLT:
    case Opcode::CmpLE:
    case Opcode::fAdd:
    case Opcode::fSub:
    case Opcode::fMult:
    case Opcode::fDiv:
    case Opcode::fMod:
    case Opcode::fNeg:
    case Opcode::fCmpEQ:
    case Opcode::fCmpNE:
    case Opcode::fCmpGT:
    case Opcode::fCmpGE:
    case Opcode::fCmpLT:
    case Opcode::fCmpLE:
    case Opcode::vAdd:
    case Opcode::vSub:
    case Opcode::vMult:
    case Opcode::vDiv:
    case Opcode::vNeg:
    case Opcode::And:
    case Opcode::Or:
    case Opcode::Xor:
    case Opcode::ItoF:
    case Opcode::FtoI:
    case Opcode::FtoV:
    case Opcode::pGet:
    case Opcode::pSet:
    case Opcode::pPeekSet:
    case Opcode::ToStack:
    case Opcode::FromStack:
    case Opcode::GetArrayPs:
    case Opcode::GetLocalPs:
    case Opcode::GetStaticPs:
    case Opcode::GetImmPs:
    case Opcode::PushStringS:
    case Opcode::Dup:
    case Opcode::Drop:
    case Opcode::MemCopy:
    case Opcode::Catch:
    case Opcode::Throw:
    case Opcode::pCall:
    case Opcode::GetXProtect:
    case Opcode::SetXProtect:
    case Opcode::RefXProtect:
    case Opcode::Exit:
    case Opcode::BitTest:
    case Opcode::GetLocalS:
    case Opcode::SetLocalS:
    case Opcode::SetLocalSR:
    case Opcode::GetStaticS:
    case Opcode::SetStaticS:
    case Opcode::SetStaticSR:
    case Opcode::pGetS:
    case Opcode::pSetS:
    case Opcode::pSetSR:
    case Opcode::GetGlobalS:
    case Opcode::SetGlobalS:
    case Opcode::SetGlobalSR:
    case Opcode::GetGlobalPs:
    case Opcode::GetHash:
        AddSingleOp(op);
        break;

    case Opcode::Pad:
        AddPad();
        break;

    case Opcode::PushB2:        AddPushB2();            break;
    case Opcode::PushB3:        AddPushB3();            break;
    case Opcode::Push:          AddPush();              break;
    case Opcode::PushF:         AddPushF();             break;
    case Opcode::CallNative:    AddCallNative();        break;
    case Opcode::Function:      AddFunction();          break;
    case Opcode::Return:        AddReturn();            break;
    case Opcode::AddImm1:
    case Opcode::AddImm2:
        AddAddImm();            break;
    case Opcode::MultImm1:
    case Opcode::MultImm2:
        AddMultImm();           break;
    case Opcode::Jump:
    case Opcode::JumpFalse:
    case Opcode::JumpTrue:
    case Opcode::JumpNE:
    case Opcode::JumpEQ:
    case Opcode::JumpLE:
    case Opcode::JumpLT:
    case Opcode::JumpGE:
    case Opcode::JumpGT:
        AddJump(op);
        break;
    case Opcode::Call:              AddCall();              break;
    case Opcode::Switch:            AddSwitch();            break;
    case Opcode::PushString:        AddPushString();        break;
    case Opcode::PushArray:         AddPushArray();         break;
    case Opcode::StrCopy:           AddStrCopy();           break;
    case Opcode::ItoS:              AddItoS();              break;
    case Opcode::StrAdd:            AddStrAdd();            break;
    case Opcode::StrAddi:           AddStrAddi();           break;

    case Opcode::SetStaticsCount:   AddSetStaticsCount();   break;
    case Opcode::SetDefaultStatic:  AddSetDefaultStatic();  break;
    case Opcode::SetStaticName:     AddSetStaticName();     break;
    case Opcode::SetLocalName:      AddSetLocalName();      break;
    case Opcode::SetEnum:           AddSetEnum();           break;
    case Opcode::SetParamCount:     AddSetParamCount();     break;
    case Opcode::SetSignature:      AddSetSignature();     break;

    case Opcode::GetArrayP1:
    case Opcode::GetArrayP2:
        AddVarOp(Opcode::GetArrayP1, Opcode::GetArrayP2);         break;
    case Opcode::GetArray1:
    case Opcode::GetArray2:
        AddVarOp(Opcode::GetArray1, Opcode::GetArray2);          break;
    case Opcode::SetArray1:
    case Opcode::SetArray2:
        AddVarOp(Opcode::SetArray1, Opcode::SetArray2);          break;
    case Opcode::GetLocalP1:
    case Opcode::GetLocalP2:
        AddVarOp(Opcode::GetLocalP1, Opcode::GetLocalP2);         break;
    case Opcode::GetLocal1:
    case Opcode::GetLocal2:
        AddVarOp(Opcode::GetLocal1, Opcode::GetLocal2);          break;
    case Opcode::SetLocal1:
    case Opcode::SetLocal2:
        AddVarOp(Opcode::SetLocal1, Opcode::SetLocal2);          break;
    case Opcode::GetStaticP1:
    case Opcode::GetStaticP2:
    case Opcode::GetStaticP3://rdr2   
        AddVarOp(Opcode::GetStaticP1, Opcode::GetStaticP2, Opcode::GetStaticP3);        break;
    case Opcode::GetStatic1:
    case Opcode::GetStatic2:
    case Opcode::GetStatic3://rdr2    
        AddVarOp(Opcode::GetStatic1, Opcode::GetStatic2, Opcode::GetStatic3);         break;
    case Opcode::SetStatic1:
    case Opcode::SetStatic2:
    case Opcode::SetStatic3://rdr2    
        AddVarOp(Opcode::SetStatic1, Opcode::SetStatic2, Opcode::SetStatic3);         break;
    case Opcode::GetImmP1:
    case Opcode::GetImmP2:
        AddVarOp(Opcode::GetImmP1, Opcode::GetImmP2);          break;
    case Opcode::GetImm1:
    case Opcode::GetImm2:
        AddVarOp(Opcode::GetImm1, Opcode::GetImm2);          break;
    case Opcode::SetImm1:
    case Opcode::SetImm2:
        AddVarOp(Opcode::SetImm1, Opcode::SetImm2);          break;
    case Opcode::GetGlobalP2:
    case Opcode::GetGlobalP3:
        AddVarOp(Opcode::Uninitialized, Opcode::GetGlobalP2, Opcode::GetGlobalP3);        break;
    case Opcode::GetGlobal2:
    case Opcode::GetGlobal3:
        AddVarOp(Opcode::Uninitialized, Opcode::GetGlobal2, Opcode::GetGlobal3);         break;
    case Opcode::SetGlobal2:
    case Opcode::SetGlobal3:
        AddVarOp(Opcode::Uninitialized, Opcode::SetGlobal2, Opcode::SetGlobal3);         break;
    default:
        break;

    }

}

void CompileBase::Compile(const string& _scriptOutPath)
{
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
        if (!Utils::IO::LoadCSVMap("Data/" + nativeTransFile, true, 16, NativeTranslationMap, true))
            Utils::System::Warn("Could not process natives translation file " + nativeTransFile + " ... using default natives");

        cout << "Natives Translation file " << nativeFile << " applied" << endl;
    }

    for (auto tok = GetNextToken(); tok.Res != false; tok = GetNextToken())
    {
        ParseNextLine(tok.Data);
    }

    FixMissedLabels();


    WriteScript(_scriptOutPath);

}


#pragma endregion

#pragma region CompileGTAIV
void CompileGTAIV::FixMissedLabels()
{
    for (auto i : MissedLabels)
    {
        auto res = LabelToCodePos.find(i.first);
        if (res != LabelToCodePos.end())
        {
            for (auto j : i.second)//indexes
            {
                //codePos, codePosToPlace
                CodeBuilder->SetUInt32(res->second, j.Position);

            }
        }
        else
        {
            Utils::System::Throw("Could not find label " + i.first + " on line " + to_string(i.second[0].Line));
        }
    }
}

void CompileGTAIV::AddCallNative()
{
    uint64_t nativeValue = GetNextTokenAsNative();
    int paramCount = GetNextTokenAsInt();
    int returnCount = GetNextTokenAsInt();

    if (paramCount > 0xFF || paramCount < 0)
        Utils::System::Throw("Invalid param count for native on line " + to_string(line));
    if (returnCount > 0xFF || returnCount < 0)
        Utils::System::Throw("Invalid return count for native on line " + to_string(line));

    FixCodePage(7);
    AddSingleOp(Opcode::CallNative);
    CodeBuilder->WriteUInt8(paramCount);
    CodeBuilder->WriteUInt8(returnCount);

    if (nativeValue > 0xFFFFFFFF)
        Utils::System::Throw("Invalid native value for native on line " + to_string(line));

    CodeBuilder->WriteUInt32(nativeValue);

}

void CompileGTAIV::AddPushString()
{
    std::string str = GetNextTokenInLine();

    if (str.size() + 1 > 0xFF)
    {
        Utils::System::Warn("String size " + to_string(str.size()) + " too large for byte on line " + to_string(line) + " truncating");
        str.resize(0xFF - 1);
    }


    if (str.size() == 0)
    {
        AddSingleOp(Opcode::PushStringNull);
    }
    else
    {
        AddSingleOp(Opcode::PushString);
        CodeBuilder->WriteUInt8(str.size() + 1);
        CodeBuilder->WriteString(str);
    }
}

void CompileGTAIV::AddJump(Opcode jumpOp)
{
    string label = GetNextTokenInLine();
    FixCodePage(5);
    AddSingleOp(jumpOp);
    AddJumpPos(label);
}

void CompileGTAIV::AddJumpPos(std::string& label)
{
    if (label[0] == '@')
    {
        label[0] = ':';
        auto res = LabelToCodePos.find(label);

        if (res != LabelToCodePos.end())
        {
            CodeBuilder->WriteUInt32(res->second);
        }
        else
        {
            MissedLabels[label].push_back({ (uint32_t)CodeBuilder->Data.size(), false, line });
            CodeBuilder->WriteUInt32(0);
        }
    }
}

void CompileGTAIV::AddCall()
{
    string label = GetNextTokenInLine();
    if (label[0] == '@')
    {
        label[0] = ':';
        auto res = LabelToCodePos.find(label);

        FixCodePage(5);

        if (res != LabelToCodePos.end())
        {

            AddSingleOp(Opcode::Call);

            CodeBuilder->WriteUInt32(res->second);
        }
        else
        {
            AddSingleOp(Opcode::Call);
            MissedLabels[label].push_back({ (uint32_t)CodeBuilder->Data.size(), false, line });
            CodeBuilder->WriteUInt32(0);
        }
    }
}

void CompileGTAIV::AddFunction()
{
    int paramCount = GetNextTokenAsInt();
    int varCount = GetNextTokenAsInt();

    FixCodePage(4);
    AddSingleOp(Opcode::Function);
    CodeBuilder->WriteUInt8(paramCount);
    CodeBuilder->WriteUInt16(varCount);
}

void CompileGTAIV::AddSwitch()
{
    //CurrentOpSize = 2 + caseCount * 8;

    ParseSwitch(
        [=](const vector<pair<int, string>>& casesAndLabels)
        {
            if (casesAndLabels.size() == 0 || casesAndLabels.size() > 0xFF)
                Utils::System::Throw("Unsupported case count for switch on line " + to_string(line - 1));

            FixCodePage(2 + casesAndLabels.size() * 8);

            AddSingleOp(Opcode::Switch);
            CodeBuilder->WriteUInt8(casesAndLabels.size());//caseCount

            for (auto i : casesAndLabels)
            {
                CodeBuilder->WriteInt32(i.first);
                AddJumpPos(i.second);
            }
        }
    );

}

void CompileGTAIV::AddPush(int val)
{
    if (val >= -16)
    {
        if (val <= 156)
        {
            auto res = CommonOpsToTargetOps->find(Opcode::Push_0);

            if (res == CommonOpsToTargetOps->end())
                Utils::System::Throw("Invalid Opcode on line " + to_string(line));

            uint8_t targetOp = res->second;
            CodeBuilder->WriteUInt8(targetOp + val);
        }
        else if (val <= 0x7FFF)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::PushS);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::Push);
            CodeBuilder->WriteInt32(val);
        }
    }
    else//neg
    {
        if (val >= -32768)
        {
            FixCodePage(3);
            AddSingleOp(Opcode::PushS);
            CodeBuilder->WriteInt16(val);
        }
        else
        {
            FixCodePage(5);
            AddSingleOp(Opcode::Push);
            CodeBuilder->WriteInt32(val);
        }
    }
}

void CompileGTAIV::AddPushF(float val)
{
    FixCodePage(5);
    AddSingleOp(Opcode::PushF);
    CodeBuilder->WriteFloat(val);
}

void CompileGTAIV::AddVarOp(Opcode byteOp, Opcode shortOp, Opcode int24Op)
{
    int val = GetNextTokenAsInt();

    if (val < 0)
        Utils::System::Throw("Var op under 0 on line " + to_string(line));

    switch (byteOp)
    {
    case Opcode::GetLocalP1:
        AddGetLocalP(val);
        return;
    case Opcode::GetLocal1:
        AddGetLocalP(val);
        AddSingleOp(Opcode::pGet);
        return;
    case Opcode::SetLocal1:
        AddGetLocalP(val);
        AddSingleOp(Opcode::pSet);
        return;
    case Opcode::GetStaticP1:
        AddPush(val);
        AddSingleOp(Opcode::GetStaticPs);
        return;
    case Opcode::GetStatic1:
        AddPush(val);
        AddSingleOp(Opcode::GetStaticPs);
        AddSingleOp(Opcode::pGet);
        return;
    case Opcode::SetStatic1:
        AddPush(val);
        AddSingleOp(Opcode::GetStaticPs);
        AddSingleOp(Opcode::pSet);
        return;

    }

    switch (shortOp)
    {

    case Opcode::GetGlobalP2:
        AddPush(val);
        AddSingleOp(Opcode::GetGlobalPs);
        return;
    case Opcode::GetGlobal2:
        AddPush(val);
        AddSingleOp(Opcode::GetGlobalPs);
        AddSingleOp(Opcode::pGet);
        return;
    case Opcode::SetGlobal2:
        AddPush(val);
        AddSingleOp(Opcode::GetGlobalPs);
        AddSingleOp(Opcode::pSet);
        return;
    }

    Utils::System::Throw("Unsupported var op on line " + to_string(line));

}

void CompileGTAIV::WriteScript(const std::string& scriptOutPath)
{
    DataBuilderLit script;
    script.BlockSize = 16384;

    script.Data.reserve(CodeBuilder->Data.size() + Statics.size() * sizeof(uint32_t));


    uint32_t codeOffset = script.Data.size();
    script.WriteData(CodeBuilder->Data.data(), CodeBuilder->Data.size());


    uint32_t staticsOffset = script.Data.size();
    for (auto i : Statics)
    {
        script.WriteInt32(i);
    }

    GTAIVHeader header;

    header.FormatType = (SCRFlag)Utils::Bitwise::SwapEndian((uint32_t)SCRFlag::CompressedEncrypted);
    header.CodeLength = CodeBuilder->Data.size();
    header.StaticsCount = Statics.size();
    header.GlobalsCount = 0;
    header.ScriptFlags = 0;
    header.Signature = SignatureType;


    vector<uint8_t> compressedData(script.Data.size());
    Utils::Compression::ZLIB_CompressNew(script.Data, compressedData);
    script.Data = compressedData;
    
    if (!Utils::Crypt::AES_Encrypt(script.Data.data(), script.Data.size(), GTAIVKey))
        Utils::System::Throw("Encryption Failed");

    uint32_t compressedSize = compressedData.size();

    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(scriptOutPath.c_str(), file))
    {
        fwrite(&header, 1, sizeof(header), file);
        fwrite(&compressedSize, 1, 4, file);
        fwrite(script.Data.data(), 1, script.Data.size(), file);
        fclose(file);
    }

}


#pragma endregion

#pragma region CompileRDR
vector<uint32_t> CompileRDR::GetPageSizes(uint32_t& size)
{
    int rem = size % 4096;
    if (rem != 0)
        size += 4096 - rem;

    std::vector<uint32_t> RetData;

    uint32_t dwPageCounts[4] = { 0, 0, 0, 0x7FFFFFFF };

    uint32_t dwPageSize = 0x80000; // largest possible page size
    uint32_t dwTotalSize = 0;
    uint32_t dwLeft = size;
    for (uint32_t i = 0; i < 4; i++)
    {
        for (uint32_t j = 0; j < dwPageCounts[i] && dwLeft; j++)
        {
            while (dwPageSize > dwLeft)
                dwPageSize >>= 1;

            assert(dwPageSize && "PageSize = 0");

            dwTotalSize += dwPageSize;
            dwLeft -= dwPageSize;

            RetData.push_back(dwPageSize);
        }
        dwPageSize >>= 1;
    }
    return RetData;
}

uint32_t CompileRDR::GetHeaderPagePos(const vector<uint32_t>& DataSizePages)
{
    size_t SmallestSize = DataSizePages[DataSizePages.size() - 1];
    for (uint32_t i = 0, total = 0; i < DataSizePages.size(); total += DataSizePages[i], i++)
    {
        if (DataSizePages[i] == SmallestSize)
            return total;
    }
    assert(false && "HeaderStartIndex == NULL");
    return -1;
}

void CompileRDR::FixMissedLabels()
{
    for (auto i : MissedLabels)
    {
        auto res = LabelToCodePos.find(i.first);
        if (res != LabelToCodePos.end())
        {

            for (auto j : i.second)//indexes
            {
                //is relative
                if (j.IsRelative)
                    //codePos relative, codePosToPlace
                    CodeBuilder->SetInt16(res->second - (j.Position + 2), j.Position);
                else
                    //codePos, codePosToPlace
                {
                    int callIndex = (res->second & 0x000F0000) >> 16;

                    if (res->second <= 0x0FFFFF)
                    {

                        auto op = CommonOpsToTargetOps->find((Opcode)((int)Opcode::Call2 + callIndex));

                        if (op == CommonOpsToTargetOps->end())
                            Utils::System::Throw("Invalid Opcode on line " + to_string(line));

                        uint8_t targetOp = op->second;
                        CodeBuilder->SetUInt8(targetOp, j.Position - 1);

                    }
                    else
                        Utils::System::Throw("Unsupported data size for op on line " + to_string(line));

                    CodeBuilder->SetUInt16(res->second, j.Position);
                }

            }
        }
        else
        {
            Utils::System::Throw("Could not find label " + i.first + " on line " + to_string(i.second[0].Line));
        }
    }

}

void CompileRDR::AddCallNative()
{
    uint64_t nativeValue = GetNextTokenAsNative();
    int paramCount = GetNextTokenAsInt();//5bits
    int returnCount = GetNextTokenAsInt();//1bit


    if (paramCount > 31 || paramCount < 0)
        Utils::System::Throw("Invalid param count for native on line " + to_string(line));
    if (returnCount > 1 || returnCount < 0)
        Utils::System::Throw("Invalid return count for native on line " + to_string(line));

    FixCodePage(3);
    AddSingleOp(Opcode::CallNative);


    auto res = NativeIndexes.find(nativeValue);
    if (res != NativeIndexes.end())
    {
        if (res->second >= 1024)
            Utils::System::Throw("Invalid native index for native on line " + to_string(line));


        CodeBuilder->WriteUInt16(CreateRDRCallNative(res->second, paramCount, returnCount));

    }
    else
    {
        uint16_t nativeIndex = NativeIndexes.size();

        if (nativeIndex >= 1024)
            Utils::System::Throw("Native index too large on line " + to_string(line));

        NativeIndexes.insert({ nativeValue, nativeIndex });
        CodeBuilder->WriteUInt16(CreateRDRCallNative(nativeIndex, paramCount, returnCount));

    }
}

void CompileRDR::AddPushString()
{
    std::string str = GetNextTokenInLine();

    if (str.size() + 1 > 0xFF)
    {
        Utils::System::Warn("String size " + to_string(str.size()) + " too large for byte on line " + to_string(line) + " truncating");
        str.resize(0xFF - 1);
    }

    AddSingleOp(Opcode::PushString);
    CodeBuilder->WriteUInt8(str.size() + 1);
    CodeBuilder->WriteString(str);

}

void CompileRDR::AddPushArray()
{
    std::string hexData = GetNextTokenInLine();

    if (hexData.size() > 0xFFFFFFFF)
    {
        Utils::System::Warn("Array size " + to_string(hexData.size()) + " too large for uint on line " + to_string(line) + " truncating");
        hexData.resize(0xFFFFFFFF);
    }

    AddSingleOp(Opcode::PushArray);
    CodeBuilder->WriteUInt32(hexData.size());

    auto vec = Utils::DataConversion::HexToData(hexData);
    CodeBuilder->WriteData(vec.data(), hexData.size());
}

void CompileRDR::AddCall()
{
    string label = GetNextTokenInLine();
    if (label[0] == '@')
    {
        label[0] = ':';
        auto res = LabelToCodePos.find(label);

        FixCodePage(3);

        if (res != LabelToCodePos.end())
        {
            int callIndex = (res->second & 0x000F0000) >> 16;

            if (res->second <= 0x0FFFFF)
            {
                AddSingleOp((Opcode)((int)Opcode::Call2 + callIndex));
            }
            else
                Utils::System::Throw("Unsupported data size for op on line " + to_string(line));

            CodeBuilder->WriteUInt16(res->second);
        }
        else
        {
            AddSingleOp(Opcode::Call2);//fix op
            MissedLabels[label].push_back({ (uint32_t)CodeBuilder->Data.size(), false, line });
            CodeBuilder->WriteUInt16(0);
        }
    }
}

void CompileRDR::AddReturn()
{
    int paramCount = GetNextTokenAsInt();
    int varCount = GetNextTokenAsInt();

    if (paramCount <= 3 && varCount <= 3)
    {
        //return conpact
        /*
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
        */
        AddSingleOp((Opcode)((int)Opcode::ReturnP0R0 + paramCount * 4 + varCount));
    }
    else
    {
        FixCodePage(3);
        AddSingleOp(Opcode::Return);
        CodeBuilder->WriteUInt8(paramCount);
        CodeBuilder->WriteUInt8(varCount);
    }
}

void CompileRDR::WriteScript(const std::string& scriptOutPath)
{
    DataBuilderBig script;
    script.BlockSize = 16384;

    script.Data.reserve(sizeof(RDRHeader) + CodeBuilder->Data.size() + Statics.size() * sizeof(uint32_t) + NativeIndexes.size() * sizeof(uint32_t) + script.BlockSize);

    uint32_t codeOffset = script.Data.size();
    script.WriteData(CodeBuilder->Data.data(), CodeBuilder->Data.size());
    script.Pad(16, 0xCD);

    uint32_t nativesOffset = script.Data.size();
    script.PadDirect(NativeIndexes.size() * sizeof(uint32_t));
    for (auto i : NativeIndexes)
    {
        script.SetUInt32((uint32_t)i.first, nativesOffset + i.second * sizeof(uint32_t));
    }
    script.Pad(16, 0xCD);

    uint32_t staticsOffset = script.Data.size();

    for (auto i : Statics)
    {
        script.WriteInt32(i);
    }
    script.Pad(16, 0xCD);

    uint32_t codeBlocksOffset = script.Data.size();

    for (int i = 0; i < CodeBuilder->Data.size(); i += CodeBuilder->BlockSize)
        script.WriteUInt32(RelPtr(codeOffset + i).Value);

    script.Pad(16, 0xCD);

    uint32_t pageMap[8] = { 0, 0, 1, 0, 0, 0, 0, 0 };
    uint32_t pageMapOffset = script.Data.size();
    script.WriteData(pageMap, sizeof(uint32_t) * 8);



    uint32_t totalSize = script.Data.size() + sizeof(RDRHeader);
    script.Data.resize(totalSize, 0xCD);

    vector<uint32_t> DataSizePages = GetPageSizes(totalSize);
    uint32_t HeaderStartIndex = GetHeaderPagePos(DataSizePages);
    if (totalSize != script.Data.size())
        script.Data.resize(totalSize, 0xCD);

    //poor mans header pos
    //should be shrunk
    while (HeaderStartIndex < pageMapOffset)
    {
        totalSize = script.Data.size() + 16384;
        script.Data.resize(totalSize, 0xCD);

        DataSizePages = GetPageSizes(totalSize);
        HeaderStartIndex = GetHeaderPagePos(DataSizePages);
        if (totalSize != script.Data.size())
            script.Data.resize(totalSize, 0xCD);
    }


    script.SetUInt32(0xA8D74300, HeaderStartIndex + offsetof(RDRHeader, PgBase));
    script.SetUInt32((uint32_t)SignatureType, HeaderStartIndex + offsetof(RDRHeader, Signature));
    script.SetUInt32((uint32_t)CodeBuilder->Data.size(), HeaderStartIndex + offsetof(RDRHeader, CodeLength));
    script.SetUInt32(ParameterCount, HeaderStartIndex + offsetof(RDRHeader, ParameterCount));
    script.SetUInt32(Statics.size(), HeaderStartIndex + offsetof(RDRHeader, StaticsCount));
    script.SetUInt32(NativeIndexes.size(), HeaderStartIndex + offsetof(RDRHeader, NativesCount));
    script.SetUInt32(RelPtr(nativesOffset).Value, HeaderStartIndex + offsetof(RDRHeader, NativesOffset));
    script.SetUInt32(RelPtr(staticsOffset).Value, HeaderStartIndex + offsetof(RDRHeader, StaticsOffset));
    script.SetUInt32(RelPtr(codeBlocksOffset).Value, HeaderStartIndex + offsetof(RDRHeader, CodeBlocksOffset));
    script.SetUInt32(RelPtr(pageMapOffset).Value, HeaderStartIndex + offsetof(RDRHeader, PageMapOffset));


    switch (Options::Platform)
    {
    case Platform::XBOX:
    {
        DataBuilderBig compressedData;
        compressedData.Data.resize(totalSize + 8);

        uint64_t compressedSize = 0;

        compressedData.SetUInt32(0x0FF512F1, 0);//LZX Signature?


        Utils::Compression::XCompress_Compress(script.Data.data(), totalSize, compressedData.Data.data() + 8, &compressedSize);

        compressedData.Data.resize(compressedSize + 8);

        compressedData.SetUInt32(compressedSize, 4);

        script.Data = compressedData.Data;

    }
    break;
    case Platform::PSX:
    {
        vector<uint8_t> compressedData(totalSize);
        uint32_t compressedSize = 0;

        Utils::Compression::ZLIB_Compress(CurrentReadPos, totalSize, compressedData.data(), compressedSize);
        compressedData.resize(compressedSize);

        script.Data = compressedData;
    }
    break;
    }


    if (!Utils::Crypt::AES_Encrypt(script.Data.data(), script.Data.size(), RDRKey))
        Utils::System::Throw("Encryption Failed");


    CSRHeader csr =
    {
        Utils::Bitwise::SwapEndian(Options::Platform == Platform::XBOX ? 0x85435352u : 0x86435352u),
        Utils::Bitwise::SwapEndian(0x00000002u)
    };


    csr.Flags.bResource = true;
    csr.Flags.bUseExtSize = true;
    csr.Flags.TotalVSize = totalSize >> 12;//platform dependent? (currently xbox)
    csr.Flags.ObjectStartPage = ObjectStartPageSizeToFlag(DataSizePages[DataSizePages.size() - 1]);

    csr.Flags.Flag[0] = Utils::Bitwise::SwapEndian(csr.Flags.Flag[0]);
    csr.Flags.Flag[1] = Utils::Bitwise::SwapEndian(csr.Flags.Flag[1]);

    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(scriptOutPath.c_str(), file))
    {
        fwrite(&csr, 1, sizeof(csr), file);
        fwrite(script.Data.data(), 1, script.Data.size(), file);
        fclose(file);
    }


}

#pragma endregion

#pragma region CompileGTAVConsole
inline void CompileGTAVConsole::AddCallNative()
{
    uint64_t nativeValue = GetNextTokenAsNative();
    int paramCount = GetNextTokenAsInt();//6bits
    int returnCount = GetNextTokenAsInt();//2bits


    if (paramCount > 0b00111111 || paramCount < 0)
        Utils::System::Throw("Invalid param count for native on line " + to_string(line));
    if (returnCount > 0b00000011 || returnCount < 0)
        Utils::System::Throw("Invalid return count for native on line " + to_string(line));


    FixCodePage(4);
    AddSingleOp(Opcode::CallNative);
    CodeBuilder->WriteUInt8((paramCount << 2) | (returnCount & 3));

    auto res = NativeIndexes.find(nativeValue);
    if (res != NativeIndexes.end())
    {
        //force big endian
        CodeBuilder->WriteUInt16(IsLittleEndian ? Utils::Bitwise::SwapEndian(res->second) : res->second);
    }
    else
    {
        uint16_t nativeIndex = NativeIndexes.size();

        if (nativeIndex > 0xFFFF)
            Utils::System::Throw("Native index too large on line " + to_string(line));

        NativeIndexes.insert({ nativeValue, nativeIndex });
        //force big endian
        CodeBuilder->WriteUInt16(IsLittleEndian ? Utils::Bitwise::SwapEndian(nativeIndex) : nativeIndex);
    }
}

void CompileGTAVConsole::WriteScript(const std::string& scriptOutPath)
{
    DataBuilderBig script;
    script.BlockSize = 16384;

    script.Data.reserve(sizeof(GTAVHeader) + CodeBuilder->Data.size() + StringBuilder.Size + Statics.size() * sizeof(uint32_t) + NativeIndexes.size() * sizeof(uint32_t) + script.BlockSize);

    script.PadDirect(sizeof(GTAVHeader));

    script.SetUInt32(0, offsetof(GTAVHeader, PgBase));
    script.SetUInt32((uint32_t)SignatureType, offsetof(GTAVHeader, Signature));
    script.SetUInt32((uint32_t)CodeBuilder->Data.size(), offsetof(GTAVHeader, CodeLength));
    script.SetUInt32(ParameterCount, offsetof(GTAVHeader, ParameterCount));
    script.SetUInt32(Statics.size(), offsetof(GTAVHeader, StaticsCount));
    script.SetUInt32(NativeIndexes.size(), offsetof(GTAVHeader, NativesCount));
    script.SetUInt32(0, offsetof(GTAVHeader, GlobalsCount));
    script.SetUInt32(RelPtr().Value, offsetof(GTAVHeader, GlobalsOffset));
    script.SetUInt32(0, offsetof(GTAVHeader, Unk3));
    script.SetUInt32(0, offsetof(GTAVHeader, Unk4));
    script.SetUInt32(1, offsetof(GTAVHeader, Unk5));
    script.SetUInt32(StringBuilder.Size, offsetof(GTAVHeader, TotalStringLength));
    script.SetUInt32(0, offsetof(GTAVHeader, Unk6));


    uint32_t codeOffset = script.Data.size();
    script.WriteData(CodeBuilder->Data.data(), CodeBuilder->Data.size());
    script.Pad(16);


    uint32_t stringOffset = script.Data.size();
    script.PadDirect(StringBuilder.Size);

    for (auto i : StringBuilder.Strings)
    {
        script.SetString(i.first, stringOffset + i.second.Index);
    }

    script.Pad(16);


    uint32_t nativesOffset = script.Data.size();
    script.SetUInt32(RelPtr(nativesOffset).Value, offsetof(GTAVHeader, NativesOffset));
    script.PadDirect(NativeIndexes.size() * sizeof(uint32_t));
    for (auto i : NativeIndexes)
    {
        script.SetUInt32((uint32_t)i.first, nativesOffset + i.second * sizeof(uint32_t));
    }
    script.Pad(16);

    script.SetUInt32(RelPtr(script.Data.size()).Value, offsetof(GTAVHeader, StaticsOffset));
    for (auto i : Statics)
    {
        script.WriteInt32(i);
    }
    script.Pad(16);

    script.SetUInt32(RelPtr(script.Data.size()).Value, offsetof(GTAVHeader, ScriptNameOffset));
    const string& scriptName = (filesystem::path(scriptOutPath)).filename().string();
    script.WriteString(scriptName);
    script.SetUInt32(Utils::Hashing::Joaat(scriptName), offsetof(GTAVHeader, NameHash));
    script.Pad(16);


    script.SetUInt32(RelPtr(script.Data.size()).Value, offsetof(GTAVHeader, StringBlocksOffset));
    for (uint32_t i = 0; i < StringBuilder.Size; i += StringBuilder.BlockSize)
        script.WriteUInt32(RelPtr(stringOffset + i).Value);

    script.Pad(16);

    script.SetUInt32(RelPtr(script.Data.size()).Value, offsetof(GTAVHeader, CodeBlocksOffset));
    for (uint32_t i = 0; i < CodeBuilder->Data.size(); i += CodeBuilder->BlockSize)
        script.WriteUInt32(RelPtr(codeOffset + i).Value);

    script.Pad(16);


    uint32_t pageMap[8] = { 0, 0, 1, 0, 0, 0, 0, 0 };
    script.SetUInt32(RelPtr(script.Data.size()).Value, offsetof(GTAVHeader, PageMapOffset));
    script.WriteData(pageMap, sizeof(uint32_t) * 8);



    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(scriptOutPath.c_str(), file))
    {
        fwrite(script.Data.data(), 1, script.Data.size(), file);
        fclose(file);
    }
}

CompileGTAVConsole::CompileGTAVConsole(string& text) : CompileBase(text)
{
    Is64Bit = false;
    GameTarget = GameTarget::GTAV;
    MaxPageSize = 16384;
    CommonOpsToTargetOps = &_CommonOpsToTargetOps;

    SetEndian(false);

    CodeBuilder->BlockSize = 16384;
}


#pragma endregion

#pragma region CompileGTAVPC

CompileGTAVPC::CompileGTAVPC(string& text) : CompileGTAVConsole(text)
{
    Is64Bit = true;
    GameTarget = GameTarget::GTAV;
    MaxPageSize = 16384;
    CommonOpsToTargetOps = &_CommonOpsToTargetOps;

    SetEndian(true);

    CodeBuilder->BlockSize = 16384;

}

void CompileGTAVPC::WriteScript(const std::string& scriptOutPath)
{
    DataBuilderLit script;
    script.BlockSize = 16384;

    GTAVPCHeader header;
    vector<uint64_t> natives(NativeIndexes.size());

    script.Data.reserve(sizeof(header) + CodeBuilder->Data.size() + StringBuilder.Size + Statics.size() * 8 + natives.size() * 8 + script.BlockSize);

    header.PgBase = 0;//0
    header.Signature = SignatureType;//24
    header.CodeLength = CodeBuilder->Data.size();//28
    header.ParameterCount = ParameterCount;//32
    header.StaticsCount = Statics.size();//36
    header.GlobalsCount = 0;//40
    header.NativesCount = natives.size();//44
    header.GlobalsOffset = RelPtr64();//56
    header.Unk3 = 0;//72
    header.Unk4 = 0;//80
    header.Unk5 = 1;//92(typically 1)
    header.TotalStringLength = StringBuilder.Size;//112
    header.Unk6 = 0;//116
    header.Unk7 = 0;//120

    for (auto i : NativeIndexes)
        natives[i.second] = i.first;


    for (int i = 0; i < natives.size(); i++)
        natives[i] = Utils::Bitwise::rotr(natives[i], (header.CodeLength + i) & 0x3F);


    script.WriteData(&header, sizeof(header));

    uint32_t codeOffset = script.Data.size();
    script.WriteData(CodeBuilder->Data.data(), CodeBuilder->Data.size());
    script.Pad(16);


    uint32_t stringOffset = script.Data.size();
    script.PadDirect(StringBuilder.Size);

    for (auto i : StringBuilder.Strings)
    {
        script.SetString(i.first, stringOffset + i.second.Index);
    }

    script.Pad(16);

    header.NativesOffset = RelPtr64(script.Data.size());//64
    script.WriteData(natives.data(), natives.size() * 8);
    script.Pad(16);

    header.StaticsOffset = RelPtr64(script.Data.size());//48
    for (auto i : Statics)
    {
        script.WriteInt32(i);
        script.WriteInt32(0);
    }
    script.Pad(16);

    header.ScriptNameOffset = RelPtr64(script.Data.size());//96
    script.WriteString((filesystem::path(scriptOutPath)).filename().string());
    header.NameHash = Utils::Hashing::Joaat(header.ScriptNameOffset.GetPtr<const char>(script.Data.data()));//88
    script.Pad(16);


    header.StringBlocksOffset = RelPtr64(script.Data.size());//104
    for (uint32_t i = 0; i < StringBuilder.Size; i += StringBuilder.BlockSize)
        script.WriteUInt64(RelPtr64(stringOffset + i).Value);

    script.Pad(16);

    header.CodeBlocksOffset = RelPtr64(script.Data.size());//16
    for (uint32_t i = 0; i < CodeBuilder->Data.size(); i += CodeBuilder->BlockSize)
        script.WriteUInt64(RelPtr64(codeOffset + i).Value);

    script.Pad(16);


    uint32_t pageMap[8] = { 0, 0, 2, 1, 0, 0, 0, 0 };
    header.PageMapOffset = RelPtr64(script.Data.size());//8
    script.WriteData(pageMap, sizeof(uint32_t) * 8);


    //update header
    script.SetData(&header, sizeof(header), 0);

    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(scriptOutPath.c_str(), file))
    {
        fwrite(script.Data.data(), 1, script.Data.size(), file);
        fclose(file);
    }
}

#pragma endregion

#pragma region CompileRDR2Console

CompileRDR2Console::CompileRDR2Console(string& text) : CompileGTAVPC(text)
{
    Is64Bit = true;
    GameTarget = GameTarget::RDR2;
    MaxPageSize = 16384;
    CommonOpsToTargetOps = &_CommonOpsToTargetOps;

    LoadOpcodes();

    SetEndian(true);

    CodeBuilder->BlockSize = 16384;
}

void CompileRDR2Console::AddSwitch()
{
    //CurrentOpSize = 3 + caseCount * 6;
    ParseSwitch(
        [=](const vector<pair<int, string>>& casesAndLabels)
        {
            if (casesAndLabels.size() == 0 || casesAndLabels.size() > 0xFFFF)
                Utils::System::Throw("Unsupported case count for switch on line " + to_string(line - 1));


            FixCodePage(3 + casesAndLabels.size() * 6);

            AddSingleOp(Opcode::Switch);
            CodeBuilder->WriteUInt16(casesAndLabels.size());//caseCount

            for (auto i : casesAndLabels)
            {
                CodeBuilder->WriteInt32(i.first);
                AddJumpPos(i.second);
            }
        }
    );
}

void CompileRDR2Console::LoadOpcodes()
{
    if (Options::DecompileOptions::OpcodeVersion == 0)
    {
        _CommonOpsToTargetOps.erase(Opcode::GetStaticP3);
        _CommonOpsToTargetOps.erase(Opcode::GetStatic3);
        _CommonOpsToTargetOps.erase(Opcode::SetStatic3);
        return;
    }

}

inline void CompileRDR2Console::WriteScript(const std::string& scriptOutPath)
{
    DataBuilderLit script;
    script.BlockSize = 16384;

    RDR2Header header;
    vector<uint64_t> natives(NativeIndexes.size());

    script.Data.reserve(sizeof(header) + CodeBuilder->Data.size() + StringBuilder.Size + Statics.size() * 8 + natives.size() * 8 + script.BlockSize);

    header.PgBase = 0;//0
    header.Signature = SignatureType;//24
    header.CodeLength = CodeBuilder->Data.size();//28
    header.ParameterCount = ParameterCount;//32
    header.StaticsCount = Statics.size();//36
    header.GlobalsCount = 0;//40
    header.NativesCount = natives.size();//44
    header.GlobalsOffset = RelPtr64();//56
    header.Unk3 = 0;//72
    header.Unk4 = 0;//80
    header.Unk5 = 1;//92(typically 1)
    header.TotalStringLength = StringBuilder.Size;//112
    header.Unk6 = 0;//116
    header.Unk7 = 0;//120
    header.Unk8 = 1;//128
    header.Unk9 = 0;//144
    header.Unk10 = 64;//160


    for (auto i : NativeIndexes)
        natives[i.second] = i.first;


    for (int i = 0; i < natives.size(); i++)
        natives[i] = Utils::Bitwise::rotr(natives[i], (header.CodeLength + i) & 0x3F);

    if (Options::Platform == Platform::PC)
    {
        //encryption
        uint8_t carry = header.CodeLength;
        for (uint32_t i = 0; i < header.NativesCount * 8; i++)
        {
            uint8_t* bPtr = ((uint8_t*)natives.data() + i);

            *bPtr ^= carry;
            carry = *bPtr;
        }
    }

    script.WriteData(&header, sizeof(header));

    uint32_t codeOffset = script.Data.size();
    script.WriteData(CodeBuilder->Data.data(), CodeBuilder->Data.size());
    script.Pad(16);


    uint32_t stringOffset = script.Data.size();
    script.PadDirect(StringBuilder.Size);

    for (auto i : StringBuilder.Strings)
    {
        script.SetString(i.first, stringOffset + i.second.Index);
    }

    script.Pad(16);

    header.NativesOffset = RelPtr64(script.Data.size());//64
    script.WriteData(natives.data(), natives.size() * 8);
    script.Pad(16);

    header.StaticsOffset = RelPtr64(script.Data.size());//48
    for (auto i : Statics)
    {
        script.WriteInt32(i);
        script.WriteInt32(0);
    }
    script.Pad(16);

    header.ScriptNameOffset = RelPtr64(script.Data.size());//96
    script.WriteString((filesystem::path(scriptOutPath)).filename().string());
    header.NameHash = Utils::Hashing::Joaat(header.ScriptNameOffset.GetPtr<const char>(script.Data.data()));//88
    script.Pad(16);


    header.StringBlocksOffset = RelPtr64(script.Data.size());//104
    for (uint32_t i = 0; i < StringBuilder.Size; i += StringBuilder.BlockSize)
        script.WriteUInt64(RelPtr64(stringOffset + i).Value);

    script.Pad(16);

    header.CodeBlocksOffset = RelPtr64(script.Data.size());//16
    for (uint32_t i = 0; i < CodeBuilder->Data.size(); i += CodeBuilder->BlockSize)
        script.WriteUInt64(RelPtr64(codeOffset + i).Value);

    script.Pad(16);

    uint8_t unk[16] = { 0 };
    header.Unk8Ptr = RelPtr64(script.Data.size());//136
    script.WriteData(unk, 16);

    header.Unk9Ptr = RelPtr64(script.Data.size());//136
    script.WriteData(unk, 16);

    uint32_t pageMap[8] = { 0, 0, 2, 1, 0, 0, 0, 0 };
    header.PageMapOffset = RelPtr64(script.Data.size());//8
    script.WriteData(pageMap, sizeof(uint32_t) * 8);


    uint8_t unk10[256] = { 0x03,0xF1,0x76,0xDE,0xFC,0xC7,0xE0,0x05,0x7B,0xBC,0x85,0x96,0x00,0x00,0x00,0x00,0x44,0xFA,0x50,0x10,0xBD,0xAB,0xF8,0x44,0x33,0xEF,0xEA,0xA8,0x52,0xD1,0xC0,0xA1,0x6D,0xAF,0xCE,0xCE,0x8D,0xB4,0x85,0xBD,0xD6,0x3B,0x0A,0x9A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    header.Unk10Ptr = RelPtr64(script.Data.size());//168
    script.WriteData(unk10, 256);

    //update header
    script.SetData(&header, sizeof(header), 0);

    FILE* file = nullptr;
    if (Utils::IO::CreateFileWithDir(scriptOutPath.c_str(), file))
    {
        fwrite(script.Data.data(), 1, script.Data.size(), file);
        fclose(file);
    }
}


#pragma endregion

#pragma region CompileRDR2PC

CompileRDR2PC::CompileRDR2PC(string& text) : CompileRDR2Console(text)
{
    Is64Bit = true;
    GameTarget = GameTarget::RDR2;
    MaxPageSize = 16384;
    CommonOpsToTargetOps = &_CommonOpsToTargetOps;

    LoadOpcodes();

    SetEndian(true);

    CodeBuilder->BlockSize = 16384;
}

void CompileRDR2PC::LoadOpcodes()
{
    if (Options::DecompileOptions::OpcodeVersion == 0)
    {
        _CommonOpsToTargetOps.erase(Opcode::GetStaticP3);
        _CommonOpsToTargetOps.erase(Opcode::GetStatic3);
        _CommonOpsToTargetOps.erase(Opcode::SetStatic3);
        return;
    }

    string opcodeFile = Utils::IO::GetLastFileWithVersion(GetOptionsDataName("Opcodes"), Options::DecompileOptions::OpcodeVersion);

    if (opcodeFile != "")
    {
        //base ops, new ops
        std::unordered_map<uint64_t, uint64_t> map;
        if (Utils::IO::LoadCSVMap("Data/" + opcodeFile, true, 10, map, true))
        {
            for (auto& i : _CommonOpsToTargetOps)
            {
                auto res = map.find(i.second);

                if (res != map.end())
                {
                    if (res->second >= 255)
                    {
                        Utils::System::Warn("Opcode File " + opcodeFile + " at new opcode " + std::to_string(res->second) + " > 255 on opcode parsing ... skipping opcode");
                    }
                    else
                        i.second = res->second;
                }
                else
                {
                    Utils::System::Warn("Opcode File" + opcodeFile + " at base opcode " + std::to_string(i.second) + " could not find translation ... skipping opcode");
                }
            }

            CommonOpsToTargetOps = &_CommonOpsToTargetOps;

        }
        else
            Utils::System::Warn("Could not process opcode file " + opcodeFile + " ... using default ops");
    }
    else
        Utils::System::Warn("Opcode file " + opcodeFile + " not found ... using default ops");

}

#pragma endregion

