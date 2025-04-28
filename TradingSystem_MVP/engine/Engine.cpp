/**============================================================================
Name        : Engine.cpp
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Engine.cpp
============================================================================**/

#include "Engine.h"
#include "Utilities.h"
#include "Parser.h"

#include <iostream>
#include <nlohmann/json.hpp>


namespace
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
}


namespace engine
{
    using market_data::Ticker;

    void OrderBook::processEvent(const market_data::Event& event) const
    {

    }
}

namespace engine
{
    ExchangeBookKeeper::ExchangeBookKeeper()
    {
        for (const std::string& ticker: { "BTCUSDT"s, "XRPUSDT"s, "MEMEUSDT"s }){
            orderBooksByTicker.emplace(ticker, std::make_unique<OrderBook>(ticker));
        }
    }

    // TODO: Rename??
    void ExchangeBookKeeper::push(beast::flat_buffer* message)
    {
        queue.push(message);
    }

    void ExchangeBookKeeper::start(const uint32_t cpuId)
    {
        worker = std::jthread{ [this, cpuId]
        {
            const auto threadId { std::this_thread::get_id() };
            if (!utilities::setThreadCore(cpuId)) {
                std::cerr << "Failed to set core " << cpuId << " for " << threadId << std::endl;
                return;
            } else {
                std::cout << "Starting ExchangeBookKeeper on CPU: " << utilities::getCpu() << std::endl;
            }

            market_data::Event event;
            decltype(queue)::Wrapper dataWrapper;
            while (true) {
                if (queue.pop(dataWrapper))
                {
                    const auto& book = orderBooksByTicker[event.pair];
                    book->processEvent(event);  // TODO: Rename method
                }
            }
        }};
        worker.detach();
    }
}

namespace engine
{
    void PricingEngine::start()
    {
        uint32_t cpuId = 2;
        for (ExchangeBookKeeper* bookKeeper: books)
            bookKeeper->start(cpuId++);
    }

    void PricingEngine::push(common::Exchange exchange, beast::flat_buffer* message)
    {
        // std::cout << "push (CPU: " << Utils::getCpu() << ") : " << jsonMessage << std::endl;
        books[static_cast<uint32_t>(exchange)]->push(message);
    }
}