/**============================================================================
Name        : MatchingEngine.cpp
Created on  : 10.08.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngine.cpp
============================================================================**/

#include "MatchingEngine.h"
#include "PerfUtilities.h"
#include "Order.h"

#include <iostream>
#include <numeric>
#include <list>
#include <vector>
#include <unordered_map>

#include <boost/container/flat_map.hpp>

namespace
{
    uint64_t getNextOrderID()
    {
        static uint64_t id { 1'000 };
        return id++;
    }
}


namespace TestMatchingEngine
{
    using namespace Common;

    struct Trade
    {
        struct OrderInfo
        {
            Order::OrderID id = 0;
            Order::OrderID price = 0;
        };

        OrderInfo buyOrderInfo;
        OrderInfo sellOrderInfo;
        uint32_t  quantity = 0;

        static void addOrder(const Order& order, OrderInfo& orderInfo)
        {
            orderInfo.id = order.orderId;
            orderInfo.price = order.price;
        }

        Trade& setBuyOrder(const Order& order) {
            addOrder(order, buyOrderInfo);
            return *this;
        }

        Trade& setSellOrder(const Order& order) {
            addOrder(order, sellOrderInfo);
            return *this;
        }

        Trade& setQuantity(const uint32_t qnty) {
            quantity = qnty;
            return *this;
        }
    };

    struct Trades
    {
        std::vector<Trade> trades {};

        Trade& addTrade() {
            return trades.emplace_back();
        }
    };

    struct OrderMatchingEngine
    {
        using OrderIter = typename std::list<Order>::iterator;
        using PriceOrderList = std::list<OrderIter>;
        using PriceOrderIter = typename PriceOrderList::iterator;

        struct ReferencesBlock
        {
            OrderIter orderIter;
            PriceOrderIter priceOrderIter;
            PriceOrderList* priceLevelOrderList;
        };

        std::list<Order> orders {};
        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        // TODO: Test replace std::map --> boost::flat_map [std::list --> shall be pointer?]
        //       Since look performance of this lookup is more critical one

#if 0
        std::map<Order::Price, PriceOrderList, std::less<>> buyOrders;
        std::map<Order::Price, PriceOrderList, std::greater<>> sellOrders;
#else
        boost::container::flat_map<Order::Price, PriceOrderList, std::less<>> buyOrders;
        boost::container::flat_map<Order::Price, PriceOrderList, std::greater<>> sellOrders;
#endif

        Trades trades;

        void processOrder(Order& order)
        {
            // TODO: Remove branching ???
            switch (order.action)
            {
                case OrderActionType::NEW:
                    return handleOrderNew(order);
                case OrderActionType::CANCEL:
                    return handleOrderCancel(order);
                case OrderActionType::AMEND:
                    return handleOrderAmend(order);
                default:
                    return;
            }
        }

        unsigned long long matchOrder(Order& order)
        {
            // TODO: Remove branching ???
            if (OrderSide::SELL == order.side) {
                matchOrder(order, buyOrders);
            } else {
                matchOrder(order, sellOrders);
            }

            // Return remaining quantity
            return order.quantity;
        }

        template<typename OrderSideMap>
        void matchOrder(Order& order, OrderSideMap& oppositeSideOrdersPriceMap)
        {
            auto matchedPriceLevelIter = oppositeSideOrdersPriceMap.lower_bound(order.price);
            while (oppositeSideOrdersPriceMap.end() != matchedPriceLevelIter && order.quantity > 0)
            {
                matchOrderList(order, matchedPriceLevelIter->second);
                ++matchedPriceLevelIter;
            }
        }

