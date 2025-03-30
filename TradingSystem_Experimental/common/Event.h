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
    using Price = int64_t;
    using Quantity = int64_t;

    enum class EventType : uint8_t
    {
        DepthSnapshot = 1,
        DepthUpdate = 2
    };

    struct PriceLevel
    {
        Price price { 0 };
        Quantity quantity { 0 };
    };

    struct DepthEvent
    {
        EventType type { EventType::DepthSnapshot };
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> akss;

        // TODO: Refactor std::array<char, 6>
        std::string symbol;

        uint64_t id { 0 };
        uint64_t lastUpdateId { 0 };
        uint64_t firstUpdateId { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EVENT_H
