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
    // collections::StaticSortedArray();
    // collections::StaticSortedFlatMap();
    collections::StaticSortedFlatMap_WithDeletion();

    /**
    std::map      :  0.268529 seconds.
    std::flat_map :  0.0738449 seconds.
    map_boost     :  0.0517849 seconds.
    SortedArray   :  0.0292803 seconds.
    FlatMap       :  0.0307549 seconds.
    FlatMap_Del   :  0.0303184 seconds.
    **/
    return EXIT_SUCCESS;
}
