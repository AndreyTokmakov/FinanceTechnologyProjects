/**============================================================================
Name        : MarketData.hpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP

#include <string>
#include <string_view>
#include <vector>
#include <variant>


#include "Common.hpp"

namespace market_data::binance
{
    using Price      = common::Price;
    using Quantity   = common::Quantity;
    using Volume     = common::Volume;
    using Number     = common::Number;
    using Timestamp  = common::Timestamp;
    using PriceLevel = common::PriceLevel;

    // TODO: ---> implement StaticString (stack only)
    using Symbol = std::string;

    struct JsonParams
    {
        static constexpr std::string_view data { "data" };
        static constexpr std::string_view stream { "stream" };
        static constexpr std::string_view symbol { "s" };
        static constexpr std::string_view quantity{ "q" };
        static constexpr std::string_view eventType { "e" };
        static constexpr std::string_view eventTime { "E" };
        static constexpr std::string_view pair { "ps" };

        struct Ticker
        {
            static constexpr std::string_view weightedAveragePrice { "w" };
            static constexpr std::string_view previousClose { "x" };
            static constexpr std::string_view statisticsOpenTime { "O" };
            static constexpr std::string_view statisticsCloseTime { "C" };
            static constexpr std::string_view totalTrades { "n" };
            static constexpr std::string_view priceChange { "p" };
            static constexpr std::string_view priceChangePercent { "P" };
            static constexpr std::string_view lastQuantity { "Q" };
            static constexpr std::string_view bestAskPrice { "a" };
            static constexpr std::string_view bestAskQuantity { "A" };
            static constexpr std::string_view bestBuyPrice { "b" };
            static constexpr std::string_view bestBuyQuantity { "B" };
            static constexpr std::string_view lastPrice { "c" };
            static constexpr std::string_view openPrice { "o" };
            static constexpr std::string_view highPrice { "h" };
            static constexpr std::string_view lowPrice { "l" };
            static constexpr std::string_view totalTradedVolume { "v" };
            static constexpr std::string_view totalTradedBaseAssetVolume{ "q" };
            static constexpr std::string_view firstTradeId { "F" };
            static constexpr std::string_view lastTradeId { "L" };
            static constexpr std::string_view totalTradesNumber { "n" };
        };

        struct BookTicker
        {
            static constexpr std::string_view bestBuyPrice { "b" };
            static constexpr std::string_view bestBuyQuantity { "B" };
            static constexpr std::string_view bestAskPrice { "a" };
            static constexpr std::string_view bestAskQuantity { "A" };
            static constexpr std::string_view orderBookUpdateId { "u" };
        };

        struct DepthUpdate
        {
            static constexpr std::string_view firstUpdateId { "U" };
            static constexpr std::string_view finalUpdateId { "u" };
            static constexpr std::string_view bids { "b" };
            static constexpr std::string_view asks { "a" };
        };

        struct Trade
        {
            static constexpr std::string_view price { "p" };
            static constexpr std::string_view quantity { "q" };
            static constexpr std::string_view tradeTime { "T" };
            static constexpr std::string_view tradeId { "t" };
            static constexpr std::string_view buyerOrderID { "b" };
            static constexpr std::string_view sellerOrderId { "a" };
            static constexpr std::string_view isMarketMaker { "m" };
        };

        struct AggTrade
        {
            static constexpr std::string_view price { "p" };
            static constexpr std::string_view quantity { "q" };
            static constexpr std::string_view tradeTime { "T" };
            static constexpr std::string_view aggregateTradeId { "a" };
            static constexpr std::string_view firstTradeId { "f" };
            static constexpr std::string_view lastTradeId { "l" };
            static constexpr std::string_view isMarketMaker { "m" };
        };

        struct BookSnapshot
        {
            static constexpr std::string_view lastUpdateId { "lastUpdateId" };
            static constexpr std::string_view bids { "bids" };
            static constexpr std::string_view asks { "asks" };
        };
    };

    struct EventTypeNames
    {
        static constexpr std::string_view depthUpdate { "depthUpdate" };
        static constexpr std::string_view ticker { "24hrTicker" };
        static constexpr std::string_view miniTicker { "24hrMiniTicker" };
        static constexpr std::string_view aggTrade { "aggTrade" };
        static constexpr std::string_view trade { "trade" };
    };

    struct StreamNames
    {
        static constexpr std::string_view trade { "trade" };
        static constexpr std::string_view aggTrade { "aggTrade" };
        static constexpr std::string_view ticker { "ticker" };
        static constexpr std::string_view bookTicker { "bookTicker" };
        static constexpr std::string_view depth { "depth" };
        static constexpr std::string_view depth20 { "depth20" };
        static constexpr std::string_view kline_1m { "kline_1m" };
        static constexpr std::string_view miniTicker { "miniTicker" };
    };
}

namespace market_data::binance
{
    struct MiniTicker
    {
        Symbol    symbol {}; //               <---------- REMOVE
        Timestamp timestamp { 0 };
        Price     close { 0.0 };
        Price     open { 0.0 };
        Price     high { 0.0 };
        Price     low { 0.0 };
        Volume    volume { 0.0 };
        Quantity  quantity { 0.0 };
    };

    struct BookTicker
    {
        Symbol   symbol {}; //               <---------- REMOVE
        Price    bidPrice { 0.0 };
        Quantity bidQuantity { 0.0 };
        Price    askPrice { 0.0 };
        Quantity askQuantity { 0.0 };
        Number   updateId { 0 };
    };

    struct DepthUpdate
    {
        Symbol symbol {}; //               <---------- REMOVE
        Timestamp firstUpdateId { 0 };
        Timestamp finalUpdateId { 0 };
        Timestamp eventTime { 0 };
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
    };

    struct BookSnapshot
    {
        Timestamp lastUpdateId { 0 };
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
    };

    struct Ticker
    {
        Symbol symbol;
        std::string eventType;
        Timestamp eventTime { 0 };
        Price priceChange   { 0.0 };
        Price priceChangePercent { 0.0 };
        Price weightedAvgPrice   { 0.0 };
        Price prevClosePrice     { 0.0 };
        Price lastPrice { 0.0 };
        Quantity lastQuantity { 0.0 };
        Price bestBid { 0.0 };
        Quantity bestBidQuantity { 0.0 };
        Price bestAsk { 0.0 };
        Quantity bestAskQuantity { 0.0 };
        Price openPrice { 0.0 };
        Price highPrice { 0.0 };
        Price lowPrice  { 0.0 };
        Volume volume   { 0.0 };
        Volume quoteVolume  { 0.0 };
        Timestamp openTime  { 0 };
        Timestamp closeTime { 0 };
        Number firstTradeId { 0 };
        Number lastTradeId  { 0 };
        Number numTrades    { 0 };
    };

    struct Trade
    {
        Symbol symbol;
        Timestamp eventTime { 0 };
        Number tradeId { 0 };
        Price price { 0.0 };
        Quantity quantity { 0.0 };
        Number buyerOrderId { 0 };
        Number sellerOrderId { 0 };
        Timestamp tradeTime { 0 };
        bool isBuyerMaker { false };
    };

    struct AggTrade
    {
        Symbol symbol;
        Timestamp eventTime { 0 };
        Number aggregateTradeId { 0 };
        Price price { 0.0 };
        Quantity quantity { 0.0 };
        Number firstTradeId { 0 };
        Number lastTradeId { 0 };
        Timestamp tradeTime { 0 };
        bool isBuyerMaker { false };
    };

    struct NoYetImplemented {
        std::string streamName {};
    };

    using BinanceMarketEvent = std::variant<
        BookTicker,
        MiniTicker,
        Trade,
        AggTrade,
        DepthUpdate,
        NoYetImplemented
    >;

}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP
