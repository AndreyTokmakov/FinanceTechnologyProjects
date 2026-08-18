/**============================================================================
Name        : market_data_parser.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data parser interface.
============================================================================**/

/*
    IMarketDataParser converts a raw market data message into normalized
    market data events.

    The parser is responsible for understanding the protocol and message
    format used by a particular market data provider.

    Data Flow:

        Raw market data message
                   |
                   v
            IMarketDataParser
                   |
                   | BookUpdate
                   v
            IBookUpdateHandler
                   |
                   v
              BookBuilder

    The parser does not know anything about OrderBook or MarketEvent.

    Exchange-specific implementations are responsible for converting their
    native protocol representation into the common market_data types used by
    the rest of the application.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP

#include <string_view>

namespace trading::market_data
{
    class IMarketDataParser
    {
    public:
        virtual ~IMarketDataParser() = default;

        virtual void parse(std::string_view message) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_PARSER_HPP