/**============================================================================
Name        : MarketDepthBook.hpp
Created on  : 04.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketDepthBook.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETDEPTHBOOK_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKETDEPTHBOOK_HPP

#include <optional>

namespace depth_book
{
    struct MarketDepthBook
    {
        constexpr static size_t MaxDepth { 1'000 };
        using Price    = double;
        using Quantity = double;

        flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Ascending>   asks { MaxDepth };
        flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Descending>  bids { MaxDepth };

        template<flat_map::SortOrder Ordering>
        static void handlePriceUpdate(flat_map::FlatMap<Price, Quantity, Ordering>& priceMap,
                                      const Price price,
                                      const Quantity quantity)
        {
            if (0 == quantity) {
                priceMap.erase(price);
                return;
            }
            if (const auto result = priceMap.push(price, quantity)) {
                result->value = quantity;
            }
        }

        template<flat_map::SortOrder Ordering>
        [[nodiscard]]
        static std::optional<std::pair<Price, Quantity>>
        getBestPrice(const flat_map::FlatMap<Price, Quantity, Ordering>& priceMap) noexcept
        {
            if (priceMap.empty()) {
                return std::nullopt;
            }
            const auto [price, quantity] = *(priceMap.front());
            return std::make_optional(std::make_pair(price, quantity));
        }

        void buyUpdate(const Price price, const Quantity quantity) {
            handlePriceUpdate(bids, price, quantity);
        }

        void askUpdate(const Price price, const Quantity quantity) {
            handlePriceUpdate(asks, price, quantity);
        }

        [[nodiscard]]
        std::optional<std::pair<Price, Quantity>> getBestBid() const noexcept {
            return getBestPrice(bids);
        }

        [[nodiscard]]
        std::optional<std::pair<Price, Quantity>> getBestAsk() const noexcept {
            return getBestPrice(asks);
        }

        [[nodiscard]]
        std::optional<Price> getMarketPrice() const noexcept
        {
            if (asks.empty() || bids.empty()) {
                return std::nullopt;
            }
            return std::midpoint(asks.front()->key, bids.front()->key);
        }

        [[nodiscard]]
        Price getSpread() const noexcept
        {
            if (asks.empty() || bids.empty()) {
                return 0.0;
            }
            return std::abs(asks.front()->key - bids.front()->key);
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDEPTHBOOK_HPP