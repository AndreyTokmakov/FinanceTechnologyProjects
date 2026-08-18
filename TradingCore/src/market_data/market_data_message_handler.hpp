/**============================================================================
Name        : market_data_message_handler.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data message handler.
============================================================================**/

/*
    MarketDataMessageHandler receives raw market data messages from a market
    data source and forwards them to a market data parser.

    The handler is intentionally unaware of the message format.

    Data Flow:

        MarketDataSource
               |
               | raw message
               v
        MarketDataMessageHandler
               |
               | raw message
               v
        IMarketDataParser
               |
               | BookUpdate
               v
        IBookUpdateHandler

    The parser is responsible for understanding the exchange-specific
    protocol. The message handler is responsible only for forwarding the
    message to the parser.

    This separation allows the same message handling mechanism to be used
    with different exchanges and protocols.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP

#include "interfaces/market_data_parser.hpp"

#include <string_view>

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
        explicit MarketDataMessageHandler(IMarketDataParser& parser) noexcept;

        void onMessage(std::string_view message) override;

    private:
        IMarketDataParser& parser;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_DATA_MESSAGE_HANDLER_HPP