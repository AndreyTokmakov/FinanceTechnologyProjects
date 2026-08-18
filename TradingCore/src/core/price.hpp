/**============================================================================
Name        : price.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : price.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_PRICE_HPP
#define FINANCETECHNOLOGYPROJECTS_PRICE_HPP

#include <compare>
#include <cstdint>
#include <limits>

namespace trading
{
    class Price
    {
    public:
        using Value = int64_t;

        static constexpr int DecimalPlaces = 8;
        static constexpr Value Scale = 100'000'000;

        constexpr Price() noexcept = default;

        explicit constexpr Price(const Value value) noexcept: value { value } {
        }

        [[nodiscard]]
        static constexpr Price fromInteger(const Value value) noexcept{
            return Price { value * Scale };
        }

        [[nodiscard]]
        constexpr Value raw() const noexcept {
            return value;
        }

        [[nodiscard]]
        constexpr bool isZero() const noexcept {
            return value == 0;
        }

        [[nodiscard]]
        constexpr bool isPositive() const noexcept {
            return value > 0;
        }

        [[nodiscard]]
        constexpr Price operator+(const Price other) const noexcept {
            return Price { value + other.value };
        }

        [[nodiscard]]
        constexpr Price operator-(const Price other) const noexcept {
            return Price { value - other.value };
        }

        [[nodiscard]]
        constexpr Price operator*(const Value multiplier) const noexcept{
            return Price { value * multiplier };
        }

        constexpr Price& operator+=(const Price other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr Price& operator-=(const Price other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr auto operator<=>(const Price&) const noexcept = default;

    private:
        Value value { 0 };
    };
}
#endif //FINANCETECHNOLOGYPROJECTS_PRICE_HPP
