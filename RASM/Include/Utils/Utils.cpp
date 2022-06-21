#include <iostream>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <string>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "Utils/Utils.h"
#include <windows.h>
#include "ConsoleColor.h"
#include <filesystem>
#include <zlib.h>
#include "Compression/xcompress.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/tokenizer.hpp>
#include <unordered_map>


using namespace std;
using namespace Utils::System;
using namespace Utils::Bitwise;

namespace Utils
{

    namespace IO
    {
        void LoadData(const char* path, vector<uint8_t>& out)
        {
            ifstream is(path, std::ios::in | std::ios::binary | std::ios::ate);

            if (is.is_open())
            {
                is.seekg(0, is.end);
                assert(is.tellg() <= 0xFFFFFFFF);
                size_t datasize = static_cast<uint32_t>(is.tellg());
                is.seekg(0, is.beg);

                out.resize(datasize, 0);
                is.read(reinterpret_cast<char*>(out.data()), datasize);
                is.close();
            }
            else
                Throw("File " + string(path) + " Could Not Be Opened");

        }

        bool CheckFopenFile(const char* path, FILE* file)
        {
            if (file == NULL)
            {
                Throw("Could Not Open File: " + string(path));
                return false;
            }
            return true;
        }

        bool CreateFileWithDir(const char* filePath, FILE*& file)
        {

            string dir = GetDir(filePath);

            bool status = false;
            try
            {
                status = std::filesystem::exists(std::filesystem::path(dir)) ?
                    true :
                    std::filesystem::create_directories(std::filesystem::path(dir));
            }
            catch (exception)
            {

            }


            file = fopen(filePath, "wb");
            status = file != NULL;

            if (!status)
                Throw("Could Not create File: " + string(filePath));

            return status;
        }

        string GetLastFileContainingString(string str)
        {
            string out = "";
            for (auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
            {
                if (entry.exists())
                {
                    auto path = entry.path();

                    if (path.has_filename())
                    {
                        if (path.filename().string().find(str) != std::string::npos)
                        {
                            out = path.filename().string();
                        }
                    }
                }
            }
            return out;
        }

        string GetLastFileWithVersion(string str, int64_t version)
        {
            string out = "";
            for (auto& entry : std::filesystem::directory_iterator("Data/"))
            {
                if (entry.exists())
                {
                    auto path = entry.path();

                    if (path.has_filename())
                    {
                        if (path.filename().string().find(str) != std::string::npos)
                        {
                            out = path.filename().string();

                            if (out.length() > str.length())
                            {
                                char* data = out.data();

                                char* pEnd = data + str.length();
                                uint64_t fileVersion = strtoll(pEnd, &pEnd, 10);

                                if (version == fileVersion)
                                    return out;
                            }
                            // ==
                            else if (version == 0)
                            {
                                return out;
                            }

                        }
                    }
                }
            }
            return out;
        }

        bool LoadCSVMap(const string& path, bool hasHeader, int keyBase, unordered_map<uint64_t, uint64_t>& map, bool reverse)
        {
            std::ifstream file(path);
            if (file.is_open())
            {
                string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                file.close();
                boost::char_separator<char> sep(",\n");
                boost::tokenizer<boost::char_separator<char>> tokens(content, sep);
                int index = 0;

                try
                {
                    for (boost::tokenizer<boost::char_separator<char>>::iterator tokeni = tokens.begin(); tokeni != tokens.end(); tokeni++)
                    {
                        if (hasHeader)
                        {
                            hasHeader = false;
                            tokeni++;
                            continue;
                        }

                        string keyStr = tokeni.current_token();
                        try
                        {
                            uint64_t token1 = stoull(keyStr, nullptr, keyBase);
                            tokeni++;
                            uint64_t token2 = stoull(tokeni.current_token(), nullptr, keyBase);

                            if (reverse)
                                map.insert({ token2, token1 });
                            else
                                map.insert({ token1, token2 });

                        }
                        catch (exception)
                        {
                            Utils::System::Warn("Could not parse token " + keyStr + " in file " + path);
                        }
                        index++;
                    }
                }
                catch (exception)
                {
                    Utils::System::Warn("Failed parsing opcodes at line " + std::to_string(index) + " in file " + path);
                }


                return true;
            }

            return false;
        }

        bool LoadCSVStringMap(const string& path, bool hasHeader, int keyBase, unordered_map<uint64_t, string>& map)
        {
            std::ifstream file(path);
            if (file.is_open())
            {
                string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                file.close();
                boost::char_separator<char> sep(",\n");
                boost::tokenizer<boost::char_separator<char>> tokens(content, sep);
                for (boost::tokenizer<boost::char_separator<char>>::iterator tokeni = tokens.begin(); tokeni != tokens.end(); tokeni++)
                {
                    if (hasHeader)
                    {
                        hasHeader = false;
                        tokeni++;
                        continue;
                    }

                    string keyStr = tokeni.current_token();
                    try
                    {
                        uint64_t token1 = stoull(keyStr, nullptr, keyBase);
                        tokeni++;
                        map.insert({ token1, tokeni.current_token() });

                    }
                    catch (exception)
                    {
                        Utils::System::Warn("Could not parse token " + keyStr + " in file " + path);
                    }
                }
                return true;
            }

            return false;
        }

        bool LoadCSVStringMap(const string& path, bool hasHeader, int keyBase, unordered_map<string, uint64_t>& map)
        {
            std::ifstream file(path);
            if (file.is_open())
            {
                string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
                file.close();
                boost::char_separator<char> sep(",\n");
                boost::tokenizer<boost::char_separator<char>> tokens(content, sep);
                for (boost::tokenizer<boost::char_separator<char>>::iterator tokeni = tokens.begin(); tokeni != tokens.end(); tokeni++)
                {
                    if (hasHeader)
                    {
                        hasHeader = false;
                        tokeni++;
                        continue;
                    }

                    string keyStr = tokeni.current_token();
                    try
                    {
                        uint64_t token1 = stoull(keyStr, nullptr, keyBase);
                        tokeni++;
                        map.insert({ tokeni.current_token(), token1 });

                    }
                    catch (exception)
                    {
                        Utils::System::Warn("Could not parse token " + keyStr + " in file " + path);
                    }
                }
                return true;
            }

            return false;
        }

    }

