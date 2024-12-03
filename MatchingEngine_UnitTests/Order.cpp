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
    void printOrder(const Order& order,
                    const std::string& offset)
    {
        std::cout << offset << "Order (id: " << order.orderId
                  << ", side: " << (order.side == OrderSide::BUY ? "BUY" : "SELL")
                  << ", price: " << order.price
                  << ", quantity: " << order.quantity << ")\n";
    }


    std::ostream& operator<<(std::ostream& stream, const Order& order)
    {
        stream << "Order { id: " << order.orderId
               << ", side: " << (order.side == OrderSide::BUY ? "BUY" : "SELL")
               << ", price: " << order.price
               << ", quantity: "
               << order.quantity << '}';
        return stream;
    }
}


namespace Common
{
    Order OrderBuilder::build() const noexcept
    {
        return order;
    }

    OrderBuilder& OrderBuilder::setOrderSide(const OrderSide side) noexcept
    {
        order.side = side;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderType(const OrderType type) noexcept {
        order.type = type;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderActionType(const OrderActionType actionType) noexcept {
        order.action = actionType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderTimeCondition(const OrderTimeCondition condition) noexcept {
        order.timeCondition = condition;
        return *this;
    }

    OrderBuilder& OrderBuilder::setTriggerType(const TriggerType triggerType) noexcept {
        order.triggerType = triggerType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderStatusType(const OrderStatusType statusType) noexcept {
        order.status = statusType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderMatchedType(const OrderMatchedType matchedType) noexcept {
        order.matchedType = matchedType;
        return *this;
    }

    OrderBuilder& OrderBuilder::setOrderId(const Order::OrderID id) noexcept
    {
        order.orderId = id;
        return *this;
    }

    OrderBuilder& OrderBuilder::setMarketId(const Order::IDType marketId) noexcept {
        order.marketId = marketId;
        return *this;
    }

    OrderBuilder& OrderBuilder::setAmount(const unsigned long long amount) noexcept {
        order.amount = amount;
        return *this;
    }

    OrderBuilder& OrderBuilder::setRemainAmount(const unsigned long long remainAmount) noexcept {
        order.remainAmount = remainAmount;
        return *this;
    }

    OrderBuilder& OrderBuilder::setQuantity(const unsigned long long quantity) noexcept {
        order.quantity = quantity;
        return *this;
    }

    OrderBuilder& OrderBuilder::setDisplayQuantity(const unsigned long long displayQuantity) noexcept {
        order.displayQuantity = displayQuantity;
        return *this;
    }

    OrderBuilder& OrderBuilder::setRemainQuantity(const unsigned long long remainQuantity) noexcept {
        order.remainQuantity = remainQuantity;
        return *this;
    }

    OrderBuilder& OrderBuilder::setPrice(const long long price) noexcept {
        order.price = price;
        return *this;
    }

    OrderBuilder& OrderBuilder::setAccountId(const Order::IDType accountId) noexcept {
        order.accountId = accountId;
        return *this;
    }

    OrderBuilder& OrderBuilder::setParentAccountId(const Order::IDType parentAccountId) noexcept {
        order.parentAccountId = parentAccountId;
        return *this;
    }
}