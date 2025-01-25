/**============================================================================
Name        : OrderBook.cpp
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.cpp
============================================================================**/

#include "OrderBook.h"
#include <iostream>


namespace OrderBook
{
    template<class LevelMap>
    void MarketDepthBook::addLevel(const Price price,
                                   const Quantity quantity,
                                   LevelMap& levelMap)
    {
        auto [iter, inserted] = levelMap.emplace(price, quantity);
        if (!inserted) {
            iter->second = quantity;
        }
    }

    template<class LevelMap>
    size_t MarketDepthBook::removeLevel(const Price price,
                                        LevelMap& levelMap)
    {
        return levelMap.erase(price);
    }

    void MarketDepthBook::addBidPrice(const Price price,
                                          const Quantity quantity) {
        addLevel(price, quantity, bidPriceLevelMap);
    }

    void MarketDepthBook::addAskPrice(const Price price,
                                          const Quantity quantity) {
        addLevel(price, quantity, askPriceLevelMap);
    }

    void MarketDepthBook::removeBidPrice(const Price price) {
        removeLevel(price, bidPriceLevelMap);
    }

    void MarketDepthBook::removeAskPrice(const Price price) {
        removeLevel(price, askPriceLevelMap);
    }
}


namespace OrderBook
{
    Engine::Engine(Common::Queue<Common::DepthEvent>& queue): eventQueue {queue} {
    }

    void Engine::start()
    {
        // FIXME
        eventHandlerThread = std::jthread(&Engine::handleEvents, this);
    }

    void Engine::handleEvents()
    {
        Common::DepthEvent event;
        while (true)
        {
            if (eventQueue.pop(event))
            {
                std::cout <<  (EventType::DepthSnapshot == event.type ? "DepthSnapshot" : "DepthUpdate")
                    << ": { "
                    << "symbol: "  << event.symbol
                    << ", id: "  << event.id
                    << ", firstUpdateId: "  << event.firstUpdateId
                    << ", lastUpdateId: "  << event.lastUpdateId
                    << ", asks: "  << event.akss.size()
                    << ", bids: " << event.bids.size() << " }\n";
            }
        }
    }
}