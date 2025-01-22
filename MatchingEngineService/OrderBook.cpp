/**============================================================================
Name        : OrderBook.cpp
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : OrderBook.cpp
============================================================================**/

#include "OrderBook.h"
#include <boost/container/flat_map.hpp>

#include <iostream>
#include <print>
#include <unordered_map>

namespace MatchingEngine
{
    using Price = int64_t;
    using Quantity = int64_t;

    enum class Side : uint8_t
    {
        Buy,
        Sell,
    };

    struct OrderMatchingEngine
    {
        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::greater<>> bidPriceLevelMap;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::less<>> askPriceLevelMap;

        template<class LevelMap>
        void addLevel(const Price price,
                      const Quantity quantity,
                      LevelMap& levelMap)
        {
            auto [iter, inserted] = levelMap.emplace(price, quantity);
            if (!inserted) {
                iter->second = quantity;
            }
        }

        template<class LevelMap>
        size_t removeLevel(const Price price,
                           LevelMap& levelMap)
        {
            return levelMap.erase(price);
        }

        void addBidPrice(const Price price, const Quantity quantity) {
            addLevel(price, quantity, bidPriceLevelMap);
        }

        void addAskPrice(const Price price, const Quantity quantity) {
            addLevel(price, quantity, askPriceLevelMap);
        }

        void removeBidPrice(const Price price) {
            removeLevel(price, bidPriceLevelMap);
        }

        void removeAskPrice(const Price price) {
            removeLevel(price, askPriceLevelMap);
        }

        void print()
        {
            std::cout << "===================== BID =====================\n";
            for (const auto& [price, quantity]: bidPriceLevelMap)
            {
                std::println("\t[{}, {}]", price, quantity);
            }
            std::cout << "===================== ASK =====================\n";
            for (const auto& [price, quantity]: askPriceLevelMap)
            {
                std::println("\t[{}, {}]", price, quantity);
            }
        }
    };

    void test()
    {
        OrderMatchingEngine engine;

        engine.addBidPrice(100, 1000);
        engine.addBidPrice(101, 1001);
        engine.addBidPrice(102, 1002);
        engine.addBidPrice(100, 1004);


        engine.addAskPrice(103, 1003);
        engine.addAskPrice(104, 1004);
        engine.addAskPrice(105, 1005);

        engine.print();

        engine.removeBidPrice(102);
        engine.removeAskPrice(103);

        engine.print();
    }
}

void OrderBook::TestAll()
{
    MatchingEngine::test();
}
