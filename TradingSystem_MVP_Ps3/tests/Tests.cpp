/**============================================================================
Name        : Tests.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests.cpp
============================================================================**/

#include "Tests.hpp"

#include <iostream>
#include <print>

#include "RingBuffer.hpp"
#include "BinanceDataParser.hpp"
#include "MarketDepthBook.hpp"
#include "Utils.hpp"


namespace connectors
{
    bool DataFileDummyConnector::init()
    {
        data = utilities::readFile(utilities::getDataDir() / "depth.json");
        return !data.empty();
    }

    void DataFileDummyConnector::run(ring_buffer::two_phase_push::RingBuffer<1024>& queue)
    {
        worker = std::jthread { &DataFileDummyConnector::produceTestEvents, this, std::ref(queue) };
    }

    void DataFileDummyConnector::produceTestEvents(ring_buffer::two_phase_push::RingBuffer<1024>& queue)
    {
        buffer::Buffer* response { nullptr };
        while (true)
        {
            if (readPost == data.size()) {
                std::println(std::cerr, "No more data to read");
                std::terminate();
            }

            const std::string& message { data[readPost % data.size()] };
            const size_t bytes = message.size();

            std::cout << message << std::endl;

            response = queue.getItem();
            std::memcpy(response->tail(bytes), message.data(), bytes);
            response->incrementLength(bytes);
            queue.commit();

            std::this_thread::sleep_for(std::chrono::milliseconds (250U));
            ++readPost;
        }
    }
}

namespace connectors
{
    DataFileDummyConnector2::DataFileDummyConnector2(price_engine::MarketStateManager& marketStateManager):
            marketStateManager { marketStateManager } {
    }

    bool DataFileDummyConnector2::init()
    {
        data = utilities::readFile(utilities::getDataDir() / "depth.json");
        return !data.empty();
    }

    void DataFileDummyConnector2::run()
    {
        worker = std::jthread { &DataFileDummyConnector2::produceTestEvents, this};
    }

    void DataFileDummyConnector2::produceTestEvents()
    {

        if (!utilities::setThreadCore(1)) {
            std::cerr << "Failed to pin DataFileDummyConnector2 thread to  CPU " << 1  << std::endl;
            return;
        }

        while (true)
        {
            /*if (readPost == data.size()) {
                std::println(std::cerr, "No more data to read");
                std::terminate();
            }*/

            const std::string& message { data[readPost % data.size()] };

            std::cout << "Connector [CPU: " << utilities::getCpu() << "] : " << message << std::endl;
            marketStateManager.push(common::Exchange::Binance, message);

            std::this_thread::sleep_for(std::chrono::milliseconds (250U));
            ++readPost;
        }
    }
}


namespace pricer_test
{
    using common::Price;
    using common::Quantity;

    using namespace common;
    using namespace ring_buffer;
    using namespace market_data::binance;

    struct NoYetImplemented {};

    using BinanceMarketEvent = std::variant<
        BookTicker,
        MiniTicker,
        Trade,
        AggTrade,
        DepthUpdate,
        NoYetImplemented
    >;

    using PricingEngine = price_engine::MarketDepthBook<Price, Quantity>;

    struct EventPrinter
    {
        PricingEngine& pricingEngine;

        void operator()(const BookTicker&) const { }
        void operator()(const MiniTicker&) const { }
        void operator()(const Trade&) const {}
        void operator()(const AggTrade&) const { }
        void operator()(const NoYetImplemented&) const { }
        void operator()(const DepthUpdate& depthUpdate) const
        {
            for (const auto&[price, quantity]: depthUpdate.bids) {
                pricingEngine.buyUpdate(price, quantity);
            }
            for (const auto&[price, quantity]: depthUpdate.asks) {
                pricingEngine.askUpdate(price, quantity);
            }
            std::cout << "DepthUpdate { bids: " << depthUpdate.bids.size() << ". asks: " << depthUpdate.asks.size() << "}  "
                    << " Spread: " << pricingEngine.getSpread()
                    << ", Market Price: " << pricingEngine.getMarketPrice().value_or(0)
                    << ", Book [bids: " << pricingEngine.bids.size() << ", asks: " << pricingEngine.asks.size() << "]"
                    << ", Best [bid: " << pricingEngine.getBestBid().value_or({}).first
                    << ", ask: " << pricingEngine.getBestAsk().value_or({}).first << "]"
                    << std::endl;
        }
    };

    void print(const PricingEngine&)
    {
        /*
        std::cout << "BIDS:" << std::endl;
        for (const auto& [price, quantity]: pricingEngine.bidPriceLevelMap) {
            std::cout << "\t { price: " << price << ", quantity: " << quantity << "}\n";
        }
        std::cout << "ASKS:" << std::endl;
        for (const auto& [price, quantity]: pricingEngine.askPriceLevelMap) {
            std::cout << "\t { price: " << price << ", quantity: " << quantity << "}\n";
        }*/
    }

    void pricerTests()
    {
        const std::vector<std::string> data = utilities::readFile(utilities::getDataDir() / "depth.json");

        PricingEngine pricingEngine;
        EventPrinter eventPrinter { .pricingEngine = pricingEngine };
        int n = 0;
        for (const auto& entry: data)
        {
            const nlohmann::json jsonData = nlohmann::json::parse(entry);
            BinanceMarketEvent event = BinanceParserJson::parseDepthUpdate(jsonData[JsonParams::data]);
            std::visit(eventPrinter, event);

            std::cout << n++ << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds (1U));
        }

        // print(pricingEngine);
    }
}

void tests::pricerTests()
{
    pricer_test::pricerTests();
}
