/**============================================================================
Name        : binance_market_data_parser.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data parser.
============================================================================**/

/*
    BinanceMarketDataParser converts Binance market data messages into the
    normalized market_data representation used by the trading engine.

    Binance-specific JSON and protocol details remain inside this class.

    Data Flow:

        BinanceMarketDataSource
                  |
                  | raw Binance message
                  v
        MarketDataMessageHandler
                  |
                  v
        BinanceMarketDataParser
                  |
                  | BookUpdate
                  v
        IBookUpdateHandler
                  |
                  v
              BookBuilder

    BinanceMarketDataParser does not know about OrderBook and does not modify
    market state directly.

    Its responsibility ends after converting Binance messages into one or more
    normalized market data events.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP

#include "book_update_handler.hpp"
#include "market_data_parser.hpp"

#include <string_view>

namespace trading::exchanges::binance
{
    class BinanceMarketDataParser final : public market_data::IMarketDataParser
    {
    public:
        explicit BinanceMarketDataParser(
            trading::market_data::IBookUpdateHandler& handler) noexcept;

        void parse(std::string_view message) override;

    private:
        trading::market_data::IBookUpdateHandler& handler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP