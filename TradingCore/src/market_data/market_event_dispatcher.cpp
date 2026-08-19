/**============================================================================
Name        : market_event_dispatcher.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_event_dispatcher.cpp
============================================================================**/

/*
    MarketEventDispatcher implementation.

    Data Flow:

        BookBuilder
             |
             | MarketEvent
             v
        MarketEventDispatcher
             |
             +------------------+
             |                  |
             v                  v
          Strategy           Recorder
*/

#include "market_event_dispatcher.hpp"

namespace trading::market_data
{
    MarketEventDispatcher::MarketEventDispatcher(IMarketEventHandler& strategy,
                                                 recording::IRecorder& recorder) noexcept:
        strategy { strategy },recorder { recorder }
    {
    }

    void MarketEventDispatcher::onMarketEvent(const MarketEvent& event)
    {
        strategy.onMarketEvent(event);
        recorder.record(event);
    }
}