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

namespace SymbolTicker
{
    /**
    {
      "e":"24hrTicker",             // Event type
      "E":1591268262453,            // Event time
      "s":"BTCUSD_200626",          // Symbol
      "ps":"BTCUSD",                // Pair
      "p":"-43.4",                  // Price change
      "P":"-0.452",                 // Price change percent
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
        "F":1798873,"L":1893166,"O":1745386421988,"P":"-0.886","Q":"0.00026000","a":"92671.21000000",
        "b":"92671.20000000","c":"92671.20000000","e":"24hrTicker","h":"120000.00000000","l":"22009.80000000",
        "n":94294,"o":"93499.99000000","p":"-828.79000000","q":"216672881.48598570","s":"BTCUSDT","v":"2319.45099000",
        "w":"93415.58947360","x":"93499.99000000"},"stream":"btcusdt@ticker"})"
    };

    struct Ticker
    {
        int eventTime { 0 };
        std::string symbol {};
        double priceChange {};
        double priceChangePercent {};

        double lastPrice {};
        double lastQuantity {};

        double openPrice {};
        double highPrice {};
        double lowPrice {};

        int totalTradedVolume {};
        double totalTradedBaseAssetVolume {};

        int firstTradeId {};
        int lastTradeId {};
        int totalTradesNumber {};
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


    void parse()
    {
        simdjson::dom::parser parser;
        simdjson::dom::element doc;

        const simdjson::error_code error = parser.parse(symbolTicker).get(doc);
        std::cout << error << std::endl;

        auto data = doc.at('data');


        std::string_view symbol = doc["data"]["s"].get_string();
        std::cout <<  symbol << std::endl;
    }
}




int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    SymbolTicker::parse();

    SymbolTicker::Ticker ticker;

    std::cout << ticker << std::endl;

    return EXIT_SUCCESS;
}
