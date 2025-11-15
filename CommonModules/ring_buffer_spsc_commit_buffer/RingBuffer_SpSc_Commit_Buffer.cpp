/**============================================================================
Name        : RingBuffer_SpSc_Commit_Buffer.cpp
Created on  : 15.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SpSc_Commit_Buffer.cpp
============================================================================**/

#include "RingBuffer_SpSc_Commit_Buffer.hpp"
#include "StringUtilities.hpp"
#include "DateTimeUtilities.hpp"

#include <cstdint>
#include <memory>
#include <cstring>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>
#include <atomic>
#include <thread>
#include <syncstream>
#include <random>

namespace
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }

    int32_t getCpu() noexcept{
        return sched_getcpu();
    }

    bool setThreadCore(const uint32_t coreId) noexcept
    {
        cpu_set_t cpuSet {};
        CPU_ZERO(&cpuSet);
        CPU_SET(coreId, &cpuSet);
        return 0 == pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
    }

    std::random_device rd{};
    auto generator = std::mt19937{ rd() };

    int getRandomInRange(const int start, const int end) noexcept {

        auto distribution = std::uniform_int_distribution<>{ start, end };
        return distribution(generator);
    }
}

namespace
{
    struct Buffer
    {
        using value_type      = char;
        using size_type       = uint32_t;
        using array_type      = value_type[];
        using pointer         = value_type*;
        using const_pointer   = const pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

        Buffer() = default;

        explicit Buffer(const size_type initialCapacity):
            capacity { initialCapacity },
            size { 0 },
            data { std::make_unique_for_overwrite<array_type>(initialCapacity) }  {
        }

        Buffer(const Buffer& other):
            capacity { other.capacity },
            size { other.size },
            data { std::make_unique_for_overwrite<array_type>(other.capacity) }
        {
            std::memcpy(data.get(), other.data.get(), other.size);
        }

        Buffer& operator=(const Buffer& other)
        {
            capacity = other.capacity;
            size = other.size;
            data = std::make_unique_for_overwrite<array_type>(capacity);
            std::memcpy(data.get(), other.data.get(), size);
            return *this;
        }

        Buffer(Buffer&& other) noexcept:
            capacity { std::exchange(other.capacity, 0) },
            size { std::exchange(other.size, 0) },
            data { std::move(other.data) }
        {
            other.size = 0;
        }

        Buffer& operator=(Buffer&& other) noexcept
        {
            capacity = std::exchange(other.capacity, 0);
            size = std::exchange(other.size, 0);
            data = std::move(other.data);
            return *this;
        }

        // TODO: Rename
        void push_back(const value_type c)
        {
            ensure_capacity(size + 1);
            data[size++] = c;
        }

        void append(const_pointer str, const size_type len)
        {
            ensure_capacity(size + len);
            std::memcpy(data.get() + size, str, len);
            size += len;
        }

        [[nodiscard]]
        pointer head() const noexcept {
            return data.get();
        }

        [[nodiscard]]
        pointer tail() const noexcept {
            return data.get() + size;
        }

        [[nodiscard]]
        pointer tail(const size_type n) noexcept
        {
            validateCapacity(n);
            return data.get() + size;
        }

        [[nodiscard]]
        size_type length() const noexcept {
            return size;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return 0 == size;
        }

        void clear() noexcept {
            size = 0;
        }

        void reset() noexcept {
            size = 0;
        }

        reference operator[](const size_type idx) {
            return data[idx];
        }

        const_reference operator[](const size_type idx) const {
            return data[idx];
        }

        void incrementLength(const size_type incrSize) noexcept {
            size += incrSize;
        }

        void validateCapacity(const size_type n)
        {
            if ((capacity - size) >= n)
                return;
            ensure_capacity(capacity + n);
        }

    private:

        static constexpr uint32_t round_up_to_pow2(const uint32_t value) {
            return 1u << (32 - std::countl_zero(value - 1));
        }

        void ensure_capacity(size_type newCapacity)
        {
            newCapacity = round_up_to_pow2(newCapacity);
            auto new_data = std::make_unique_for_overwrite<array_type>(newCapacity);
            if (data)
                std::memcpy(new_data.get(), data.get(), size);
            data = std::move(new_data);
            capacity = newCapacity;
        }

        size_type capacity { 0 };
        size_type size { 0 };
        std::unique_ptr<array_type> data;
    };
}

namespace
{

    template<uint32_t Capacity>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = Buffer;
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

void RingBuffer_SpSc_Commit_Buffer::TestAll()
{
    RingBuffer<1024> queue {};

    auto produce = [&queue]
    {
        Buffer* buffer { nullptr };
        while ((buffer = queue.getItem())) {
            const int32_t size = getRandomInRange(0, 1024);
            const std::string text = StringUtilities::randomString(size);
            std::memcpy(buffer->tail(text.length()), text.data(), text.length());
            buffer->incrementLength(size);
            queue.commit();
            std::osyncstream { std::cout } << DateTimeUtilities::getCurrentTime()
                    << "Producer: " << size << " bytes written\n";
            std::this_thread::sleep_for(std::chrono::milliseconds (10U));
        }
    };

    auto consume = [&queue]
    {
        Buffer* item { nullptr };
        while (true)
        {
            if ((item = queue.pop())) {
                const std::string_view data = std::string_view(item->head(), item->length());
                std::osyncstream { std::cout } << DateTimeUtilities::getCurrentTime()
                        << "Consumer: " << data.length() << " bytes read\n";
                item->clear();
            }
        }
    };

    std::jthread producer { produce },
                 consumer { consume };
}
