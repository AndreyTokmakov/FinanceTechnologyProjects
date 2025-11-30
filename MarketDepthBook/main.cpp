/**============================================================================
Name        : main.cpp
Created on  : 30.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <optional>

#include "FlatMap.hpp"


namespace
{
    template<typename K, typename V, flat_map::SortOrder ordering = flat_map::SortOrder::Ascending>
    std::ostream& operator<<(std::ostream& stream, const flat_map::FlatMap<K, V, ordering>& map)
    {
        for (uint32_t idx = 0; idx < map.size(); ++idx) {
            stream << map.data()[idx].key << " => " << map.data()[idx].value << std::endl;
        }
        return stream;
    }
}


struct MarketDepthBook
{
    constexpr static size_t MaxDepth { 1'000 };
    using Price = double;
    using Quantity = double;

    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Descending> asks { MaxDepth };
    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Ascending>  bids { MaxDepth };

    void buyUpdate(const Price price, const Quantity quantity)
    {
        if (quantity != 0) {
            const auto result = bids.push(price, quantity);
            result->value = quantity;
        }
        else {
            bids.erase(price);
        }
    }

    void askUpdate(const Price price, const Quantity quantity)
    {
        if (quantity != 0) {
            const auto result = asks.push(price, quantity);
            result->value = quantity;
        }
        else {
            asks.erase(price);
        }
    }

    [[nodiscard]]
    std::optional<std::pair<Price, Quantity>> getBestBid() const noexcept
    {
        if (bids.size() == 0) {
            return std::nullopt;
        }
        const auto [price, quantity] = *(bids.front());
        return std::make_optional(std::make_pair(price, quantity));
    }

    // TODO:
    //  - bid()
    //  - ask()
    //  - spread()
    //  - number of asks / bids ?
};

namespace testing
{
    enum class Side {
        Buy,
        Sell
    };

    struct DepthUpdate
    {
        double price { 0.0 };
        double quantity { 0.0 };
        Side side { Side::Buy };
    };

    void handleUpdate(MarketDepthBook& book,
                      const DepthUpdate& depthUpdate)
    {
        if (Side::Buy == depthUpdate.side) {
            book.buyUpdate(depthUpdate.price, depthUpdate.quantity);
        }
        else {
            book.askUpdate(depthUpdate.price, depthUpdate.quantity);
        }
    }

    void handleEvents(MarketDepthBook& book,
                      const std::vector<DepthUpdate>& events)
    {
        for (const auto& event : events) {
            handleUpdate(book, event);
        }
    }

    void test1()
    {
        MarketDepthBook book;
        handleEvents(book, { { 100.0, 100.0, Side::Buy } });
    }
}


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    MarketDepthBook book;

    book.buyUpdate(100.0, 100.0);
    std::cout << book.bids << std::endl;

    book.buyUpdate(100.0, 22.0);
    std::cout << book.bids << std::endl;

    book.buyUpdate(100.0, 11.0);
    std::cout << book.bids << std::endl;


    return EXIT_SUCCESS;
}
