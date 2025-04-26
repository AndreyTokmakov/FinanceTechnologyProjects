/**============================================================================
Name        : Parser.h
Created on  : 26.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Parser.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_PARSER_H
#define FINANCETECHNOLOGYPROJECTS_PARSER_H

#include <variant>
#include "simdjson.h"
#include "Ticker.h"

namespace market_data
{
    struct Result {};

    std::variant<Ticker, Result> parse(const std::string_view& str);
    std::variant<Ticker, Result> parse(const char *data,
                                       const size_t length);
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_H
