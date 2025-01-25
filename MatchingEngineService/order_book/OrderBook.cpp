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
    void MarketDepthBook::addLevel(const PriceLevel& priceLevel,
                                   LevelMap& levelMap)
    {
        auto [iter, inserted] = levelMap.emplace(priceLevel.price, priceLevel.quantity);
        if (!inserted) {
            iter->second = priceLevel.quantity;
        }
    }

    template<class LevelMap>
    size_t MarketDepthBook::removeLevel(const PriceLevel& priceLevel,
                                        LevelMap& levelMap)
    {
        return levelMap.erase(priceLevel.price);
    }

    void MarketDepthBook::addBidPrice(const PriceLevel& priceLevel) {
        addLevel(priceLevel, bidPriceLevelMap);
    }

    void MarketDepthBook::addAskPrice(const PriceLevel& priceLevel) {
        addLevel(priceLevel, askPriceLevelMap);
    }

    void MarketDepthBook::removeBidPrice(const PriceLevel& priceLevel) {
        removeLevel(priceLevel, bidPriceLevelMap);
    }

    void MarketDepthBook::removeAskPrice(const PriceLevel& priceLevel) {
        removeLevel(priceLevel, askPriceLevelMap);
    }

    void MarketDepthBook::applySnapshot(const Common::DepthEvent& snapshot)
    {
        for (const PriceLevel& bidLvl: snapshot.bids) {
            addBidPrice(bidLvl);
        }
        for (const PriceLevel &askLvl: snapshot.akss) {
            addAskPrice(askLvl);
        }
    }

    void MarketDepthBook::handleDepthUpdate(const Common::DepthEvent& snapshot)
    {
        for (const PriceLevel& bidLvl: snapshot.bids) {
            if (bidLvl.price != 0) {
                addBidPrice(bidLvl);
            } else {
                removeBidPrice(bidLvl);
            }
        }
        for (const PriceLevel &askLvl: snapshot.akss) {
            if (askLvl.price != 0) {
                addAskPrice(askLvl);
            } else {
                removeAskPrice(askLvl);
            }
        }
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
            if (!eventQueue.pop(event)) {
                continue;
            }

            if (EventType::DepthUpdate == event.type)
            {
                /*std::cout << "DepthUpdate : { " << "symbol: "  << event.symbol
                          << ", id: "  << event.id << ", firstUpdateId: "  << event.firstUpdateId
                          << ", lastUpdateId: "  << event.lastUpdateId
                          << ", asks: "  << event.akss.size() << ", bids: " << event.bids.size() << " }";*/

                if (DepthBookStatus::Ready == status){
                    depthBook.handleDepthUpdate(event);
                } else {
                    depthEvents.push_back(std::move(event));
                }

                // FIXME
                if (DepthBookStatus::Ready == status)
                {
                    const Price bidPrice = depthBook.bidPriceLevelMap.begin()->first;
                    const Price askPrice = depthBook.askPriceLevelMap.begin()->first;
                    const Price spread = askPrice - bidPrice;

                    std::cout << " Book [Buy: " << depthBook.bidPriceLevelMap.size()
                              << ", Ask: " << depthBook.askPriceLevelMap.size()
                              << ", Spread: " << bidPrice << ", " << askPrice << "." << spread
                              << "]\n";
                }
            }
            else if (EventType::DepthSnapshot == event.type && DepthBookStatus::NoReady == status)
            {
                depthBook.applySnapshot(event);
                for (const Common::DepthEvent& bufferedUpdateEven: depthEvents) {
                   if (bufferedUpdateEven.lastUpdateId > event.lastUpdateId)
                   {
                       depthBook.handleDepthUpdate(bufferedUpdateEven);
                       std::cout << "[ lastUpdateId:: " << bufferedUpdateEven.lastUpdateId
                                 << ", Snapshot UpdateID: " << event.lastUpdateId
                                 << "] " << (bufferedUpdateEven.lastUpdateId > event.lastUpdateId ? "OK" : "Ignore" ) << std::endl;
                   }
                }
                depthEvents.clear();
                status = DepthBookStatus::Ready;
            }
        }
    }
}