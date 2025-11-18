/**============================================================================
Name        : Experiments.cpp
Created on  : 18.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Experiments.cpp
============================================================================**/

#include "Experiments.hpp"

#include <iostream>
#include <functional>


namespace branchless_method_call
{
    using Price = double;
    using Quantity = double;

    struct PriceLevel
    {
        Price price { 0 };
        Quantity quantity { 0 };
    };

    struct Worker
    {
        void add(const PriceLevel lvl)
        {
            std::cout << "add: " << lvl.price  << " with quantity: " << lvl.quantity << std::endl;
        }

        void remove(const PriceLevel lvl)
        {
            std::cout << "remove: " << lvl.price  << " with quantity: " << lvl.quantity << std::endl;
        }

        using methodPtr = void (Worker::*)(PriceLevel);

        const std::array<methodPtr, 2> methods {
            &Worker::remove, &Worker::add
        };

        void handle(const PriceLevel lvl)
        {
            /*
            const methodPtr ptr = methods[static_cast<bool>(lvl.quantity)];
            (this->*ptr)(lvl);
            */

            const bool isZero = static_cast<bool>(lvl.quantity);
            std::invoke(methods[isZero], this, lvl);
        }
    };

    void test()
    {
        Worker worker;
        worker.handle({11, 100});
        worker.handle({22, 0});
    }
}

void experiments::TestAll()
{
    branchless_method_call::test();

}
