/**============================================================================
Name        : main.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Collections
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "Collections.hpp"


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // collections::MapWithConstantSize();
    collections::StaticSortedArray();

    /**
    1000
    std::map      :  0.275893 seconds.
    1000
    map_boost     :  0.0519193 seconds.
    1000
    SortedArray   :  0.0304988 seconds.
    **/

    return EXIT_SUCCESS;
}
