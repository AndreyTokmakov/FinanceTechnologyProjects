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

template<typename T>
concept PricerType = requires(T& priceEngine, common::Exchange exchange,
        market_data::binance::BinanceMarketEvent& event) {
    { priceEngine.push(exchange, event) } -> std::same_as<void>;
};

namespace parser
{
    using market_data::binance::BinanceMarketEvent;

    BinanceMarketEvent parseEventData(const nlohmann::json& jsonData);
    BinanceMarketEvent parseBuffer(const buffer::Buffer& buffer);

    template<PricerType PricerT>
    struct DummyParser
    {
        PricerT& pricer;

        explicit DummyParser(PricerT& pricer): pricer { pricer } {
        }

        void parse(const buffer::Buffer& buffer)
        {
            BinanceMarketEvent event = parseBuffer(buffer);

            // FIXME: 1. Get Exchange Type
            // FIXME: 2. Use right Exchange
            pricer.push(common::Exchange::Binance, event);
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP