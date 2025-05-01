/**============================================================================
Name        : OrderBook.cpp
Created on  : 01.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.cpp
============================================================================**/

#include "OrderBook.h"


namespace engine
{
    using market_data::Ticker;

    void OrderBook::processEvent(const market_data::Event& event) const
    {
        std::cout << "Book '" << pair << "' processing event: " <<  event.type << std::endl;
        std::cout << "\t{ bids: " << event.depth.bid.size() << ", asks: " << event.depth.ask.size() << " }\n";
        //std::cout << event.symbol << std::endl;
        //std::cout << event.pair << std::endl;
    }
}
