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
#include "Utils.hpp"

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
            debug(depthUpdate);
            /*
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
            */
        }

        void operator()([[maybe_unused]] const NoYetImplemented& nonImpl) const {
            std::println(std::cerr, "NoYetImplemented(strean: {})", nonImpl.streamName);
        }

    private:

        template<typename Event>
        static void debug(const Event& event)
        {
            std::cout << "Pricer [CPU: " << utilities::getCpu() << "] : " << typeid(event).name() << std::endl;
            std::cout << event << std::endl;
        }
    };
}

namespace price_engine
{
    void ExchangeBookKeeper::run(const uint32_t cpuId) {
        worker = std::jthread { &ExchangeBookKeeper::handleEvents, this, cpuId };
    }

    void ExchangeBookKeeper::handleEvents(const uint32_t cpuId)
    {
        if (!utilities::setThreadCore(cpuId)) {
            std::cerr << "Failed to pin Parser thread to  CPU " << cpuId  << std::endl;
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

    void ExchangeBookKeeper::push(BinanceMarketEvent& event)
    {
        const auto _ = queue.put(event);
    }
}

namespace price_engine
{
    void PricerEngine::run() const
    {
        uint32_t cpuId = 3;
        for (ExchangeBookKeeper* bookKeeper: books) {
            bookKeeper->run(cpuId++);
        }
    }

    void PricerEngine::push(common::Exchange exchange,
                           BinanceMarketEvent& event) const
    {
        // std::cout << "push (CPU: " << Utils::getCpu() << ") : " << jsonMessage << std::endl;
        books[static_cast<uint32_t>(exchange)]->push(event);
    }
}
