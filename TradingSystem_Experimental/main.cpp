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
#include "Experiments.h"
#include "Tests.h"


// FIXME:
#define SERVER_SOCK_PATH "/tmp/unix_socket"

// TODO: *** Unit tests ***
//  - Add UnitTests
//  - Make unit test an additional CompileTarget ?


// TODO:
//  - Storage: need to stored historical data for analysis
//  - MarketData: Handle Different Types of Events (now only Snapshots and DepthUpdates)

// TODO: Renames consumer/Consumer -->

// TODO: Components && Modules
//  - Logger | NanoLog - SPDLog
//  - RingBuffer-LockFree Queue
//  - Price Converter:  --> From String --> Pair[INT, INT]  + SIMD ???

// TODO: **** DATABASE ****
//  - DB worker abstraction
//  - In memory Cache ???? Read Through

// TODO: Multiple Trading pairs [BTCUSDT, ETHUSDT, ....] && Multiple Exchanges
//  - 4 (for example - 1 per thread) RingBuffer-LockFree - queues <--- Data from Connectors
//  - 4 LOB (Price Engine) on the same Threads

// TODO: - Message Processing -
//  - FlatBuffers

// TODO: Consumer : Shall get data from Queue (update by Gateway) and parse it for specific Exchange
//  - Create a BaseConsumer
//  - BinanceConsumer

// TODO
//  - Rename Project - TradingSystom
//  - Add / Find performant cast Double --> Pair[Long, Long]
//  - MarketDataGatewayUDS  - Move to separate class + extract base
//  - Queue --> Try Lock-Free Ring-Bufer





int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    // UnitTests::TestAll();
    // Tests::Module_Impl_Test::TestAll();

    // Experiments::Service_Demo_One::TestAll();
    // Experiments::Service_Demo_Two::TestAll();
    // Experiments::Service_Callback_Tests::TestAll();
    Experiments::Modular_Service_Final::TestAll();


    // Tests::BufferTests::TestAll();

    /*
    Common::Queue<Common::DepthEvent> queue;
    EventConsumer::Server server { queue };
    OrderBook::Engine engine { queue };

    server.runServer();
    engine.start();
    */


    /*
    Common::Queue<std::string> queue;
    Gateway::UDS::UDSAsynchServer consumerServer { queue, SERVER_SOCK_PATH };
    const std::expected<bool, std::string> ok = consumerServer.init();
    if (!ok.has_value()) {
        std::cerr << "Failed to initialize server. Error: " << ok.error() << std::endl;
        return EXIT_FAILURE;
    }

    Manager::Manager manager { queue };
    manager.start();

    consumerServer.start();*/


    /*
    std::vector<int> buffer {1,2,3,4,5};
    std::vector<int> input {6,7,8,9,10};

    buffer.insert(buffer.end(), input.begin(), input.end());

    for (int v: buffer)
        std::cout << v << std::endl;
    */


    return EXIT_SUCCESS;
}
