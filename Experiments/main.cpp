/**============================================================================
Name        : main.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Various Experiments
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>


void wsTest();
void demoOne();

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // wsTest();
    demoOne();

    return EXIT_SUCCESS;
}
