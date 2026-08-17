/**============================================================================
Name        : binance_market_data_source.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data source.
============================================================================**/

/*
    BinanceMarketDataSource provides raw market data received from Binance.

    The class is responsible for Binance-specific transport and subscription
    logic.

    Received messages are forwarded to IMarketDataMessageHandler.

    Data Flow:

        Binance
           |
           | raw market data message
           v
        BinanceMarketDataSource
           |
           | raw message
           v
        IMarketDataMessageHandler
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
           |
           v
        OrderBook

    BinanceMarketDataSource does not parse market data and does not know
    anything about BookUpdate, OrderBook or MarketEvent.

    Transport-specific implementation details such as WebSocket connection,
    reconnection and subscriptions belong to this class.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_SOURCE_HPP

#include "market_data_message_handler.hpp"
#include "market_data_source.hpp"

namespace trading::exchanges::binance
{
    class BinanceMarketDataSource final : public market_data::IMarketDataSource
    {
    public:
        BinanceMarketDataSource() noexcept = default;

        void start() override;
        void stop() override;

        void setMessageHandler(market_data::IMarketDataMessageHandler& handler) override;

    private:
        market_data::IMarketDataMessageHandler* messageHandler { nullptr };
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_MARKET_DATA_SOURCE_HPP