    namespace System
    {
        void Test(bool Condition, const std::string& str)
        {
            if (!Condition)
            {
                cout << brightred << "Error: " << white << str << flush;
                assert(false);
                exit(EXIT_FAILURE);
            }
        }

        void Throw(const std::string& str)
        {
            cout << brightred << "Error: " << white << str << "\r\nPress ENTER to exit..." << flush;
            cin.clear();
            cin.ignore(STREAMSIZE_MAX, '\n');
            assert(false);
            exit(EXIT_FAILURE);
        }

        void Warn(const std::string& str)
        {
            cout << brightyellow << "Warning: " << white << str << endl;
        }

        void Pause(const std::string& str)
        {
            if (str != "")
                cout << str << "\n";
            cout << "Press ENTER to continue..." << flush;
            cin.clear();
            cin.ignore(STREAMSIZE_MAX, '\n');
        }
    }

    namespace DataConversion
    {
        vector<NamedUint32> ReorderUint32Vector(vector<uint32_t> vec, vector<const char*> name, bool isSmallToBig)
        {
            vector<NamedUint32> ret;
            ret.reserve(vec.size() + name.size());


            for (uint32_t i = 0; vec.size() != 0; i++)
            {
                uint32_t index = -(vec.begin() - (isSmallToBig ? min_element(vec.begin(), vec.end()) : max_element(vec.begin(), vec.end())));
                if (index > vec.size())
                    Throw("ReorderUint32Vector Out Of Bounds");

                ret.push_back({ name[index],vec[index] });
                vec.erase(vec.begin() + index);
                name.erase(name.begin() + index);

            }

            return ret;
        }

        void ArrayReverse(vector<uint8_t> InArr, vector<uint8_t>& OutArr)
        {
            uint32_t InSize = InArr.size(), InSizeLoop = InSize - 1, i = 0;

            if (OutArr.size() != InSize)
                OutArr.resize(InSize);

            for (; i < InSize; i++, InSizeLoop--)
                OutArr[i] = InArr[InSizeLoop];
        }

        std::string DataToHex(uint8_t* val, size_t length)
        {
            const char* hex_str = "0123456789ABCDEF";

            std::string str(length * 2, '\0');

            for (uint32_t i = 0; i < length; i++)
            {
                str[i * 2 + 0] = hex_str[(val[i] >> 4) & 0x0F];
                str[i * 2 + 1] = hex_str[(val[i]) & 0x0F];
            }
            return str;
        }

