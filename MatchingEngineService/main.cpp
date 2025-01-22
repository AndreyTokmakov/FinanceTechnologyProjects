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


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // EventConsumer::TestAll();
    // MatchingEngine::TestAll();

    std::string strPrice("104300.010");
    std::string quantity("0.00250001");

    {
        double value{0.0};
        std::from_chars(strPrice.data(), strPrice.data() + strPrice.size(), value);
        std::cout << value << std::endl;
        double d = stod( strPrice) ;
        std::cout << d << std::endl;
    }

    {
        double value{0.0};
        std::from_chars(quantity.data(), quantity.data() + quantity.size(), value);
        std::cout << value << std::endl;
        std::cout << atof( quantity.data()) << std::endl;
    }

    return EXIT_SUCCESS;
}
