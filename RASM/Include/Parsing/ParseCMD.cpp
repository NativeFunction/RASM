#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>
#include "Decompiler\Decompiler.h"
#include "Utils\Utils.h"
#include "Compiler/Compiler.h"
#include "Parsing/ParseCMD.h"

typedef std::string::const_iterator iter;
typedef boost::iterator_range<iter> string_view;

using namespace std;
using namespace boost::program_options;



void PrintBadCommandAndExit()
{
    cerr << "Error: No Path Selected.\n"
        << "Reason: Not Enough Args.\n"
        << "Use -h For Help.\n";
    Utils::System::Pause();
    exit(EXIT_FAILURE);
}

void PrintBadPathAndExit(const string& Path)
{
    cerr << "Error: Path \"" << Path << "\" Is Not A Valid Path For Script Input Path.\n"
        << "Use -h For Help.\n";
    Utils::System::Pause();
    exit(EXIT_FAILURE);
}

void CheckRequiredPlatformOptions()
{
    if (Options::GameTarget == GameTarget::UNK)
    {
        cerr << "Error: Target was undefined\n"
            << "Use -h For Help.\n";
        Utils::System::Pause();
        exit(EXIT_FAILURE);
    }
    else if (Options::Platform == Platform::UNK)
    {
        cerr << "Error: Platform was undefined.\n"
            << "Use -h For Help.\n";
        Utils::System::Pause();
        exit(EXIT_FAILURE);
    }

}