        std::vector<uint8_t> HexToData(const std::string& str)
        {
            int i = 0;

            if (str[0] == '0' && tolower(str[0]) == 'x')
                i = 2;


            const char* strPtr = str.data();
            vector<uint8_t> out;
            out.reserve(str.size() / 2 + 1);

            if (str.size() % 2 != 0)
            {
                const char strByte[2] = { str[i], 0 };
                out.push_back(strtoul(strByte, nullptr, 16));
                i++;

            }

            for (; i < str.size(); i += 2)
            {
                if (i + 1 < str.size())
                {
                    const char strByte[3] = { str[i], str[i + 1] , 0 };
                    out.push_back(strtoul(strByte, nullptr, 16));
                }
                else
                {
                    int k = 0x0300;
                }
            }

            return out;
        }

        std::string StringToDataString(char* str)
        {
            std::string out;
            for (int i = 0; str[i] != 0; i++)
            {
                switch (str[i])
                {
                case '\"': out += "\\\""; break;
                case '\r': out += "\\r"; break;
                case '\n': out += "\\n"; break;
                case '\t': out += "\\t"; break;
                case '\v': out += "\\v"; break;
                case '\a': out += "\\a"; break;
                case '\\': out += "\\\\"; break;
                case '\0': out += "\\0"; break;
                default:
                    if (str[i] >= 32 && str[i] <= 126)
                        out.push_back(str[i]);
                    else
                        out += "\\x" + Utils::DataConversion::IntToHex<uint8_t>(str[i]);
                }
            }
            return out;
        }

        std::string StringToDataString(const std::string& str)
        {
            std::string out;
            for (auto i : str)
            {
                switch (i)
                {
                case '\"': out += "\\\""; break;
                case '\r': out += "\\r"; break;
                case '\n': out += "\\n"; break;
                case '\t': out += "\\t"; break;
                case '\v': out += "\\v"; break;
                case '\a': out += "\\a"; break;
                case '\\': out += "\\\\"; break;
                case '\0': out += "\\0"; break;
                default:
                    if (i >= 32 && i <= 126)
                        out.push_back(i);
                    else
                        out += "\\x" + Utils::DataConversion::IntToHex<uint8_t>(i);
                }
            }
            return out;
        }

        bool IsStringDataString(char* str)
        {
            std::string out;
            for (int i = 0; str[i] != 0; i++)
            {
                if (!(str[i] > 32 && str[i] < 127))
                    return true;
            }
            return false;
        }

        bool IsStringDataString(std::string str)
        {
            std::string out;
            for (int i = 0; i < str.length(); i++)
            {
                if (!(str[i] > 32 && str[i] < 127))
                    return true;
            }
            return false;
        }


    }

    namespace Hashing
    {
        uint32_t Joaat(const char* key)
        {
            uint32_t hash, i;
            for (hash = i = 0; key[i]; ++i)
            {
                hash += tolower(key[i]);
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }
            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);
            return hash;
        }

        uint32_t Joaat(const string& key)
        {
            uint32_t hash, i;
            for (hash = i = 0; i < key.size(); ++i)
            {
                hash += tolower(key[i]);
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }
            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);
            return hash;
        }

