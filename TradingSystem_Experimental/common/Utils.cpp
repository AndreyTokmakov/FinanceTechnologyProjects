/**============================================================================
Name        : Utilities.cpp
Created on  : 23.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.cpp
============================================================================**/

#include "Utils.h"

namespace Utils
{
    constexpr int32_t offset = static_cast<int32_t>('0');

    int64_t priceToLong(const std::string& strPrice)
    {
        return priceToLong(strPrice.data(), strPrice.size());
    }

    int64_t priceToLong(const char* strPrice, const uint32_t size)
    {
        int64_t price = 0;
        uint32_t pos = 0;
        while (size > pos && '.' != strPrice[pos])
        {
            price *= 10;
            price += static_cast<int32_t>(strPrice[pos]) - offset;
            ++pos;
        }
        ++pos;
        for (int i = 0, v = 0; i < 9; ++i)
        {
            v = size > pos ? static_cast<int32_t>(strPrice[pos]) -  offset : 0;
            price *= 10;
            price += v;
            ++pos;
        }
        return price;
    }
}