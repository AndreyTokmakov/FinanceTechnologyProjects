/**============================================================================
Name        : signal.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Trading strategy signal definition.
============================================================================**/

/*
    Signal represents the result of a strategy evaluation.

    A strategy receives market data and evaluates the current market
    conditions. The result of this evaluation is represented by Signal.

    Signal intentionally contains only the direction of the trading decision.
    It does not contain an order, price, quantity, instrument or execution
    information.

    Responsibilities:

        - represent a Buy trading decision;
        - represent a Sell trading decision;
        - represent the absence of a trading decision.

    Signal does not:

        - create orders;
        - communicate with OrderManager;
        - communicate with an execution gateway;
        - perform risk checks;
        - manage positions;
        - contain market data;
        - contain order quantity or price.

    The separation is intentional.

    The strategy is responsible only for answering the question:

        "Given the current market state, should I Buy, Sell, or do nothing?"

    Conversion of a Signal into an OrderRequest belongs to the strategy
    execution layer.

    Data flow:

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
*/

#ifndef FINANCETECHNOLOGYPROJECTS_SIGNAL_HPP
#define FINANCETECHNOLOGYPROJECTS_SIGNAL_HPP
#include <cstdint>

namespace trading::strategy
{
    enum class Signal: uint8_t
    {
        None,
        Buy,
        Sell
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_SIGNAL_HPP