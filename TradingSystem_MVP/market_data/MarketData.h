/**============================================================================
Name        : MarketData.h
Created on  : 27.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketData.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H
#define FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H

#include <iostream>
#include <vector>

namespace market_data
{
    using Price     = double;
    using Quantity  = double;
    using Timestamp = int64_t;
    using Number    = int64_t;

    // TODO: ---> implement StaticString (stack only)
    using String = std::string;

    struct Result
    {
        int id { 0 };
    };

    enum class EventType: uint8_t
    {
        None,
        Result,
        Ticker,
        MiniTicker,
        BookTicker,
        Trade,
        AggTrade,
        MarkPrice,
        DepthUpdate
    };

    std::ostream& operator<<(std::ostream& stream, const EventType& eventType);

    struct Ticker
    {
        Price priceChange {};
        Price priceChangePercent {};
        Price lastPrice {};
        Price lastQuantity {};
        Price openPrice {};
        Price highPrice {};
        Price lowPrice {};

        Quantity totalTradedVolume {};
        Quantity totalTradedBaseAssetVolume {};

        Number firstTradeId {};
        Number lastTradeId {};
        Number totalTradesNumber {};
    };

    struct Depth
    {
        struct PriceUpdate
        {
            Number price { 0 };
            Number quantity { 0 };
        };

        Number firstUpdateId { 0 };
        Number finalUpdateId { 0 };
        std::vector<PriceUpdate> bid;
        std::vector<PriceUpdate> ask;
    };

    struct Event
    {
        EventType type { EventType::None };
        String symbol;
        String pair;
        Timestamp eventTime { 0 };

        Ticker ticker;
        Result result;
        Depth depth;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H
