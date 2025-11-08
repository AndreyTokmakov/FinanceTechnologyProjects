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
#include <ostream>
#include <cstdint>

namespace binance::market_data
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

    struct JsonParams
    {
        static constexpr std::string_view data { "data" };
        static constexpr std::string_view pair { "ps" };
        static constexpr std::string_view symbol { "s" };
        static constexpr std::string_view eventType { "e" };
        static constexpr std::string_view eventTime { "E" };
        static constexpr std::string_view priceChange { "p" };
        static constexpr std::string_view priceChangePercent { "P" };
        static constexpr std::string_view lastPrice { "c" };
        static constexpr std::string_view lastQuantity { "Q" };
        static constexpr std::string_view openPrice { "o" };
        static constexpr std::string_view highPrice { "h" };
        static constexpr std::string_view lowPrice { "l" };
        static constexpr std::string_view totalTradedVolume { "v" };
        static constexpr std::string_view totalTradedBaseAssetVolume{ "q" };
        static constexpr std::string_view firstTradeId { "F" };
        static constexpr std::string_view lastTradeId { "L" };
        static constexpr std::string_view totalTradesNumber { "n" };

        /** Depth **/
        static constexpr std::string_view firstUpdateId { "U" };
        static constexpr std::string_view finalUpdateId { "u" };
        static constexpr std::string_view bids { "b" };
        static constexpr std::string_view asks { "a" };
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
        std::string symbol {};
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
        std::string symbol;
        Price    bidPrice { 0.0 };
        Quantity bidQuantity { 0.0 };
        Price    askPrice { 0.0 };
        Quantity askQuantity { 0.0 };
        uint64_t updateId { 0 };
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
        return std::format_to(ctx.out(),
            "MiniTicker(""\n\tsymbol: {},\n\ttimestamp: {}, \n\tclose: {},\n\topen: {},\n\thigh: {},\n\tlow: {},\n\tvolume: {},\n\tquantity: {}\n)",
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
        return std::format_to(ctx.out(),
            "BookTicker(\n\tsymbol: {},\n\tbidPrice: {}, \n\tbidQuantity: {},\n\taskPrice: {},\n\taskQuantity: {},\n\tupdateId: {}\n)",
            bookTicker.symbol,
            bookTicker.bidPrice,
            bookTicker.bidQuantity,
            bookTicker.askPrice,
            bookTicker.askQuantity,
            bookTicker.updateId);
    }
};

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::BookTicker& ticker) {
    return stream << std::format("{}", ticker);
}

inline std::ostream& operator<<(std::ostream& stream, const binance::market_data::MiniTicker& ticker) {
    return stream << std::format("{}", ticker);
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP