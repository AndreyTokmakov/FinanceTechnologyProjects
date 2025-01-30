/**============================================================================
Name        : EventParser.h
Created on  : 30.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : EventParser.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EVENTPARSER_H
#define FINANCETECHNOLOGYPROJECTS_EVENTPARSER_H

#include <string>

namespace Parser
{
    struct IEventParser
    {
        virtual ~IEventParser() = default;
        virtual void parseEvent(const std::string&) = 0;
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_EVENTPARSER_H
