#pragma once
#include <sstream>
#include <fstream>
#include "Utils/types.h"
#include <unordered_map>
#include <cassert>

struct StringPageBuilderIndex
{
    uint32_t Index = 0;
    uint32_t NullTermCount = 1;
};

class StringPageBuilder
{
    std::string LastAddedString = "";

public:
    std::unordered_map<std::string, StringPageBuilderIndex> Strings;
    uint32_t Size = 0;
    uint32_t BlockSize = 0;
    
    void Reserve(uint32_t s)
    {
        Strings.reserve(s);
    }

    uint32_t GetOrAdd(std::string& str)
    {
        auto res = Strings.find(str);

        if (res == Strings.end())
        {
            uint32_t newSize = (Size + str.size() + 1);
            if (BlockSize != 0 && newSize / BlockSize > Size / BlockSize && (newSize % BlockSize != 0))//page flip
            {
                uint32_t lastNullTermAddCount = BlockSize - Size % BlockSize;

                
                auto res2 = Strings.find(LastAddedString);
                if (res2 != Strings.end())
                {
                    res2->second.NullTermCount += lastNullTermAddCount;
                    Size += lastNullTermAddCount;

                }
                else
                    assert(false);

            }

            uint32_t index = Size;
            Strings.insert({ str , {index, 1} });

            LastAddedString = str;
            Size += str.size() + 1;
            return index;
        }
        else
            return res->second.Index;

    }


    StringPageBuilder(uint32_t blockSize) : BlockSize(blockSize)
    {

    }

};