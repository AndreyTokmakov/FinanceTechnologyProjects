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
#include "simdjson.h"
#include "PerfUtilities.h"

namespace
{
    const std::string json { R"( {"data":{"A":"0.08737000","B":"0.10420000","C":1745472821988,"E":1745472822004,
        "F":1798873,"L":1893166,"O":1745386421988,"P":"-0.886","Q":"0.00026000","a":"92671.21000000",
        "b":"92671.20000000","c":"92671.20000000","e":"24hrTicker","h":"120000.00000000","l":"22009.80000000",
        "n":94294,"o":"93499.99000000","p":"-828.79000000","q":"216672881.48598570","s":"BTCUSDT","v":"2319.45099000",
        "w":"93415.58947360","x":"93499.99000000"},"stream":"btcusdt@ticker"})"
    };
}


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    simdjson::dom::parser parser;
    simdjson::dom::element doc;

    const simdjson::error_code error = parser.parse(json).get(doc);
    std::cout << error << std::endl;

    auto value = doc["data"]["s"].get_string();

    std::cout << value << std::endl;

    return EXIT_SUCCESS;
}
