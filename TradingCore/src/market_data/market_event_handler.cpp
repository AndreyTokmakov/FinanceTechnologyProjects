/**============================================================================
Name        : market_event_handler.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_event_handler.cpp
============================================================================**/

/*
    MarketEventHandler implementation.

    Data Flow:

        BookBuilder
             |
             | MarketEvent
             v
        MarketEventHandler
             |
             +------------------+
             |                  |
             v                  v
          Strategy           Recorder

    MarketEventHandler does not modify the event. The same MarketEvent is
    forwarded to all registered consumers.
*/

#include "market_event_handler.hpp"

namespace trading::market_data
{
    MarketEventHandler::MarketEventHandler(
        IMarketEventHandler& strategy,
        recording::IRecorder& recorder) noexcept
        : strategy { strategy },
          recorder { recorder }
    {
    }

    void MarketEventHandler::onMarketEvent(const MarketEvent& event)
    {
        strategy.onMarketEvent(event);
        recorder.record(event);
    }
}