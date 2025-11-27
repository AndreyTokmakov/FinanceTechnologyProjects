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

    collections::MapWithConstantSize();
    collections::StaticSortedArray();
    collections::StaticSortedFlatMap();

    /**
    std::map      :  0.288727 seconds.
    std::flat_map :  0.0746102 seconds.
    map_boost     :  0.0513938 seconds.
    SortedArray   :  0.0308039 seconds.
    FlatMap       :  0.0296755 seconds.
    **/

    return EXIT_SUCCESS;
}
