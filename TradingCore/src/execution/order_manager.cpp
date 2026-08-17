/**============================================================================
Name        : order_manager.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.cpp
============================================================================**/

#include "order_manager.hpp"

namespace trading::execution
{
    OrderManager::OrderManager(IExecutionGateway& gateway) noexcept
        : gateway { gateway }
    {
    }

    OrderId OrderManager::createOrder(const OrderRequest& request)
    {
        const OrderId orderId = nextOrderId++;

        Order order {
            .clientOrderId = orderId,
            .instrument = request.instrument,
            .side = request.side,
            .type = request.type,
            .price = request.price,
            .quantity = request.quantity
        };

        orders.emplace(orderId, order);

        gateway.send(order);

        return orderId;
    }

    bool OrderManager::applyExecutionReport(const ExecutionReport& report)
    {
        const auto it = orders.find(report.clientOrderId);

        if (it == orders.end())
            return false;

        auto& order = it->second;

        order.exchangeOrderId = report.exchangeOrderId;
        order.status = report.status;
        order.filledQuantity = report.filledQuantity;

        return true;
    }

    const Order* OrderManager::find(OrderId orderId) const noexcept
    {
        const auto it = orders.find(orderId);

        if (it == orders.end())
            return nullptr;

        return &it->second;
    }

    bool OrderManager::cancel(OrderId orderId)
    {
        const auto it = orders.find(orderId);

        if (it == orders.end())
            return false;

        gateway.cancel(orderId);

        return true;
    }

}