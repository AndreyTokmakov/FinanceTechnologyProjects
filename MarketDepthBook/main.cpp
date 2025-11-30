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

    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Descending> asks {MaxDepth };
    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Ascending>  bids { MaxDepth };

    void buyUpdate(const Price price, const Quantity quantity)
    {
        if (quantity != 0) {
            const flat_map::FlatMap<double, double>::node_pointer result = bids.push(price, quantity);
            result->value = quantity;
            return;
        }
        bids.erase(price);
    }

    // TODO:
    //  - bid()
    //  - ask()
    //  - spread()
    //  - number of asks / bids ?
};



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
