/**============================================================================
Name        : position.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : position.cpp
============================================================================**/

/*
    Position implementation.

    Position represents the current signed position for one trading instrument.

    Data Flow:

        ExecutionReport
               |
               | Trade
               v
        PositionManager
               |
               | side / price / quantity
               v
        Position::applyTrade()
               |
               v
        Position state
               |
               +------------------+
               |                  |
               v                  v
        PnLCalculator        RiskManager
        / Strategy

    Responsibilities:

        - apply an executed Buy or Sell quantity;
        - maintain signed position quantity;
        - calculate the weighted average entry price when increasing an
          existing position;
        - preserve the average entry price when partially reducing a
          position;
        - reset the average entry price when the position becomes flat;
        - use the execution price as the entry price when a position reverses.

    PnL is not calculated here.

    Price and Quantity use fixed-point representation with 8 decimal places.
    Weighted average price calculation therefore performs the required scale
    normalization when combining price and quantity values.
*/

#include "position.hpp"

namespace trading::position
{
    void Position::applyTrade(const Side side,
                              const Price price,
                              const Quantity quantity) noexcept
    {
        if (quantity.isZero())
            return;

        const Value executionQuantity = quantity.raw();
        const Value signedQuantity = side == Side::Buy ? executionQuantity : -executionQuantity;

        const bool sameDirection = (currentQuantity > 0 && signedQuantity > 0) ||
            (currentQuantity < 0 && signedQuantity < 0);

        /* if (sameDirection)
        {
            const Value absoluteCurrentQuantity = currentQuantity > 0 ? currentQuantity : -currentQuantity;
            const Value absoluteExecutionQuantity = signedQuantity > 0 ? signedQuantity : -signedQuantity;
            const Value totalQuantity = absoluteCurrentQuantity + absoluteExecutionQuantity;
            const Value weightedPrice = (averageEntryPrice.raw() * absoluteCurrentQuantity +
                 price.raw() * absoluteExecutionQuantity) / totalQuantity;

            currentQuantity += signedQuantity;
            averageEntryPrice = Price { weightedPrice };
            return;
        }*/

        if (sameDirection)
        {
            const Value absoluteCurrentQuantity = currentQuantity > 0 ? currentQuantity : -currentQuantity;
            const Value absoluteExecutionQuantity = signedQuantity > 0 ? signedQuantity : -signedQuantity;
            const Value totalQuantity = absoluteCurrentQuantity + absoluteExecutionQuantity;
            const __int128 weightedPrice =
                static_cast<__int128>(averageEntryPrice.raw()) * absoluteCurrentQuantity +
                static_cast<__int128>(price.raw()) * absoluteExecutionQuantity;
            currentQuantity += signedQuantity;
            averageEntryPrice = Price { static_cast<Value>(weightedPrice / totalQuantity)};
            return;
        }

        const Value absoluteCurrentQuantity = currentQuantity > 0 ? currentQuantity : -currentQuantity;
        const Value absoluteExecutionQuantity = signedQuantity > 0 ? signedQuantity : -signedQuantity;
        if (absoluteExecutionQuantity < absoluteCurrentQuantity)
        {
            currentQuantity += signedQuantity;
            return;
        }

        if (absoluteExecutionQuantity == absoluteCurrentQuantity)
        {
            currentQuantity = 0;
            averageEntryPrice = {};
            return;
        }

        currentQuantity += signedQuantity;
        averageEntryPrice = price;
    }
}