/**============================================================================
Name        : RingBuffer_SPSC_Two.cpp
Created on  : 27.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SPSC_Two.cpp
============================================================================**/

#include "RingBuffer_SPSC_Two.hpp"

#include <string>
#include <cstdint>
#include <string_view>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>
#include <iostream>
#include <ostream>

namespace
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & value - 1));
    }
}

namespace lock_free_ring_buffer
{
    template<typename T>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        // static_assert(is_pow_of_2(Capacity),  "ERROR: Capacity must be a power of 2");

        explicit RingBuffer(const size_type capacity):
            capacity { capacity }, buffer(capacity), tail(0), head()
        {
            assert(is_pow_of_2(capacity) == true && "Capacity must be a power of 2");
        }

        bool add(const value_type& value)
        {
            const size_type headLocal = head.load(std::memory_order::relaxed);
            const size_type headNext = fast_modulo(headLocal + 1, capacity);

            if (headNext == tail.load(std::memory_order::acquire)) {
                return false;
            }

            buffer[headLocal] = value;
            head.store(headNext, std::memory_order::release);

            return true;
        }

        [[nodiscard]]
        std::optional<value_type> pop()
        {
            const size_type tailLocal = tail.load(std::memory_order::relaxed);
            if (tailLocal == head.load(std::memory_order::acquire)) {
                return std::nullopt;
            }

            const value_type item = buffer[tailLocal];
            tail.store(fast_modulo(tailLocal + 1, capacity), std::memory_order::release);
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
            return size() == capacity;
        }

    private:

        size_type capacity { 0 };
        std::vector<value_type> buffer;

        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> tail {0};
        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> head {0};
    };
}

namespace lock_free_ring_buffer::testing
{
    void Test()
    {
        RingBuffer<int> queue(1024);
        constexpr int64_t eventsCount {200'000'000}, initialValue {0};
        int64_t reads {0}, writes {0}, errors {0};

        // ScopedTimer timer {"benchmark"};

        auto consume = [&] {
            int previousValue = initialValue - 1;
            std::optional<int> result;
            while (eventsCount > reads) {
                result = queue.pop();
                if (result.has_value()) {
                    if (previousValue + 1 != result.value()) {
                        ++errors;
                    }
                    previousValue = result.value();
                    ++reads;
                }
            }
        };

        auto produce = [&] {
            for (int64_t idx = initialValue; idx <= eventsCount; ++idx)
                {
                while (true) {
                    if (queue.add(idx)) {
                        ++writes;
                        break;
                    }
                }
            }
        };

        std::jthread consumer {consume}, producer {produce};
        consumer.join();
        producer.join();

        std::cout << "Reads: " << reads << ", writes: " << writes << ", errors: " << errors << std::endl;
    }
}

void RingBuffer_SPSC_Two::TestAll()
{
    lock_free_ring_buffer::testing::Test();
}