/**============================================================================
Name        : Order.h
Created on  : 29.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_H
#define FINANCETECHNOLOGYPROJECTS_ORDER_H

#include <cstdint>

namespace Common
{
    enum class OrderSide : uint8_t
    {
        Buy = 0x00,
        Sell = 0x01,
    };

    enum class OrderType : uint8_t
    {
        Limit = 0x00,
        Market = 0x01,
        StopLimit = 0x02,
        StopMarket = 0x03,
        TakeProfitLimit = 0x04,
        TakeProfitMarket = 0x05,
    };

    enum class OrderActionType : uint8_t
    {
        New = 0x00,
        Amend = 0x01,
        Cancel = 0x02,
        Recovery = 0x03,
        RecoveryEnd = 0x04,
    };

    struct Order
    {
        using IDType   = uint64_t;
        using OrderID  = IDType;
        using Price    = uint64_t;
        using Quantity = uint64_t;

        OrderSide side { OrderSide::Buy };
        OrderType type { OrderType::Limit };
        OrderActionType action { OrderActionType::New };

        OrderID orderId { 0 };
        Price price { 0 };
        Quantity quantity { 0 };
    };


    struct OrderBuilder
    {
        OrderBuilder& setOrderSide(OrderSide side) noexcept;
        OrderBuilder& setOrderType(OrderType type) noexcept;
        OrderBuilder& setOrderActionType(OrderActionType actionType) noexcept;

        OrderBuilder& setOrderId(Order::OrderID id) noexcept;
        OrderBuilder& setQuantity(unsigned long long quantity) noexcept;
        OrderBuilder& setPrice(long long price) noexcept;

        [[nodiscard]]
        Order build() const noexcept;

    private:
        Order order;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_H
