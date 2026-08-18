/**============================================================================
Name        : pnl_calculator.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : pnl_calculator.hpp
============================================================================**/

/*
    PnLCalculator calculates realized and unrealized profit and loss.

    PnLCalculator is a stateless calculation component. It does not own or
    modify Position objects and does not maintain accumulated PnL state.

    Data Flow:

        ExecutionReport
               |
               | Trade
               v
        Position before trade
               |
               +----------------+
                                |
                                v
                         PnLCalculator
                                |
                                v
                         Realized PnL


        Position
           |
           | open position
           v
        PnLCalculator <──── MarketEvent
           |
           | mark price
           v
        Unrealized PnL

    Responsibilities:

        - calculate realized PnL produced by an execution;
        - calculate unrealized PnL for an open position;
        - calculate complete PnL from realized and unrealized components;
        - use the appropriate market side when marking a position;
        - perform fixed-point arithmetic safely.

    Realized PnL:

        Long position closed by Sell:

            (execution price - average entry price) * closed quantity

        Short position closed by Buy:

            (average entry price - execution price) * closed quantity

    Only the quantity that closes an existing position contributes to realized
    PnL. If an execution reverses the position, the part that closes the old
    position is realized and the remaining quantity opens a new position.

    Unrealized PnL:

        Long position:
            (best bid - average entry price) * position quantity

        Short position:
            (average entry price - best ask) * absolute position quantity

    The calculator uses the bid price to value a long position and the ask
    price to value a short position. This provides a conservative executable
    mark.

    PnLCalculator does not:

        - own Position state;
        - modify Position;
        - process ExecutionReport lifecycle events;
        - calculate risk limits;
        - communicate with exchanges.

    Price and Quantity use fixed-point representation with 8 decimal places.
    Intermediate arithmetic uses a wider integer type to avoid overflow.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_PNL_CALCULATOR_HPP
#define FINANCETECHNOLOGYPROJECTS_PNL_CALCULATOR_HPP

#include "execution_report.hpp"
#include "market_event.hpp"
#include "position.hpp"
#include "price.hpp"
#include "pnl.hpp"

namespace trading::pnl
{
    class PnLCalculator
    {
    public:
        [[nodiscard]]
        static Price calculateRealized(const position::Position& position,
                                       const execution::ExecutionReport& report) noexcept;

        [[nodiscard]]
        static Price calculateUnrealized(const position::Position& position,
                                         const market_data::MarketEvent& marketEvent) noexcept;

        [[nodiscard]]
        static PnL calculate(const position::Position& position,
                             const execution::ExecutionReport& report,
                             const market_data::MarketEvent& marketEvent) noexcept;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PNL_CALCULATOR_HPP