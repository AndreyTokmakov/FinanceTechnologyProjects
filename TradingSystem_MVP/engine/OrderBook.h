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

#include "Exchange.h"
#include "Types.h"
#include "MarketData.h"

namespace engine
{
    using common::Pair;

    struct OrderBook
    {
        // TODO: pair ?? symbol ??
        Pair pair;

        // TODO: Rename method
        void processEvent(const market_data::Event& event) const;
    };
};

#endif //ORDERBOOK_H
