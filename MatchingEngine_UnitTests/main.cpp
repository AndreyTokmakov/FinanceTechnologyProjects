/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "MatchingEngine.h"
#include "PerfUtilities.h"


namespace Testing
{
    using namespace Common;

    uint64_t getNextOrderID()
    {
        static uint64_t id { 1'000 };
        return id++;
    }

    struct OrderMatchingEngineTester : MatchingEngine
    {
        using MatchingEngine::OrderPtr;

        void info([[maybe_unused]] bool printTrades = true)
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

        // TODO: Add MetaData [quantity, ...] to each askPriceLevelMap
        size_t getBuyOrdersCount() const noexcept
        {
            size_t count { 0 };
            for (const auto& [price, orderList]: bidPriceLevelMap)
                count += orderList->orders.size();
            return count;
        }

        // TODO: Add MetaData [quantity, ...] to each askPriceLevelMap
        size_t getSellOrdersCount() const noexcept
        {
            size_t count { 0 };
            for (const auto& [price, orderList]: askPriceLevelMap)
                count += orderList->orders.size();
            return count;
        }
    };

    using OrderPtr = OrderMatchingEngineTester::OrderPtr;
    Memory::ObjectPool<Order> ordersPool;

    OrderPtr createOrder(const Common::OrderSide orderSide,
                         const Common::OrderActionType action = Common::OrderActionType::NEW,
                         const Common::Order::Price price = 1,
                         unsigned long long quantity = 0,
                         const  Common::Order::OrderID orderId = getNextOrderID())
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
                    const std::vector<Order::Price>& prices,
                    const Common::OrderSide orderSide = OrderSide::BUY,
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


namespace Tests
{
    using namespace Testing;
    using OrderPtr = OrderMatchingEngineTester::OrderPtr;

    void Load_Test()
    {
        constexpr uint32_t pricesCount { 50  }, initialPrice { 10 };
        constexpr uint32_t buyOrders { 100 }, sellOrders { buyOrders };

        OrderMatchingEngineTester engine;

        std::vector<int32_t> prices(pricesCount);
        std::iota(prices.begin(), prices.end(), initialPrice);

        std::vector<uint64_t> iDs;
        iDs.reserve(pricesCount * buyOrders * 10);


        PerfUtilities::ScopedTimer timer { "MatchingEngineEx"};
        uint64_t count = 0;

        for (int i = 0; i < 400; ++i)
        {
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::BUY;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::SELL;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::SELL;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::BUY;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            iDs.clear();
        }
        std::cout << count << std::endl;
    }

}



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    Tests::Load_Test();

    return EXIT_SUCCESS;
}
