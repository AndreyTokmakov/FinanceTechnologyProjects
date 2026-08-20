/**============================================================================
Name        : scaled_value.hpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : scaled_value.hpp
============================================================================**/

/*
    ScaledValue provides the common fixed-point representation used by
    domain-specific numeric types such as Price and Quantity.

    The value is stored as an integer together with an implicit decimal scale.

    For example, with DecimalPlaces = 8:

        12.34567890

    is represented internally as:

        1234567890

    Data Flow:

        External / parsed value
               |
               v
        Domain numeric type
          (Price / Quantity)
               |
               v
        ScaledValue
               |
               v
        int64_t raw storage


    Responsibilities:

        - store a fixed-point numeric value;
        - provide the common fixed-point scale;
        - construct values from integer values;
        - expose the raw representation;
        - provide basic arithmetic;
        - provide zero and sign checks;
        - provide comparisons.


    ScaledValue uses CRTP so that arithmetic operations return the concrete
    domain type rather than the base type.

    For example:

        Price first;
        Price second;

        const Price result = first + second;


    This is important because Price and Quantity must remain distinct types.

    The following operations must not be allowed:

        Price + Quantity
        Price - Quantity
        Price == Quantity

    Price and Quantity therefore share implementation but do not share their domain type.
    The number of decimal places is configurable through the DecimalPlaces template parameter.
    The default value is 8 decimal places, which is currently used by Price  and Quantity.

    ScaledValue is an implementation detail of the core numeric types.

    It does not define domain semantics such as:

        - what a Price represents;
        - what a Quantity represents;
        - whether a value is valid for a particular instrument;
        - exchange-specific precision rules;
        - rounding policies;
        - overflow handling.


    Those responsibilities belong to the corresponding domain types and
    higher-level components.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_SCALED_VALUE_HPP
#define FINANCETECHNOLOGYPROJECTS_SCALED_VALUE_HPP

#include <compare>
#include <cstdint>

namespace trading::details
{
    consteval int64_t pow10(const int exponent) noexcept
    {
        int64_t result { 1 };
        for (int index { 0 }; index < exponent; ++index)
            result *= 10;
        return result;
    }

    template<typename Derived, int DecimalPlacesCount = 8>
    class ScaledValue
    {
    public:
        using Value = int64_t;

        static constexpr int DecimalPlaces = DecimalPlacesCount;
        static constexpr Value Scale = pow10(DecimalPlaces);

        constexpr ScaledValue() noexcept = default;

        explicit constexpr ScaledValue(const Value value) noexcept:
            value { value }
        {
        }

        [[nodiscard]]
        static constexpr Derived fromInteger(const Value value) noexcept
        {
            return Derived { value * Scale };
        }

        [[nodiscard]]
        constexpr Value raw() const noexcept
        {
            return value;
        }

        [[nodiscard]]
        constexpr bool isZero() const noexcept
        {
            return value == 0;
        }

        [[nodiscard]]
        constexpr bool isPositive() const noexcept
        {
            return value > 0;
        }

        [[nodiscard]]
        constexpr Derived operator+(const Derived& other) const noexcept
        {
            return Derived { value + other.raw() };
        }

        [[nodiscard]]
        constexpr Derived operator-(const Derived& other) const noexcept
        {
            return Derived { value - other.raw() };
        }

        [[nodiscard]]
        constexpr Derived operator*(const Value multiplier) const noexcept
        {
            return Derived { value * multiplier };
        }

        constexpr Derived& operator+=(const Derived& other) noexcept
        {
            value += other.raw();
            return derived();
        }

        constexpr Derived& operator-=(const Derived& other) noexcept
        {
            value -= other.raw();
            return derived();
        }

        constexpr auto operator<=>(const ScaledValue&) const noexcept = default;

    private:
        [[nodiscard]]
        constexpr Derived& derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }

        Value value { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_SCALED_VALUE_HPP