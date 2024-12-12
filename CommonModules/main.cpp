/**============================================================================
Name        : main.cpp
Created on  : 12.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include <atomic>


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // std::atomic<int32_t> value = 10;
    // int32_t result = value.fetch_add(1, std::memory_order_relaxed);
    // std::cout << result << " -> " << value.load(std::memory_order_relaxed) << std::endl;

    auto [a, _] = std::make_pair(1, 1);
    auto [b, _] = std::make_pair(1, 1);


    return EXIT_SUCCESS;
}
