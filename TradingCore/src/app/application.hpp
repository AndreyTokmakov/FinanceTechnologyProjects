/**============================================================================
Name        : application.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : application.hpp
============================================================================**/

/*
    Application is the composition root of the trading system.

    Application constructs application components, connects their dependencies
    and controls the application lifecycle.

    Data Flow:

        Exchange
           |
           v
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
           +----------------------+
           |                      |
           v                      v
        Strategy              Recorder

    Responsibilities:

        - construct application components;
        - establish dependencies between components;
        - configure the market-data pipeline;
        - control application lifecycle.

    Application does not implement trading logic. Domain responsibilities
    remain inside the corresponding modules.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP
#define FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP

#include "binance_market_data_parser.hpp"
#include "binance_market_data_source.hpp"
#include "book_builder.hpp"
#include "imbalance_strategy.hpp"
#include "market_data_message_handler.hpp"
#include "market_event_handler.hpp"
#include "order_book.hpp"
#include "trade_recorder.hpp"

namespace trading::app
{
    class Application final
    {
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        void start();
        void stop();

    private:
        void configureMarketData();

        market_data::OrderBook orderBook;

        recording::TradeRecorder recorder;
        strategy::ImbalanceStrategy strategy;

        market_data::MarketEventHandler marketEventHandler;
        market_data::BookBuilder bookBuilder;

        exchanges::binance::BinanceMarketDataParser marketDataParser;
        market_data::MarketDataMessageHandler marketDataMessageHandler;
        exchanges::binance::BinanceMarketDataSource marketDataSource;

        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_APPLICATION_HPP