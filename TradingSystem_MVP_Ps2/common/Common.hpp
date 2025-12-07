/**============================================================================
Name        : Common.hpp
Created on  : 01.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common.hppd
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_COMMON_HPP
#define FINANCETECHNOLOGYPROJECTS_COMMON_HPP

#include <cstdint>

namespace common
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
};

#endif //FINANCETECHNOLOGYPROJECTS_COMMON_HPP