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
        for (const auto& [price, quantity] : depthUpdate.bid) {
            if (0 == quantity)
                bidPrices.erase(price);
            else
                bidPrices.insert_or_assign(price, quantity);
        }

        for (const auto& [price, quantity] : depthUpdate.ask) {
            if (0 == quantity)
                askPrices.erase(price);
            else
                askPrices.insert_or_assign(price, quantity);
        }

        std::cout << pair <<  " [ " << bidPrices.size() << ", " << askPrices.size() << " ] ";
        if (!bidPrices.empty())
            std::cout << pair <<  " [ BIDS: " << bidPrices.begin()->first << ", " << std::prev(bidPrices.end())->first << " ] ";
        if (!askPrices.empty())
            std::cout << pair <<  " [ ASKS: " << askPrices.begin()->first << ", " << std::prev(askPrices.end())->first << " ] ";
        std::cout << std::endl;
    }
}
