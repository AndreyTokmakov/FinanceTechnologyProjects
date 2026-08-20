/**============================================================================
Name        : market_data_parser.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_parser.hpp
============================================================================**/

/*
    IMarketDataParser converts raw market-data messages into domain-level
    market-data objects.

    The parser is intentionally independent from all consumers of the parsed
    data.

    Data Flow:

        Raw Market Data
               |
               v
        IMarketDataParser
               |
               | std::expected<std::vector<BookUpdate>, ParseError>
               |
          +----+----+
          |         |
          v         v
      BookUpdates ParseError
          |
          v
    MarketDataMessageHandler
          |
          v
    IBookUpdateHandler

    Responsibilities:

        - parse a raw market-data message;
        - validate the message format;
        - convert exchange-specific fields into domain objects;
        - report parsing errors.

    IMarketDataParser does not:

        - know about IBookUpdateHandler;
        - forward BookUpdate instances;
        - modify OrderBook;
        - generate MarketEvent;
        - interact with Strategy;
        - interact with Risk or Execution modules.

    Concrete implementations contain exchange-specific parsing logic.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP

#include <expected>
#include <string_view>
#include <vector>

#include "book_update.hpp"
#include "parse_error.hpp"

namespace trading::market_data
{
    using MarketDataParseResult = std::expected<
        std::vector<BookUpdate>,
        ParseError
    >;

    struct IMarketDataParser
    {
        virtual ~IMarketDataParser() = default;

        [[nodiscard]]
        virtual MarketDataParseResult parse(std::string_view message) const = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP