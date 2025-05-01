/**============================================================================
Name        : OrderBook.cpp
Created on  : 01.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.cpp
============================================================================**/

#include <iostream>
#include "OrderBook.h"

namespace engine
{
    using market_data::EventType;
    using market_data::Depth;

    void OrderBook::processEvent(const market_data::Event& event)
    {
        // std::cout << "Book '" << pair << "' processing event: " <<  event.type << std::endl;
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

    void OrderBook::handleDepthUpdate(const market_data::Depth& depthUpdate)
    {
        // std::cout << "\t{ bids: " << depthUpdate.bid.size() << ", asks: " << depthUpdate.ask.size() << " }\n";

        std::cout << "BIDS:" << std::endl;
        for (const auto& [price, quantity] : depthUpdate.bid)
        {
            std::cout << "\t{ price: " << price << ", quantity: " << quantity << " }\n";
        }

        std::cout << "ASKS:" << std::endl;
        for (const auto& [price, quantity] : depthUpdate.ask)
        {
            std::cout << "\t{ price: " << price << ", quantity: " << quantity << " }\n";
        }
    }
}
