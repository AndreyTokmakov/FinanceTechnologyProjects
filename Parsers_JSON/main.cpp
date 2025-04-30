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
#include <iostream>

#include "binance/BinanceDataParser.h"



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    binance::TestAll();


    return EXIT_SUCCESS;
}