        void matchOrderList(Order& order,
                            PriceOrderList& matchedOrderList)
        {
            for (auto orderIter = matchedOrderList.begin(); matchedOrderList.end() != orderIter;)
            {
                Order& matchedOrder = *(*orderIter);

                Trade& trade = trades.addTrade();
                trade.setQuantity(std::min(matchedOrder.quantity,order.quantity));
                // TODO: Remove branching ???
                if (OrderSide::SELL == order.side) {
                    trade.setBuyOrder(matchedOrder).setSellOrder(order);
                } else {
                    trade.setBuyOrder(order).setSellOrder(matchedOrder);
                }

                if (order.quantity >= matchedOrder.quantity)
                {
                    order.quantity -= matchedOrder.quantity;
                    matchedOrder.quantity = 0;

                    /** Deleting order **/
                    orderByIDMap.erase(matchedOrder.orderId);
                    matchedOrderList.erase(orderIter++);
                } else {
                    matchedOrder.quantity -= order.quantity;
                    order.quantity = 0;
                    ++orderIter;
                    return;;
                }
            }
        }

        void handleOrderNew(Order& order)
        {
            if (0 == matchOrder(order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(order.orderId, ReferencesBlock{});
            if (inserted)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = iterOrderMap->second;
                orderIter = orders.insert(orders.end(), order);
                // TODO: Remove branching ???
                priceLevelOrderList = (OrderSide::BUY == order.side) ?
                                      &buyOrders[order.price] : &sellOrders[order.price];
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
            }
        }

        void handleOrderCancel(const Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = orderByIDIter->second;
                priceLevelOrderList->erase(priceOrderIter);
                orders.erase(orderIter);
                orderByIDMap.erase(orderByIDIter);
            }
        }

        void handleOrderAmend(Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                // TODO: Remove branching ???
                Order& orderOriginal = *(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order.side) {
                    return;
                } else if (orderOriginal.price != order.price) {
                    handleOrderCancel(order);
                    handleOrderNew(order);
                } else if (orderOriginal.price == order.price) {
                    // TODO: update order parameters
                    orderOriginal.quantity = order.quantity;
                }
            }
        }

        void info(bool printTrades = true)
        {
            for (const auto& [orderId, orderIter]: orderByIDMap) {
                Order& orderOne = *orderIter.orderIter;
                Order& orderTwo = *(*orderIter.priceOrderIter);
                if (orderId != orderOne.orderId || orderId != orderTwo.orderId) {
                    std::cerr << "ERROR: ID: " << orderId << "!= " << orderOne.orderId << std::endl;
                }
            }

            auto printOrders = [](const auto& orderMap) {
                for (const auto& [price, ordersList]: orderMap) {
                    std::cout << "\tPrice: [" << price << "]" << std::endl;
                    for (const auto & orderIter: ordersList) {
                        Common::printOrder(*orderIter);
                    }
                }
            };

            std::cout << "BUY:  " << std::endl; printOrders(buyOrders);
            std::cout << "SELL: " << std::endl; printOrders(sellOrders);

            std::cout << std::string(160, '=') << std::endl;
            if (!printTrades)
                return;
            for (const auto& trade: trades.trades)
            {
                std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                          << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                          << "quantity: " << trade.quantity << ")\n";
            }
        }
    };
}

namespace Tests
{
    using namespace TestMatchingEngine;

    void Trade_SELL()
    {
        OrderMatchingEngine engine;
        for (int i = 0, price = 10; i < 5; ++i)
        {
            if (price > 16)
                price = 10;

            Order order;
            order.side = OrderSide::BUY;
            order.price = price+=2;
            order.quantity = 3;
            order.orderId = getNextOrderID();

            engine.processOrder(order);
        }
        engine.info();
        std::cout << std::string(160, '=') << std::endl;


        {
            Order order;
            order.side = OrderSide::SELL;
            order.price = 15;
            order.quantity = 10;
            order.orderId = getNextOrderID();
            engine.processOrder(order);
        }

        std::cout << std::string(160, '=') << std::endl;

        engine.info();
        std::cout << std::string(160, '=') << std::endl;
    }

    void Trade_BUY()
    {
        OrderMatchingEngine engine;
        for (int i = 0, price = 10; i < 10; ++i)
        {
            if (price > 16)
                price = 10;

            Order order;
            order.side = OrderSide::SELL;
            order.price = price+=2;
            order.quantity = 3;
            order.orderId = getNextOrderID();

            engine.processOrder(order);
        }
        engine.info();
        std::cout << std::string(160, '=') << std::endl;

        {
            Order order;
            order.side = OrderSide::BUY;
            order.price = 15;
            order.quantity = 11;
            order.orderId = getNextOrderID();
            engine.processOrder(order);
        }

        engine.info();
        std::cout << std::string(160, '=') << std::endl;
    }

