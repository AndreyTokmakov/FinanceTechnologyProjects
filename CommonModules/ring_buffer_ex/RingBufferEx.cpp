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
#include <iostream>

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
        using size_type  = int_fast32_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;
        // using collection_type = std::array<value_type, Capacity>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity),  "ERROR: Capacity must be a power of 2");

        collection_type buffer {};
        size_type idxWrite { 0 };
        size_type idxRead { 0 };
        bool full { false };

        RingBuffer(): buffer(Capacity) {
        }

        void put(value_type&& value)
        {
            buffer[idxWrite] = std::move(value);

            if (full) {
                idxRead = fast_modulo(idxRead + 1, Capacity);
            }

            idxWrite = fast_modulo(idxWrite + 1, Capacity);
            full = (idxWrite == idxRead);
        }

        bool get(value_type& value)
        {
            if (true == empty())
                return false;

            value = std::move(buffer[idxRead]);
            full = false;
            idxRead = fast_modulo(idxRead + 1, Capacity);

            return true;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return (!full && (idxWrite == idxRead));
        }

        [[nodiscard]]
        bool isFull() const noexcept {
            return full;
        }

        static size_type capacity() noexcept {
            return Capacity;
        }
    };
}



void RingBufferEx::TestAll()
{
    RingBuffer<int, 16> buffer;

    for (int i = 1; i <= 32; ++i)
    {
        buffer.put(std::move(i));
    }

    int value = 0;
    while (buffer.get(value)) {
        std::cout << value << " ";
    }
}