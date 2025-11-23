/**============================================================================
Name        : Unroll_Data_Processing.cpp
Created on  : 23.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Unroll_Data_Processing.cpp
============================================================================**/

#include "includes.hpp"

#include <tuple>
#include <utility>
#include <iostream>
#include <print>

namespace unroll_data_processing_loop::demo_one
{
    constexpr size_t SlotCount = 8;

    // HFT-style slot processor
    struct SlotProcessor
    {
        alignas(64) std::array<int, SlotCount> slots {};   // cache-line aligned

        void process_one(const std::size_t i)
        {
            // Simulate some hot-path processing
            slots[i] += 1;
        }
    };

    template <typename F, std::size_t... I>
    void unrolled_impl(F&& f, std::index_sequence<I...>)
    {
        // Fold expression expands into: (f(0), f(1), f(2), ...)
        ( (f(I), void()), ... );
    }

    template <std::size_t N, typename F>
    void unrolled(F&& f)
    {
        unrolled_impl(std::forward<F>(f), std::make_index_sequence<N>{});
    }

    /**
    * The compiler emits 8 inlined calls process_one(0), process_one(1) …
    * No loop counters. No conditional jumps.
    *
    * ✔ No loop-carried dependencies
    *   Better instruction-level parallelism → CPUs execute multiple ops per cycle.
    *
    * ✔ Perfectly predictable loads/stores
    *   Caches love linear prefetch-friendly access.
    *
    * ✔ Works great on pre-allocated arrays
    *   Very common in HFT engines (ring-buffers, book levels, recycling pools, etc.)
    */
    void test()
    {
        SlotProcessor proc;

        // Fully unrolled at compile time: equivalent to 8 manual calls
        unrolled<SlotCount>([&](const std::size_t i) {
            proc.process_one(i);
        });

        for (const auto v : proc.slots)
            std::cout << v << " ";

        // Output:
        //  1 1 1 1 1 1 1 1
    }
}

void experiments::unroll_data_processing_loop()
{
    unroll_data_processing_loop::demo_one::test();
}
