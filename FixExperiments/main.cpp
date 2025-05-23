/**============================================================================
Name        : main.cpp
Created on  : 22.05.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "server/Server.h"
#include "client/Client.h"



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    //FixTests::Server::TestAll();
    FixTests::Client::TestAll();

    return EXIT_SUCCESS;
}
