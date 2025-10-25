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
#include "PerfUtilities.hpp"
#include "Testing.h"



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
            for (const uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n)
                {
                    iDs.push_back(getNextOrderID());
                    OrderPtr order = createOrder(
                        Common::OrderSide::BUY, Common::OrderActionType::NEW, price, 10, iDs.back());
                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (const uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());
                    OrderPtr order = createOrder(
                        Common::OrderSide::SELL, Common::OrderActionType::NEW, price, 10, iDs.back());
                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (const uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());
                    OrderPtr order = createOrder(
                        Common::OrderSide::SELL, Common::OrderActionType::NEW, price, 10, iDs.back());
                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (const uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());
                    OrderPtr order = createOrder(
                        Common::OrderSide::BUY, Common::OrderActionType::NEW, price, 10, iDs.back());
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
