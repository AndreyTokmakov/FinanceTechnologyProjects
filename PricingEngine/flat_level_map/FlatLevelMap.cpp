/**============================================================================
Name        : FlatLevelMap.cpp
Created on  : 20.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : FlatLevelMap.cpp
============================================================================**/

#include "FlatLevelMap.hpp"

#include <iostream>


namespace
{
    using Price = double;
    using Quantity = double;

    struct alignas(64) PriceLevel
    {
        Price price { 0.0 };
        Quantity qty { 0.0 };

        [[nodiscard]]
        bool empty() const noexcept {
            return qty == 0.0;
        }
    };

    template<bool IsBid, size_t MaxLevels = 64>
    struct FlatSide
    {
        PriceLevel lvls[MaxLevels];
        size_t size = 0;

        static bool better(const Price a, const Price b) noexcept
        {
            if constexpr (IsBid)
                return a > b;   // bids: high → low
            else
                return a < b;   // asks: low → high
        }

        // ----- binary search -----
        [[nodiscard]]
        int32_t find_price(const Price price) const noexcept
        {
            size_t left = 0, right = size;
            while (left < right) {
                const size_t mid = (left + right) >> 1;
                const Price p = lvls[mid].price;
                if (p == price)
                    return static_cast<int32_t>(mid);
                if (better(price, p))
                    right = mid;
                else
                    left = mid + 1;
            }
            return -1;
        }

        // ----- find insertion position -----
        [[nodiscard]]
        size_t find_insert_pos(const Price price) const noexcept
        {
            size_t left = 0, right = size;
            while (left < right) {
                const size_t mid = (left + right) >> 1;
                if (better(price, lvls[mid].price))
                    right = mid;
                else
                    left = mid + 1;
            }
            return left;
        }

        // ----- update (insert/modify/remove) -----
        inline void update(double price, double qty) noexcept {
            int idx = find_price(price);

            if (idx >= 0) {
                // ----- update existing -----
                if (qty == 0.0) {
                    // remove level
                    for (size_t i = idx; i + 1 < size; ++i)
                        lvls[i] = lvls[i + 1];
                    size--;
                } else {
                    lvls[idx].qty = qty;
                }
                return;
            }

            // ----- create -----
            if (qty == 0.0) return; // nothing to do

            size_t pos = find_insert_pos(price);

            // shift right
            for (size_t i = size; i > pos; --i)
                lvls[i] = lvls[i - 1];

            lvls[pos].price = price;
            lvls[pos].qty = qty;
            size++;
        }

        PriceLevel* best() noexcept {
            return size ? &lvls[0] : nullptr;
        }
    };

    struct DepthBook
    {
        FlatSide<true>  bids; // bids: descending
        FlatSide<false> asks; // asks: ascending

        [[nodiscard]]
        double best_bid() const noexcept {
            return bids.size ? bids.lvls[0].price : 0.0;
        }

        [[nodiscard]]
        double best_ask() const noexcept {
            return asks.size ? asks.lvls[0].price : 0.0;
        }

        [[nodiscard]]
        double mid() const noexcept {
            return (best_bid() + best_ask()) * 0.5;
        }
    };

}

void flat_level_map::TestAll()
{

}
