/**============================================================================
Name        : OrderBook.h
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDERBOOK_H
#define FINANCETECHNOLOGYPROJECTS_ORDERBOOK_H

#include <thread>

#include "Event.h"
#include "Queue.h"
#include <boost/container/flat_map.hpp>

namespace OrderBook
{
    using namespace Common;

    enum class Side : uint8_t
    {
        Buy,
        Sell,
    };

    enum class DepthBookStatus : uint8_t
    {
        Ready,
        NoReady,
    };

    struct MarketDepthBook
    {
        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::greater<>> bidPriceLevelMap;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::less<>> askPriceLevelMap;

        template<class LevelMap>
        void addLevel(const PriceLevel& priceLevel,
                      LevelMap& levelMap);

        template<class LevelMap>
        size_t removeLevel(const PriceLevel& priceLevel,
                           LevelMap& levelMap);

        void addBidPrice(const PriceLevel& priceLevel);
        void addAskPrice(const PriceLevel& priceLevel);

        void removeBidPrice(const PriceLevel& priceLevel);
        void removeAskPrice(const PriceLevel& priceLevel);

        void applySnapshot(const Common::DepthEvent& snapshot);
        void handleDepthUpdate(const Common::DepthEvent& snapshot);
    };

    struct Engine
    {
        Common::Queue<Common::DepthEvent>& eventQueue;
        std::jthread eventHandlerThread;

        DepthBookStatus status { DepthBookStatus::NoReady };
        MarketDepthBook depthBook;
        std::vector<Common::DepthEvent> depthEvents;

        explicit Engine(Common::Queue<Common::DepthEvent>& queue);

        void start();
        void handleEvents();
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDERBOOK_H
