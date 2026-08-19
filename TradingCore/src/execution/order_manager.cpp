/**============================================================================
Name        : order_manager.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.cpp
============================================================================**/

/*
    OrderManager implementation.

    Order creation flow:

        OrderRequest
             |
             v
        RiskManager
             |
       +-----+-----+
       |           |
    Reject       Accept
       |           |
       v           v
    return 0    create Order
                   |
                   v
                store
                   |
                   v
            gateway.send()

    Execution reports are handled separately and update the locally stored
    order state.

    Position changes are intentionally not performed when an order is created.
    PositionManager must update the position only after an execution report
    confirms an actual trade.
*/

#include "order_manager.hpp"

namespace trading::execution
{
    OrderManager::OrderManager(IExecutionGateway& gateway,
                               risk::IRiskManager& riskManager,
                               position::Position& position) noexcept:
        gateway { gateway },
        riskManager { riskManager },
        position { position }
    {
    }

    OrderId OrderManager::createOrder(const OrderRequest& request)
    {
        if (riskManager.checkOrder(request, position) == risk::RiskResult::Rejected)
            return {};

        const OrderId orderId = nextOrderId++;

        Order order {
            .clientOrderId = orderId,
            .exchangeOrderId = 0,
            .instrument = request.instrument,
            .side = request.side,
            .type = request.type,
            .price = request.price,
            .quantity = request.quantity,
            .filledQuantity = {},
            .status = OrderStatus::New
        };

        orders.emplace(orderId, order);
        gateway.send(order);

        return orderId;
    }

    bool OrderManager::applyExecution(const ExecutionReport& report)
    {
        const auto it = orders.find(report.clientOrderId);
        if (it == orders.end())
            return false;

        Order& order = it->second;

        order.exchangeOrderId = report.exchangeOrderId;
        order.status = report.status;
        order.filledQuantity = report.filledQuantity;

        return true;
    }

    const Order* OrderManager::find(const OrderId orderId) const noexcept
    {
        const auto it = orders.find(orderId);
        if (it == orders.end())
            return nullptr;

        return &it->second;
    }

    bool OrderManager::cancel(const OrderId orderId)
    {
        if (const auto it = orders.find(orderId); it == orders.end())
            return false;
        gateway.cancel(orderId);

        return true;
    }
}