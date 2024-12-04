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


    void OrderMatchingEngineTester::info([[maybe_unused]] bool printTrades)
    {
        auto printOrders = [](const auto& orderMap)
        {
            for (const auto& [price, priceLevel]: orderMap)
            {
                std::cout << "\tPrice Level : [" << price << ", quantity: "
                          << priceLevel->quantity << "]" << std::endl;
                for (const auto & orderIter: priceLevel->orders) {
                    Common::printOrder(*orderIter);
                }
            }
        };

        std::cout << "BUY:  " << std::endl; printOrders(bidPriceLevelMap);
        std::cout << "SELL: " << std::endl; printOrders(askPriceLevelMap);
        std::cout << std::string(160, '=') << std::endl;

        /*
        if (!printTrades)
            return;
        for (const auto& trade: trades.trades)
        {
            std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                      << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                      << "quantity: " << trade.quantity << ")\n";
        }*/
    }

    size_t OrderMatchingEngineTester::getBuyOrdersCount() const noexcept
    {
        size_t count { 0 };
        for (const auto& [price, orderList]: bidPriceLevelMap)
            count += orderList->orders.size();
        return count;
    }

    size_t OrderMatchingEngineTester::getSellOrdersCount() const noexcept
    {
        size_t count { 0 };
        for (const auto& [price, orderList]: askPriceLevelMap)
            count += orderList->orders.size();
        return count;
    }


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