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
        BUY = 0x00,
        SELL = 0x01,
    };

    enum class OrderType : uint8_t
    {
        LIMIT = 0x00,
        MARKET = 0x01,
        STOP_LIMIT = 0x02,
        STOP_MARKET = 0x03,
        TAKE_PROFIT_LIMIT = 0x04,
        TAKE_PROFIT_MARKET = 0x05,
    };

    enum class OrderStopCondition : uint8_t
    {
        NONE = 0x00,
        GREATER_EQUAL = 0x01,
        LESS_EQUAL = 0x02,
    };

    enum class OrderActionType : uint8_t
    {
        NEW = 0x00,
        AMEND = 0x01,
        CANCEL = 0x02,
        RECOVERY = 0x03,
        RECOVERY_END = 0x04,
    };

    enum class OrderTimeCondition : uint8_t
    {
        GTC = 0x00,
        IOC = 0x01,
        FOK = 0x02,
        MAKER_ONLY = 0x03,
        MAKER_ONLY_REPRICE = 0x04,
    };

    enum class TriggerType : uint8_t
    {
        MARK_PRICE,
        LAST_PRICE,
        TRIGGER_NONE,
    };

    enum class SelfTradeProtectionType : uint8_t
    {
        STP_NONE,
        STP_TAKER,
        STP_MAKER,
        STP_BOTH,
    };

    enum class OrderStatusType : uint8_t
    {
        OPEN = 0x00,
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
    };

    enum class OrderMatchedType : uint8_t
    {
        MAKER = 0x00,
        TAKER = 0x01
    };

    struct Order
    {
        using IDType  = uint64_t;
        using OrderID = IDType;
        using Price   = uint64_t;

        OrderSide side { OrderSide::BUY };
        OrderType type { OrderType::LIMIT };
        TriggerType triggerType { TriggerType::LAST_PRICE };
        OrderStopCondition stopCondition { OrderStopCondition::NONE };
        OrderTimeCondition timeCondition { OrderTimeCondition::GTC };
        OrderActionType action { OrderActionType::NEW };
        OrderStatusType status { OrderStatusType::OPEN };
        OrderMatchedType matchedType { OrderMatchedType::MAKER };
        SelfTradeProtectionType selfTradeProtectionType { SelfTradeProtectionType::STP_NONE };

        OrderID orderId { 0 };
        IDType accountId { 0 };
        IDType parentAccountId { 0 };
        IDType marketId { 0 };
        long long price { 0 };
        unsigned long long quantity { 0 };
        unsigned long long displayQuantity { 0 };
        unsigned long long remainQuantity { 0 };
        unsigned long long amount { 0 };
        unsigned long long remainAmount { 0 };

        unsigned char buffer[512] {};
    };

    struct OrderBuilder
    {
        OrderBuilder& setOrderSide(OrderSide side) noexcept;
        OrderBuilder& setOrderType(OrderType type) noexcept;
        OrderBuilder& setOrderActionType(OrderActionType actionType) noexcept;
        OrderBuilder& setOrderTimeCondition(OrderTimeCondition condition) noexcept;
        OrderBuilder& setTriggerType(TriggerType triggerType) noexcept;
        OrderBuilder& setOrderStatusType(OrderStatusType statusType) noexcept;
        OrderBuilder& setOrderMatchedType(OrderMatchedType matchedType) noexcept;

        OrderBuilder& setOrderId(Order::OrderID id) noexcept;
        OrderBuilder& setMarketId(Order::IDType marketId) noexcept;
        OrderBuilder& setAmount(unsigned long long amount) noexcept;
        OrderBuilder& setRemainAmount(unsigned long long remainAmount) noexcept;
        OrderBuilder& setQuantity(unsigned long long quantity) noexcept;
        OrderBuilder& setDisplayQuantity(unsigned long long displayQuantity) noexcept;
        OrderBuilder& setRemainQuantity(unsigned long long remainQuantity) noexcept;
        OrderBuilder& setPrice(long long price) noexcept;
        OrderBuilder& setAccountId(Order::IDType accountId) noexcept;
        OrderBuilder& setParentAccountId(Order::IDType parentAccountId) noexcept;

        [[nodiscard]]
        Order build() const noexcept;

    private:
        Order order;
    };

    void printOrder(const Order& order, const std::string& offset = "\t\t");
}

#endif //CPPPROJECTS_ORDER_H
