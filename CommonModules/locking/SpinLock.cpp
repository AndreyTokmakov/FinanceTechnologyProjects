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
#include "PerfUtilities.hpp"

#include <cstdio>
#include <cstdlib>


namespace SpinLockMtx
{
    struct SlowSpinLock
    {
        std::mutex mtx;

        void lock()
        {
            mtx.lock();
        }

        void unlock()
        {
            mtx.unlock();
        }

        ~SlowSpinLock()
        {
            mtx.unlock();
        }
    };

}


namespace SpinLock
{
    struct FastSpinLock
    {
        using intType = int_fast32_t;
        alignas(std::hardware_destructive_interference_size) std::atomic<intType> isLocked { 0 };

        void lock()
        {
            static constexpr timespec ns {0, 1};
            intType expected = 0;
            for (intType n = 0; !isLocked.compare_exchange_weak(expected, 1, std::memory_order_acquire); ++n) {
                expected = 0;
                if (8 == n) /// to tune thread scheduler
                {
                    n = 0;
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
        using intType = int_fast32_t;
        alignas(std::hardware_destructive_interference_size) std::atomic<intType> isLocked { 0 };

        void lock()
        {
            static constexpr timespec ns {0, 1};
            intType expected = 0;
            for (intType n = 0; !isLocked.compare_exchange_weak(expected, 1,
                                                                std::memory_order_relaxed,
                                                                std::memory_order_release); ++n) {
                expected = 0;
                if (8 == n) /// to tune thread scheduler
                {
                    n = 0;
                    nanosleep(&ns, nullptr);
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

    struct FastSpinLockChrono
    {
        alignas(std::hardware_destructive_interference_size) std::atomic<int32_t> isLocked { 0 };

        void lock()
        {
            int32_t expected = 0;
            for (int i = 0; !isLocked.compare_exchange_weak(expected, 1, std::memory_order_acquire); ++i) {
                expected = 0;
                if (8 == i) /// to tune thread scheduler
                {
                    i = 0;
                    std::this_thread::sleep_for(std::chrono::nanoseconds (1U));
                }
            }
        }

        void unlock()
        {
            isLocked.store(0, std::memory_order_release);
        }

        ~FastSpinLockChrono()
        {
            isLocked.store(false, std::memory_order_release);
        }
    };
};

namespace Experimental
{
    struct SpinLock
    {
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
        int_fast32_t retries { 0 };

        void lock()
        {
            retries = 0;
            while (flag.test_and_set(std::memory_order_acquire)) {
                backoff();
                retries++;
            }
        }

        void unlock() {
            flag.clear(std::memory_order_release);
        }

        void backoff()
        {
            const int max_retries = 8;
            if (retries < max_retries) {
                std::this_thread::yield();
            } else {
                auto delay = std::chrono::microseconds(static_cast<unsigned>(1 << (retries - max_retries)));
                std::this_thread::sleep_for(delay);
            }
        }


    };
}

namespace Tests
{
    constexpr int threadsMax { 16 };
    constexpr size_t iterCount { 10'000'000 };

    template<typename SpinLockType, bool warmUp = false>
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

        auto job = [&] {
            std::vector<std::jthread> jobs;
            for (int t = 0; t < threadsMax; ++t)
                jobs.emplace_back(task);
        };

        if constexpr (!warmUp)
        {
            PerfUtilities::ScopedTimer timer { testName };
            job();
        }
        else
        {
            job();
        }
    }

    void benchmark()
    {
        // test<SpinLock::FastSpinLock, true>("FastSpinLock");
        // test<SpinLock::FastSpinLockChrono, true>("FastSpinLockChrono");

        test<SpinLock::FastSpinLock>("FastSpinLock");
        test<SpinLock::FastSpinLock2>("FastSpinLock2");
        test<SpinLock::FastSpinLockChrono>("FastSpinLockChrono");
        // test<Experimental::SpinLock>("Experimental::SpinLock");
        // test<SpinLockMtx::SlowSpinLock>("SlowSpinLock");
    }
}

void SpinLock::TestAll()
{
    Tests::benchmark();

}
