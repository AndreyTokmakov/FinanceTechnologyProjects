/**============================================================================
Name        : Prefetch_Unroll_Processing.cpp
Created on  : 23.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Prefetch_Unroll_Processing.cpp
============================================================================**/

#include "includes.hpp"

#include <tuple>
#include <utility>
#include <iostream>
#include <print>

namespace prefetch_unrolled_data_processing
{
    template <std::size_t... I>
    void process_batch_impl(std::index_sequence<I...>,
                            int* data,
                            std::size_t mask,
                            std::size_t head)
    {
        // Prefetch future cache lines
        __builtin_prefetch(&data[(head + 32) & mask], 0, 1);

        // Unroll processing of 8 messages
        ( ( data[(head + I) & mask] += 1 ), ... );
    }

    template <std::size_t BatchSize>
    void process_batch(int* data, std::size_t mask, std::size_t head)
    {
        process_batch_impl(std::make_index_sequence<BatchSize>{},data, mask, head);
    }

    void test()
    {
        // ring-buffer size = 1024 (must be power of two)
        static constexpr size_t N = 1024;
        alignas(64) int rb[N]{};
        constexpr size_t head = 0;

        // process 8 messages branch-less + with prefetch
        process_batch<8>(rb, N - 1, head);
    }
}



void experiments::prefetch_unroll_processing()
{
    prefetch_unrolled_data_processing::test();
}
