/**============================================================================
Name        : PriceEngine.hpp
Created on  : 18.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceEngine.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP
#define FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP

#include <boost/container/flat_map.hpp>

namespace price_engine
{
    using Price = double;
    using Quantity = double;

    struct PriceLevel
    {
        Price price { 0 };
        Quantity quantity { 0 };
    };

    struct PricingEngine
    {
        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::greater<>> bidPriceLevelMap;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Price, Quantity, std::less<>> askPriceLevelMap;

        template<class LevelMap>
        void addLevel(const PriceLevel& priceLevel,
                      LevelMap& levelMap);

        template<class LevelMap>
        static size_t removeLevel(const PriceLevel& priceLevel,
                                  LevelMap& levelMap);

        void addBidPrice(const PriceLevel& priceLevel);
        void addAskPrice(const PriceLevel& priceLevel);

        void removeBidPrice(const PriceLevel& priceLevel);
        void removeAskPrice(const PriceLevel& priceLevel);

        void handleBidUpdate(const PriceLevel& priceLevel);
        void handleAskUpdate(const PriceLevel& priceLevel);
    };
};

#endif //FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP