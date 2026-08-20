/**============================================================================
Name        : parse_error.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : parse_error.hpp
============================================================================**/

/*
    ParseError describes errors that may occur while converting a raw market
    data message into domain-level market data objects.

    Data Flow:

        Raw Market Data
               |
               v
        IMarketDataParser
               |
          +----+----+
          |         |
          v         v
    BookUpdates  ParseError
          |
          v
    IBookUpdateHandler

    ParseError belongs to the parsing layer.

    It describes why a raw message could not be converted into a valid
    market-data representation.

    Parser errors are not responsible for handling the error. The parser only
    reports the error to its caller.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_PARSE_ERROR_HPP
#define FINANCETECHNOLOGYPROJECTS_PARSE_ERROR_HPP

namespace trading::market_data
{
    enum class ParseError
    {
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

#endif //FINANCETECHNOLOGYPROJECTS_PARSE_ERROR_HPP