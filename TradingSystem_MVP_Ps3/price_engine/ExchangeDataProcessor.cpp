/**============================================================================
Name        : ExchangeDataProcessor.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : ExchangeDataProcessor.cpp
============================================================================**/

#include <iostream>
#include <print>
#include <cstring>
#include "ExchangeDataProcessor.hpp"
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
            std::cout << "EventHandler [CPU: " << utilities::getCpu() << "] : " << typeid(event).name() << std::endl;
            std::cout << event << std::endl;
        }
    };
}

namespace price_engine
{
    ExchangeDataProcessor::ExchangeDataProcessor(const Parser parser): parser { parser } {
    }

    void ExchangeDataProcessor::run(const uint32_t cpuId) {
        worker = std::jthread { &ExchangeDataProcessor::handleEvents, this, cpuId };
    }

    void ExchangeDataProcessor::handleEvents(const uint32_t cpuId)
    {
        if (!utilities::setThreadCore(cpuId)) {
            std::cerr << "Failed to pin Parser thread to  CPU " << cpuId  << std::endl;
            return;
        }

        buffer::Buffer* item { nullptr };
        auto parseEvent = [&](const auto& parser) {
            return parser.parse(*item);
        };

        EventHandler eventHandler { marketDepthBook } ;
        uint32_t misses { 0 };

        while (true)
        {
            if ((item = queue.pop()))
            {
                const BinanceMarketEvent event = std::visit(parseEvent,parser);
                std::visit(eventHandler, event);

                item->clear();
                misses = 0;
                continue;
            }

            /** Sleep when do not have messages for some time **/
            if (misses++ > maxSessionBeforeSleep) {
                std::this_thread::sleep_for(std::chrono::microseconds (10U));
            }
        }
    }

    void ExchangeDataProcessor::push(const std::string& eventData)
    {
        buffer::Buffer* response = queue.getItem();
        const size_t bytes = eventData.size();

        std::memcpy(response->tail(bytes), eventData.data(), bytes);
        response->incrementLength(bytes);
        queue.commit();
    }
}

