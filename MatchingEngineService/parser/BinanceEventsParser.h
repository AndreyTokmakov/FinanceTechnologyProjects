/**============================================================================
Name        : BinanceEventsParser.h
Created on  : 30.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BinanceEventsParser.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCEEVENTSPARSER_H
#define FINANCETECHNOLOGYPROJECTS_BINANCEEVENTSPARSER_H

#include "EventParser.h"

namespace Parser
{
    struct BinanceEventsParser: IEventParser
    {
        void parseEvent(const std::string& event) override;
    };
}


#endif //FINANCETECHNOLOGYPROJECTS_BINANCEEVENTSPARSER_H
