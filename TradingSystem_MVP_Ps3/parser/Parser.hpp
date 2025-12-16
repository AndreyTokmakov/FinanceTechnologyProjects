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

#include "MarketData.hpp"
#include "Buffer.hpp"
#include "BinanceDataParser.hpp"

namespace parser
{
    using market_data::binance::BinanceMarketEvent;

    struct DummyParser
    {
        BinanceMarketEvent parse(const buffer::Buffer& buffer);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP