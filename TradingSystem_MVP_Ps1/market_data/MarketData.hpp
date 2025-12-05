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

#include <string_view>
#include <format>
#include <vector>
#include <ostream>

#include "Types.hpp"

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

    struct Result
    {
        int id { 0 };
    };

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
}

template<>
struct std::formatter<market_data::binance::Trade>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::Trade& trade, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "Trade(\n\tsymbol: {},\n\teventTime: {}, \n\ttradeId: {},"
        "\n\tprice: {},\n\tquantity: {},\n\tbuyerOrderId: {},\n\tsellerOrderId: {},"
        "\n\ttradeTime: {},\n\tisBuyerMaker: {}\n)",
            trade.symbol,
            trade.eventTime,
            trade.tradeId,
            trade.price,
            trade.quantity,
            trade.buyerOrderId,
            trade.sellerOrderId,
            trade.tradeTime,
            trade.isBuyerMaker);
    }
};
template<>
struct std::formatter<market_data::binance::AggTrade>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::AggTrade& aggTrade, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "Trade(\n\tsymbol: {},\n\teventTime: {}, \n\taggregateTradeId: {},"
        "\n\tprice: {},\n\tquantity: {},\n\tfirstTradeId: {},\n\tlastTradeId: {},"
        "\n\ttradeTime: {},\n\tisBuyerMaker: {}\n)",
            aggTrade.symbol,
            aggTrade.eventTime,
            aggTrade.aggregateTradeId,
            aggTrade.price,
            aggTrade.quantity,
            aggTrade.firstTradeId,
            aggTrade.lastTradeId,
            aggTrade.tradeTime,
            aggTrade.isBuyerMaker);
    }
};


template<>
struct std::formatter<market_data::binance::MiniTicker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::MiniTicker& miniTicker, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "MiniTicker(\n\tsymbol: {},\n\ttimestamp: {}, \n\tclose: {},"
            "\n\topen: {},\n\thigh: {},\n\tlow: {},\n\tvolume: {},\n\tquantity: {}\n)",
            miniTicker.symbol,
            miniTicker.timestamp,
            miniTicker.close,
            miniTicker.open,
            miniTicker.high,
            miniTicker.low,
            miniTicker.volume,
            miniTicker.quantity);
    }
};

template<>
struct std::formatter<market_data::binance::BookTicker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::BookTicker& bookTicker, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(),"BookTicker(\n\tsymbol: {},\n\tbidPrice: {}, \n\tbidQuantity: {},"
            "\n\taskPrice: {},\n\taskQuantity: {},\n\tupdateId: {}\n)",
            bookTicker.symbol,
            bookTicker.bidPrice,
            bookTicker.bidQuantity,
            bookTicker.askPrice,
            bookTicker.askQuantity,
            bookTicker.updateId);
    }
};

template<>
struct std::formatter<market_data::binance::PriceLevel>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::PriceLevel& lvl, std::format_context& ctx) -> decltype(auto) {
        return std::format_to(ctx.out(),"({}, {})", lvl.price,lvl.quantity);
    }
};

template<>
struct std::formatter<market_data::binance::DepthUpdate>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::DepthUpdate& update, std::format_context& ctx) -> decltype(auto)
    {
        std::string strAsks;
        for (const auto&[price, quantity]: update.asks) {
            strAsks += std::format("({}, {})", price,quantity) + " ";
        }
        std::string strBids;
        for (const auto&[price, quantity]: update.bids) {
            strBids += std::format("({}, {})", price,quantity) + " ";
        }

        return std::format_to(ctx.out(),"DepthUpdate(\n\tsymbol: {},\n\tfirstUpdateId: {}, "
            "\n\tfinalUpdateId: {},\n\teventTime: {},\n\tbids: [ {}],\n\tasks: [ {}]\n)",
            update.symbol,
            update.firstUpdateId,
            update.finalUpdateId,
            update.eventTime,
            strAsks,
            strBids);
    }
};

template<>
struct std::formatter<market_data::binance::BookSnapshot>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::BookSnapshot& bookSnapshot, std::format_context& ctx) -> decltype(auto)
    {
        std::string strAsks;
        for (const auto&[price, quantity]: bookSnapshot.asks) {
            strAsks += std::format("({}, {})", price,quantity) + " ";
        }
        std::string strBids;
        for (const auto&[price, quantity]: bookSnapshot.bids) {
            strBids += std::format("({}, {})", price,quantity) + " ";
        }

        return std::format_to(ctx.out(),"BookSnapshot(\n\tlastUpdateId: {},""\n\tbids: [ {}],\n\tasks: [ {}]\n)",
             bookSnapshot.lastUpdateId,
             strBids,
             strAsks);
    }
};


template<>
struct std::formatter<market_data::binance::Ticker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const market_data::binance::Ticker& ticker, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "Ticker("
             "\n\tsymbol: {},\n\teventType: {},\n\teventTime: {},\n\tpriceChange: {},"
            "\n\tpriceChangePercent: {},\n\tweightedAvgPrice: {},\n\tprevClosePrice: {},"
            "\n\tlastPrice: {},\n\tlastQuantity: {},\n\tbestBid: {},\n\tbestBidQuantity: {},"
            "\n\tbestAskQuantity: {},\n\topenPrice: {},\n\thighPrice: {},\n\tlowPrice: {},"
            "\n\tvolume: {},\n\tquoteVolume: {},\n\topenTime: {},\n\tcloseTime: {},\n\tfirstTradeId: {},"
            "\n\tlastTradeId: {},\n\tnumTrades: {}\n)",
            ticker.symbol,
            ticker.eventType,
            ticker.eventTime,
            ticker.priceChange,
            ticker.priceChangePercent,
            ticker.weightedAvgPrice,
            ticker.prevClosePrice,
            ticker.lastPrice,
            ticker.lastQuantity,
            ticker.bestBid,
            ticker.bestBidQuantity,
            ticker.bestAskQuantity,
            ticker.openPrice,
            ticker.highPrice,
            ticker.lowPrice,
            ticker.volume,
            ticker.quoteVolume,
            ticker.openTime,
            ticker.closeTime,
            ticker.firstTradeId,
            ticker.lastTradeId,
            ticker.numTrades);
    }
};

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::BookTicker& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::MiniTicker& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::DepthUpdate& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::BookSnapshot& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::Ticker& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::Trade& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const market_data::binance::AggTrade& item) {
    return stream << std::format("{}", item);
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP
