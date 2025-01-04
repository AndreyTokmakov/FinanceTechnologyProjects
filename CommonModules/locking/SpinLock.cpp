/**============================================================================
Name        : SpinLock.cpp
Created on  : 04.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : SpinLock.cpp
============================================================================**/

#include <atomic>
#include <future>
#include <thread>

#include "SpinLock.h"
#include "PerfUtilities.h"

#include "stdio.h"
#include "stdlib.h"

namespace SpinLock
{
    struct FastSpinLock
    {
        alignas(std::hardware_destructive_interference_size) std::atomic<int32_t> isLocked { 0 };

        void lock()
        {
            static constexpr timespec ns {0, 1};
            int32_t expected = 0;
            for (int i = 0; !isLocked.compare_exchange_weak(expected, 1, std::memory_order_acquire); ++i) {
                expected = 0;
                if (8 == i) /// to tune thread scheduler
                {
                    i = 0;
                    nanosleep(&ns, nullptr);
                }
            }
        }

        void unlock()
        {
            isLocked.store(0, std::memory_order_release);
        }

        ~FastSpinLock()
        {
            isLocked.store(false, std::memory_order_release);
        }
    };

    struct FastSpinLock2
    {
        alignas(std::hardware_destructive_interference_size) std::atomic<int32_t> isLocked { 0 };

        void lock()
        {
            static constexpr timespec ns {0, 1};
            int32_t expected = 0;
            for (int i = 0; !isLocked.compare_exchange_weak(expected, 1, std::memory_order_acquire); ++i) {
                expected = 0;
                if (8 == i) /// to tune thread scheduler
                {
                    i = 0;
                    nanosleep(&ns, nullptr);
                    __builtin_ia32_pause();
                }
            }
        }

        void unlock()
        {
            isLocked.store(0, std::memory_order_release);
        }

        ~FastSpinLock2()
        {
            isLocked.store(false, std::memory_order_release);
        }
    };
};


namespace Tests
{
    constexpr int threadsMax { 16 };
    constexpr size_t iterCount { 1'000'000 };

    template<typename SpinLockType>
    void test(const std::string_view testName)
    {
        SpinLockType spinLock;
        uint64_t counter = 0;

        auto task = [&] {
            for (size_t idx  = 0; idx < iterCount; ++idx) {
                spinLock.lock();
                ++counter;
                spinLock.unlock();
            }
        };

        PerfUtilities::ScopedTimer timer { testName };
        std::vector<std::jthread> jobs;
        for (int t = 0; t < threadsMax; ++t)
            jobs.emplace_back(task);
    }

    void benchmark()
    {
        test<SpinLock::FastSpinLock>("FastSpinLock");
        test<SpinLock::FastSpinLock2>("FastSpinLock2");
    }
}

void SpinLock::TestAll()
{
    Tests::benchmark();

}
