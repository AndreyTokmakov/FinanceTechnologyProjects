/**============================================================================
Name        : pnl.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : pnl.hpp
============================================================================**/

/*
    PnL represents profit and loss for a trading position.
    PnL is expressed in the same fixed-point representation as Price.

    Data Flow:

        Position
           |
           | current position
           |
        MarketEvent
           |
           | current market price
           v
        PnLCalculator
           |
           v
        PnL

    ExecutionReport
           |
           | Trade
           v
        PnLCalculator
           |
           | closed quantity
           v
        Realized PnL

    Responsibilities:

        - represent realized PnL;
        - represent unrealized PnL;
        - provide total PnL.

    PnL does not:

        - maintain positions;
        - process orders;
        - communicate with an exchange;
        - process raw market data;
        - perform risk checks.

    realized:
        PnL produced by trades that reduce or close an existing position.

    unrealized:
        PnL of the currently open position based on the current market price.

    total:
        realized + unrealized.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_PNL_HPP
#define FINANCETECHNOLOGYPROJECTS_PNL_HPP

#include "price.hpp"

namespace trading::pnl
{
    struct PnL
    {
        Price realized {};
        Price unrealized {};

        [[nodiscard]]
        constexpr Price total() const noexcept {
            return realized + unrealized;
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PNL_HPP