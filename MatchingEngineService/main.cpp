/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <iostream>
#include <string_view>
#include <vector>

#include "order_book/OrderBook.h"
#include "consumers/EventConsumer.h"
#include "common/Utils.h"
#include "common/Queue.h"
#include "tests/UnitTests.h"


// TODO:
//  - AddLogging | NanoLog - SPDLog
//  - How to use Multiple Trading pairs [BTCUSDT, ETHUSDT, ....]
//  - Storage: need to stored historical data for analysis
//  - MarketData: Handle Different Types of Events (now only Snapshots and DepthUpdates)

// TODO: Consumer && Producer
//  - Shared memory exchange queue:
//    should be possible to exchange data between Py / Java / Rust / C++ apps

// TODO: *** Unit tests ***
//  - Add UnitTests
//  - Make unit test an additional CompileTarget ?


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    Common::Queue<Common::DepthEvent> queue;
    EventConsumer::Server server { queue };
    OrderBook::Engine engine { queue };

    server.runServer();
    engine.start();


    //UnitTests::TestAll();

    return EXIT_SUCCESS;
}
