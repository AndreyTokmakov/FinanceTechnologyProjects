/**============================================================================
Name        : main.cpp
Created on  : 14.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <string_view>
#include <vector>

#include "debug_one/PriceEngine.hpp"
#include "experiments/Experiments.hpp"
#include "flat_level_map/FlatLevelMap.hpp"

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // pricing_engine::debug_one::TestAll();
    // experiments::TestAll();
    flat_level_map::TestAll();

    return EXIT_SUCCESS;
}
