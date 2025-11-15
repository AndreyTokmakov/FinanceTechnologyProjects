/**============================================================================
Name        : Parser.cpp
Created on  : 15.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parser.cpp
============================================================================**/

#include "Parser.hpp"

namespace
{
    constexpr inline double asDouble(const nlohmann::json& data){
        return std::stod(data.get_ref<const std::string&>());
    };
}

namespace Parser
{
    using market_data::binance::JsonParams;

    market_data::binance::Trade parseTrade(const nlohmann::json& data)
    {
        market_data::binance::Trade trade;
        data.at(JsonParams::symbol).get_to(trade.symbol);
        data.at(JsonParams::eventTime).get_to(trade.eventTime);
        data.at(JsonParams::Trade::tradeId).get_to(trade.tradeId);
        // data.at(JsonParams::Trade::buyerOrderID).get_to(trade.buyerOrderId);
        // data.at(JsonParams::Trade::sellerOrderId).get_to(trade.sellerOrderId);
        data.at(JsonParams::Trade::tradeTime).get_to(trade.tradeTime);
        data.at(JsonParams::Trade::isMarketMaker).get_to(trade.isBuyerMaker);
        trade.price    = asDouble(data.at(JsonParams::Trade::price));
        trade.quantity = asDouble(data.at(JsonParams::Trade::quantity));
        return trade;
    }

    market_data::binance::AggTrade parseAggTrade(const nlohmann::json& data)
    {
        market_data::binance::AggTrade aggTrade;
        data.at(JsonParams::symbol).get_to(aggTrade.symbol);
        data.at(JsonParams::eventTime).get_to(aggTrade.eventTime);
        data.at(JsonParams::AggTrade::aggregateTradeId).get_to(aggTrade.aggregateTradeId);
        data.at(JsonParams::AggTrade::firstTradeId).get_to(aggTrade.firstTradeId);
        data.at(JsonParams::AggTrade::lastTradeId).get_to(aggTrade.lastTradeId);
        data.at(JsonParams::AggTrade::tradeTime).get_to(aggTrade.tradeTime);
        data.at(JsonParams::AggTrade::isMarketMaker).get_to(aggTrade.isBuyerMaker);
        aggTrade.price    = asDouble(data.at(JsonParams::AggTrade::price));
        aggTrade.quantity = asDouble(data.at(JsonParams::AggTrade::quantity));
        return aggTrade;
    }

    market_data::binance::MiniTicker parseMiniTicker(const nlohmann::json& data)
    {
        market_data::binance::MiniTicker ticker;
        data.at(JsonParams::eventTime).get_to(ticker.timestamp);
        data.at(JsonParams::symbol).get_to(ticker.symbol);
        ticker.close    = asDouble(data.at("c"));
        ticker.open     = asDouble(data.at("o"));
        ticker.high     = asDouble(data.at("h"));
        ticker.low      = asDouble(data.at("l"));
        ticker.volume   = asDouble(data.at("v"));
        ticker.quantity = asDouble(data.at("q"));
        return ticker;
    }

    market_data::binance::BookTicker parseBookTicker(const nlohmann::json& data)
    {
        market_data::binance::BookTicker ticker;
        data.at(JsonParams::symbol).get_to(ticker.symbol);
        ticker.bidPrice    = asDouble(data.at(JsonParams::BookTicker::bestBuyPrice));
        ticker.bidQuantity = asDouble(data.at(JsonParams::BookTicker::bestBuyQuantity));
        ticker.askPrice    = asDouble(data.at(JsonParams::BookTicker::bestAskPrice));
        ticker.askQuantity = asDouble(data.at(JsonParams::BookTicker::bestAskQuantity));
        data.at(JsonParams::BookTicker::orderBookUpdateId).get_to(ticker.updateId);
        return ticker;
    }

