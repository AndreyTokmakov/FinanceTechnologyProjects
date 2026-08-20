/**============================================================================
Name        : market_data_message_handler.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data message handler implementation.
============================================================================**/

#include "market_data_message_handler.hpp"

/*
    MarketDataMessageHandler implementation.

    Data Flow:

        Raw Market Data
               |
               v
        MarketDataMessageHandler
               |
               v
        IMarketDataParser
               |
               | BookUpdates
               v
        IBookUpdateHandler
               |
               v
           BookBuilder

    The handler is responsible only for connecting the parser output with the
    BookUpdate processing pipeline.
*/

#include "market_data_message_handler.hpp"

namespace trading::market_data
{
    MarketDataMessageHandler::MarketDataMessageHandler(IMarketDataParser& parser,
                                                       IBookUpdateHandler& bookUpdateHandler) noexcept
        : parser { parser }, bookUpdateHandler { bookUpdateHandler }
    {
    }

    void MarketDataMessageHandler::onMessage(const std::string_view message)
    {
        const auto result = parser.parse(message);
        if (!result)
            return;
        for (const auto& update : *result)
            bookUpdateHandler.onBookUpdate(update);
    }
}