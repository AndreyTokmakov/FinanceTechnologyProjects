/**============================================================================
Name        : Manager.cpp
Created on  : 30.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Manager.cpp
============================================================================**/

#include "Manager.h"
#include <iostream>

namespace
{
    using namespace Manager;
    using namespace std::string_view_literals;


    std::string_view extractType(const std::string_view message)
    {
        size_t pos = message.find(R"("connector")");
        if (std::string::npos == pos)
            return {};

        const size_t start = message.find('\"', pos + 11);
        if (std::string::npos == start)
            return {};

        pos = message.find('\"', start + 1);
        if (std::string::npos == pos)
            return {};

        return message.substr(start + 1, pos - start - 1);
    }

    ConnectorType fromString(std::string_view strType)
    {
        if (strType == "Binance"sv)
            return ConnectorType::Binance;

        return ConnectorType::Unknown;
    }
}

namespace Manager
{
    Manager::Manager(Common::Queue<std::string>& queue): eventQueue {queue}
    {
        parsers[static_cast<int32_t>(ConnectorType::Binance)] = std::make_unique<Parser::BinanceEventsParser>();
    }

    void Manager::start()
    {
        // FIXME
        eventHandlerThread = std::jthread(&Manager::handleEvents, this);
    }

    void Manager::handleEvents()
    {
        std::string event;
        while (true)
        {
            if (!eventQueue.pop(event)) {
                continue;
            }

            const std::string_view strType = extractType(event);
            const ConnectorType connectorType = fromString(strType);
            if (ConnectorType::Unknown == connectorType) {
                std::cerr << "Unknown connector type '" << strType << "'" << std::endl;
                continue;
            }

            parsers[static_cast<int32_t>(connectorType)]->parseEvent(event);
        }
    }
}

