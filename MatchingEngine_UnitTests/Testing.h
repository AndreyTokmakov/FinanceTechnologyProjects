/**============================================================================
Name        : Testing.h
Created on  : 03.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Testing harness
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_TESTING_H
#define FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_TESTING_H


#include "MatchingEngine.h"

namespace Testing
{
    uint64_t getNextOrderID();

    struct OrderMatchingEngineTester : MatchingEngine
    {
        using MatchingEngine::OrderPtr;

        void info([[maybe_unused]] bool printTrades = true);

        size_t getBuyOrdersCount() const noexcept;
        size_t getSellOrdersCount() const noexcept;
    };

    using OrderPtr = OrderMatchingEngineTester::OrderPtr;


    OrderPtr createOrder(Common::OrderSide orderSide,
                         Common::OrderActionType action = Common::OrderActionType::NEW,
                         Common::Order::Price price = 1,
                         unsigned long long quantity = 0,
                         Common::Order::OrderID orderId = getNextOrderID());

    void PostOrders(OrderMatchingEngineTester& engine,
                    std::vector<Common::Order::Price>& prices,
                    Common::OrderSide orderSide = Common::OrderSide::BUY,
                    Common::OrderActionType action = Common::OrderActionType::NEW,
                    uint32_t orderPerPrice = 1,
                    uint32_t quantity = 1);
};

#endif // FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_TESTING_H
