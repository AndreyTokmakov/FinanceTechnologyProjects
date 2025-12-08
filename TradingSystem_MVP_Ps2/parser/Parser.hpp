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
#include <string_view>
#include "Buffer.hpp"
#include "MarketData.hpp"
#include "BinanceDataParser.hpp"

template<typename T>
concept PricerType = requires(T& parser, market_data::binance::BinanceMarketEvent& event) {
    { parser.push(event) } -> std::same_as<void>;
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
            // std::cout << "Parser [CPU: " << getCpu() << "] : " << buffer.length() << std::endl;
            BinanceMarketEvent event = parseBuffer(buffer);
            pricer.push(event);
        }
    };

    struct EventParser
    {
        void parse(const buffer::Buffer& buffer);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP