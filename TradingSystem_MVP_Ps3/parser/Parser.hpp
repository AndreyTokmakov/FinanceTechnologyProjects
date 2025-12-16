/**============================================================================
Name        : Parser.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parser.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_PARSER_HPP
#define FINANCETECHNOLOGYPROJECTS_PARSER_HPP

#include <concepts>
#include "Buffer.hpp"
#include "Exchange.hpp"
#include "MarketData.hpp"
#include "BinanceDataParser.hpp"

namespace parser
{
    using market_data::binance::BinanceMarketEvent;

    BinanceMarketEvent parseEventData(const nlohmann::json& jsonData);
    BinanceMarketEvent parseBuffer(const buffer::Buffer& buffer);

    // template<MarketEventProcessor _EventProcessor>
    template<typename _EventProcessor>
    struct DummyParser
    {
        _EventProcessor& marketStateManager;

        explicit DummyParser(_EventProcessor& eventProcessor): marketStateManager { eventProcessor } {
        }

        void parse(const buffer::Buffer& buffer)
        {
            BinanceMarketEvent event = parseBuffer(buffer);

            // FIXME: 1. Get Exchange Type
            // FIXME: 2. Use right Exchange
            marketStateManager.push(common::Exchange::Binance, event);
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP