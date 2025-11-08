/**============================================================================
Name        : main.cpp
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parsers_JSON
============================================================================**/

#include <vector>
#include <string_view>
#include <cstdlib>

#include "binance/Parsers.hpp"



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    binance::Experiments::TestAll();
    // binance::SimdJson::TestAll();

    return EXIT_SUCCESS;
}
