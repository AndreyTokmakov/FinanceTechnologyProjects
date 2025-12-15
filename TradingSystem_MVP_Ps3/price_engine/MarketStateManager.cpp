/**============================================================================
Name        : MarketStateManager.cpp
Created on  : 12.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketStateManager.cpp
============================================================================**/

#include "MarketStateManager.hpp"

namespace price_engine
{
    void MarketStateManager::run() const
    {
        uint32_t cpuId = 3;
        for (ExchangeDataProcessor* bookKeeper: books) {
            bookKeeper->run(cpuId++);
        }
    }

    void MarketStateManager::push(common::Exchange exchange,
                           BinanceMarketEvent& event) const
    {
        books[static_cast<uint32_t>(exchange)]->push(event);
    }
}
