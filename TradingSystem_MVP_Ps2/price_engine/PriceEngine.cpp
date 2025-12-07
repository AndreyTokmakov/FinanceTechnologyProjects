/**============================================================================
Name        : PriceEngine.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceEngine.cpp
============================================================================**/

#include <iostream>
#include <print>

#include "PriceEngine.hpp"
#include "Formatting.hpp"

namespace price_engine
{
    template<class P, class Q>
    struct EventHandler
    {
        price_engine::MarketDepthBook<P, Q>& marketDepthBook;

        void operator()([[maybe_unused]] const BookTicker& ticker) const {
            // debug(ticker);
        }

        void operator()([[maybe_unused]] const MiniTicker& ticker) const {
            // debug(ticker);
        }

        void operator()([[maybe_unused]] const Trade& trade) const {
            // debug(trade);
        }

        void operator()([[maybe_unused]] const AggTrade& aggTrade) const {
            // debug(aggTrade);
        }

        void operator()([[maybe_unused]] const DepthUpdate& depthUpdate) const
        {
            // debug(depthUpdate);
            for (const auto&[price, quantity]: depthUpdate.bids) {
                marketDepthBook.buyUpdate(price, quantity);
            }
            for (const auto&[price, quantity]: depthUpdate.asks) {
                marketDepthBook.askUpdate(price, quantity);
            }

            std::cout << "DepthUpdate { bids: " << depthUpdate.bids.size() << ". asks: " << depthUpdate.asks.size() << "}  "
                    << " Spread: " << marketDepthBook.getSpread()
                    << ", Market Price: " << marketDepthBook.getMarketPrice().value_or(0)
                    << ", Book [bids: " << marketDepthBook.bids.size() << ", asks: " << marketDepthBook.asks.size() << "]"
                    << ", Best [bid: " << marketDepthBook.getBestBid().value_or({}).first
                         << ", ask: " << marketDepthBook.getBestAsk().value_or({}).first << "]"
                    << std::endl;
        }

        void operator()([[maybe_unused]] const NoYetImplemented& nonImpl) const {
            std::println(std::cerr, "NoYetImplemented(strean: {})", nonImpl.streamName);
        }

    private:

        template<typename Event>
        static void debug(const Event& event)
        {
            std::cout << "Pricer [CPU: " << common::getCpu() << "] : " << typeid(event).name() << std::endl;
            std::cout << event << std::endl;
        }
    };
}

namespace price_engine
{
    void PricerEngine::run() {
        worker = std::jthread { [&] { handleEvents(); } };
    }

    void PricerEngine::handleEvents()
    {
        if (!common::setThreadCore(3)) {
            std::cerr << "Failed to pin Parser thread to  CPU " << 3  << std::endl;
            return;
        }

        EventHandler eventHandler { book } ;
        BinanceMarketEvent event;
        while (true) {
            if (queue.pop(event)) {
                std::visit(eventHandler, event);
            }
            //std::this_thread::sleep_for(std::chrono::milliseconds (1u));
        }
    }

    void PricerEngine::push(BinanceMarketEvent& event)
    {
        const auto _ = queue.put(event);
    }
}