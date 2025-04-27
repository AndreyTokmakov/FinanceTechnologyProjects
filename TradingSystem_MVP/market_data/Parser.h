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

#include "simdjson.h"
#include "MarketDataTypes.h"

namespace market_data
{
    MarketEvent parse(const std::string_view& str);
    MarketEvent parse(const char *data, size_t length);
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_H
