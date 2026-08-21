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
#include <cmath>

namespace trading::position
{
    /*
        Applies an executed trade to the current position.

        Position model:

            - positive quantity  -> Long position;
            - negative quantity  -> Short position;
            - zero quantity      -> Flat position.

        The method receives a single executed trade and updates the position
        quantity and average entry price according to the trade direction.

        The method does not represent an order submission, order update, or
        execution report processing stage. It operates only on an already
        executed trade. Validation and routing of ExecutionReport objects are
        responsibilities of PositionManager.

        Trade direction:

            Buy  -> increases the position quantity;
            Sell -> decreases the position quantity.

        The resulting signed quantity is calculated as:

            Buy  -> +quantity
            Sell -> -quantity

        Processing rules:

        1. Zero-quantity trade

            A trade with zero quantity has no effect.

            This is handled before any position calculations. In particular,
            a zero-quantity trade must not create a position or modify the
            average entry price.

        2. Flat position

            If the current position is flat, any non-zero trade opens a new position.

            Buy:

                current quantity = +execution quantity
                average price   = execution price

            Sell:

                current quantity = -execution quantity
                average price   = execution price

            There is no previous position from which an average price needs to be calculated.

        3. Trade in the same direction as the existing position

            If the existing position and the new trade have the same direction, the position is increased.

            Examples:

                Long  + Buy  -> larger Long
                Short + Sell -> larger Short

            The new average entry price is calculated using a quantity-weighted
            average:

                new average price =
                    (old price * old absolute quantity +
                     trade price * trade absolute quantity)
                    / total absolute quantity

            Absolute quantities are used because the direction of the position
            is represented separately by the sign of currentQuantity.

            For example:

                existing position:
                    quantity = +100
                    average  = 6000

                new trade:
                    Buy 100 @ 7000

                result:
                    quantity = +200
                    average  = 6500

            The same calculation applies to increasing a Short position.

        4. Trade in the opposite direction, but smaller than the position

            If the trade direction is opposite to the current position and the
            executed quantity is smaller than the absolute current quantity,
            the position is partially reduced.

            Examples:

                Long 100 + Sell 40 -> Long 60
                Short 100 + Buy 40 -> Short 60

            The average entry price does not change.

            This is important because the remaining position still consists of
            the same original entry position. The execution price belongs to
            the reducing trade and therefore does not become the entry price of
            the remaining quantity.

        5. Trade exactly closes the position

            If the opposite-side execution quantity is equal to the absolute
            current position quantity, the position becomes flat.

            Examples:

                Long 100 + Sell 100 -> Flat
                Short 100 + Buy 100 -> Flat

            When the position becomes flat, the average entry price is reset
            to zero because there is no longer an open position to which the
            price belongs.

        6. Trade reverses the position

            If the opposite-side execution quantity is greater than the
            absolute current position quantity, the existing position is fully
            closed and the remaining execution quantity opens a position in the
            opposite direction.

            Examples:

                Long 100 + Sell 150 -> Short 50
                Short 100 + Buy 150 -> Long 50

            The new position uses the execution price of the reversing trade as
            its average entry price.

            The previous average entry price is not carried into the new
            position because the previous position has been completely closed.

        Numerical precision:

            Price and Quantity use fixed-point integer representation with
            8 decimal places.

            Weighted-average calculation multiplies two fixed-point values:

                price.raw() * quantity.raw()

            which can exceed the range of int64_t even when both individual
            values are valid. Therefore the intermediate weighted-price
            calculation uses __int128 to avoid overflow during multiplication.

            The final result is converted back to Price::Value after division
            by the total quantity.

        Realized PnL:

            This method currently does not calculate realized PnL.

            Closing or reducing a position changes the position quantity and
            preserves or resets the average entry price as described above,
            but realizedPnL is not updated here.

            PnL calculation is intentionally kept outside this method and is
            the responsibility of the PnL module.

        Complexity:

            Time complexity: O(1)
            Memory complexity: O(1)

        Thread safety:

            Position is a mutable value object and this method is not thread-safe.
            Synchronization, if required, must be provided by the component that
            owns and accesses the Position.

        Preconditions:

            - quantity is represented using the same fixed-point scale as the
              Position model;
            - price belongs to the same instrument as the Position;
            - the trade has already been validated by the caller.

        Postconditions:

            - currentQuantity reflects the executed trade;
            - averageEntryPrice represents the remaining open position;
            - a flat position always has a zero average entry price;
            - a reversed position uses the execution price as its new entry price;
            - zero-quantity trades leave the position unchanged.

        Typical call path:

            Exchange execution
                    |
                    v
            ExecutionReport
                    |
                    v
            PositionManager::applyExecution()
                    |
                    | validates ExecType::Trade
                    | selects Position by InstrumentId
                    v
            Position::applyTrade()
                    |
                    v
            Updated Position
    */

    void Position::applyTrade(const Side side,
                              const Price price,
                              const Quantity quantity) noexcept
    {
        if (quantity.isZero())
            return;

        const Value executionQuantity = quantity.raw();
        const Value signedQuantity = side == Side::Buy ? executionQuantity : -executionQuantity;

        if (currentQuantity == 0)
        {
            currentQuantity = signedQuantity;
            averageEntryPrice = price;
            return;
        }

        const Value absoluteCurrentQuantity = std::abs(currentQuantity);
        const Value absoluteExecutionQuantity = std::abs(signedQuantity);

        const bool sameDirection = (currentQuantity > 0) == (signedQuantity > 0);
        if (sameDirection)
        {
            const Value totalQuantity = absoluteCurrentQuantity + absoluteExecutionQuantity;
            const __int128 weightedPrice =
                static_cast<__int128>(averageEntryPrice.raw()) * absoluteCurrentQuantity +
                static_cast<__int128>(price.raw()) * absoluteExecutionQuantity;

            currentQuantity += signedQuantity;
            averageEntryPrice = Price { static_cast<Value>(weightedPrice / totalQuantity) };
            return;
        }

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