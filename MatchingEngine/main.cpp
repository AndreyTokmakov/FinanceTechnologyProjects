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

#include "MatchingEngine.h"

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    MatchingEngine::TestAll();
    MatchingEngine_PriceLvLPtr::TestAll();
    MatchingEngine_OrderAsPtr::TestAll();
    MatchingEngine_OrderAsPtr_Alloc::TestAll();


    return EXIT_SUCCESS;
}
