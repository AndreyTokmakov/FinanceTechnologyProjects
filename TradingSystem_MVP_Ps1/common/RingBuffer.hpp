/**============================================================================
Name        : RingBuffer.hpp
Created on  : 27.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer.cpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_RINGBUFFER_SPSC_123434343_HPP
#define FINANCETECHNOLOGYPROJECTS_RINGBUFFER_SPSC_123434343_HPP

#include <vector>
#include <atomic>
#include "Common.hpp"
#include "Buffer.hpp"

namespace ring_buffer::static_buffer
{
    using namespace common;

    template<uint32_t Capacity>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = buffer::Buffer;
        using pointer    = value_type*;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity), "ERROR: Capacity must be a power of 2");

        size_type headCached { 0 };
        size_type headNext { 0 };

        RingBuffer(): buffer(Capacity) {
        }

        [[nodiscard]]
        pointer getItem()
        {
            headCached = head.load(std::memory_order::relaxed);
            headNext = fast_modulo(headCached + 1, Capacity);
            if (headNext == tail.load(std::memory_order::acquire)) {
                return nullptr;
            }

            return &buffer[headCached];
        }

        void commit()
        {
            head.store(headNext, std::memory_order::release);
        }

        [[nodiscard]]
        pointer pop()
        {
            const size_type tailLocal = tail.load(std::memory_order::relaxed);
            if (tailLocal == head.load(std::memory_order::acquire)) {
                return nullptr;
            }

            pointer item = &buffer[tailLocal];
            tail.store(fast_modulo(tailLocal + 1, Capacity), std::memory_order::release);

            return item;
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

        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> tail { 0 };
        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> head { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RINGBUFFER_SPSC_123434343_HPP
