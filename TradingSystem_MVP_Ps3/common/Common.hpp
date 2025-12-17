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
    using Price     = double;
    using Quantity  = double;
    using Volume    = double;
    using Timestamp = uint64_t;
    using Number    = uint64_t;

    struct PriceLevel
    {
        Price price { 0.0 };
        Quantity quantity { 0.0 };
    };

    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
};

#endif //FINANCETECHNOLOGYPROJECTS_COMMON_HPP