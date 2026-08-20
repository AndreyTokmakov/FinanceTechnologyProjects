/**============================================================================
Name        : binance_market_data_parser.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_market_data_parser.hpp
============================================================================**/

/*
    BinanceMarketDataParser converts Binance-specific raw market-data
    messages into domain-level BookUpdate objects.

    Data Flow:

        BinanceMarketDataSource
               |
               | raw Binance message
               v
        IMarketDataMessageHandler
               |
               v
        BinanceMarketDataParser
               |
               | fills reusable BookUpdates
               v
        BookUpdates
          |
          v
    IBookUpdateHandler
          |
          v
      BookBuilder
               |
               v
        OrderBook

    Responsibilities:

        - parse Binance market-data messages;
        - validate Binance-specific fields;
        - convert Binance data into BookUpdate objects;
        - return the parsing result.

    The parser does not know about IBookUpdateHandler or BookBuilder.

    The supplied BookUpdates buffer is reused between calls. The parser must
    clear it before adding new updates.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP

#include "market_data_parser.hpp"

namespace trading::exchanges::binance
{
    class BinanceMarketDataParser final : public market_data::IMarketDataParser
    {
    public:
        [[nodiscard]]
        market_data::ParseResult parse(std::string_view message,
                                       market_data::BookUpdates& bookUpdates) const override;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP