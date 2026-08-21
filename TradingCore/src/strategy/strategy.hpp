/**============================================================================
Name        : strategy.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Trading strategy interface.
============================================================================**/

/*
    IStrategy defines the interface for trading strategies.

    A strategy receives a snapshot of the current market state represented by
    MarketEvent and evaluates whether the current conditions justify a
    trading action.

    The result of the evaluation is a Signal:

        Signal::Buy  - the strategy wants to buy;
        Signal::Sell - the strategy wants to sell;
        Signal::None - the strategy does not want to trade.

    The strategy is intentionally separated from order management and
    execution.

    Responsibilities:

        - consume market state;
        - evaluate trading conditions;
        - produce a trading Signal.

    IStrategy does not:

        - create Order objects;
        - create OrderRequest objects;
        - submit orders;
        - communicate with IExecutionGateway;
        - perform risk validation;
        - modify positions;
        - record executions;
        - manage exchange-specific communication.

    This separation allows the same strategy to be tested independently from
    the execution infrastructure.

    The expected data flow is:

        MarketEvent
             |
             v
        IStrategy::evaluate()
             |
             v
          Signal
             |
             v
        StrategyExecutor
             |
             v
        OrderRequest
             |
             v
        OrderManager
             |
             v
        IExecutionGateway

    evaluate() is const because evaluating a market event should not require
    modification of the strategy's externally visible state.

    Individual strategy implementations may keep internal state if their
    algorithm requires it. Such state should represent strategy-specific
    information rather than execution state.

    MarketEvent contains the market information available to the strategy,
    including:

        - instrument identifier;
        - sequence number;
        - exchange timestamp;
        - receive timestamp;
        - best bid price and quantity;
        - best ask price and quantity.

    The strategy therefore operates on a well-defined market snapshot and
    remains independent from the components responsible for executing the
    resulting trading decision.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_STRATEGY_HPP
#define FINANCETECHNOLOGYPROJECTS_STRATEGY_HPP

#include "signal.hpp"
#include "model/market_event.hpp"

namespace trading::strategy
{
    struct IStrategy
    {
        virtual ~IStrategy() = default;

        [[nodiscard]]
        virtual Signal evaluate(const market_data::MarketEvent& event) const = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_STRATEGY_HPP