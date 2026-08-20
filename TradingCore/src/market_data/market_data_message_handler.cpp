/**============================================================================
Name        : market_data_message_handler.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_message_handler.cpp
============================================================================**/

/*
    MarketDataMessageHandler implementation.

    Data Flow:

        Raw Market Data
               |
               v
        MarketDataMessageHandler
               |
               | parse(message, bookUpdates)
               v
        IMarketDataParser
               |
               | fills reusable buffer
               v
        BookUpdates
               |
               v
        IBookUpdateHandler
               |
               v
        BookBuilder

    The BookUpdates buffer is reused between messages to avoid allocations
    on the market-data hot path.
*/

#include "market_data_message_handler.hpp"

namespace trading::market_data
{
    MarketDataMessageHandler::MarketDataMessageHandler(IMarketDataParser& parser,
                                                       IBookUpdateHandler& bookUpdateHandler) noexcept:
        bookUpdates {},
        parser { parser },
        bookUpdateHandler { bookUpdateHandler }
    {
    }

    void MarketDataMessageHandler::onMessage(std::string_view message)
    {
        const ParseResult result = parser.parse(message, bookUpdates);

        if (result != ParseResult::Success)
            return;
        for (const auto& update : bookUpdates)
            bookUpdateHandler.onBookUpdate(update);
    }
}