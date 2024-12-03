/**============================================================================
Name        : Testing.cpp
Created on  : 03.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing harness
============================================================================**/

#include "Testing.h"


namespace Testing
{
    Memory::ObjectPool<Common::Order> ordersPool;

    uint64_t getNextOrderID()
    {
        static uint64_t id { 1'000 };
        return id++;
    }

    OrderPtr createOrder(const Common::OrderSide orderSide,
                         const Common::OrderActionType action,
                         const Common::Order::Price price ,
                         unsigned long long quantity,
                         const  Common::Order::OrderID orderId)
    {
        OrderPtr order { ordersPool.acquireObject() };
        order->action = action;
        order->side = orderSide;
        order->price = price;
        order->quantity = quantity;
        order->orderId = orderId;
        return order;
    }

    void PostOrders(OrderMatchingEngineTester& engine,
                    const std::vector<Common::Order::Price>& prices,
                    const Common::OrderSide orderSide = Common::OrderSide::BUY,
                    const Common::OrderActionType action = Common::OrderActionType::NEW,
                    const uint32_t orderPerPrice = 1,
                    const uint32_t quantity = 1)
    {
        for (uint32_t n = 0, priceIdx = 0; n < prices.size() * orderPerPrice; ++n)
        {
            OrderPtr order = createOrder(orderSide,  action, prices[priceIdx++], quantity);
            engine.processOrder(std::move(order));

            if (priceIdx >= prices.size())
                priceIdx = 0;
        }
    }
}