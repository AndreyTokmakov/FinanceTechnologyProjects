/**============================================================================
Name        : main.cpp
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parsers_JSON
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <format>

#include "simdjson.h"
#include "PerfUtilities.h"
#include "FileUtilities.h"


namespace Data
{
    /**
    {
      "e":"24hrTicker",             // Event type
      "E":1745472822004,            // Event time
      "s":"BTCUSD_200626",          // Symbol
      "ps":"BTCUSD",                // Pair
      "p":"-828.79000000",          // Price change
      "P":"-0.886",                 // Price change percent
      "w":"0.00147974",             // Weighted average price
      "c":"9548.5",                 // Last price
      "Q":"2",                      // Last quantity
      "o":"9591.9",                 // Open price
      "h":"10000.0",                // High price
      "l":"7000.0",                 // Low price
      "v":"487850",                 // Total traded volume
      "q":"32968676323.46222700",   // Total traded base asset volume
      "O":1591181820000,            // Statistics open time
      "C":1591268262442,            // Statistics close time
      "F":512014,                   // First trade ID
      "L":615289,                   // Last trade Id
      "n":103272                    // Total number of trades
    }
    **/


    const std::string symbolTicker { R"( {"data":{"A":"0.08737000","B":"0.10420000","C":1745472821988,"E":1745472822004,
"F":1798873,"L":1893166,"O":1745386421988,"P":"-0.886","Q":"0.00026000","a":"92671.21000000","b":"92671.20000000",
"c":"92671.20000000","e":"24hrTicker","h":"120000.00000000","l":"22009.80000000","n":94294,"o":"93499.99000000",
"p":"-828.79000000","q":"216672881.48598570","s":"BTCUSDT","v":"2319.45099000","w":"93415.58947360","x":"93499.99000000"},
"stream":"btcusdt@ticker"})"
    };
}

namespace market_data
{
    struct Ticker
    {
        using price_t = double;
        using quantity_t = double;
        using time_t = int64_t;
        using number_t = int64_t;

        time_t eventTime { 0 };
        std::string symbol {};
        price_t priceChange {};
        price_t priceChangePercent {};

        price_t lastPrice {};
        price_t lastQuantity {};

        price_t openPrice {};
        price_t highPrice {};
        price_t lowPrice {};

        quantity_t totalTradedVolume {};
        quantity_t totalTradedBaseAssetVolume {};

        number_t firstTradeId {};
        number_t lastTradeId {};
        number_t totalTradesNumber {};
    };

    std::ostream& operator<<(std::ostream& stream, const Ticker& ticker)
    {
        stream << std::format("Ticker(\n\teventTime: {},\n\tsymbol: {},"
                              "\n\tpriceChange: {},\n\tpriceChangePercent: {},\n\tlastPrice: {},\n\tlastQuantity: {},\n\topenPrice: {},"
                              "\n\thighPrice: {},\n\tlowPrice: {},\n\ttotalTradedVolume: {},\n\ttotalTradedBaseAssetVolume: {},"
                              "\n\tfirstTradeId: {},\n\tlastTradeId: {},\n\ttotalTradesNumber: {}\n)",
                              ticker.eventTime, ticker.symbol, ticker.priceChange, ticker.priceChangePercent, ticker.lastPrice,
                              ticker.lastQuantity, ticker.openPrice, ticker.highPrice, ticker.lowPrice, ticker.totalTradedVolume,
                              ticker.totalTradedBaseAssetVolume, ticker.firstTradeId, ticker.lastTradeId, ticker.totalTradesNumber
        );
        return stream;
    }


    struct Result {};
}

namespace SymbolTicker
{
    using market_data::Ticker;
    using market_data::Result;

    simdjson::ondemand::parser parser;
    simdjson::ondemand::document document;
    simdjson::padded_string jsonBuffer;

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

    // TODO:
    //  std::string_view {"E"}
    //  std::string_view {"p"}
    //  std::string_view {"P"}
    //  std::string_view {"c"}


    Ticker parseTicker(simdjson::ondemand::object& data)
    {
        Ticker ticker;
        {
            ticker.eventTime = data["E"].get_int64();
            data["s"].get_string(ticker.symbol);
            ticker.priceChange = data["p"].get_double_in_string();
            ticker.priceChangePercent = data["P"].get_double_in_string();
            ticker.lastPrice = data["c"].get_double_in_string();
            ticker.lastQuantity = data["Q"].get_double_in_string();
            ticker.openPrice = data["o"].get_double_in_string();
            ticker.highPrice = data["h"].get_double_in_string();
            ticker.lowPrice = data["l"].get_double_in_string();
            ticker.totalTradedVolume = data["v"].get_double_in_string();
            ticker.totalTradedBaseAssetVolume = data["q"].get_double_in_string();
            ticker.firstTradeId = data["F"].get_int64();
            ticker.lastTradeId = data["L"].get_int64();
            ticker.totalTradesNumber = data["n"].get_int64();
        }
        return ticker;
    }

    Ticker parseTicker1(simdjson::ondemand::object& data)
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
        //parser.iterate(get_padded_string(binanceTickerJson, jsonBuffer)).get(document);
        parser.iterate(get_padded_string(payload, jsonBuffer)).get(document);

        simdjson::ondemand::object data;
        const simdjson::error_code result = document["data"].get(data);

        if (simdjson::SUCCESS != result) {
            return Result{};
        }

        return parseTicker(data);
    }


    void parse()
    {
        const std::string binanceTickerJson { FileUtilities::ReadFile(
                R"(../../Parsers_JSON/data/binance/binance_ticker.json)")
        };
        const std::string result { FileUtilities::ReadFile(
                R"(../../Parsers_JSON/data/binance/result.json)")
        };
        // const std::string_view binanceTickerJsonSv { binanceTickerJson };




        const std::variant event = parse(binanceTickerJson);
        if (std::holds_alternative<Ticker>(event))
        {
            const Ticker& ticker = std::get<Ticker>(event);
            std::cout << ticker << std::endl;
        }
    }
}


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    SymbolTicker::parse();


    return EXIT_SUCCESS;
}
