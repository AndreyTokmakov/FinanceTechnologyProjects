/**============================================================================
Name        : RingBuffer_SpSc_Commit.cpp
Created on  : 29.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SpSc_Commit.cpp
============================================================================**/

#include "RingBuffer_SpSc_Commit.hpp"

#include <vector>
#include <atomic>
#include <iostream>
#include <thread>


namespace ring_buffer
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & value - 1));
    }
}

namespace ring_buffer::static_capacity_with_commit
{
    template<typename T, uint32_t Capacity>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity), "ERROR: Capacity must be a power of 2");

        size_type headCached { 0 };
        size_type headNext { 0 };

        RingBuffer(): buffer(Capacity) {
        }

        [[nodiscard]]
        bool getItem(value_type& item)
        {
            headCached = head.load(std::memory_order::relaxed);
            headNext = fast_modulo(headCached + 1, Capacity);
            if (headNext == tail.load(std::memory_order::acquire)) {
                return false;
            }

            item = buffer[headCached];
            return true;
        }

        void commit(const value_type& item)
        {
            buffer[headCached] = item;
            head.store(headNext, std::memory_order::release);
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

namespace ring_buffer::static_capacity_with_commit::testing
{
    void Test()
    {
        RingBuffer<int32_t, 1024 * 8> queue;
        constexpr int64_t eventsCount {10'000'000}, initialValue {0};
        int64_t reads {0}, writes {0}, errors {0}, waits {0};

        auto consume = [&]
        {
            int32_t prev = initialValue - 1, result = 0;
            while (eventsCount > reads) {
                if (queue.pop(result)) {
                    if (prev + 1 != result) {
                        ++errors;
                    }
                    prev = result;
                    ++reads;
                }
            }
        };

        auto produce = [&]
        {
            int32_t item = 0;
            for (int32_t idx = initialValue; idx <= eventsCount; ++idx)
            {
                while (true) {
                    if (queue.getItem(item))
                    {
                        item = idx;
                        queue.commit(item);
                        ++writes;
                        break;
                    }
                    ++waits;
                }
            }
        };

        std::jthread consumer {consume}, producer {produce};
        consumer.join();
        producer.join();

        std::cout << "Reads: " << reads << ", writes: " << writes
            << ", errors: " << errors  << ", waits: " << waits << std::endl;
    }
}

void RingBuffer_SpSc_Commit::TestAll()
{
    ring_buffer::static_capacity_with_commit::testing::Test();
}
