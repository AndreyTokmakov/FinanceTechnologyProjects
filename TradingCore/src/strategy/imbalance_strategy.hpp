/**============================================================================
Name        : imbalance_strategy.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book imbalance trading strategy.
============================================================================**/

/*
    ImbalanceStrategy generates trading signals based on the imbalance
    between the best bid and best ask quantities.

    The strategy compares the quantity available at the best bid with the
    quantity available at the best ask.

    The imbalance is conceptually calculated as:

        bidQuantity - askQuantity
        --------------------------
        bidQuantity + askQuantity

    The resulting value is in the range [-1, +1]:

        +1  -> only bid liquidity is present;
         0  -> bid and ask liquidity are equal;
        -1  -> only ask liquidity is present.

    A configurable threshold determines whether the imbalance is strong
    enough to generate a trading signal.

        imbalance >= threshold   -> Signal::Buy
        imbalance <= -threshold  -> Signal::Sell
        otherwise                -> Signal::None

    The implementation does not use floating-point arithmetic. Instead,
    the comparison is performed using integer arithmetic to avoid floating
    point operations in the market-data hot path.

    For a threshold represented as:

        thresholdNumerator / thresholdDenominator

    the comparison is performed as:

        |bid - ask| * thresholdDenominator
        >=
        (bid + ask) * thresholdNumerator

    This preserves deterministic behaviour and avoids floating-point
    rounding issues.

    Responsibilities:

        - evaluate the current best bid and ask quantities;
        - calculate the order book imbalance;
        - compare the imbalance with the configured threshold;
        - return Buy, Sell or None.

    ImbalanceStrategy does not:

        - create orders;
        - calculate order quantity;
        - perform risk checks;
        - manage positions;
        - communicate with OrderManager;
        - communicate with an execution gateway;
        - record trading events.

    Data flow:

        MarketEvent
             |
             v
        ImbalanceStrategy::evaluate()
             |
             v
          Signal
             |
             v
        StrategyExecutor
             |
             v
        OrderRequest

    A market event with zero bid and ask quantity produces Signal::None.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP
#define FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP

#include "strategy.hpp"

namespace trading::strategy
{
    class ImbalanceStrategy final : public IStrategy
    {
    public:
        using Value = int64_t;

        static constexpr Value DefaultThresholdNumerator { 7 };
        static constexpr Value DefaultThresholdDenominator { 10 };

        explicit ImbalanceStrategy(Value thresholdNumerator = DefaultThresholdNumerator,
                                   Value thresholdDenominator = DefaultThresholdDenominator) noexcept;

        [[nodiscard]]
        Signal evaluate(const market_data::MarketEvent& event) const override;

    private:
        Value thresholdNumerator;
        Value thresholdDenominator;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_IMBALANCE_STRATEGY_HPP