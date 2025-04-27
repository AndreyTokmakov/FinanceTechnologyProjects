/**============================================================================
Name        : MarketDataTypes.h
Created on  : 27.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketDataTypes.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H
#define FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H

#include <variant>
#include "Ticker.h"

namespace market_data
{
    struct Result {};
    using MarketEvent = std::variant<Ticker, Result>;
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATATYPES_H
