/**============================================================================
Name        : market_event_handler.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_event_handler.hpp
============================================================================**/

/*
    MarketEventHandler is the application-level consumer of MarketEvent.

    It receives MarketEvent instances produced by BookBuilder and forwards
    them to the components interested in market-data events.

    Data Flow:

        Exchange
           |
           v
        MarketDataSource
           |
           v
        MarketDataMessageHandler
           |
           v
        MarketDataParser
           |
           | BookUpdate
           v
        BookBuilder
           |
           | MarketEvent
           v
        MarketEventHandler
           |
           +----------------------+
           |                      |
           v                      v
        Strategy              Recorder

    Responsibilities:

        - receive MarketEvent from BookBuilder;
        - forward MarketEvent to the strategy;
        - forward MarketEvent to the recorder.

    MarketEventHandler does not:

        - create MarketEvent;
        - modify OrderBook;
        - generate trading signals;
        - perform risk validation;
        - manage orders;
        - manage positions;
        - calculate PnL.

    The IMarketEventHandler interface defines the contract used by BookBuilder.
    MarketEventHandler provides the concrete implementation of that contract.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP

#include "interfaces/market_event_handler.hpp"
#include "recorder.hpp"
#include "strategy.hpp"
#include "strategy_executor.hpp"

namespace trading::market_data
{
    class MarketEventHandler final : public IMarketEventHandler
    {
    public:
        MarketEventHandler(strategy::IStrategy& strategy,
                           strategy::StrategyExecutor& executor,
                           recording::IRecorder& recorder) noexcept;

        void onMarketEvent(const MarketEvent& event) override;

    private:
        strategy::IStrategy& strategy;
        strategy::StrategyExecutor& executor;
        recording::IRecorder& recorder;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP