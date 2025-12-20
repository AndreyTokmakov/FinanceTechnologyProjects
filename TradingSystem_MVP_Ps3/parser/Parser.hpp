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

#include "Buffer.hpp"
#include "BinanceDataParser.hpp"

namespace parser
{
    using market_data::binance::BinanceMarketEvent;

    // TODO: Create ::parse(buffer) concepts for Parser

    struct BinanceParser
    {
        BinanceMarketEvent parse(const buffer::Buffer& buffer) const;
    };

    struct ByBitParser
    {
        // FIXME: Return different type
        BinanceMarketEvent parse(const buffer::Buffer& buffer) const;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP