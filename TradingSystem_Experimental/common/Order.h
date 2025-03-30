/**============================================================================
Name        : Order.h
Created on  : 10.08.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order.h
============================================================================**/

#ifndef CPPPROJECTS_ORDER_H
#define CPPPROJECTS_ORDER_H

#include <cstdint>
#include <string>
#include <iostream>


namespace Common
{
    enum class OrderSide : uint8_t
    {
        BUY,
        SELL,
    };

    enum class OrderType : uint8_t
    {
        LIMIT,
        MARKET,
        STOP_LIMIT,
        STOP_MARKET,
        TAKE_PROFIT_LIMIT,
        TAKE_PROFIT_MARKET,
    };

    enum class OrderStopCondition : uint8_t
    {
        NONE,
        GREATER_EQUAL,
        LESS_EQUAL,
    };

    enum class OrderAction : uint8_t
    {
        NEW,
        AMEND,
        CANCEL
    };

    enum class OrderTimeCondition : uint8_t
    {
        GTC,
        IOC,
        FOK,
        MAKER_ONLY,
        MAKER_ONLY_REPRICE,
    };

    enum class OrderStatus : uint8_t
    {
        OPEN,
        PARTIAL_FILL,
        FILLED,
        CANCELED_BY_USER,
        CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED,
        CANCELED_BY_MARKET_ORDER_NOTHING_MATCH,
        CANCELED_ALL_BY_IOC,
        CANCELED_PARTIAL_BY_IOC,
        CANCELED_BY_FOK,
        CANCELED_BY_MAKER_ONLY,
        CANCELED_BY_AMEND,
        CANCELED_BY_SELF_TRADE_PROTECTION,
        REJECT_CANCEL_ORDER_ID_NOT_FOUND,
        REJECT_AMEND_ORDER_ID_NOT_FOUND,
        REJECT_DISPLAY_QUANTITY_ZERO,
        REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY,
        REJECT_QUANTITY_AND_AMOUNT_ZERO,
        REJECT_LIMIT_ORDER_WITH_MARKET_PRICE,
        REJECT_MATCHING_ENGINE_RECOVERING,
        REJECT_STOP_CONDITION_IS_NONE,
        REJECT_STOP_TRIGGER_PRICE_IS_NONE,
        REJECT_QUANTITY_AND_AMOUNT_LARGER_ZERO,
        REJECT_PRICE_LESS_ZERO,
        REJECT_ORDER_QUANTITY_GREATER_THAN_MAX,
        REJECT_ORDER_AMOUNT_GREATER_THAN_MAX,
        REJECT_ORDER_TIMEOUT,
        REJECT_MAXIMUM_OPEN_ORDERS_LIMIT_REACHED
    };

    enum class OrderMatchedType : uint8_t
    {
        MAKER,
        TAKER,
    };

    struct Order
    {
        using IDType   = uint64_t;
        using OrderID  = IDType;
        using Price    = uint64_t;
        using Quantity = uint64_t;
        using Amount   = uint64_t;

        OrderSide side { OrderSide::BUY };
        OrderType type { OrderType::LIMIT };
        OrderAction action { OrderAction::NEW };
        OrderStatus status { OrderStatus::OPEN };

        OrderStopCondition stopCondition { OrderStopCondition::NONE };
        OrderTimeCondition timeCondition { OrderTimeCondition::GTC };
        OrderMatchedType matchedType { OrderMatchedType::MAKER };

        OrderID orderId { 0 };
        Price price { 0 };
        Quantity quantity { 0 };
    };

    struct OrderBuilder
    {
        OrderBuilder& setOrderSide(OrderSide side) noexcept;
        OrderBuilder& setOrderType(OrderType type) noexcept;
        OrderBuilder& setOrderActionType(OrderAction action) noexcept;
        OrderBuilder& setOrderTimeCondition(OrderTimeCondition condition) noexcept;
        OrderBuilder& setOrderStatusType(OrderStatus statusType) noexcept;
        OrderBuilder& setOrderMatchedType(OrderMatchedType matchedType) noexcept;

        OrderBuilder& setOrderId(Order::OrderID id) noexcept;
        OrderBuilder& setQuantity(unsigned long long quantity) noexcept;
        OrderBuilder& setPrice(long long price) noexcept;

        [[nodiscard]]
        Order build() const noexcept;

    private:
        Order order;
    };

    void printOrder(const Order& order, const std::string& offset = "\t\t");
}

#endif //CPPPROJECTS_ORDER_H
