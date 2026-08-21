/**============================================================================
Name        : application.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.cpp
============================================================================**/

/*
    Application implementation.
    Application constructs and wires the market-data components.

    Data Flow:

        BinanceMarketDataSource
                 |
                 | raw message
                 v
        MarketDataMessageHandler
                 |
                 v
        BinanceMarketDataParser
                 |
                 | BookUpdate
                 v
             BookBuilder
                 |
                 v
             OrderBook
                 |
                 | MarketEvent
                 v
        MarketEventDispatcher
                 |
                 +------------------+
                 |                  |
                 v                  v
              Strategy           Recorder

    Application does not process market-data events itself. It only creates
    the components and establishes their relationships.
*/

#include "application.hpp"

namespace trading::app
{
    Application::Application():
        orderBook {},
        recorder {},
        position { InstrumentId { 1 } },
        riskManager {},
        strategy {},
        binanceExecutionGateway {},
        orderManager { binanceExecutionGateway, riskManager, position },
        strategyExecutor { orderManager, Quantity { 100'000'000 } },
        marketEventHandler { strategy, strategyExecutor, recorder },
        bookBuilder { InstrumentId { 1 }, orderBook, marketEventHandler },
        marketDataParser {},
        marketDataMessageHandler { marketDataParser, bookBuilder },
        marketDataSource {}
    {
        configureRisk();
        configureMarketData();
    }

    Application::~Application()
    {
        stop();
    }

    void Application::configureRisk()
    {
        constexpr risk::RiskLimits limits {
            .maxOrderQuantity = Quantity { 100'000'000 },
            .maxPositionQuantity = Quantity { 500'000'000 },
            .maxNotional = Price { 100'000'000'000'000 }
        };

        riskManager.setLimits(limits);
    }

    void Application::configureMarketData()
    {
        marketDataSource.setMessageHandler(marketDataMessageHandler);
    }

    void Application::start()
    {
        if (running)
            return;

        running = true;
        marketDataSource.start();
    }

    void Application::stop()
    {
        if (!running)
            return;

        marketDataSource.stop();
        running = false;
    }
}