        uint32_t JoaatCased(const char* key)
        {
            uint32_t hash, i;
            for (hash = i = 0; key[i]; ++i)
            {
                hash += key[i];
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }
            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);
            return hash;
        }

        uint32_t JoaatCased(const string& key)
        {
            uint32_t hash, i;
            for (hash = i = 0; i < key.size(); ++i)
            {
                hash += key[i];
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }
            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);
            return hash;
        }
    }

    namespace Bitwise
    {
        uint32_t Flip2BytesIn4(uint32_t value)
        {
            short* ptr = (short*)&value;
            short ret[2] = { ptr[1], ptr[0] };
            return *(uint32_t*)ret;
        }

        /**
        Copys bits from source num at start 0 into varToSet at index rangeStart until rangeEnd.
        Notes:-----------------------------------------------
        Total range can't be larger then 31 bits
        rangeStart can be from 0 - 31
        rangeEnd can be from 0 - 31
        rangeStart must be less then or equal to rangeEnd
        sign bit on sourceNum cannot be set
        */
        int32_t __fastcall set_bits_in_range(uint32_t* varToSet, uint32_t rangeStart, uint32_t rangeEnd, int32_t sourceNum)
        {
            int32_t result = 0;
            if (sourceNum >= 0 && (int32_t)rangeStart <= (int32_t)rangeEnd && rangeStart <= 31 && rangeEnd <= 31)
            {
                result = (sourceNum << rangeStart) | *varToSet & (uint32_t)~(((1 << (rangeEnd - rangeStart + 1)) - 1) << rangeStart);
                *varToSet = result;
            }
            return result;
        }

        int32_t __fastcall get_bits_in_range(int32_t value, uint32_t rangeStart, uint32_t rangeEnd)
        {
            int32_t result;
            if ((int32_t)rangeStart > (int32_t)rangeEnd || rangeStart > 31 || rangeEnd > 31)
                result = 0;
            else
                result = (value >> rangeStart) & (uint32_t)((1 << (rangeEnd - rangeStart + 1)) - 1);
            return result;
        }

        uint32_t revbitmask(uint32_t index)
        {
            if (!(index % 32))
                return 0xFFFFFFFF;
            return ~(0xFFFFFFFF << index);
        }

        uint32_t bitCountToIntEnd(uint32_t rangeStart, uint32_t count)
        {
            assert(count && "count cannot be 0");
            assert(count < 32 && "count too large");
            uint32_t endIndex = rangeStart + count;
            if (count == 1)
                return rangeStart;
            else if (endIndex >= 32)
                return 31;
            else
                return endIndex - 1;
        }

    }

    namespace Compression
    {

        int32_t XCompress_Decompress(uint8_t* compressedData, uint64_t compressedLen, uint8_t* decompressedData, uint64_t* decompressedLen)
        {

            // Setup our decompression context
            XMEMDECOMPRESSION_CONTEXT DecompressionContext = 0;
            auto hr = XMemCreateDecompressionContext(XMEMCODEC_TYPE::XMEMCODEC_LZX, 0, 0, &DecompressionContext);

            try
            {
                hr = XMemDecompress(DecompressionContext, decompressedData, decompressedLen, compressedData, compressedLen);
            }
            catch (exception ex)
            {
                XMemResetDecompressionContext(DecompressionContext);
                XMemDestroyDecompressionContext(DecompressionContext);
                return -1;
            }


            XMemResetDecompressionContext(DecompressionContext);
            XMemDestroyDecompressionContext(DecompressionContext);

            // Return our hr
            return hr;
        }

        int32_t XCompress_Compress(uint8_t* Data, uint64_t DataLen, uint8_t* CompressedData, uint64_t* OutCompressedLen)
        {
            // Setup our decompression context
            XMEMCOMPRESSION_CONTEXT CompressionContext = 0;

            int32_t hr = XMemCreateCompressionContext(XMEMCODEC_TYPE::XMEMCODEC_LZX, 0, 0, &CompressionContext);
            uint64_t CompressedLen = DataLen;

            try
            {
                hr = XMemCompress(CompressionContext, CompressedData, &CompressedLen, Data, DataLen);
            }
            catch (exception ex)
            {
                XMemResetCompressionContext(CompressionContext);
                XMemDestroyCompressionContext(CompressionContext);
                return -1;
            }

            XMemResetCompressionContext(CompressionContext);
            XMemDestroyCompressionContext(CompressionContext);

            *OutCompressedLen = CompressedLen;

            // Return our hr
            return hr;
        }

        void ZLIB_Decompress(uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t& outSize)
        {
            z_stream infstream;
            infstream.zalloc = Z_NULL;
            infstream.zfree = Z_NULL;
            infstream.opaque = Z_NULL;
            // setup "b" as the input and "c" as the compressed output
            infstream.avail_in = inSize; // size of input
            infstream.next_in = in; // input char array
            infstream.avail_out = outSize; // size of output
            infstream.next_out = out; // output char array

            int32_t res;



            res = inflateInit(&infstream);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB InflateInit Failed");
            }
            res = inflate(&infstream, Z_NO_FLUSH);
            if (!(res == Z_STREAM_END || res == Z_OK))
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB Inflate Failed");
            }
            res = inflateEnd(&infstream);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB InflateEnd Failed");
            }

            outSize = infstream.next_out - out;


        }

        void ZLIB_Compress(uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t& outSize)
        {
            z_stream defstream;
            defstream.zalloc = Z_NULL;
            defstream.zfree = Z_NULL;
            defstream.opaque = Z_NULL;
            defstream.data_type = Z_BINARY;
            // setup "a" as the input and "b" as the compressed output

            defstream.next_in = in; // input char array
            defstream.avail_in = inSize; // size of input
            defstream.next_out = out; // output char array
            defstream.avail_out = outSize; // size of output


            int32_t res = deflateInit(&defstream, Z_BEST_COMPRESSION);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB DeflateInit Failed");
            }

            res = deflate(&defstream, Z_FINISH);
            if (!(res == Z_STREAM_END || res == Z_OK))
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB deflate Failed ");
            }


            res = deflateEnd(&defstream);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB deflateEnd Failed");
            }

            outSize = defstream.next_out - out;
        }

