/**============================================================================
Name        : main.cpp
Created on  : 12.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "queues/LockFreeQueue.h"
#include "logging/LowLatencyLogger.h"


// TODO:
//  -  Low Latency logger



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // LockFreeQueue::TestAll();
    LowLatencyLogger::TestAll();


    return EXIT_SUCCESS;
}
