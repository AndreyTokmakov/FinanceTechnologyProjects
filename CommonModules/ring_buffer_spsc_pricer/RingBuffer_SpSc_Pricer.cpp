/**============================================================================
Name        : RingBuffer_SpSc_Pricer.cpp
Created on  : 15.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SpSc_Pricer.cpp
============================================================================**/

#include "RingBuffer_SpSc_Pricer.hpp"

#include <vector>
#include <cstdint>
#include <iostream>
#include <atomic>


namespace
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
}

namespace
{

    template<typename T, uint32_t Capacity>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity), "ERROR: Capacity must be a power of 2");

        RingBuffer(): buffer(Capacity) {
        }

        [[nodiscard]]
        bool put(value_type& item)
        {
            size_type headCached = head.load(std::memory_order::relaxed);
            const size_type headNext = fast_modulo(headCached + 1, Capacity);
            if (headNext == tail.load(std::memory_order::acquire)) {
                return false;
            }

            item = buffer[headCached];
            buffer[headCached] = item;
            head.store(headNext, std::memory_order::release);

            return true;
        }

        [[nodiscard]]
        bool pop(value_type& item)
        {
            const size_type tailLocal = tail.load(std::memory_order::relaxed);
            if (tailLocal == head.load(std::memory_order::acquire)) {
                return false;
            }

            item = buffer[tailLocal];
            tail.store(fast_modulo(tailLocal + 1, Capacity), std::memory_order::release);

            return true;
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return head.load(std::memory_order::relaxed) - tail.load(std::memory_order::relaxed);
        }

        [[nodiscard]]
        size_type empty() const noexcept
        {
            // TODO: can 'acquire' be replaced with 'relaxed' ?
            return head.load(std::memory_order::acquire) == tail.load(std::memory_order::acquire);
        }

        [[nodiscard]]
        size_type full() const noexcept
        {
            return size() == Capacity;
        }

    private:

        std::vector<value_type> buffer;

        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> tail {0};
        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> head {0};
    };
}

void RingBuffer_SpSc_Pricer::TestAll()
{

}