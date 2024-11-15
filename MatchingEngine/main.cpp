/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "Includes.h"

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    /*
    if (const int value = args.empty() ? 1 : std::atoi(args.front().data()); 1 == value) {
        MatchingEngine::TestAll();
    } else if (2 == value)  {
        MatchingEngine_PriceLvLPtr::TestAll();
    } else if (3 == value)  {
        MatchingEngine_OrderAsPtr::TestAll();
    } else if (4 == value)  {
        MatchingEngine_OrderAsPtr_Alloc::TestAll();
    }
    */

    /*
    for (int i = 0; i < 20; ++i)
    {
        std::cout << std::string(160, '-') << '\n' << std::string(10, '\t')
                  << i << '\n' << std::string(160, '-') << std::endl;
        // MatchingEngine_Simple::LoadTest();
        // MatchingEngine_PriceLvLPtr::LoadTest();
        // MatchingEngine_OrderAsPtr::LoadTest();
        // MatchingEngine_OrderAsPtr_Alloc::LoadTest(); // ~ 0.52
        MatchingEngineEx::LoadTest(); // ~ 0.6
        // MatchingEngine::LoadTest();   // ~ 0.8 - 1.8
    }
    */

    // MatchingEngine::TestAll();
    MatchingEngineEx::TestAll();

    return EXIT_SUCCESS;
}
