/**============================================================================
Name        : Parser.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parser.cpp
============================================================================**/

#include "Parser.hpp"
#include "Formatting.hpp"
#include "Utils.hpp"

#include <iostream>

namespace
{
    using namespace market_data::binance;
}

namespace parser
{
    BinanceMarketEvent parseEventData(const nlohmann::json& jsonData)
    {
        std::string_view stream = jsonData[JsonParams::stream].get<std::string_view>();
        const size_t pos = stream.find('@');
        stream.remove_prefix(pos + 1);

        const nlohmann::json& data = jsonData[JsonParams::data];
        if (stream.starts_with(StreamNames::miniTicker))
            return BinanceParserJson::parseMiniTicker(data);
        if (stream.starts_with(StreamNames::bookTicker))
            return BinanceParserJson::parseBookTicker(data);
        if (stream.starts_with(StreamNames::trade))
            return BinanceParserJson::parseTrade(data);
        if (stream.starts_with(StreamNames::aggTrade))
            return BinanceParserJson::parseAggTrade(data);
        if (stream.starts_with(StreamNames::depth))
            return BinanceParserJson::parseDepthUpdate(data);

        return NoYetImplemented { std::string(stream) };
    }

    BinanceMarketEvent DummyParser::parse(const buffer::Buffer& buffer)
    {
        // std::cout << "Parser [CPU: " << utilities::getCpu() << "] : " << buffer.length() << std::endl;

        // TODO: Need to be optimised --> SIMD-Json
        const nlohmann::json jsonData = nlohmann::json::parse(buffer.head(), buffer.tail());
        return parseEventData(jsonData);
    }
}