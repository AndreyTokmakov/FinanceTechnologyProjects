/**============================================================================
Name        : types.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : types.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TYPES_HPP
#define FINANCETECHNOLOGYPROJECTS_TYPES_HPP

#include <cstdint>

namespace trading
{
    using InstrumentId    = uint32_t;
    using OrderId         = uint64_t;
    using ExchangeOrderId = uint64_t;
    using SequenceNumber  = uint64_t;

    enum class Side : uint8_t
    {
        Buy,
        Sell
    };

    enum class OrderType : uint8_t
    {
        Market,
        Limit
    };

    enum class OrderStatus : uint8_t
    {
        New,
        PartiallyFilled,
        Filled,
        Cancelled,
        Rejected
    };

    enum class ExecType : uint8_t
    {
        New,
        Trade,
        Cancel,
        Reject
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_HPP
