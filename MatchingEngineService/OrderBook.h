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

    struct MarketDepthBook
    {
        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::greater<>> bidPriceLevelMap;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::less<>> askPriceLevelMap;

        template<class LevelMap>
        void addLevel(Price price,
                      Quantity quantity,
                      LevelMap& levelMap);

        template<class LevelMap>
        size_t removeLevel(Price price,
                           LevelMap& levelMap);

        void addBidPrice(Price price, Quantity quantity) ;
        void addAskPrice(Price price, Quantity quantity);

        void removeBidPrice(Price price);
        void removeAskPrice(Price price);
    };

    struct Engine
    {
        Common::Queue<Common::DepthEvent>& eventQueue;
        std::jthread eventHandlerThread;

        explicit Engine(Common::Queue<Common::DepthEvent>& queue);

        void start();
        void handleEvents();
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDERBOOK_H