int ParseCommandLine(int argc, const char* argv[])
{
    int i = 0;
    while (*argv[i] != '-')
    {
        i++;
        if (i >= argc)
        {
            cout << "Unknown Command. Use -h For Help.\n";
            exit(0);
        }
    }

    for (uint32_t TopLevelOptionCounter = 0; TopLevelOptionCounter < 3; TopLevelOptionCounter++, i++)
    {
        switch (Utils::Hashing::Joaat((argv[i] + 1)))
        {
        case Utils::Hashing::JoaatConst("target"):
        {
            if (i + 1 > argc)
                PrintBadCommandAndExit();

            const char* GameTarget = argv[++i];

            switch (Utils::Hashing::Joaat(GameTarget))
            {
            case Utils::Hashing::JoaatConst("GTAIV"):		Options::GameTarget = GameTarget::GTAIV; break;
            case Utils::Hashing::JoaatConst("RDR"):  		Options::GameTarget = GameTarget::RDR; break;
            case Utils::Hashing::JoaatConst("RDR_SCO"):		Options::GameTarget = GameTarget::RDR_SCO; break;
            case Utils::Hashing::JoaatConst("GTAV"):		Options::GameTarget = GameTarget::GTAV; break;
            case Utils::Hashing::JoaatConst("RDR2"):		Options::GameTarget = GameTarget::RDR2; break;
            }
        }

        break;
        case Utils::Hashing::JoaatConst("platform"):
        {
            if (i + 1 > argc)
                PrintBadCommandAndExit();

            const char* Platform = argv[++i];

            switch (Utils::Hashing::Joaat(Platform))
            {
            case Utils::Hashing::JoaatConst("XBOX"):	Options::Platform = Platform::XBOX; break;
            case Utils::Hashing::JoaatConst("PSX"):		Options::Platform = Platform::PSX; break;
            case Utils::Hashing::JoaatConst("PC"):		Options::Platform = Platform::PC; break;
            }
        }
        break;
        case Utils::Hashing::JoaatConst("dec")://dec (decompile)
        {
            CheckRequiredPlatformOptions();

            //dec requires 2 inputs
            if (++i + 2 > argc)
                PrintBadCommandAndExit();


            string ScriptInputPath = argv[i];

            if (!boost::filesystem::exists(ScriptInputPath))
                PrintBadPathAndExit(ScriptInputPath);

            i++;
            string ScriptOutputPath = argv[i];

            //parse optional dec args
            for (; i < argc; i++)
            {
                if (*argv[i] != '-')
                    continue;
                switch (Utils::Hashing::Joaat((argv[i] + 1)))
                {
                case Utils::Hashing::JoaatConst("v")://-v(Verbose)
                    Options::DecompileOptions::Verbose = true;
                    break;
                case Utils::Hashing::JoaatConst("ow")://-ow(Overwrite) -> AutoOverwrite
                    Options::DecompileOptions::Overwrite = true;
                    break;
                case Utils::Hashing::JoaatConst("dcs")://-dcs decompile strings
                    Options::DecompileOptions::OnlyDecompileStrings = true;
                    break;
                case Utils::Hashing::JoaatConst("dcn")://-dcs decompile nops
                    Options::DecompileOptions::DecompileNops = true;
                    break;
                case Utils::Hashing::JoaatConst("nv")://-nv native version
                {
                    if (i + 1 > argc)
                        PrintBadCommandAndExit();

                    const char* rdr2v = argv[++i];
                    char* pEnd = 0;
                    Options::DecompileOptions::NativeVersion = strtol(rdr2v, &pEnd, 10);

                }
                break;
                case Utils::Hashing::JoaatConst("ov")://-ov opcode version
                {
                    if (i + 1 > argc)
                        PrintBadCommandAndExit();

                    const char* rdr2v = argv[++i];
                    char* pEnd = 0;
                    Options::DecompileOptions::NativeVersion = strtol(rdr2v, &pEnd, 10);

                }
                break;
                }
            }

            vector<uint8_t> Data;
            Utils::IO::LoadData((char*)ScriptInputPath.c_str(), Data);

            unique_ptr<DecompileBase> Decompiler;

            switch (Options::GameTarget)
            {
            case GameTarget::GTAIV: Decompiler = make_unique<DecompileGTAIV>(GameTarget::GTAIV); break;
            case GameTarget::RDR: Decompiler = make_unique<DecompileRDR>(GameTarget::RDR); break;
            case GameTarget::RDR_SCO: Decompiler = make_unique<DecompileRDR>(GameTarget::RDR_SCO); break;
            case GameTarget::GTAV:
            {
                if(Options::Platform == Platform::PC)
                    Decompiler = make_unique<DecompileGTAVPC>();
                else
                    Decompiler = make_unique<DecompileGTAVConsole>();
            }
            break;
            case GameTarget::RDR2: 
                
                if (Options::Platform == Platform::PC)
                    Decompiler = make_unique<DecompileRDR2PC>();
                else
                    Decompiler = make_unique<DecompileRDR2Console>();
                
                break;
            default: Utils::System::Throw("Invalid Target"); break;
            }

            Decompiler->OpenScript(Data);

            if (boost::filesystem::exists(ScriptOutputPath))
            {
                if (!Options::DecompileOptions::Overwrite)
                {
                    cout << "Warning: Path \"" << ScriptOutputPath << "\" For Source Output Path Already Exists.\nDo You Want To Overwrite? Y = Yes, N = No:";
                    char in;
                    cin >> in;
                    if (tolower(in) != 'y')
                    {
                        cerr << "Operation Canceled.\n";
                        exit(0);
                    }
                }
            }

            if (Options::DecompileOptions::OnlyDecompileStrings)
                Decompiler->GetString(ScriptOutputPath);
            else
                Decompiler->GetCode(ScriptOutputPath);

        }
        return EXIT_SUCCESS;
        case Utils::Hashing::JoaatConst("com")://com (compile)
        {
            CheckRequiredPlatformOptions();

            bool AutoOvrC = false, IsXbox = true, IgnoreNops = false, Rsc7 = true, CustomName = false;
            const char* DefaultName = (const char*)"Script";
            char* name = (char*)DefaultName;
            uint32_t Pagebase = 3023717632, ParamCount = 0;
            
            if (++i + 2 > argc)
                PrintBadCommandAndExit();
            else if (!boost::filesystem::exists(argv[i]))
            	PrintBadPathAndExit(argv[i]);
            
            
            std::ifstream ScriptFile(argv[i]);//xsa file
            string content((std::istreambuf_iterator<char>(ScriptFile)), (std::istreambuf_iterator<char>()));
            ScriptFile.close();
            i++;


            string output = string(argv[i]);
            
            for (i; i < argc; i++)
            {
            	if (*argv[i] != '-')
            		continue;
            	switch (Utils::Hashing::Joaat((argv[i] + 1)))
            	{
            		case Utils::Hashing::JoaatConst("spb")://-spb(SetPageBase)
            		{
            			i++;
            			char* endp;
            			uint32_t value = strtoul(argv[i], &endp, 0);
            			if (endp == argv[i])
            			{
            				//failed
            				cout << "Error: Could Not Parse " << argv[i] << " for Set Page Base. \nUse -h For Help.\n";
            				exit(0);
            			}
            			else if (*endp != 0)
            			{
            				//invalid char
            				cout << "Error: Could Not Parse " << argv[i] << " for Set Page Base. \nUse -h For Help.\n";
            				exit(0);
            			}
            			else
            			{
            				Pagebase = value;
            			}
            		}
            		break;
            		case Utils::Hashing::JoaatConst("in")://-in(IgnoreNops)
            		IgnoreNops = true;
            		break;
            		case Utils::Hashing::JoaatConst("ssn")://-ssn(Set Script Name)
            		i++;
            		name = (char*)argv[i];
            		CustomName = true;
            		break;
            		case Utils::Hashing::JoaatConst("norsc7")://-norsc7(no rsc7 header)
            		Rsc7 = false;
            		break;
            		case Utils::Hashing::JoaatConst("ow")://-ow(Overwrite) -> AutoOverwrite
                        Options::CompileOptions::Overwrite = true;
            		break;
            	}
            }
            
            if (boost::filesystem::exists(output))
            {
            	if (!Options::CompileOptions::Overwrite)
            	{
            		cout << "Warning: Path \"" << output << "\" For Script Output Path Already Exists.\nDo You Want To Overwrite? Y = Yes, N = No:";
            		char in;
            		cin >> in;
            		if (tolower(in) != 'y')
            		{
            			cout << "Operation Canceled.\n";
            			exit(0);
            		}
            	}
            }
            else
            {
            	if (output[0] == '-')
            	{
            		cout << "Error: Path \"" << output << "\" Is Not A Valid Path For Script Output Path.\nUse -h For Help.\n";
            		exit(0);
            	}
            }
            
            //
            //if (!CustomName)
            //{
            //	uint32_t inc = 0, start = 0;
            //	while (true)
            //	{
            //		switch (outputc[inc])
            //		{
            //			case '\0':
            //			goto ExitNameParse;
            //			case '\\':
            //			start = inc + 1;
            //			break;
            //			case '.':
            //			uint32_t ssize = inc - start;
            //			name = new char[ssize + 1];
            //			memcpy(name, outputc + start, ssize);
            //			name[ssize] = '\0';
            //			goto ExitNameParse;
            //		}
            //		inc++;
            //	}
            //}
            //ExitNameParse:

            unique_ptr<CompileBase> compiler;

            switch (Options::GameTarget)
            {
            case GameTarget::GTAIV: compiler = make_unique<CompileGTAIV>(content); break;
            //case GameTarget::GTAIV_TLAD: Compiler = make_unique<CompileGTAIV>(GameTarget::GTAIV_TLAD); break;
            //case GameTarget::GTAIV_TBOGT: Compiler = make_unique<CompileGTAIV>(GameTarget::GTAIV_TBOGT); break;
            case GameTarget::RDR: compiler = make_unique<CompileRDR>(content); break;
            case GameTarget::RDR_SCO: compiler = make_unique<CompileRDR>(content); break;
            case GameTarget::GTAV:
            {
                if (Options::Platform == Platform::PC)
                    compiler = make_unique<CompileGTAVPC>(content);
                else
                    compiler = make_unique<CompileGTAVConsole>(content);
            }
            break;
            case GameTarget::RDR2: 
                if (Options::Platform == Platform::PC)
                    compiler = make_unique<CompileRDR2PC>(content);
                else
                    compiler = make_unique<CompileRDR2Console>(content);
                
                break;
            default: Utils::System::Throw("Invalid Target"); break;
            }

            compiler->Compile(output);
            
            //XSC xsc;
            //xsc.CompileXSC(content, Pagebase, name, IgnoreNops, ParamCount);
            //FILE* file;
            //if (IsXbox)
            //	xsc.Write(file, outputc, Constants::Platform::Xbox, PikIO::Endianess::Big, Rsc7);
            //else
            //	xsc.Write(file, outputc, Constants::Platform::PSX, PikIO::Endianess::Big, Rsc7);
            
        }
        return EXIT_SUCCESS;
        case Utils::Hashing::JoaatConst("h")://h (help)
        {
            cout
                << "USAGE: rasm.exe <Required Platform Options> <Operation>\n\n"
                << "Required Platform Options:\n\n"
                << "\t-target = {GTAIV | RDR | RDR_SCO | GTAV | RDR2}\n"
                << "\t-platform = {XBOX | PSX | PC}\n"
                << "Operations:\n\n"
                << "\t-dec = Decompile {Script Input Path} {Source Output Path} {Optional Decompile Flags}\n"
                << "\t-com = Compile {Source Input Path} {Script Output Path} {Optional Compile Flags}\n"
                << "Decompile Flags:\n\n"
                << "\t-v = Show Verbose Information\n"
                << "\t-ow = Automatically Overwrite Source Output Path\n"
                << "\t-dcs = Only Decompile Strings\n"
                << "\t-dcs = Decompile Nops\n"
                << "\t-nv = Specify Version for Natives {%d}\n"
                << "\t-ov = Specify Version for Opcodes {%d}\n"
                << "Compile Flags:\n\n"
                << "\t-ssn = Set Script Name {Script Name}\n"
                << "\t-spb = Set Page Base {uint32 Pagebase}\n"
                << "\t-in = Ignore Nops\n"
                << "\t-norsc = Don't Write The RSC Header\n"
                << "\t-ow = Automatically Overwrite Script Output Path\n"
                ;
        }
        return EXIT_SUCCESS;
        default:
            cerr << "Unknown Command. Use -h For Help.\n";
            return EXIT_FAILURE;

        }
    }
    return EXIT_FAILURE;
}