    void Trade_AMEND()
    {
        int count = 10;
        uint64_t orderIdInitial = getNextOrderID(), orderId = orderIdInitial;
        OrderMatchingEngine engine;
        Order orderAmend;
        for (int i = 0, price = 10; i < count; ++i)
        {
            if (price > 16)
                price = 10;

            Order order;
            order.side = OrderSide::SELL;
            order.price = price+=2;
            order.quantity = 3;
            order.orderId = ++orderId;

            engine.processOrder(order);

            if (orderId ==  orderIdInitial + (count) / 2){
                orderAmend = order;
            }
        }

        engine.info();

        orderAmend.action = OrderActionType::AMEND;
        orderAmend.quantity = 2323;

        engine.processOrder(orderAmend);
        std::cout << orderAmend.orderId << std::endl;
        std::cout << std::string(160, '=') << std::endl;

        engine.info();
    }

    void Trade_AMEND_PriceUpdate()
    {
        int count = 10;
        uint64_t orderIdInitial = getNextOrderID(), orderId = orderIdInitial;
        OrderMatchingEngine engine;
        Order orderAmend;
        for (int i = 0, price = 10; i < count; ++i)
        {
            if (price > 16)
                price = 10;

            Order order;
            order.side = OrderSide::SELL;
            order.price = price+=2;
            order.quantity = 3;
            order.orderId = ++orderId;

            engine.processOrder(order);

            if (orderId ==  orderIdInitial + (count) / 2){
                orderAmend = order;
            }
        }

        engine.info();

        orderAmend.action = OrderActionType::AMEND;
        orderAmend.price = orderAmend.price - 3;

        engine.processOrder(orderAmend);
        std::cout << orderAmend.orderId << std::endl;
        std::cout << std::string(160, '=') << std::endl;

        engine.info();
    }

    void Load_Test()
    {
        constexpr uint32_t pricesCount { 50  }, initialPrice { 10 };
        constexpr uint32_t buyOrders { 100 }, sellOrders { 100 }, cancelOrders { 30 };

        OrderMatchingEngine engine;

        std::vector<int32_t> prices(pricesCount);
        std::iota(prices.begin(), prices.end(), initialPrice);

        std::vector<uint64_t> iDs;
        iDs.reserve(pricesCount * buyOrders * 10);


        PerfUtilities::ScopedTimer timer { "MatchingEngine"};
        uint64_t count = 0;
        Order order;

        for (int i = 0; i < 400; ++i)
        {
            for (int32_t price: prices) {
                for (int32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    order.side = OrderSide::BUY;
                    order.price = price;
                    order.quantity = 10;
                    order.orderId = iDs.back();

                    engine.processOrder(order);
                    ++count;
                }
            }
            for (int32_t price: prices) {
                for (int32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    order.side = OrderSide::SELL;
                    order.price = price;
                    order.quantity = 10;
                    order.orderId = iDs.back();

                    engine.processOrder(order);
                    ++count;
                }
            }
            for (int32_t price: prices) {
                for (int32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    order.side = OrderSide::SELL;
                    order.price = price;
                    order.quantity = 10;
                    order.orderId = iDs.back();

                    engine.processOrder(order);
                    ++count;
                }
            }
            for (int32_t price: prices) {
                for (int32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    order.side = OrderSide::BUY;
                    order.price = price;
                    order.quantity = 10;
                    order.orderId = iDs.back();

                    engine.processOrder(order);
                    ++count;
                }
            }

            iDs.clear();
        }

        //engine.info(false);
        std::cout << count << std::endl;
    }
}

// TODO:
//  1. Add allocator to create Orders --> may help when Order size is large
//     OrderPool -- Tests

void MatchingEngine::TestAll()
{
    using namespace Tests;

    // Trade_SELL();
    // Trade_BUY();
    // Trade_AMEND();
    // Trade_AMEND_PriceUpdate();

    Load_Test(); // 0.977487 seconds

}