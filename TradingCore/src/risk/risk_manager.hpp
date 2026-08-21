/**============================================================================
Name        : risk_manager.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : risk_manager.hpp
============================================================================**/

/*
    IRiskManager and RiskManager validate orders before they reach the
    execution gateway.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        OrderManager
           |
           | checkOrder()
           v
        IRiskManager
           |
           v
        RiskManager
           |
           +----------------------+
           |                      |
           | Accepted             | Rejected
           v                      v
        OrderManager           caller
           |
           v
        IExecutionGateway
           |
           v
        Exchange

    Position is supplied by PositionManager and represents the current
    position before the requested order is executed.

    Responsibilities:

        - validate a requested order;
        - validate maximum order quantity;
        - validate maximum resulting position;
        - validate maximum order notional;
        - expose the reason for the most recent rejection.

    RiskManager does not:

        - send or cancel orders;
        - modify Position;
        - modify Order;
        - communicate with an exchange;
        - calculate PnL.

    The risk check is performed before the order is inserted into OrderManager
    and before IExecutionGateway::send() is called.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RISK_MANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_RISK_MANAGER_HPP

#include "order.hpp"
#include "position.hpp"
#include "risk.hpp"
#include "risk_limits.hpp"

namespace trading::risk
{
    struct IRiskManager
    {
        virtual ~IRiskManager() = default;

        [[nodiscard]]
        virtual RiskResult checkOrder(const execution::OrderRequest& request,
                                      const position::Position& position) = 0;

        [[nodiscard]]
        virtual RiskReason lastReason() const noexcept = 0;
    };


    class RiskManager final : public IRiskManager
    {
    public:
        RiskManager() noexcept;
        explicit RiskManager(const RiskLimits& limits) noexcept;

        void setLimits(const RiskLimits& newLimits) noexcept;

        [[nodiscard]]
        RiskResult checkOrder(const execution::OrderRequest& request,
                              const position::Position& position) override;

        [[nodiscard]]
        RiskReason lastReason() const noexcept override;

    private:
        RiskLimits limits {};
        RiskReason reason { RiskReason::None };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RISK_MANAGER_HPP