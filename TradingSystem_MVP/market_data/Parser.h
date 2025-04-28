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
#include "MarketData.h"

namespace market_data
{
    bool parse(const char *data,
               size_t length,
               market_data::Event& event);
}

#endif //FINANCETECHNOLOGYPROJECTS_PARSER_H
