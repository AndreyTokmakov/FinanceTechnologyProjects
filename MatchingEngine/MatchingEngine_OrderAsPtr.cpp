/**============================================================================
Name        : MatchingEngine_OrderAsPtr.cpp
Created on  : 03.10.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngine_OrderAsPtr.cpp
============================================================================**/

#include "Includes.h"

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

namespace MatchingEngine_OrderAsPtr
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

        Trades()
        {
            trades.reserve(10'000'000);
        }

        Trade& addTrade() {
            return trades.emplace_back();
        }
    };


    /*
    struct Trades
    {
        // std::vector<Trade> trades {};
        Trade dummyTread;

        Trade& addTrade() {
            // return trades.emplace_back();
            return dummyTread;
        }
    };
     */

    struct OrderMatchingEngine
    {
        using OrderPtr = std::unique_ptr<Order>;
        using OrderIter = typename std::list<OrderPtr>::iterator;
        using PriceOrderList = std::list<OrderIter>;
        using PriceOrderListPtr = PriceOrderList*;
        using PriceOrderListIter = typename PriceOrderList::iterator;

        struct ReferencesBlock
        {
            OrderIter orderIter;
            PriceOrderListIter priceOrderIter;
            PriceOrderListPtr priceLevelOrderList;
        };

        std::list<OrderPtr> orders {};
        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        // TODO: Test replace std::map --> boost::flat_map [std::list --> shall be pointer?]
        //       Since look performance of this lookup is more critical one

#if 0
        std::map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        std::map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
#else
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
#endif

        Trades trades;

        void processOrder(OrderPtr&& order)
        {
            // TODO: Remove branching ???
            switch (order->action)
            {
                case OrderActionType::NEW:
                    return handleOrderNew(std::move(order));
                case OrderActionType::CANCEL:
                    return handleOrderCancel(*order);
                case OrderActionType::AMEND:
                    return handleOrderAmend(std::move(order));
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
                            PriceOrderList* matchedOrderList)
        {
            for (auto orderIter = matchedOrderList->begin(); matchedOrderList->end() != orderIter;)
            {
                Order& matchedOrder = **(*orderIter);

                // FIXME : Required performance improvements: cause of ~35% CPU usage
#if 1
                Trade& trade = trades.addTrade();
                trade.setQuantity(std::min(matchedOrder.quantity,order.quantity));
                // TODO: Remove branching ???
                if (OrderSide::SELL == order.side) {
                    trade.setBuyOrder(matchedOrder).setSellOrder(order);
                } else {
                    trade.setBuyOrder(order).setSellOrder(matchedOrder);
                }
#endif

                if (order.quantity >= matchedOrder.quantity)
                {
                    order.quantity -= matchedOrder.quantity;
                    matchedOrder.quantity = 0;

                    /** Deleting order **/
                    orderByIDMap.erase(matchedOrder.orderId);
                    matchedOrderList->erase(orderIter++);
                } else {
                    matchedOrder.quantity -= order.quantity;
                    order.quantity = 0;
                    ++orderIter;
                    return;;
                }
            }
        }

        template<class Map>
        PriceOrderListPtr getOrderPriceList(Map& map, const Order::Price& price)
        {
            const auto [iter, inserted] = map.emplace(price, nullptr);
            if (inserted) {
                iter->second = new PriceOrderList() ;
            }
            return iter->second;
        };

        void handleOrderNew(OrderPtr&& order)
        {
            if (0 == matchOrder(*order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(order->orderId, ReferencesBlock{});
            if (inserted)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = iterOrderMap->second;
                orderIter = orders.insert(orders.end(), nullptr);
                priceLevelOrderList = (OrderSide::BUY == order->side) ? getOrderPriceList(buyOrders, order->price ) :
                                     getOrderPriceList(sellOrders, order->price );
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
                *orderIter = std::move(order);
            }
        }

        void handleOrderCancel(const Order& order)
        {
            if (const auto itOrderById = orderByIDMap.find(order.orderId);
                orderByIDMap.end() != itOrderById)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = itOrderById->second;
                priceLevelOrderList->erase(priceOrderIter);
                orders.erase(orderIter);
                orderByIDMap.erase(itOrderById);
            }
        }

        void handleOrderAmend(OrderPtr&& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                // TODO: Remove branching ???
                Order& orderOriginal = **(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order->side) {
                    return;
                } else if (orderOriginal.price != order->price) {
                    handleOrderCancel(*order);
                    handleOrderNew(std::move(order));
                } else if (orderOriginal.price == order->price) {
                    // TODO: update order parameters
                    orderOriginal.quantity = order->quantity;
                }
            }
        }

        void info([[maybe_unused]] bool printTrades = true)
        {
            auto printOrders = [](const auto& orderMap) {
                for (const auto& [price, ordersList]: orderMap) {
                    std::cout << "\tPrice: [" << price << "]" << std::endl;
                    for (const auto & orderIter: *ordersList) {
                        Common::printOrder(**orderIter);
                    }
                }
            };


            std::cout << "BUY:  " << std::endl; printOrders(buyOrders);
            std::cout << "SELL: " << std::endl; printOrders(sellOrders);
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
    };
}


