/**============================================================================
Name        : Order.cpp
Created on  : 10.08.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order.cpp
============================================================================**/

#include "Order.h"

namespace Common
{
    void printOrder(const Order& order, const std::string& offset)
    {
        std::cout << offset << "Order (id: " << order.orderId << ", price: " << order.price
                  << ", quantity: " << order.quantity << ")\n";
    }
}


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

    OrderBuilder& OrderBuilder::setOrderTimeCondition(OrderTimeCondition condition) noexcept {
        order.timeCondition = condition;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderMatchedType(OrderMatchedType matchedType) noexcept {
        order.matchedType = matchedType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderId(Order::OrderID id) noexcept
    {
        order.orderId = id;
        return *this;
    }

    OrderBuilder& OrderBuilder::setQuantity(unsigned long long quantity) noexcept {
        order.quantity = quantity;
        return *this;
    }

    OrderBuilder& OrderBuilder::setPrice(long long price) noexcept {
        order.price = price;
        return *this;
    }
}