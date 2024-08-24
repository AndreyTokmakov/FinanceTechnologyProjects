/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <optional>
#include <iostream>
#include <string_view>
#include <cstring>
#include <fstream>
#include <functional>
#include <filesystem>

#include "DaVinchiTest.h"
#include "DaVinchiTest_LinkedList.h"

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    DaVinchiTest::TestAll();
    DaVinchiTest_LinkedList::TestAll();

    return EXIT_SUCCESS;
}
