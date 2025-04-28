/**============================================================================
Name        : MarketData.cpp
Created on  : 28.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketData.h
============================================================================**/

#include "MarketData.h"

namespace market_data
{
    std::ostream& operator<<(std::ostream& stream, const EventType& eventType)
    {
        switch (eventType) {
            case EventType::None: stream << "None"; break;
            case EventType::Result: stream << "Result"; break;
            case EventType::Ticker: stream << "Ticker"; break;
            case EventType::MiniTicker: stream << "MiniTicker"; break;
            case EventType::BookTicker: stream << "BookTicker"; break;
            case EventType::Trade: stream << "Trade"; break;
            case EventType::AggTrade: stream << "AggTrade"; break;
            case EventType::MarkPrice: stream << "MarkPrice"; break;
            case EventType::DepthUpdate: stream << "DepthUpdate"; break;
        }
        return stream;
    }
}