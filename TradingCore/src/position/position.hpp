/**============================================================================
Name        : position.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Position state for a single trading instrument.
============================================================================**/
/*
    Position represents the current position for a single trading instrument.

    A position is expressed as a signed quantity:

        positive quantity -> long position
        negative quantity -> short position
        zero quantity     -> flat position

    Data Flow:

        ExecutionReport
               |
               | Trade
               v
        PositionManager
               |
               | apply execution
               v
        Position
               |
               +-------------------+
               |                   |
               v                   v
        PnLCalculator         RiskManager
        / Strategy

    Responsibilities:

        - identify the instrument;
        - maintain the signed position quantity;
        - maintain the average entry price of the open position;
        - provide the current position state.

    Position does not:

        - communicate with an exchange;
        - create or send orders;
        - manage order lifecycle;
        - calculate PnL;
        - perform risk checks;
        - process market data.

    Average entry price rules:

        - opening or increasing a position recalculates the weighted average
          entry price;
        - partially closing a position keeps the existing average entry price;
        - fully closing a position resets the average entry price to zero;
        - reversing a position starts a new position at the execution price.

    PnL is intentionally not stored in Position at this stage.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_POSITION_HPP
#define FINANCETECHNOLOGYPROJECTS_POSITION_HPP

#include "price.hpp"
#include "types.hpp"
#include "quantity.hpp"

#include <cstdint>

namespace trading::position
{
    class Position
    {
    public:
        using Value = int64_t;

        constexpr Position() noexcept = default;

        explicit constexpr Position(const InstrumentId instrument) noexcept :
            instrument { instrument }
        {
        }

        [[nodiscard]]
        constexpr InstrumentId instrumentId() const noexcept {
            return instrument;
        }

        [[nodiscard]]
        constexpr Value quantity() const noexcept {
            return currentQuantity;
        }

        [[nodiscard]]
        constexpr Price averagePrice() const noexcept {
            return averageEntryPrice;
        }

        [[nodiscard]]
        constexpr Price realizedPnl() const noexcept {
            return realizedPnL;
        }

        [[nodiscard]]
        constexpr bool isLong() const noexcept {
            return currentQuantity > 0;
        }

        [[nodiscard]]
        constexpr bool isShort() const noexcept {
            return currentQuantity < 0;
        }

        [[nodiscard]]
        constexpr bool isFlat() const noexcept {
            return currentQuantity == 0;
        }

        void applyTrade(Side side, Price price, Quantity quantity) noexcept;

    private:
        InstrumentId instrument { 0 };
        Value currentQuantity { 0 };
        Price averageEntryPrice {};
        Price realizedPnL {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_POSITION_HPP