namespace Tests_OrderAsPtr
{
    using namespace MatchingEngine_OrderAsPtr;
    using OrderPtr = MatchingEngine_OrderAsPtr::OrderMatchingEngine::OrderPtr ;

    void Trade_DEBUG()
    {
        OrderMatchingEngine engine;

        OrderPtr order { std::make_unique<Order>()};
        order->side = OrderSide::BUY;
        order->price = 123;
        order->quantity = 3;
        order->orderId = getNextOrderID();

        engine.processOrder(std::move(order));

        engine.info();
    }

    void Trade_SELL()
    {
        OrderMatchingEngine engine;
        for (int i = 0, price = 10; i < 5; ++i)
        {
            if (price > 16)
                price = 10;

            OrderPtr order { std::make_unique<Order>()};
            order->side = OrderSide::BUY;
            order->price = price+=2;
            order->quantity = 3;
            order->orderId = getNextOrderID();

            engine.processOrder(std::move(order));
        }
        engine.info();
        std::cout << std::string(160, '=') << std::endl;

        {
            OrderPtr order { std::make_unique<Order>()};
            order->side = OrderSide::SELL;
            order->price = 15;
            order->quantity = 10;
            order->orderId = getNextOrderID();
            engine.processOrder(std::move(order));
        }

        std::cout << std::string(160, '=') << std::endl;

        engine.info();
        std::cout << std::string(160, '=') << std::endl;
    }

    /*
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
    */

    void Load_Test()
    {
        constexpr uint32_t pricesCount { 50  }, initialPrice { 10 };
        constexpr uint32_t buyOrders { 100 }, sellOrders { 100 };

        OrderMatchingEngine engine;

        std::vector<int32_t> prices(pricesCount);
        std::iota(prices.begin(), prices.end(), initialPrice);

        std::vector<uint64_t> iDs;
        iDs.reserve(pricesCount * buyOrders * 10);


        PerfUtilities::ScopedTimer timer { "MatchingEngine_OrderAsPtr"};
        uint64_t count = 0;

        for (int i = 0; i < 400; ++i)
        {
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { std::make_unique<Order>()};
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

                    OrderPtr order { std::make_unique<Order>()};
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

                    OrderPtr order { std::make_unique<Order>()};
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

                    OrderPtr order { std::make_unique<Order>()};
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


        //engine.info(false);
        std::cout << count << std::endl;
    }
}

// TODO:
//  1. Add allocator to create Orders --> may help when Order size is large
//     OrderPool -- Tests

void MatchingEngine_OrderAsPtr::TestAll()
{
    using namespace Tests_OrderAsPtr;

    // Trade_DEBUG();
    // Trade_SELL();
    // Trade_BUY();
    // Trade_AMEND();
    // Trade_AMEND_PriceUpdate();
    // Load_Test
}

void MatchingEngine_OrderAsPtr::LoadTest()
{
    using namespace Tests_OrderAsPtr;
    Load_Test();
}