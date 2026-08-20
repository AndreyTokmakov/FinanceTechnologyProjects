/**============================================================================
Name        : price.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : price.hpp
============================================================================**/

/*
    Price represents a fixed-point price value used throughout the trading
    core.

    Price uses ScaledValue as its underlying numeric representation with
    8 decimal places.

    The type is intentionally distinct from Quantity even though both use the
    same fixed-point representation.

    Data Flow:

        Market Data / Order Request
                  |
                  v
                Price
                  |
                  v
            ScaledValue
                  |
                  v
              int64_t


    Responsibilities:

        - represent a price value;
        - provide type-safe price arithmetic;
        - provide fixed-point conversion and raw representation.


    Price does not:

        - validate instrument-specific tick size;
        - perform price normalization;
        - communicate with exchanges;
        - parse market data;
        - calculate PnL;
        - represent quantity.


    Price and Quantity intentionally remain separate types even though they
    share the same implementation.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_PRICE_HPP
#define FINANCETECHNOLOGYPROJECTS_PRICE_HPP

#include "scaled_value.hpp"

namespace trading
{
    class Price final : public details::ScaledValue<Price>
    {
    public:
        using details::ScaledValue<Price>::ScaledValue;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PRICE_HPP