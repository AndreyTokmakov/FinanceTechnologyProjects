/**============================================================================
Name        : market_data_source.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_source.hpp
============================================================================**/

/*
    IMarketDataSource represents a source of raw market data.

    The source is responsible for obtaining market data from an external
    system and forwarding raw messages to IMarketDataMessageHandler.

    The source does not parse exchange-specific messages and does not know
    anything about OrderBook, BookUpdate or MarketEvent.

    Data Flow:

        Exchange
           |
           | raw market data
           v
        IMarketDataSource
           |
           | raw message
           v
        IMarketDataMessageHandler
           |
           v
        MarketDataParser
           |
           | BookUpdate
           v
        IBookUpdateHandler
           |
           v
        BookBuilder

    Responsibilities:

        - establish the market data connection;
        - receive raw market data messages;
        - forward raw messages to the configured message handler;
        - manage the source lifecycle;
        - stop receiving data when requested.

    The implementation may use WebSocket, TCP, UDP or another transport.
    Transport-specific details must remain inside the implementation.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_SOURCE_HPP

namespace trading::market_data
{
    struct IMarketDataSource
    {
        virtual ~IMarketDataSource() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual void setMessageHandler(IMarketDataMessageHandler& handler) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_SOURCE_HPP