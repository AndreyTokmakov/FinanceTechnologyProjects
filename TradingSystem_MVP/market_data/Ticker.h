/**============================================================================
Name        : Ticker.h
Created on  : 26.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Ticker.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TICKER_H
#define FINANCETECHNOLOGYPROJECTS_TICKER_H

#include <string>
#include <iostream>
#include "Types.h"

namespace market_data
{
    using common::price_t;
    using common::quantity_t;
    using common::number_t;

    // TODO: Restructure
    //  - Place variables uses together close to each other
    //  - CacheLine Alignement - try to Fit all data in the same CacheLine
    struct Ticker
    {
        std::string symbol {};
        price_t priceChange {};
        price_t priceChangePercent {};

        price_t lastPrice {};
        price_t lastQuantity {};

        time_t eventTime { 0 };

        price_t openPrice {};
        price_t highPrice {};
        price_t lowPrice {};

        quantity_t totalTradedVolume {};
        quantity_t totalTradedBaseAssetVolume {};

        number_t firstTradeId {};
        number_t lastTradeId {};
        number_t totalTradesNumber {};
    };


    std::ostream& operator<<(std::ostream& stream, const Ticker& ticker);
}


#endif //FINANCETECHNOLOGYPROJECTS_TICKER_H
