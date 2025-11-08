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
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATA_HPP