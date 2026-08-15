/**============================================================================
Name        : market_event.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_event.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "timestamp.hpp"
#include "types.hpp"

#include <cstdint>

namespace trading::market_data
{
    // Это пока минимальное событие, которое может передаваться от OrderBook дальше в Strategy.
    struct MarketEvent
    {
        InstrumentId instrument;
        SequenceNumber sequence;
        Timestamp exchangeTimestamp;
        Timestamp receiveTimestamp;
        Price bestBid;
        Quantity bestBidQuantity;
        Price bestAsk;
        Quantity bestAskQuantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP
