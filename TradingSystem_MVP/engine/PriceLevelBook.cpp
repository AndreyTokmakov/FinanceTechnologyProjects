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
        /*
        std::cout << "BIDS: [" << pair << "]\n";
        for (const auto& [price, quantity] : depthUpdate.bid) {
            std::cout << "\t{ price: " << price << ", quantity: " << quantity << " }\n";
        }

        std::cout << "ASKS: [" << pair << "]\n";
        for (const auto& [price, quantity] : depthUpdate.ask) {
            std::cout << "\t{ price: " << price << ", quantity: " << quantity << " }\n";
        }
        */

        if (!depthUpdate.bid.empty()) {
            std::cout << "[" << pair << "] BID { " << depthUpdate.bid.begin()->price << " - "
                    << std::prev(depthUpdate.bid.end())->price << "}\n";
        }
        if (!depthUpdate.ask.empty()) {
            std::cout << "[" << pair << "] ASK { " << depthUpdate.ask.begin()->price << " - "
                      << std::prev(depthUpdate.ask.end())->price << "}\n";
        }
    }
}

