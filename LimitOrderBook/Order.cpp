/**============================================================================
Name        : Order.cpp
Created on  : 29.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order.cpp
============================================================================**/

#include "Order.h"

namespace Common
{
    Order OrderBuilder::build() const noexcept
    {
        return order;
    }

    OrderBuilder& OrderBuilder::setOrderSide(OrderSide side) noexcept
    {
        order.side = side;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderType(OrderType type) noexcept {
        order.type = type;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderActionType(OrderActionType actionType) noexcept {
        order.action = actionType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderId(Order::OrderID id) noexcept
    {
        order.orderId = id;
        return *this;
    }

    OrderBuilder& OrderBuilder::setPrice(long long price) noexcept {
        order.price = price;
        return *this;
    }


    OrderBuilder& OrderBuilder::setQuantity(unsigned long long quantity) noexcept {
        order.quantity = quantity;
        return *this;
    }
}