/**============================================================================
Name        : Parser.cpp
Created on  : 26.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parser.cpp
============================================================================**/

#include "Parser.h"


namespace
{
    const uint64_t pageSize = sysconf(_SC_PAGESIZE);

    bool need_allocation(const char *data,
                         const size_t length)
    {
        return ((reinterpret_cast<uintptr_t>(data + length - 1) % pageSize) + simdjson::SIMDJSON_PADDING > pageSize);
    }

    simdjson::padded_string_view
    to_padded_string_view(const char *data,
                          const size_t length,
                          simdjson::padded_string &jsonBuffer) {
        if (need_allocation(data, length)) [[unlikely]]
        {
            jsonBuffer = simdjson::padded_string(data, length);
            return jsonBuffer;
        }
        else [[likely]]
        {
            return simdjson::padded_string_view(data, length, length + simdjson::SIMDJSON_PADDING);
        }
    }

    simdjson::padded_string_view get_padded_string(const std::string& buffer,
                                                   simdjson::padded_string &jsonBuffer)
    {
        return to_padded_string_view(buffer.data(), buffer.size(), jsonBuffer);
    }

    simdjson::padded_string_view get_padded_string(const std::string_view& buffer,
                                                   simdjson::padded_string &jsonBuffer)
    {
        return to_padded_string_view(buffer.data(), buffer.size(), jsonBuffer);
    }

    simdjson::padded_string_view get_padded_string(const char *data,
                                                   const size_t length,
                                                   simdjson::padded_string &jsonBuffer)
    {
        return to_padded_string_view(data, length, jsonBuffer);
    }

    // FIXME ---> thread_local ???

    simdjson::ondemand::parser parser;
    simdjson::ondemand::array array;
    simdjson::ondemand::document document;
    simdjson::padded_string jsonBuffer;
}

namespace
{
    struct JsonParams
    {
        static inline constexpr std::string_view data { "data" };
        static inline constexpr std::string_view pair { "ps" };
        static inline constexpr std::string_view symbol { "s" };
        static inline constexpr std::string_view eventType { "e" };
        static inline constexpr std::string_view eventTime { "E" };
        static inline constexpr std::string_view priceChange { "p" };
        static inline constexpr std::string_view priceChangePercent { "P" };
        static inline constexpr std::string_view lastPrice { "c" };
        static inline constexpr std::string_view lastQuantity { "Q" };
        static inline constexpr std::string_view openPrice { "o" };
        static inline constexpr std::string_view highPrice { "h" };
        static inline constexpr std::string_view lowPrice { "l" };
        static inline constexpr std::string_view totalTradedVolume { "v" };
        static inline constexpr std::string_view totalTradedBaseAssetVolume{ "q" };
        static inline constexpr std::string_view firstTradeId { "F" };
        static inline constexpr std::string_view lastTradeId { "L" };
        static inline constexpr std::string_view totalTradesNumber { "n" };

        /** Depth **/
        static inline constexpr std::string_view firstUpdateId { "U" };
        static inline constexpr std::string_view finalUpdateId { "u" };
        static inline constexpr std::string_view bids { "b" };
        static inline constexpr std::string_view asks { "a" };
    };


    struct EventTypeNames
    {
        static inline constexpr std::string_view depthUpdate { "depthUpdate" };
        static inline constexpr std::string_view ticker { "24hrTicker" };
        static inline constexpr std::string_view miniTicker { "24hrMiniTicker" };
        static inline constexpr std::string_view aggTrade { "aggTrade" };
        static inline constexpr std::string_view trade { "trade" };
    };
}

namespace market_data
{
    /*
    bool parse(const std::string_view& payload, market_data::Event& event)
    {

    }*/

    bool parse(const char *data,
               const size_t length,
               market_data::Event& event)
    {
        simdjson::ondemand::object dataObj;
        parser.iterate(get_padded_string(data, length, jsonBuffer)).get(document);
        if (simdjson::SUCCESS != document[JsonParams::data].get(dataObj))
        {
            event.type = EventType::Result;
            return false;
        }

        const std::string_view eventTypeSv = dataObj[JsonParams::eventType].get_string().value();
        if (EventTypeNames::ticker == eventTypeSv)
        {
            event.type = EventType::Ticker;

            dataObj[JsonParams::symbol].get_string(event.symbol);
            dataObj[JsonParams::pair].get_string(event.pair);
            event.eventTime = dataObj[JsonParams::eventTime].get_int64();

            event.ticker.priceChange = dataObj[JsonParams::priceChange].get_double_in_string();
            event.ticker.priceChangePercent = dataObj[JsonParams::priceChangePercent].get_double_in_string();
            event.ticker.lastPrice = dataObj[JsonParams::lastPrice].get_double_in_string();
            event.ticker.lastQuantity = dataObj[JsonParams::lastQuantity].get_double_in_string();
            event.ticker.openPrice = dataObj[JsonParams::openPrice].get_double_in_string();
            event.ticker.highPrice = dataObj[JsonParams::highPrice].get_double_in_string();
            event.ticker.lowPrice = dataObj[JsonParams::lowPrice].get_double_in_string();
            event.ticker.totalTradedVolume = dataObj[JsonParams::totalTradedVolume].get_double_in_string();
            event.ticker.totalTradedBaseAssetVolume = dataObj[JsonParams::totalTradedBaseAssetVolume].get_double_in_string();
            event.ticker.firstTradeId = dataObj[JsonParams::firstTradeId].get_int64();
            event.ticker.lastTradeId = dataObj[JsonParams::lastTradeId].get_int64();
            event.ticker.totalTradesNumber = dataObj[JsonParams::totalTradesNumber].get_int64();
        }
        else if (EventTypeNames::miniTicker == eventTypeSv)
        {
            event.type = EventType::MiniTicker;
        }
        else if (EventTypeNames::aggTrade == eventTypeSv)
        {
            event.type = EventType::AggTrade;
        }
        else if (EventTypeNames::trade == eventTypeSv)
        {
            event.type = EventType::Trade;
        }
        else if (EventTypeNames::depthUpdate == eventTypeSv)
        {
            event.type = EventType::DepthUpdate;

            dataObj[JsonParams::symbol].get_string(event.symbol);
            //data[JsonParams::pair].get_string(event.pair);
            event.eventTime = dataObj[JsonParams::eventTime].get_int64();

            event.depth.bid.clear();
            if (simdjson::SUCCESS != dataObj[JsonParams::bids].get(array)) {
                return false;
            }
            for (auto entry: array) {
                auto& bid = event.depth.bid.emplace_back();
                entry.get(bid.price);
                entry.get(bid.quantity);
                bid.price = entry.at(0).get<double>();
                bid.quantity = entry.at(1).get<double>();
                std::cout << entry << " = [" << bid.price << ", " << bid.quantity << "]\n";
            }

            event.depth.ask.clear();
            if (simdjson::SUCCESS != dataObj[JsonParams::asks].get(array)) {
                return false;
            }
            for (auto entry: array) {
                auto& ask = event.depth.ask.emplace_back();
                entry.get(ask.price);
                entry.get(ask.quantity);
                std::cout << entry << std::endl;
            }
        }

        return true;
    }
}