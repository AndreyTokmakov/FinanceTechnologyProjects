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
    simdjson::ondemand::document document;
    simdjson::padded_string jsonBuffer;

    // TODO:
    //  std::string_view {"E"}
    //  std::string_view {"p"}
    //  std::string_view {"P"}
    //  std::string_view {"c"}
    //  std::string_view {"data"}


}

namespace market_data
{
    Ticker parseTicker(simdjson::ondemand::object& data)
    {
        Ticker ticker {
                .eventTime = data["E"].get_int64(),
                .priceChange = data["p"].get_double_in_string(),
                .priceChangePercent = data["P"].get_double_in_string(),
                .lastPrice = data["c"].get_double_in_string(),
                .lastQuantity = data["Q"].get_double_in_string(),
                .openPrice = data["o"].get_double_in_string(),
                .highPrice = data["h"].get_double_in_string(),
                .lowPrice = data["l"].get_double_in_string(),
                .totalTradedVolume = data["v"].get_double_in_string(),
                .totalTradedBaseAssetVolume = data["q"].get_double_in_string(),
                .firstTradeId = data["F"].get_int64(),
                .lastTradeId = data["L"].get_int64(),
                .totalTradesNumber = data["n"].get_int64(),
        };
        data["s"].get_string(ticker.symbol);
        return ticker;
    }

    std::variant<Ticker, Result> parse(const std::string_view& payload)
    {
        parser.iterate(get_padded_string(payload, jsonBuffer)).get(document);
        simdjson::ondemand::object dataObject;
        const simdjson::error_code result = document["data"].get(dataObject);

        if (simdjson::SUCCESS != result) {
            return Result{};
        }

        return parseTicker(dataObject);
    }

    std::variant<Ticker, Result> parse(const char *data,
                                       const size_t length)
    {
        parser.iterate(get_padded_string(data, length, jsonBuffer)).get(document);
        simdjson::ondemand::object dataObject;
        const simdjson::error_code result = document["data"].get(dataObject);

        if (simdjson::SUCCESS != result) {
            return Result{};
        }

        return parseTicker(dataObject);
    }
}