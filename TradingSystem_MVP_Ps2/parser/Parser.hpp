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
#include "MarketData.hpp"
#include "BinanceDataParser.hpp"

template<typename T>
concept PricerType = requires(T& parser, market_data::binance::BinanceMarketEvent& event) {
    { parser.push(event) } -> std::same_as<void>;
};

namespace parser
{
    using namespace market_data::binance;

    template<PricerType PricerT>
    struct DummyParser
    {
        PricerT& pricer;

        explicit DummyParser(PricerT& pricer): pricer { pricer } {
        }

        static BinanceMarketEvent parseEventData(const nlohmann::json& jsonData)
        {
            std::string_view stream = jsonData[JsonParams::stream].get<std::string_view>();
            const size_t pos = stream.find('@');
            // const std::string_view symbol ( stream.data(), pos);
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

        void parse(const buffer::Buffer& buffer)
        {
            // std::cout << "Parser [CPU: " << getCpu() << "] : " << buffer.length() << std::endl;
            const std::string_view data = std::string_view(buffer.head(), buffer.length());
            const nlohmann::json jsonData = nlohmann::json::parse(data);
            BinanceMarketEvent event = parseEventData(jsonData);
            pricer.push(event);
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_HPP