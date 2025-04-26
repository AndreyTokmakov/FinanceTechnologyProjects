/**============================================================================
Name        : Utilities.h
Created on  : 23.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_UTILITIES_H
#define FINANCETECHNOLOGYPROJECTS_UTILITIES_H

#include <string>
#include <cstdint>

namespace utilities
{
    int64_t priceToLong(const std::string& strPrice);
    int64_t priceToLong(const char* strPrice, uint32_t size);

    bool setThreadCore(uint32_t coreId) noexcept;
    int32_t getCpu() noexcept;

    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
}

#endif //FINANCETECHNOLOGYPROJECTS_UTILITIES_H
