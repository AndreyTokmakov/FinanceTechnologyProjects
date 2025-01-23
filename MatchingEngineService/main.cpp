/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <iostream>
#include <string_view>
#include <vector>

#include "Order.h"
#include "OrderBook.h"
#include "EventConsumer.h"
#include "Utils.h"

namespace PriceCast
{
    using namespace Utils;

    void DoubleToLong()
    {
        const std::string strPrice("771043005.010010123");
        const int64_t price = priceToLong(strPrice);

        std::cout << strPrice << " ==> " << price << std::endl;
    }
}


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // EventConsumer::TestAll();
    // MatchingEngine::TestAll();

    PriceCast::DoubleToLong();

    return EXIT_SUCCESS;
}
