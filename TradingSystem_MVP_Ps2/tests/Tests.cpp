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

#include "RingBuffer.hpp"
#include "BinanceDataParser.hpp"
#include "../price_engine/MarketDepthBook.hpp"
#include "../utilities/Utilities.hpp"



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
