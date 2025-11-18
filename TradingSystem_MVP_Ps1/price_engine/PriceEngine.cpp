/**============================================================================
Name        : PriceEngine.cpp
Created on  : 18.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceEngine.cpp
============================================================================**/

#include "PriceEngine.hpp"

namespace price_engine
{
    template<class LevelMap>
    void PricingEngine::addLevel(const PriceLevel& priceLevel,
                                 LevelMap& levelMap)
    {
        auto [iter, inserted] = levelMap.emplace(priceLevel.price, priceLevel.quantity);
        if (!inserted) {
            iter->second = priceLevel.quantity;
        }
    }

    template<class LevelMap>
    size_t PricingEngine::removeLevel(const PriceLevel& priceLevel,
                                      LevelMap& levelMap)
    {
        return levelMap.erase(priceLevel.price);
    }

    void PricingEngine::addBidPrice(const PriceLevel& priceLevel) {
        addLevel(priceLevel, bidPriceLevelMap);
    }

    void PricingEngine::addAskPrice(const PriceLevel& priceLevel) {
        addLevel(priceLevel, askPriceLevelMap);
    }

    void PricingEngine::removeBidPrice(const PriceLevel& priceLevel) {
        removeLevel(priceLevel, bidPriceLevelMap);
    }

    void PricingEngine::removeAskPrice(const PriceLevel& priceLevel) {
        removeLevel(priceLevel, askPriceLevelMap);
    }

    void PricingEngine::handleBidUpdate(const PriceLevel& priceLevel)
    {
        if (priceLevel.quantity == 0) {
            removeBidPrice(priceLevel);
        }
        else {
            addBidPrice(priceLevel);
        }
    }

    void PricingEngine:: handleAskUpdate(const PriceLevel& priceLevel)
    {
        if (priceLevel.quantity == 0) {
            removeAskPrice(priceLevel);
        }
        else {
            addAskPrice(priceLevel);
        }
    }

}