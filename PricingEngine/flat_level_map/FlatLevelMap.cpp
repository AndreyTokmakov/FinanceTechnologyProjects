/**============================================================================
Name        : FlatLevelMap.cpp
Created on  : 20.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : FlatLevelMap.cpp
============================================================================**/

#include "FlatLevelMap.hpp"

namespace
{
    struct alignas(64) PriceLevel
    {
        double price { 0.0 };
        double qty { 0.0 };

        [[nodiscard]]
        bool empty() const noexcept {
            return qty == 0.0;
        }
    };

}

void flat_level_map::TestAll()
{

}
