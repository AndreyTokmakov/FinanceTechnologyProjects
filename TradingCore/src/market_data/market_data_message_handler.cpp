/**============================================================================
Name        : market_data_message_handler.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data message handler implementation.
============================================================================**/

#include "market_data_message_handler.hpp"

namespace trading::market_data
{
    MarketDataMessageHandler::MarketDataMessageHandler(IMarketDataParser& parser) noexcept :
        parser { parser }
    {
    }

    void MarketDataMessageHandler::onMessage(const std::string_view message)
    {
        parser.parse(message);
    }
}