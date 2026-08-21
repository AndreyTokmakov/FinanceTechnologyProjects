/**============================================================================
Name        : imbalance_strategy.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book imbalance trading strategy implementation.
============================================================================**/

/*
    ImbalanceStrategy implementation.

    The strategy evaluates the liquidity imbalance between the best bid and
    best ask levels.

    Given:

        B = best bid quantity
        A = best ask quantity

    the conceptual imbalance is:

        B - A
        -----
        B + A

    A positive value indicates stronger bid-side liquidity, while a negative
    value indicates stronger ask-side liquidity.

    Instead of calculating the ratio using floating-point arithmetic, the
    implementation compares the two sides using integer arithmetic.

    For threshold:

        T = thresholdNumerator / thresholdDenominator

    Buy is generated when:

        (B - A) / (B + A) >= T

    which is equivalent to:

        (B - A) * thresholdDenominator
            >=
        (B + A) * thresholdNumerator

    Sell is generated when:

        (B - A) / (B + A) <= -T

    which is equivalent to:

        (A - B) * thresholdDenominator
            >=
        (B + A) * thresholdNumerator

    The calculation uses __int128 for intermediate multiplication in order
    to avoid overflow when multiplying fixed-point Quantity values.

    Example with the default threshold of 0.7:

        Bid quantity = 900
        Ask quantity = 100

        imbalance = 800 / 1000 = 0.8

        800 * 10 >= 1000 * 7
        8000 >= 7000

        => Signal::Buy

    Example:

        Bid quantity = 100
        Ask quantity = 900

        imbalance = -800 / 1000 = -0.8

        800 * 10 >= 1000 * 7
        8000 >= 7000

        => Signal::Sell

    Equal bid and ask quantities produce zero imbalance and therefore
    Signal::None.
*/

#include "imbalance_strategy.hpp"

namespace trading::strategy
{
    ImbalanceStrategy::ImbalanceStrategy(const Value thresholdNumerator,
                                         const Value thresholdDenominator) noexcept :
        thresholdNumerator { thresholdNumerator },
        thresholdDenominator { thresholdDenominator }
    {
    }

    Signal ImbalanceStrategy::evaluate(const market_data::MarketEvent& event) const
    {
        using BigInt = __int128;
        const BigInt bidQuantity = event.bestBidQuantity.raw();
        const BigInt askQuantity = event.bestAskQuantity.raw();
        const BigInt totalQuantity = bidQuantity + askQuantity;

        if (totalQuantity <= 0)
            return Signal::None;

        const BigInt difference = bidQuantity - askQuantity;
        if (difference > 0)
        {
            const BigInt lhs = difference * thresholdDenominator;
            const BigInt rhs = totalQuantity * thresholdNumerator;

            if (lhs >= rhs)
                return Signal::Buy;
        }
        else if (difference < 0)
        {
            const BigInt absoluteDifference = -difference;
            const BigInt lhs = absoluteDifference * thresholdDenominator;
            const BigInt rhs = totalQuantity * thresholdNumerator;

            if (lhs >= rhs)
                return Signal::Sell;
        }

        return Signal::None;
    }
}