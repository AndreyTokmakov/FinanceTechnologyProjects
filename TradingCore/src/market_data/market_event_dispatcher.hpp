/**============================================================================
Name        : market_event_dispatcher.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_event_dispatcher.hpp
============================================================================**/

/*
    MarketEventDispatcher distributes MarketEvent instances to the components interested in market-data events.
    The dispatcher separates the producer of MarketEvent from its consumers.

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
        MarketEventDispatcher
           |
           +----------------------+
           |                      |
           v                      v
        Strategy              Recorder

    Responsibilities:

        - receive MarketEvent from BookBuilder;
        - forward MarketEvent to the configured strategy;
        - forward MarketEvent to the recorder.

    The dispatcher does not:

        - create MarketEvent;
        - modify OrderBook;
        - generate trading signals;
        - perform risk validation;
        - modify positions;
        - calculate PnL.

    This class is part of the application event-distribution layer.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP

#include "market_event_handler.hpp"
#include "recorder.hpp"

namespace trading::market_data
{
    class MarketEventDispatcher final : public IMarketEventHandler
    {
    public:
        MarketEventDispatcher(IMarketEventHandler& strategy,
                              recording::IRecorder& recorder) noexcept;

        void onMarketEvent(const MarketEvent& event) override;

    private:
        IMarketEventHandler& strategy;
        recording::IRecorder& recorder;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_DISPATCHER_HPP