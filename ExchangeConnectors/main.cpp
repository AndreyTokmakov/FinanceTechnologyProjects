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

#include "Experiments.hpp"
#include "Binance.hpp"


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // WebSockerConnector::TestAll();
    // WebSockerConnectorAsynch::TestAll();
    // WSConnectorAsynch::TestAll();
    // ApiClient::TestAll();

    // experiments::WSConnectorAsynchExperimental::TestAll();
    experiments::MinimalSynchronousClient::TestAll();


    return EXIT_SUCCESS;
}
