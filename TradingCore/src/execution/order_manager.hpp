/**============================================================================
Name        : order_manager.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.hpp
============================================================================**/

/*
    OrderManager owns the local lifecycle of client orders.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        OrderManager
           |
           v
        IRiskManager
           |
           +---- Rejected
           |
           +---- Accepted
                  |
                  v
               Order
                  |
                  v
          IExecutionGateway
                  |
                  v
              Exchange
                  |
                  | ExecutionReport
                  v
             OrderManager

    Responsibilities:

        - create client order identifiers;
        - validate orders through IRiskManager;
        - store accepted orders;
        - send accepted orders to IExecutionGateway;
        - apply ExecutionReport updates;
        - request order cancellation;
        - provide access to locally tracked orders.

    RiskManager is consulted before an Order is created and before the order
    is sent to the execution gateway.

    OrderManager does not implement risk rules itself.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP

#include "execution_gateway.hpp"
#include "execution_report.hpp"
#include "order.hpp"
#include "position.hpp"
#include "risk_manager.hpp"

#include <map>

namespace trading::execution
{
    class OrderManager
    {
    public:
        OrderManager(IExecutionGateway& gateway,
                     risk::IRiskManager& riskManager,
                     position::Position& position) noexcept;

        [[nodiscard]]
        OrderId createOrder(const OrderRequest& request);

        [[nodiscard]]
        bool applyExecution(const ExecutionReport& report);

        [[nodiscard]]
        const Order* find(OrderId orderId) const noexcept;

        [[nodiscard]]
        bool cancel(OrderId orderId);

    private:
        IExecutionGateway& gateway;
        risk::IRiskManager& riskManager;
        position::Position& position;
        std::map<OrderId, Order> orders;
        OrderId nextOrderId { 1 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP