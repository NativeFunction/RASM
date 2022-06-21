#pragma once
#include "Decompiler\Decompiler.h"
#include "Utils\Utils.h"
#include "Utils\types.h"

#pragma region Options
namespace Options
{
    inline GameTarget::GameTarget GameTarget = GameTarget::UNK;
    inline Platform::Platform Platform = Platform::UNK;
    namespace DecompileOptions
    {
        inline bool Overwrite = false;
        inline bool Verbose = false;
        inline bool OnlyDecompileStrings = false;
        inline bool DecompileNops = false;
        inline int64_t NativeVersion = -1;
        inline int64_t OpcodeVersion = -1;
    }

    namespace CompileOptions
    {
        inline bool Overwrite = false;
    }
}
#pragma endregion

int ParseCommandLine(int argc, const char* argv[]);