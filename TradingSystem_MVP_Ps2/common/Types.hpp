/**============================================================================
Name        : Types.hpp
Created on  : 01.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TYPES_HPP
#define FINANCETECHNOLOGYPROJECTS_TYPES_HPP

#include <cstdint>

namespace common
{
    using Price     = double;
    using Quantity  = double;
    using Volume    = double;
    using Timestamp = uint64_t;
    using Number    = uint64_t;

    struct PriceLevel
    {
        Price price { 0.0 };
        Quantity quantity { 0.0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_HPP