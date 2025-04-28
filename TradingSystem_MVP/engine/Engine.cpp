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
        std::cout << "Book '" << pair << "' processing event: " <<  event.type << std::endl;
        //std::cout << event.symbol << std::endl;
        //std::cout << event.pair << std::endl;
    }
}

namespace engine
{
    ExchangeBookKeeper::ExchangeBookKeeper()
    {
        // TODO: Read Symbols from configuration
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
                    market_data::parse(static_cast<char*>(dataWrapper.ptr->data().data()),
                                       dataWrapper.ptr->data().size(),
                                       event);
                    if (event.type == market_data::EventType::Result)
                    {
                        std::cout << event.type << " received\n";
                        continue;
                    }

                    std::cout << std::quoted(event.symbol) << " " <<  std::quoted(event.pair) << std::endl;

                    // TODO: Check that OrderBook exists ???
                    const auto& book = orderBooksByTicker[event.symbol];

                    // TODO: Rename method
                    book->processEvent(event);
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

    void PricingEngine::push(common::Exchange exchange,
                             beast::flat_buffer* message)
    {
        // std::cout << "push (CPU: " << Utils::getCpu() << ") : " << jsonMessage << std::endl;
        books[static_cast<uint32_t>(exchange)]->push(message);
    }
}