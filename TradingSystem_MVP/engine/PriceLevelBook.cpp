/**============================================================================
Name        : OrderBook.cpp
Created on  : 01.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.cpp
============================================================================**/

#include <iostream>
#include "PriceLevelBook.h"

namespace engine
{
    using market_data::EventType;
    using market_data::Depth;

    void PriceLevelBook::processEvent(const market_data::Event& event)
    {
        switch (event.type)
        {
            case EventType::DepthUpdate:
                handleDepthUpdate(event.depth);
                break;
            default:
                std::cout << "Unhandled event type: " << event.type << std::endl;
                break;
        }
    }

    void PriceLevelBook::handleDepthUpdate(const market_data::Depth& depthUpdate)
    {
        for (const market_data::PriceLevel& lvl: depthUpdate.bid) {
            if (0 == lvl.quantity ) {
                buyOrders.erase(lvl.price);
                continue;
            }
            buyOrders.emplace(lvl.price, lvl.quantity);
        }
        for (const market_data::PriceLevel& lvl: depthUpdate.ask) {
            if (0 == lvl.quantity ) {
                sellOrders.erase(lvl.price);
                continue;
            }
            sellOrders.emplace(lvl.price, lvl.quantity);
        }

        if (!buyOrders.empty()) {
            std::cout << "[" << pair << "] BID: (" << buyOrders.size() << ") [" << buyOrders.begin()->first
                      << " - " << std::prev(buyOrders.end())->first << "]\n";
        }
        if (!sellOrders.empty()) {
            std::cout << "[" << pair << "] ASK: (" << sellOrders.size() << ") [" << sellOrders.begin()->first
                      << " - " << std::prev(sellOrders.end())->first << "]\n";
        }
    }
}

