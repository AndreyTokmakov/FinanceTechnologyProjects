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

    Data Flow:

        IMarketDataSource
               |
               | raw message
               v
        IMarketDataMessageHandler
               |
               v
        IMarketDataParser
               |
               | fills reusable BookUpdates buffer
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

        - parse a raw market-data message;
        - validate exchange-specific message format;
        - convert exchange-specific data into BookUpdate objects;
        - return the result of the parsing operation.

    The parser receives an output buffer owned by the caller.

    The buffer is intentionally reused between messages. This avoids creating
    a new std::vector and potentially allocating memory on every market-data
    message.

    The parser must clear the supplied BookUpdates buffer before filling it.

    IMarketDataParser does not:
        - know about IBookUpdateHandler;
        - forward BookUpdate instances;
        - modify OrderBook;
        - generate MarketEvent;
        - interact with Strategy;
        - interact with Execution.

    The parser is therefore responsible only for transformation:

        raw market data -> BookUpdates
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP

#include <string_view>
#include <vector>

#include "model/book_update.hpp"
#include "model/parse_result.hpp"

namespace trading::market_data
{
    using BookUpdates = std::vector<BookUpdate>;

    struct IMarketDataParser
    {
        virtual ~IMarketDataParser() = default;

        [[nodiscard]]
        virtual ParseResult parse(std::string_view message,
                                   BookUpdates& bookUpdates) const = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP