/**============================================================================
Name        : BinanceEventsParser.cpp
Created on  : 30.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BinanceEventsParser.cpp
============================================================================**/

#include "BinanceEventsParser.h"

#include <iostream>

namespace Parser
{
    void BinanceEventsParser::parseEvent(const std::string& event)
    {
        std::cout << std::string(60, '-') << " BinanceEventsParser  (" << event.size()
                  << ") " << std::string(60, '-') << std::endl;

        std::cout << event << std::endl;
    }
}