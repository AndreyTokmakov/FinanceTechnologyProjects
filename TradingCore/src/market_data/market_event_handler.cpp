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
    MarketEventHandler::MarketEventHandler(strategy::IStrategy& strategy,
                                           strategy::StrategyExecutor& executor,
                                           recording::IRecorder& recorder) noexcept :
        strategy { strategy },
        executor { executor },
        recorder { recorder }
    {
    }

    void MarketEventHandler::onMarketEvent(const MarketEvent& event)
    {
        recorder.record(event);

        const strategy::Signal signal = strategy.evaluate(event);
        const strategy::StrategyExecutionResult result = executor.execute(signal, event);

        if (!result) {
            // TODO:  Order creation failed.  log / metrics / risk event.
            return;
        }

        if (!result->has_value())
            return;

        [[maybe_unused]]
        const OrderId orderId = result->value();

        // Order was successfully created.
        // TODO: log / metrics.
    }
}