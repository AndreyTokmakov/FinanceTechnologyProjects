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
        MarketDataMessageHandler
               |
               +----------------------+
               |                      |
               v                      v
        IMarketDataParser      IBookUpdateHandler
               |                      |
               | BookUpdates           |
               +----------+-----------+
                          |
                          v
                      BookBuilder

    Responsibilities:

        - receive raw market-data messages;
        - invoke IMarketDataParser;
        - handle parser success/failure;
        - forward successfully parsed BookUpdate instances to
          IBookUpdateHandler.

    MarketDataMessageHandler is an orchestration component.

    It does not:

        - know exchange-specific message formats;
        - parse JSON itself;
        - modify OrderBook;
        - create MarketEvent.

    Parser responsibility ends at producing domain objects.
    BookUpdateHandler responsibility begins at consuming those objects.
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
        MarketDataMessageHandler(IMarketDataParser& parser, IBookUpdateHandler& bookUpdateHandler) noexcept;

        void onMessage(std::string_view message) override;

    private:
        IMarketDataParser& parser;
        IBookUpdateHandler& bookUpdateHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP