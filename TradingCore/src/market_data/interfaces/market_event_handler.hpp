/**============================================================================
Name        : market_event_handler.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market event handler.
============================================================================**/

/*
    IMarketEventHandler represents a consumer of MarketEvent objects.

    MarketEvent is produced by the market data processing pipeline after the
    OrderBook has been updated.

    Data Flow:

        BookUpdate
            |
            v
        BookBuilder
            |
            | MarketEvent
            v
        IMarketEventHandler
            |
            v
        MarketEventHandler
            |
            v
        Strategy

    The handler is responsible for forwarding market events to the next
    component of the market data processing pipeline.

    It does not modify the OrderBook and does not create MarketEvent objects.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP

#include "../model/market_event.hpp"

namespace trading::market_data
{
    struct IMarketEventHandler
    {
        virtual ~IMarketEventHandler() = default;
        virtual void onMarketEvent(const MarketEvent& event) = 0;
    };

    class MarketEventHandler final : public IMarketEventHandler
    {
    public:
        explicit MarketEventHandler(IMarketEventHandler& handler) noexcept;
        void onMarketEvent(const MarketEvent& event) override;

    private:
        IMarketEventHandler& handler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKET_EVENT_HANDLER_HPP