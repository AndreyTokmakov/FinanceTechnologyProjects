/**============================================================================
Name        : quantity.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : quantity.hpp
============================================================================**/

/*
    Quantity represents a fixed-point quantity value used throughout the
    trading core.

    Quantity uses ScaledValue as its underlying numeric representation with
    8 decimal places.

    The type is intentionally distinct from Price even though both use the
    same fixed-point representation.

    Data Flow:

        Order Request / Execution Report / Position
                         |
                         v
                      Quantity
                         |
                         v
                    ScaledValue
                         |
                         v
                      int64_t

    Responsibilities:
        - represent a quantity value;
        - provide type-safe quantity arithmetic;
        - provide fixed-point conversion and raw representation.

    Quantity does not:
        - validate instrument-specific lot size;
        - perform quantity normalization;
        - communicate with exchanges;
        - parse market data;
        - calculate Position;
        - represent price.


    Quantity and Price intentionally remain separate types even though they
    share the same implementation.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP
#define FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP

#include "scaled_value.hpp"

namespace trading
{
    class Quantity final : public details::ScaledValue<Quantity>
    {
    public:
        using details::ScaledValue<Quantity>::ScaledValue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP