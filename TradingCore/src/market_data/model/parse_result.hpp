/**============================================================================
Name        : parse_result.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : parse_result.hpp
============================================================================**/

/*
    ParseResult represents the result of parsing a raw market-data message.

    Data Flow:

        Raw Market Data
               |
               v
        IMarketDataParser
               |
               | ParseResult
               v
        MarketDataMessageHandler
               |
          +----+----+
          |         |
          v         v
       Success    Parse error
          |
          v
      BookUpdates
          |
          v
    IBookUpdateHandler

    ParseResult is intentionally a simple enum.

    The parser writes successfully parsed BookUpdate objects into a reusable
    BookUpdates buffer supplied by the caller.

    Therefore ParseResult only describes the outcome of the parsing operation.
    It does not contain parsed data.

    Responsibilities:

        - indicate successful parsing;
        - indicate why parsing failed;
        - provide a lightweight result suitable for the market-data hot path.

    ParseResult does not contain diagnostic text and does not perform error
    handling. Detailed diagnostics, logging or metrics belong to higher-level
    infrastructure.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_PARSE_RESULT_HPP
#define FINANCETECHNOLOGYPROJECTS_PARSE_RESULT_HPP

namespace trading::market_data
{
    enum class ParseResult
    {
        Success,
        InvalidMessage,
        InvalidJson,
        UnsupportedMessage,
        MissingField,
        InvalidField,
        InvalidInstrument,
        InvalidSequence,
        InvalidTimestamp,
        InvalidPrice,
        InvalidQuantity
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSE_RESULT_HPP