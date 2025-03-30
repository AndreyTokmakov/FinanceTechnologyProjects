/**============================================================================
Name        : Manager.h
Created on  : 30.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Manager.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MANAGER_H
#define FINANCETECHNOLOGYPROJECTS_MANAGER_H

#include <thread>
#include <memory>
#include <array>

#include "Event.h"
#include "Queue.h"
#include "BinanceEventsParser.h"

namespace Manager
{
    enum class ConnectorType
    {
        Binance = 0,
        Unknown
    };

    // TODO: Rename
    class Manager
    {
        Common::Queue<std::string>& eventQueue;
        std::jthread eventHandlerThread;

        std::array<std::unique_ptr<Parser::IEventParser>, 1> parsers;

        void handleEvents();

    public:

        explicit Manager(Common::Queue<std::string>& queue);
        void start();
    };
}


#endif //FINANCETECHNOLOGYPROJECTS_MANAGER_H
