/**============================================================================
Name        : market_data_message_handler.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_message_handler.hpp
============================================================================**/

/*
    MarketDataMessageHandler coordinates parsing of raw market-data messages
    and delivery of parsed BookUpdate instances.

    Data Flow:

        IMarketDataSource
               |
               | raw message
               v
        IMarketDataMessageHandler
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
               | onBookUpdate()
               v
        IBookUpdateHandler
               |
               v
        BookBuilder
               |
               v
        OrderBook

    Responsibilities:

        - receive raw market-data messages;
        - provide a reusable BookUpdates buffer to the parser;
        - invoke IMarketDataParser;
        - handle the ParseResult;
        - forward parsed BookUpdate instances to IBookUpdateHandler.

    The BookUpdates buffer is owned by MarketDataMessageHandler and reused
    between messages.

    This is intentional because onMessage() belongs to the market-data hot
    path. Returning a new std::vector from every parse operation could cause
    unnecessary allocations.

    std::vector::clear() does not release its allocated storage. Therefore,
    after the buffer reaches its normal working capacity, subsequent messages
    can reuse the same memory.

    MarketDataMessageHandler is an orchestration component.

    It does not:

        - know exchange-specific message formats;
        - parse JSON;
        - modify OrderBook directly;
        - generate MarketEvent.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP

#include <string_view>

#include "interfaces/book_update_handler.hpp"
#include "interfaces/market_data_parser.hpp"

namespace trading::market_data
{
    struct IMarketDataMessageHandler
    {
        virtual ~IMarketDataMessageHandler() = default;
        virtual void onMessage(std::string_view message) = 0;
    };

    class MarketDataMessageHandler final : public IMarketDataMessageHandler
    {
    public:
        MarketDataMessageHandler(IMarketDataParser& parser,
                                 IBookUpdateHandler& bookUpdateHandler) noexcept;

        void onMessage(std::string_view message) override;

    private:
        BookUpdates bookUpdates;
        IMarketDataParser& parser;
        IBookUpdateHandler& bookUpdateHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP