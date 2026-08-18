/**============================================================================
Name        : imbalance_strategy.hpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book imbalance trading strategy.
============================================================================**/

/*
    ImbalanceStrategy is a simple trading strategy based on the imbalance
        between the best bid and best ask quantities.
    The strategy receives MarketEvent objects produced by the market data pipeline.
    Current implementation is intentionally minimal.
    It only consumes market events and does not submit orders yet.

    Future responsibilities:

        MarketEvent
            |
            v
        Calculate imbalance
            |
            v
        Generate trading signal
            |
            v
        Create OrderRequest
            |
            v
        OrderManager
*/

#ifndef FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP
#define FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP

#include "../market_data/interfaces/market_event_handler.hpp"

namespace trading::strategy
{
    class ImbalanceStrategy final : public market_data::IMarketEventHandler
    {
    public:
        void onMarketEvent(const market_data::MarketEvent& event) override;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP