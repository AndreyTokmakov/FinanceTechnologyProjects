/**============================================================================
Name        : MarketStateManager.hpp
Created on  : 12.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketStateManager.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETSTATEMANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_MARKETSTATEMANAGER_HPP

#include "Exchange.hpp"

#include "Parser.hpp"
#include "ExchangeDataProcessor.hpp"

namespace price_engine
{
    struct MarketStateManager
    {
        /** For Exchange::Binance **/
        ExchangeDataProcessor binanceDataProcessor { parser::BinanceParser{} };

        /** For Exchange::ByBit **/
        ExchangeDataProcessor byBitDataProcessor { parser::ByBitParser{} };

        std::array<ExchangeDataProcessor*, 2> books {
            &binanceDataProcessor,
            &byBitDataProcessor
        };

        void run() const;
        void push(common::Exchange exchange, const std::string& eventData) const;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETSTATEMANAGER_HPP