/**============================================================================
Name        : quantity.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : quantity.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP
#define FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP

#include <compare>
#include <cstdint>

namespace trading
{
    class Quantity
    {
    public:
        using Value = int64_t;

        static constexpr int DecimalPlaces = 8;
        static constexpr Value Scale = 100'000'000;

        constexpr Quantity() noexcept = default;

        explicit constexpr Quantity(const Value value) noexcept: value { value }{
        }

        [[nodiscard]]
        static constexpr Quantity fromInteger(const Value value) noexcept {
            return Quantity { value * Scale };
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
        constexpr Quantity operator+(const Quantity other) const noexcept {
            return Quantity { value + other.value };
        }

        [[nodiscard]]
        constexpr Quantity operator-(const Quantity other) const noexcept{
            return Quantity { value - other.value };
        }

        [[nodiscard]]
        constexpr Quantity operator*(const Value multiplier) const noexcept {
            return Quantity { value * multiplier };
        }

        constexpr Quantity& operator+=(const Quantity other) noexcept {
            value += other.value;
            return *this;
        }

        constexpr Quantity& operator-=(const Quantity other) noexcept {
            value -= other.value;
            return *this;
        }

        constexpr auto operator<=>(const Quantity&) const noexcept = default;

    private:
        Value value { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_QUANTITY_HPP
