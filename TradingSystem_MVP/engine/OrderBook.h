/**============================================================================
Name        : OrderBook.h
Created on  : 01.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.h
============================================================================**/

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Types.h"
#include "MarketData.h"

#include <boost/container/flat_map.hpp>

namespace engine
{
    using namespace common;

    // TODO: Rename to Pricer ?? PriceEngine ??
    struct OrderBook
    {
        // TODO: pair ?? symbol ??
        Pair pair;

        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::greater<>> bidPrices;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::less<>> askPrices;

        // TODO: Rename method
        void processEvent(const market_data::Event& event);

    private:

        void handleDepthUpdate(const market_data::Depth& depthUpdate);

    };
};

#endif //ORDERBOOK_H
