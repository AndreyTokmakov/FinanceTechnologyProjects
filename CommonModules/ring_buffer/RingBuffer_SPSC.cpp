/**============================================================================
Name        : RingBuffer_SPSC.cpp
Created on  : 25.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SPSC.cpp
============================================================================**/

#include "RingBuffer_SPSC.h"

#include <iostream>
#include <string_view>
#include <vector>

#include <atomic>
#include <thread>
#include <future>



namespace RingBuffer_SPSC
{
    template<typename T>
    struct RingBuffer
    {
        using size_type = size_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        std::atomic<size_type> idxRead { 0 };
        std::atomic<size_type> idxWrite { 0 };
        std::atomic<bool> overflow {false };
        collection_type buffer {};

        explicit RingBuffer(size_t size): idxRead { 0 }, idxWrite { 0 }, overflow { false } {
            buffer.resize(size);
        }

        void put(value_type&& value)
        {
            size_type writeIdx = idxWrite.load(std::memory_order::relaxed);
            if (writeIdx == buffer.size()) {
                idxWrite = 0;
                writeIdx = 0;
                overflow = true;
            }

            const size_type readIdx = idxRead.load(std::memory_order::relaxed);
            if (overflow && writeIdx == readIdx) {
                if (1 + readIdx >= buffer.size()) {
                    idxRead.store(0, std::memory_order::release);
                    overflow = false;
                }
            }
            buffer[writeIdx++] = std::move(value);
            idxWrite.fetch_add(1, std::memory_order::release);
        }

        bool get(value_type& value)
        {
            size_type readIdx = idxRead.load(std::memory_order::relaxed);
            if (!overflow && idxWrite == readIdx) {
                return false;
            }

            if (readIdx >= buffer.size()) {
                readIdx = 0;
                idxRead.store(0, std::memory_order::release);
                overflow = false;
            }

            value = std::move(buffer[readIdx++]);
            idxRead.fetch_add(1, std::memory_order::release);
            return true;
        }
    };
}


namespace RingBuffer_SPSC::Tests
{
    void Test()
    {
        RingBuffer<std::string> buffer(32);

        auto produce = [&buffer](const std::string& text) {
            int64_t count = 0, n = 0;
            while (true) {
                buffer.put(std::string { text });
                if (10'000 == ++count) {
                    ++n;
                    //std::cout << count * n << std::endl;
                    count = 0;
                }
            }
        };

        auto consume = [&buffer]() {
            int64_t count = 0, n = 0;
            std::string str;
            while (true) {
                if (buffer.get(str)) {
                    if (100'000 == ++count) {
                        ++n;
                        std::cout << count * n << std::endl;
                        count = 0;
                    }
                }
            }
        };

        auto producer = std::async(produce, "log_1");
        auto consumer = std::async(consume);

        producer.wait();
        consumer.wait();
    }
}

void RingBuffer_SPSC::TestAll()
{
    Tests::Test();


    // std::cout << static_cast<int32_t>(std::numeric_limits<uint8_t>::max())<< std::endl;
    // std::cout << std::numeric_limits<uint16_t>::max() * 32 << std::endl;
}