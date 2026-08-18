/**============================================================================
Name        : pnl_calculator.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : pnl_calculator.cpp
============================================================================**/

/*
    PnLCalculator implementation.

    PnLCalculator calculates PnL without owning any trading state.

    Data Flow:

        Position before Trade
               |
               | + ExecutionReport
               v
        calculateRealized()
               |
               v
        Realized PnL


        Position + MarketEvent
               |
               v
        calculateUnrealized()
               |
               v
        Unrealized PnL


        Realized + Unrealized
               |
               v
              PnL

    Responsibilities:

        - calculate realized PnL for executed trades;
        - calculate unrealized PnL using the current market;
        - handle partial position closes;
        - handle position reversals;
        - perform fixed-point arithmetic using a wider intermediate type.

    The Position supplied to calculateRealized() represents the position
    immediately before the execution described by ExecutionReport.

    This is important because the calculator needs the previous position
    direction and average entry price to determine which part of the trade
    closes an existing position.

    PnLCalculator does not update Position. PositionManager remains the owner
    of position state.
*/

#include "pnl_calculator.hpp"

#include <algorithm>

namespace trading::pnl
{
    namespace
    {
        using WideValue = __int128_t;

        constexpr WideValue PRICE_SCALE = trading::Price::Scale;

        [[nodiscard]]
        constexpr WideValue absolute(const WideValue value) noexcept
        {
            return value < 0 ? -value : value;
        }

        [[nodiscard]]
        constexpr Price fromProduct(const WideValue priceDifference,
                                     const WideValue quantity) noexcept
        {
            const WideValue value = priceDifference * quantity / PRICE_SCALE;
            return Price { static_cast<Price::Value>(value) };
        }
    }

    Price PnLCalculator::calculateRealized(const position::Position& position,
                                           const execution::ExecutionReport& report) noexcept
    {
        if (report.execType != ExecType::Trade || report.quantity.isZero())
            return {};

        const int64_t positionQuantity = position.quantity();
        if (positionQuantity == 0)
            return {};

        const int64_t executionQuantity = report.quantity.raw();
        if (positionQuantity > 0 && report.side == Side::Sell)
        {
            const int64_t closedQuantity = std::min(positionQuantity, executionQuantity);
            const WideValue priceDifference = static_cast<WideValue>(report.price.raw()) -
                static_cast<WideValue>(position.averagePrice().raw());
            return fromProduct(priceDifference, closedQuantity);
        }

        if (positionQuantity < 0 && report.side == Side::Buy)
        {
            const WideValue absolutePositionQuantity = absolute(positionQuantity);
            const WideValue closedQuantity = std::min(absolutePositionQuantity, static_cast<WideValue>(executionQuantity));
            const WideValue priceDifference = static_cast<WideValue>(position.averagePrice().raw()) -
                static_cast<WideValue>(report.price.raw());
            return fromProduct(priceDifference, closedQuantity);
        }

        return {};
    }

    Price PnLCalculator::calculateUnrealized(const position::Position& position,
                                             const market_data::MarketEvent& marketEvent) noexcept
    {
        const int64_t positionQuantity = position.quantity();
        if (positionQuantity == 0)
            return {};

        if (positionQuantity > 0)
        {
            if (marketEvent.bestBidQuantity.isZero())
                return {};

            const WideValue priceDifference =static_cast<WideValue>(marketEvent.bestBid.raw()) -
                static_cast<WideValue>(position.averagePrice().raw());

            return fromProduct(priceDifference, positionQuantity);
        }

        if (marketEvent.bestAskQuantity.isZero())
            return {};

        const WideValue priceDifference =static_cast<WideValue>(position.averagePrice().raw()) -
            static_cast<WideValue>(marketEvent.bestAsk.raw());

        return fromProduct(priceDifference, absolute(positionQuantity));
    }

    PnL PnLCalculator::calculate(const position::Position& position,
                                 const execution::ExecutionReport& report,
                                 const market_data::MarketEvent& marketEvent) noexcept
    {
        return PnL {
            .realized = calculateRealized(position, report),
            .unrealized = calculateUnrealized(position, marketEvent)
        };
    }
}