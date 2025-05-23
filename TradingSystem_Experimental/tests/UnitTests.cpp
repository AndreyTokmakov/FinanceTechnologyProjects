/**============================================================================
Name        : UnitTests.cpp
Created on  : 25.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : UnitTests
============================================================================**/

#include "OrderBook.h"
#include "Utils.h"
#include "Queue.h"
#include "MarketDataGatewayUDP.h"
#include "MarketDataGatewayUDS.h"
#include "Tests.h"

#include <nlohmann/json.hpp>
#include <iostream>

#define SERVER_SOCK_PATH "/tmp/unix_socket"

namespace
{
    using namespace Utils;

    void DoubleToLong()
    {
        // const std::string strPrice("771043005.010010123");
        const std::string strPrice("0.82148000");
        const int64_t price = priceToLong(strPrice);

        std::cout << strPrice << " ==> " << price << std::endl;
    }

    void EventConsumer()
    {
        Common::Queue<Common::DepthEvent> queue;
        OrderBook::Engine engine { queue };

        engine.start();

        Common::DepthEvent snapshot { .type = Common::EventType::DepthSnapshot, .lastUpdateId = 1 };
        snapshot.akss.emplace_back(100, 1);
        snapshot.bids.emplace_back(90, 1);

        queue.push(std::move(snapshot));

        for (int i = 1; i <= 5; i++)
        {
            Common::DepthEvent event { .type = Common::EventType::DepthUpdate, .lastUpdateId = 10  };
            event.bids.emplace_back(90 + i, 1);
            queue.push(std::move(event));
        }
    }

    void EventConsumerUDS_Test()
    {
        Common::Queue<std::string> queue;
        Gateway_Experimental::UDS::UDSAsynchServer consumerServer {queue, SERVER_SOCK_PATH};
        const std::expected<bool, std::string> ok = consumerServer.init();
        if (!ok.has_value()) {
            std::cerr << "Failed to initialize server. Error: " << ok.error() << std::endl;
            return;
        }

        std::jthread consumer([&]{
            std::string message;
            while (true) {
                if (queue.pop(message)){
                    std::cout << message << std::endl;
                }
            }
        });

        consumerServer.start();
    }
}

void Tests::UnitTests::TestAll()
{

    // EventConsumer();
    EventConsumerUDS_Test();
}
