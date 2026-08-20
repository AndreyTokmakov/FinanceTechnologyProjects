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
        MarketDataMessageHandler
               |
               v
        BinanceMarketDataParser
               |
               | std::expected<std::vector<BookUpdate>, ParseError>
               |
          +----+----+
          |         |
          v         v
      BookUpdates ParseError
          |
          v
    IBookUpdateHandler
          |
          v
      BookBuilder

    Responsibilities:

        - parse Binance market-data messages;
        - validate Binance-specific fields;
        - convert Binance prices and quantities into domain types;
        - convert Binance sequence and timestamp information;
        - produce BookUpdate instances.

    BinanceMarketDataParser does not:

        - know about BookBuilder;
        - know about IBookUpdateHandler;
        - forward BookUpdate objects;
        - modify OrderBook;
        - interact with Strategy or Execution.

    The parser is therefore a pure transformation component:

        Binance raw message -> domain model
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
        market_data::MarketDataParseResult parse(std::string_view message) const override;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_PARSER_HPP