    market_data::binance::DepthUpdate parseDepthUpdate(const nlohmann::json& data)
    {
        market_data::binance::DepthUpdate update;
        data.at(JsonParams::symbol).get_to(update.symbol);
        data.at(JsonParams::eventTime).get_to(update.eventTime);
        data.at(JsonParams::DepthUpdate::firstUpdateId).get_to(update.firstUpdateId);
        data.at(JsonParams::DepthUpdate::finalUpdateId).get_to(update.finalUpdateId);

        const nlohmann::json& bids = data[JsonParams::DepthUpdate::bids];
        update.bids.reserve(bids.size());
        for (const auto& lvl: bids) {
            update.bids.emplace_back(asDouble(lvl[0]), asDouble(lvl[1]));
        }

        const nlohmann::json& asks = data[JsonParams::DepthUpdate::asks];
        update.asks.reserve(asks.size());
        for (const auto& lvl: asks) {
            update.asks.emplace_back(asDouble(lvl[0]), asDouble(lvl[1]));
        }

        return update;
    }

    market_data::binance::BookSnapshot parseBookSnapshot(const nlohmann::json& data)
    {
        market_data::binance::BookSnapshot bookSnapshot;
        data.at(JsonParams::BookSnapshot::lastUpdateId).get_to(bookSnapshot.lastUpdateId);

        const nlohmann::json& bids = data[JsonParams::BookSnapshot::bids];
        bookSnapshot.bids.reserve(bids.size());
        for (const auto& lvl: bids) {
            bookSnapshot.bids.emplace_back(asDouble(lvl[0]), asDouble(lvl[1]));
        }

        const nlohmann::json& asks = data[JsonParams::BookSnapshot::asks];
        bookSnapshot.asks.reserve(asks.size());
        for (const auto& lvl: asks) {
            bookSnapshot.asks.emplace_back(asDouble(lvl[0]), asDouble(lvl[1]));
        }

        return bookSnapshot;
    }

    market_data::binance::Ticker parseTicker(const nlohmann::json& data)
    {
        market_data::binance::Ticker ticker;

        data.at(JsonParams::symbol).get_to(ticker.symbol);
        data.at(JsonParams::eventTime).get_to(ticker.eventTime);
        data.at(JsonParams::eventType).get_to(ticker.eventType);
        ticker.priceChange = asDouble(data.at(JsonParams::Ticker::priceChange));
        ticker.priceChangePercent = asDouble(data.at(JsonParams::Ticker::priceChangePercent));
        ticker.weightedAvgPrice = asDouble(data.at(JsonParams::Ticker::weightedAveragePrice));
        ticker.prevClosePrice = asDouble(data.at(JsonParams::Ticker::previousClose));
        ticker.lastPrice = asDouble(data.at(JsonParams::Ticker::lastPrice));
        ticker.lastQuantity = asDouble(data.at(JsonParams::Ticker::lastQuantity));
        ticker.bestBid = asDouble(data.at(JsonParams::Ticker::bestBuyPrice));
        ticker.bestBidQuantity = asDouble(data.at(JsonParams::Ticker::bestBuyQuantity));
        ticker.bestAsk = asDouble(data.at(JsonParams::Ticker::bestAskPrice));
        ticker.bestAskQuantity = asDouble(data.at(JsonParams::Ticker::bestAskQuantity));
        ticker.openPrice = asDouble(data.at(JsonParams::Ticker::openPrice));
        ticker.highPrice = asDouble(data.at(JsonParams::Ticker::highPrice));
        ticker.lowPrice = asDouble(data.at(JsonParams::Ticker::lowPrice));
        ticker.volume = asDouble(data.at(JsonParams::Ticker::totalTradedVolume));
        ticker.quoteVolume = asDouble(data.at(JsonParams::Ticker::totalTradedBaseAssetVolume));
        data.at(JsonParams::Ticker::statisticsOpenTime).get_to(ticker.openTime);
        data.at(JsonParams::Ticker::statisticsCloseTime).get_to(ticker.closeTime);
        data.at(JsonParams::Ticker::firstTradeId).get_to(ticker.firstTradeId);
        data.at(JsonParams::Ticker::lastTradeId).get_to(ticker.lastTradeId);
        data.at(JsonParams::Ticker::totalTradesNumber).get_to(ticker.numTrades);

        return ticker;
    }
}