#define CHUNK 16384

        void ZLIB_DecompressNew(uint8_t* in, uint32_t inSize, std::vector<uint8_t>& out)
        {
            int ec;
            unsigned have;
            z_stream strm;
            unsigned char inbuf[CHUNK];
            unsigned char outbuf[CHUNK];
            uint32_t inIndex = 0;
            uint32_t inIndexOld = 0;

            out.clear();

            /* allocate inflate state */
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = 0;
            strm.next_in = Z_NULL;
            ec = inflateInit(&strm);
            if (ec != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                //cout << "Error: " << zError(ec) << '\n';
                Throw("ZLIB inflateInit Failed");
            }

            /* decompress until deflate stream ends or end of file */
            do
            {
                inIndexOld = inIndex;
                inIndex += CHUNK;
                if (inIndex > inSize)
                    inIndex = inSize;

                uint32_t size = inIndex - inIndexOld;
                if (size == 0)
                    break;
                memcpy(inbuf, in + inIndexOld, size);

                strm.avail_in = size;
                strm.next_in = inbuf;

                /* run inflate() on input until output buffer not full */
                do
                {
                    strm.avail_out = CHUNK;
                    strm.next_out = outbuf;
                    ec = inflate(&strm, Z_NO_FLUSH);

                    /* state not clobbered */
                    if (ec == Z_STREAM_ERROR)
                    {
                        cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                        // cout << "Error: " << zError(ec) << '\n';
                        Throw("ZLIB inflate Failed ");
                    }

                    switch (ec)
                    {
                    case Z_NEED_DICT:
                        ec = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&strm);
                        cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                        // cout << "Error: " << zError(ec) << '\n';
                        Throw("ZLIB inflate Failed ");
                    }

                    have = CHUNK - strm.avail_out;
                    out.insert(out.end(), outbuf, outbuf + have);

                } while (strm.avail_out == 0);



                /* done when inflate() says it's done */
            } while (ec != Z_STREAM_END);

            /* clean up and return */
            ec = inflateEnd(&strm);
            if (ec != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                //cout << "Error: " << zError(ec) << '\n';
                Throw("ZLIB InflateEnd Failed");
            }
        }

        void ZLIB_CompressNew(const std::vector<uint8_t>& in, std::vector<uint8_t>& out)
        {
            int32_t ec, flush;
            uint32_t have;
            z_stream strm;
            uint8_t inbuf[CHUNK];
            uint8_t outbuf[CHUNK];
            uint32_t inIndex = 0;
            uint32_t inIndexOld = 0;

            out.clear();

            /* allocate deflate state */
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            ec = deflateInit(&strm, Z_BEST_COMPRESSION);
            if (ec != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                // cout << "Error: " << zError(ec) << '\n';
                Throw("ZLIB deflateInit Failed");
            }

            /* compress until end of file */
            do
            {
                inIndexOld = inIndex;
                inIndex += CHUNK;
                if (inIndex > in.size())
                {
                    inIndex = in.size();
                    flush = Z_FINISH;
                }
                else
                    flush = Z_NO_FLUSH;

                uint32_t size = inIndex - inIndexOld;
                memcpy(inbuf, in.data() + inIndexOld, size);

                strm.avail_in = size;
                strm.next_in = inbuf;

                /* run deflate() on input until output buffer not full, finish
                compression if all of source has been read in */
                do
                {
                    strm.avail_out = CHUNK;
                    strm.next_out = outbuf;
                    ec = deflate(&strm, flush);    /* no bad return value */

                    /* state not clobbered */
                    if (ec == Z_STREAM_ERROR)
                    {
                        cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                        // cout << "Error: " << zError(ec) << '\n';
                        Throw("ZLIB deflate Failed ");
                    }

                    have = CHUNK - strm.avail_out;

                    out.insert(out.end(), outbuf, outbuf + have);

                } while (strm.avail_out == 0);
                assert(strm.avail_in == 0);     /* all input will be used */

                                                /* done when last data in file processed */
            } while (flush != Z_FINISH);

            /* stream will be complete */
            if (ec != Z_STREAM_END)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                // cout << "Error: " << zError(ec) << '\n';
                Throw("ZLIB deflate Failed ");
            }

            /* clean up and return */
            ec = deflateEnd(&strm);
            if (ec != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(ec) << '\n';
                //cout << "Error: " << zError(ec) << '\n';
                Throw("ZLIB deflateEnd Failed");
            }
        }


        void ZLIB_CompressChecksum(uint8_t* in, uint32_t inSize, uint8_t* out, uint32_t& outSize)
        {
            z_stream defstream;
            defstream.zalloc = Z_NULL;
            defstream.zfree = Z_NULL;
            defstream.opaque = Z_NULL;
            defstream.data_type = Z_BINARY;
            // setup "a" as the input and "b" as the compressed output

            defstream.next_in = in; // input char array
            defstream.avail_in = inSize; // size of input
            defstream.next_out = out; // output char array
            defstream.avail_out = outSize; // size of output

                                           //deflateSetDictionary(&defstream, )

            int32_t res = deflateInit(&defstream, Z_BEST_COMPRESSION);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB DeflateInit Failed");
            }

            res = deflate(&defstream, Z_FINISH);
            if (!(res == Z_STREAM_END || res == Z_OK))
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB deflate Failed ");
            }

            res = deflateEnd(&defstream);
            if (res != Z_OK)
            {
                cout << "Error Code: " << ZLIB_ErrorCodeToStr(res) << '\n';
                //cout << "Error: " << zError(res) << '\n';
                Throw("ZLIB deflateEnd Failed");
            }

            outSize = defstream.next_out - out;

            if (*(uint32_t*)(out + outSize - 4) != _byteswap_ulong(defstream.adler))
            {
                *(uint32_t*)(out + outSize) = _byteswap_ulong(defstream.adler);
                outSize += 4;
            }
        }

        string ZLIB_ErrorCodeToStr(int32_t errorcode)
        {
            switch (errorcode)
            {
            case Z_OK: return "Z_OK";
            case Z_STREAM_END: return "Z_STREAM_END";
            case Z_NEED_DICT: return "Z_NEED_DICT";
            case Z_ERRNO: return "Z_ERRNO";
            case Z_STREAM_ERROR: return "Z_STREAM_ERROR";
            case Z_DATA_ERROR: return "Z_DATA_ERROR";
            case Z_MEM_ERROR: return "Z_MEM_ERROR";
            case Z_BUF_ERROR: return "Z_BUF_ERROR";
            case Z_VERSION_ERROR: return "Z_VERSION_ERROR";
            }
            return "UNK_ERR" + to_string(errorcode);
        }

    }

    namespace Crypt
    {
        bool AES_Decrypt(uint8_t* data, size_t length, const uint8_t key[32])
        {
            if (length == 0)
                return false;

            uint32_t inputCount = length & -16;
            if (inputCount > 0)
            {

                aes256_context ctx;
                aes256_init(&ctx, const_cast<uint8_t*>(key));

                for (uint32_t i = 0; i < inputCount; i += 16)
                {
                    for (uint32_t b = 0; b < 16; b++)
                        aes256_decrypt_ecb(&ctx, data + i);
                }

                aes256_done(&ctx);
                return true;
            }
            return false;
        }

        bool AES_Encrypt(uint8_t* data, size_t length, const uint8_t key[32])
        {
            if (length == 0)
                return false;

            uint32_t inputCount = length & -16;
            if (inputCount > 0)
            {

                aes256_context ctx;
                aes256_init(&ctx, const_cast<uint8_t*>(key));

                for (uint32_t i = 0; i < inputCount; i += 16)
                {
                    for (uint32_t b = 0; b < 16; b++)
                        aes256_encrypt_ecb(&ctx, data + i);

                }

                aes256_done(&ctx);
                return true;
            }
            return false;
        }

    }


}
