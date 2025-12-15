/**============================================================================
Name        : Formatting.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Formatting.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_FORMATTING_HPP
#define FINANCETECHNOLOGYPROJECTS_FORMATTING_HPP

#include <format>
#include <ostream>
#include "MarketData.hpp"

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

#endif //FINANCETECHNOLOGYPROJECTS_FORMATTING_HPP

