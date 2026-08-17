/**============================================================================
Name        : binance_market_data_parser.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data parser implementation.
============================================================================**/

#include "binance_market_data_parser.hpp"

namespace trading::exchanges::binance
{
    BinanceMarketDataParser::BinanceMarketDataParser(
        trading::market_data::IBookUpdateHandler& handler) noexcept :
        handler { handler }
    {
    }

    void BinanceMarketDataParser::parse(const std::string_view message)
    {
        // TODO: Parse Binance market data message.
        // TODO: Convert Binance data into normalized BookUpdate events.
    }
}