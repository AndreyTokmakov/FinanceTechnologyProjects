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
#include <print>

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

    BinanceMarketEvent parseBuffer(const buffer::Buffer& buffer)
    {
        const std::string_view data = std::string_view(buffer.head(), buffer.length());
        const nlohmann::json jsonData = nlohmann::json::parse(data);
        return parseEventData(jsonData);
    }
}

namespace parser
{
    struct EventHandler
    {
        void operator()([[maybe_unused]] const BookTicker& ticker) const {
            // debug(ticker);
        }

        void operator()([[maybe_unused]] const MiniTicker& ticker) const {
            // debug(ticker);
        }

        void operator()([[maybe_unused]] const Trade& trade) const {
            // debug(trade);
        }

        void operator()([[maybe_unused]] const AggTrade& aggTrade) const {
            // debug(aggTrade);
        }

        void operator()([[maybe_unused]] const DepthUpdate& depthUpdate) const {
            debug(depthUpdate);
        }

        void operator()([[maybe_unused]] const NoYetImplemented& nonImpl) const {
            std::println(std::cerr, "NoYetImplemented(strean: {})", nonImpl.streamName);
        }

    private:

        template<typename Event>
        static void debug(const Event& event)
        {
            std::cout << "Pricer [CPU: " << utilities::getCpu() << "] : " << typeid(event).name() << std::endl;
            std::cout << event << std::endl;
        }
    };

    void EventParser::parse(const buffer::Buffer& buffer)
    {
        BinanceMarketEvent event = parseBuffer(buffer);
        EventHandler eventHandler {};
        std::visit(eventHandler, event);
    }
}