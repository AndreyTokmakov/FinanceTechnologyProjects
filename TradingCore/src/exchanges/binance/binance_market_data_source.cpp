/**============================================================================
Name        : binance_market_data_source.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance market data source implementation.
============================================================================**/

#include "binance_market_data_source.hpp"

namespace trading::exchanges::binance
{
    void BinanceMarketDataSource::start()
    {
        running = true;
    }

    void BinanceMarketDataSource::stop()
    {
        running = false;
    }

    void BinanceMarketDataSource::setMessageHandler(
        market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }
}