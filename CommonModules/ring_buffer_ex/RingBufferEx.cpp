/**============================================================================
Name        : RingBufferEx.cpp
Created on  : 06.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBufferEx
============================================================================**/

#include "RingBufferEx.h"
#include <vector>
#include <cstdint>

namespace
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & value - 1));
    }
}

namespace
{
    template<typename T, size_t Capacity>
    struct RingBuffer
    {
        using size_type = size_t;
        using value_type = T;
        // using collection_type = std::vector<value_type>;
        using collection_type = std::array<value_type, Capacity>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity),  "ERROR: Capacity must be a power of 2");

        collection_type buffer {};

        void put(value_type&& value)
        {
        }

        bool get(value_type& value)
        {
            return true;
        }
    };
}

void RingBufferEx::TestAll()
{
    RingBuffer<int, 32> buffer;
}