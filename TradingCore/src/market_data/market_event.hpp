/**============================================================================
Name        : market_event.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data event representing the current top-of-book state.
============================================================================**/
/*
    MarketEvent represents a normalized market data event produced from the
    OrderBook and delivered to downstream components such as Strategy.

    The event contains the current best bid and best ask after processing a market data update.
    It is intentionally limited to top-of-book data for now and can be extended later when additional market data is required.

    Data Flow:

        Exchange
           |
           v
        MarketDataGateway
           |
           | BookUpdate
           v
        BookBuilder
           |
           v
        OrderBook
           |
           | current top-of-book
           v
        MarketEvent
           |
           v
        Strategy
           |
           v
        OrderRequest
           |
           v
        RiskEngine
           |
           v
        OrderManager

    MarketEvent is created by BookBuilder after a valid snapshot or market data update has been successfully applied to OrderBook.
    MarketEvent is consumed by downstream market data consumers, primarily Strategy.
    The event represents the state of the OrderBook after the corresponding update has been applied.

    Fields:
        instrument
            Identifier of the financial instrument this event belongs to.

        sequence
            Sequence number of the market data event. It is used to preserve
            the order of market data updates and detect missing events.

        exchangeTimestamp
            Timestamp assigned by the exchange when the market data event
            was generated.

        receiveTimestamp
            Local timestamp recorded when the event was received/processed
            by the application. The difference between receiveTimestamp and
            exchangeTimestamp can be used to estimate market data latency.

        bestBid
            Highest price currently available on the buy side of the order
            book.

        bestBidQuantity
            Available quantity at the best bid price.

        bestAsk
            Lowest price currently available on the sell side of the order
            book.

        bestAskQuantity
            Available quantity at the best ask price.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "timestamp.hpp"
#include "types.hpp"

namespace trading::market_data
{
    struct MarketEvent
    {
        InstrumentId instrument { 0 };
        SequenceNumber sequence { 0 };
        Timestamp exchangeTimestamp {};
        Timestamp receiveTimestamp {};
        Price bestBid {};
        Quantity bestBidQuantity {};
        Price bestAsk {};
        Quantity bestAskQuantity {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HPP