/**============================================================================
Name        : main.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Experiments HPC
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "includes.hpp"


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // experiments::unroll_data_processing_loop();
    experiments::prefetch_unroll_processing();

    return EXIT_SUCCESS;
}
