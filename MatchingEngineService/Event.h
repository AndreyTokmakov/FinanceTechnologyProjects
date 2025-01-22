/**============================================================================
Name        : Event.h
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Event.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EVENT_H
#define FINANCETECHNOLOGYPROJECTS_EVENT_H

#include <cstdint>
#include <vector>

namespace Common
{
    using Price = double;
    using Quantity = double;

    enum class EventType : uint8_t
    {
        DepthSnapshot,
        DepthUpdate,
    };

    struct PriceLevel
    {
        Price price { 0.0 };
        Quantity quantity { 0.0 };
    };

    struct DepthEvent
    {
        EventType type { EventType::DepthSnapshot };
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> akss;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EVENT_H
