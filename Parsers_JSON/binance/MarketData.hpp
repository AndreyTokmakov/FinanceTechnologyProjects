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
#include <cstdint>

namespace binance::market_data
{
    using Price     = double;
    using Quantity  = double;
    using Volume    = double;
    using Timestamp = uint64_t;
    using Number    = uint64_t;

    // TODO: ---> implement StaticString (stack only)
    using String = std::string;

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
            static constexpr std::string_view eventType { "e" };
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

namespace binance::market_data
{
    /*{
        "e": "24hrMiniTicker",  // Event type
        "E": 1672515782136,     // Event time
        "s": "BNBBTC",          // Symbol
        "c": "0.0025",          // Close price
        "o": "0.0010",          // Open price
        "h": "0.0025",          // High price
        "l": "0.0010",          // Low price
        "v": "10000",           // Total traded base asset volume
        "q": "18"               // Total traded quote asset volume
    }*/

    struct MiniTicker
    {
        std::string symbol {}; //               <---------- REMOVE
        uint64_t timestamp { 0 };
        Price    close { 0.0 };
        Price    open { 0.0 };
        Price    high { 0.0 };
        Price    low { 0.0 };
        double   volume { 0.0 };
        Quantity quantity { 0.0 };
    };

    /*{
      "u":400900217,     // order book updateId
      "s":"BNBUSDT",     // symbol
      "b":"25.35190000", // best bid price
      "B":"31.21000000", // best bid qty
      "a":"25.36520000", // best ask price
      "A":"40.66000000"  // best ask qty
    }*/

    struct BookTicker
    {
        std::string symbol {}; //               <---------- REMOVE
        Price    bidPrice { 0.0 };
        Quantity bidQuantity { 0.0 };
        Price    askPrice { 0.0 };
        Quantity askQuantity { 0.0 };
        uint64_t updateId { 0 };
    };

    /** {
      "e": "depthUpdate",   // Event type
      "E": 1700000000123,   // Event time
      "s": "BTCUSDT",       // Symbol
      "U": 400900200,       // First update ID in event
      "u": 400900210,       // Final update ID in event
      "b": [ ["67320.10","0.002"], ["67319.80","0.500"] ],  // Bids
      "a": [ ["67321.00","0.100"], ["67321.10","0.000"] ]   // Asks
    } **/

    struct PriceLevel
    {
        Price price { 0.0 };
        Quantity quantity { 0.0 };
    };

    struct DepthUpdate
    {
        std::string symbol {}; //               <---------- REMOVE
        Timestamp firstUpdateId { 0 };
        Timestamp finalUpdateId { 0 };
        Timestamp eventTime { 0 };
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
    };

    /** {
      "e": "24hrTicker",     // Event type
      "E": 1700000000123,    // Event time
      "s": "BTCUSDT",        // Symbol
      "p": "120.45",         // Price change
      "P": "0.18",           // Price change percent
      "w": "67500.12",       // Weighted average price
      "x": "67400.00",       // Previous close
      "c": "67520.00",       // Last price
      "Q": "0.003",          // Last quantity
      "b": "67519.50",       // Best bid price
      "B": "1.253",          // Best bid quantity
      "a": "67520.10",       // Best ask price
      "A": "0.856",          // Best ask quantity
      "o": "67300.00",       // Open price
      "h": "67600.00",       // High price
      "l": "67100.00",       // Low price
      "v": "1234.567",       // Total traded base asset volume
      "q": "83200000.55",    // Total traded quote asset volume
      "O": 1699999999000,    // Statistics open time
      "C": 1700000000000,    // Statistics close time
      "F": 100,              // First trade ID
      "L": 200,              // Last trade ID
      "n": 100               // Total trades
    } **/
    struct Ticker
    {
        std::string symbol;
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
}

template<>
struct std::formatter<binance::market_data::MiniTicker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const binance::market_data::MiniTicker& miniTicker, std::format_context& ctx) -> decltype(auto)
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
struct std::formatter<binance::market_data::BookTicker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const binance::market_data::BookTicker& bookTicker, std::format_context& ctx) -> decltype(auto)
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
struct std::formatter<binance::market_data::PriceLevel>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const binance::market_data::PriceLevel& lvl, std::format_context& ctx) -> decltype(auto) {
        return std::format_to(ctx.out(),"({}, {})", lvl.price,lvl.quantity);
    }
};

template<>
struct std::formatter<binance::market_data::DepthUpdate>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const binance::market_data::DepthUpdate& update, std::format_context& ctx) -> decltype(auto)
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
struct std::formatter<binance::market_data::Ticker>
{
    static constexpr auto parse(const std::format_parse_context& ctx) -> decltype(auto) {
        return ctx.begin();
    }

    static auto format(const binance::market_data::Ticker& ticker, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "Ticker(\n\tsymbol: {},\n\teventTime: {},\n\tpriceChange: {},"
            "\n\tpriceChangePercent: {},\n\tweightedAvgPrice: {},\n\tprevClosePrice: {},"
            "\n\tlastPrice: {},\n\tlastQuantity: {},\n\tbestBid: {},\n\tbestBidQuantity: {},"
            "\n\tbestAskQuantity: {},\n\topenPrice: {},\n\thighPrice: {},\n\tlowPrice: {},"
            "\n\tvolume: {},\n\tquoteVolume: {},\n\topenTime: {},\n\tcloseTime: {},\n\tfirstTradeId: {},"
            "\n\tlastTradeId: {},\n\tnumTrades: {}\n)",
            ticker.symbol,
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

    /*
    static auto format(const binance::market_data::Ticker& ticker, std::format_context& ctx) -> decltype(auto)
    {
        return std::format_to(ctx.out(), "Ticker("
            "\n\tsymbol: {}",
            "\n\teventTime: {}",
            "\n\tpriceChange: {}",
            "\n\tpriceChangePercent: {}",
            "\n\tweightedAvgPrice: {}",
            "\n\tprevClosePrice: {}",
            "\n\tlastPrice: {}",
            "\n\tlastQuantity: {}",
            "\n\tbestBid: {}",
            "\n\tbestBidQuantity: {}",
            "\n\tbestAskQuantity: {}",
            "\n\topenPrice: {}",
            "\n\thighPrice: {}",
            "\n\tlowPrice: {}",
            "\n\tvolume: {}",
            "\n\tquoteVolume: {}",
            "\n\topenTime: {}",
            "\n\tcloseTime: {}",
            "\n\tfirstTradeId: {}",
            "\n\tlastTradeId: {}",
            "\n\tnumTrades: {}\n)",
            ticker.symbol,
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
    }*/
};

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::BookTicker& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::MiniTicker& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::DepthUpdate& item) {
    return stream << std::format("{}", item);
}

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::Ticker& item) {
    return stream << std::format("{}", item);
}


#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP
