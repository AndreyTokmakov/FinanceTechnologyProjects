/**============================================================================
Name        : main.cpp
Created on  : 29.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : LimitOrderBook
============================================================================**/

#include "Order.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <variant>

#include <map>
#include <boost/container/flat_map.hpp>
#include <functional>
#include "PerfUtilities.hpp"

namespace
{
    using Price = Common::Order::Price;
    using Quantity = Common::Order::Quantity;


    template<typename Self>
    struct ComparatorBase
    {
        inline bool operator()(const Price a, const Price b) noexcept
        {
            return static_cast<Self&>(*this).compare(a, b);
        }
    };

    struct ComparatorLess: ComparatorBase<ComparatorLess>
    {
        bool compare(const Price a, const Price b) const noexcept
        {
            return b >= a;
        }
    };

    struct ComparatorGreater: ComparatorBase<ComparatorGreater>
    {
        bool compare(const Price a, const Price b) const noexcept
        {
            return a >= b;
        }
    };



    struct Comparator {
        inline bool operator()(const Price a, const Price b) const noexcept;
    };

    struct Less: Comparator
    {
        inline bool operator()(const Price a, const Price b) const noexcept {
            return b >= a;
        }
    };

    struct Greater: Comparator
    {
        inline bool operator()(const Price a, const Price b) const noexcept {
            return b >= a;
        }
    };

    template<typename Comparator>
    using OrderMap = std::map<Price, Quantity, Comparator>;

    using AskOrders = OrderMap<std::less<>>;
    using BidOrders = OrderMap<std::greater<>>;

    struct Wrapper
    {
        std::map<Price, Quantity> map;
    };


    bool comp(std::string_view a, std::string_view b) //custom comparator
    {
        return a.length() > b.length();
    }
}

void mapWithDifferentComparators()
{
    OrderMap<std::function<bool(const Price, const Price)>> orders {std::less<>{}};
    OrderMap<std::function<bool(const Price, const Price)>> orders1 {std::greater<>{}};

    orders = orders1;

    orders.emplace(1,1);
    orders.emplace(3,3);
    orders.emplace(2,2);

    for (const auto& [k, v]: orders) {
        std::cout << k << " = " << v << std::endl;
    }

}


void mapWithDifferentComparators2()
{
    ComparatorLess less;
    ComparatorGreater greater;

    std::cout << std::boolalpha << less(1,3) << std::endl;
    std::cout << std::boolalpha << greater(1,3) << std::endl;


    //OrderMap<ComparatorBase<>> orders {greater};


    /*
    orders.emplace(1,1);
    orders.emplace(3,3);
    orders.emplace(2,2);

    for (const auto& [k, v]: orders) {
        std::cout << k << " = " << v << std::endl;
    }*/
}



void mapWithDifferentComparators3()
{
    Less less;
    Less greater;

    OrderMap<Less> orders {less};

    orders.emplace(1,1);
    orders.emplace(3,3);
    orders.emplace(2,2);

    for (const auto& [k, v]: orders) {
        std::cout << k << " = " << v << std::endl;
    }

}


namespace PerfTest
{

    struct EngineOne
    {
        AskOrders askOrders;
        BidOrders bskOrders;
    };

    struct EngineTwo
    {
        using Comarator = std::function<bool(const Price, const Price)>;

        std::array<OrderMap<Comarator>, 2> orders {
            OrderMap<Comarator> {std::less<>{} },
            OrderMap<Comarator> {std::greater<>{} }
        };
    };


    void test()
    {
        EngineOne engineOne;
        EngineTwo engineTwo;


        {
            for (int i = 0; i < 5; ++i) {
                engineOne.askOrders.emplace(i, i);
                engineOne.bskOrders.emplace(i, i);
            }

            for (const auto &[k, v]: engineOne.askOrders) {
                std::cout << "{" << k << ", " << v << "} ";
            }
            std::cout << std::endl;
            for (const auto &[k, v]: engineOne.bskOrders) {
                std::cout << "{" << k << ", " << v << "} ";
            }
            std::cout << std::endl;
        }

        {
            for (int i = 0; i < 5; ++i) {
                engineTwo.orders.front().emplace(i, i);
                engineTwo.orders.back().emplace(i, i);
            }

            for (const auto &[k, v]: engineTwo.orders.front()) {
                std::cout << "{" << k << ", " << v << "} ";
            }
            std::cout << std::endl;
            for (const auto &[k, v]: engineTwo.orders.back()) {
                std::cout << "{" << k << ", " << v << "} ";
            }
            std::cout << std::endl;
        }
    }

    void benchmark()
    {
        using Common::Order;
        using Common::OrderSide;

        EngineOne engineOne;
        EngineTwo engineTwo;

        constexpr int ordersCount { 1'000 };
        constexpr int iterCount { 100'000 };

        std::vector<Common::Order> orders;
        OrderSide side = OrderSide::Buy;
        for (int i = 0, sellBuy = 0; i < ordersCount; ++i)
        {
            side = OrderSide::Buy == side ? OrderSide::Sell : OrderSide::Buy;
            Order order = Common::OrderBuilder{}.setOrderId(0).setOrderSide(side)
                    .setPrice(i).setQuantity(i).build();
            orders.push_back(order);
        }


        {
            PerfUtilities::ScopedTimer timer { "EngineOne-1"};
            for (int i = 0; i < iterCount; ++i)
            {
                for (const Order& order: orders)
                {
                    if (OrderSide::Buy == order.side)
                        engineOne.bskOrders.emplace(order.price, order.quantity);
                    else
                        engineOne.askOrders.emplace(order.price, order.quantity);
                }

                engineOne.bskOrders.clear();
                engineOne.bskOrders.clear();
            }
        }

        {
            PerfUtilities::ScopedTimer timer { "EngineOne-2"};
            for (int i = 0, max = iterCount/ 2; i < max; ++i)
            {
                for (const Order& order: orders){
                    engineOne.askOrders.emplace(order.price, order.quantity);
                }
                engineOne.askOrders.clear();
            }
            for (int i = 0, max = iterCount/ 2; i < max; ++i)
            {
                for (const Order& order: orders){
                    engineOne.bskOrders.emplace(order.price, order.quantity);
                }
                engineOne.bskOrders.clear();
            }
        }

        {
            PerfUtilities::ScopedTimer timer { "engineTwo"};
            for (int i = 0; i < iterCount; ++i)
            {
                for (const Order& order: orders) {
                    engineTwo.orders[static_cast<uint32_t>(order.side)].emplace(order.price, order.quantity);
                }

                engineTwo.orders[0].clear();
                engineTwo.orders[1].clear();
            }
        }
    }
}


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);


    // mapWithDifferentComparators();
    // mapWithDifferentComparators2();
    // mapWithDifferentComparators3();

    //PerfTest::test();
    PerfTest::benchmark();


    return EXIT_SUCCESS;
}
