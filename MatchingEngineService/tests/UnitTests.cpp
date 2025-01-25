/**============================================================================
Name        : UnitTests.cpp
Created on  : 25.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : UnitTests
============================================================================**/

#include "UnitTests.h"
#include "OrderBook.h"
#include "Utils.h"
#include "Queue.h"

#include <nlohmann/json.hpp>

#include <iostream>

namespace PriceCast
{
    using namespace Utils;

    void DoubleToLong()
    {
        // const std::string strPrice("771043005.010010123");
        const std::string strPrice("0.82148000");
        const int64_t price = priceToLong(strPrice);

        std::cout << strPrice << " ==> " << price << std::endl;
    }
}

void UnitTests::TestAll()
{

}
