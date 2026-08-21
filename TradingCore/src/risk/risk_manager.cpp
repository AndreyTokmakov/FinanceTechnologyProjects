/**============================================================================
Name        : risk_manager.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : risk_manager.cpp
============================================================================**/

/*
    RiskManager implementation.

    Data Flow:

        OrderRequest + Position
                  |
                  v
             RiskManager
                  |
          +-------+-------+
          |               |
       Accepted         Rejected
          |               |
          v               v
      OrderManager     caller
          |
          v
    IExecutionGateway

    Responsibilities:

        - enforce configured risk limits;
        - calculate the position that would exist after the requested order;
        - reject orders that exceed configured limits.

    The position used for validation is the position immediately before the
    order is executed.

    RiskManager does not reserve risk or modify the position. Actual position
    changes happen later when ExecutionReport events are processed.
*/

#include "risk_manager.hpp"

#include <algorithm>

namespace trading::risk
{
    namespace
    {
        using WideValue = __int128_t;

        [[nodiscard]]
        constexpr WideValue absolute(const WideValue value) noexcept
        {
            return value < 0 ? -value : value;
        }

        [[nodiscard]]
        constexpr WideValue positionDelta(const execution::OrderRequest& request) noexcept
        {
            const WideValue quantity = static_cast<WideValue>(request.quantity.raw());
            return request.side == trading::Side::Buy ? quantity : -quantity;
        }
    }

    RiskManager::RiskManager() noexcept = default;

    RiskManager::RiskManager(const RiskLimits& limits) noexcept :
        limits { limits }
    {
    }

    void RiskManager::setLimits(const RiskLimits& newLimits) noexcept
    {
        this->limits = newLimits;
        reason = RiskReason::None;
    }

    RiskResult RiskManager::checkOrder(const execution::OrderRequest& request,
                                       const position::Position& position)
    {
        reason = RiskReason::None;

        if (!limits.maxOrderQuantity.isZero() && request.quantity > limits.maxOrderQuantity)
        {
            reason = RiskReason::MaxOrderQuantity;
            return RiskResult::Rejected;
        }

        const WideValue currentPosition = static_cast<WideValue>(position.quantity());
        const WideValue resultingPosition = currentPosition + positionDelta(request);

        if (!limits.maxPositionQuantity.isZero())
        {
            const WideValue maximumPosition = static_cast<WideValue>(limits.maxPositionQuantity.raw());
            if (absolute(resultingPosition) > maximumPosition)
            {
                reason = RiskReason::MaxPositionQuantity;
                return RiskResult::Rejected;
            }
        }

        if (!limits.maxNotional.isZero())
        {
            const WideValue price = static_cast<WideValue>(request.price.raw());
            const WideValue quantity = static_cast<WideValue>(request.quantity.raw());
            const WideValue notional = price * quantity / trading::Price::Scale;

            if (absolute(notional) > static_cast<WideValue>(limits.maxNotional.raw()))
            {
                reason = RiskReason::MaxNotional;
                return RiskResult::Rejected;
            }
        }

        return RiskResult::Accepted;
    }

    RiskReason RiskManager::lastReason() const noexcept
    {
        return reason;
    }
}