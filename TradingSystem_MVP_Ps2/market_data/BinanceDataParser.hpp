/**============================================================================
Name        : BinanceDataParser.hpp
Created on  : 15.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BinanceDataParser.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_DATA_PARSER_HPP

#include "MarketData.hpp"
#include <nlohmann/json.hpp>

namespace BinanceParserJson
{
    market_data::binance::Trade        parseTrade(const nlohmann::json& data);
    market_data::binance::AggTrade     parseAggTrade(const nlohmann::json& data);
    market_data::binance::MiniTicker   parseMiniTicker(const nlohmann::json& data);
    market_data::binance::BookTicker   parseBookTicker(const nlohmann::json& data);
    market_data::binance::DepthUpdate  parseDepthUpdate(const nlohmann::json& data);
    market_data::binance::BookSnapshot parseBookSnapshot(const nlohmann::json& data);
    market_data::binance::Ticker       parseTicker(const nlohmann::json& data);
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_DATA_PARSER_HPP