/**============================================================================
Name        : EventConsumer.h
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : EventConsumer.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EVENTCONSUMER_H
#define FINANCETECHNOLOGYPROJECTS_EVENTCONSUMER_H

#include <string_view>
#include <thread>
#include "Event.h"
#include "Queue.h"

namespace EventConsumer
{
    using Socket = int32_t;

    struct Server
    {
        constexpr static uint32_t receiveBufferSize { 10 * 1024 };
        constexpr static std::string_view host {"0.0.0.0" };

        std::jthread serverThread;
        Common::Queue<Common::DepthEvent>& eventQueue;

        Socket serverSocket { -1 };
        uint16_t serverPort { 52525 };

        explicit Server(Common::Queue<Common::DepthEvent>& queue,
                        uint16_t port = 52525);
        ~Server();

        void runServer();
        void consumeEvents();

        void processMessage(const std::string& message,
                            uint32_t type);
    };


    void TestAll();
}

#endif //FINANCETECHNOLOGYPROJECTS_EVENTCONSUMER_H
