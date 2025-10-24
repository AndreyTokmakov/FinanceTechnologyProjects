/**============================================================================
Name        : PriceLevelBook.h
Created on  : 01.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceLevelBook.h
============================================================================**/

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Types.h"
#include "MarketData.h"

#include <boost/container/flat_map.hpp>

namespace engine
{
    using namespace common;

    struct PriceLevelBook
    {
        template<typename Comparator>
        using PriceLevel = boost::container::flat_map<Price, Quantity, Comparator>;

        // TODO: pair ?? symbol ??
        Pair pair;

        PriceLevel<std::greater<>> buyOrders;
        PriceLevel<std::less<>> sellOrders;

        // TODO: Rename method
        void processEvent(const market_data::Event& event);

    private:

        void handleDepthUpdate(const market_data::Depth& depthUpdate);
    };
};

#endif //ORDERBOOK_H
