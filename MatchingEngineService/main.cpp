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

#include "order_book/Manager.h"
#include "order_book/OrderBook.h"
#include "MarketDataGatewayUDP.h"
#include "MarketDataGatewayUDS.h"
#include "common/Utils.h"
#include "common/Queue.h"
#include "tests/UnitTests.h"


// FIXME:
#define SERVER_SOCK_PATH "/tmp/unix_socket"



// TODO:
//  - AddLogging | NanoLog - SPDLog
//  - How to use Multiple Trading pairs [BTCUSDT, ETHUSDT, ....]
//  - Storage: need to stored historical data for analysis
//  - MarketData: Handle Different Types of Events (now only Snapshots and DepthUpdates)

// TODO: Renames consumer/Consumer -->

// TODO: Consumer : Shall get data from Queue (update by Gateway) and parse it for specific Exchange
//  -  Create a BaseConsumer
//  -  BinanceConsumer

// TODO: *** Unit tests ***
//  - Add UnitTests
//  - Make unit test an additional CompileTarget ?


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    /*
    Common::Queue<Common::DepthEvent> queue;
    EventConsumer::Server server { queue };
    OrderBook::Engine engine { queue };

    server.runServer();
    engine.start();
    */

    // UnitTests::TestAll();

    Common::Queue<std::string> queue;
    Gateway::UDS::UDSAsynchServer consumerServer { queue, SERVER_SOCK_PATH };
    const std::expected<bool, std::string> ok = consumerServer.init();
    if (!ok.has_value()) {
        std::cerr << "Failed to initialize server. Error: " << ok.error() << std::endl;
        return EXIT_FAILURE;
    }

    Manager::Manager manager { queue };
    manager.start();

    consumerServer.start();

    return EXIT_SUCCESS;
}
