#pragma once
#include <cstdint>

template<typename T>
class Result
{
public:
    bool Res = false;
    T Data;

    Result()
    {}

    Result(T& data) : Data(data), Res(true)
    {}



};
