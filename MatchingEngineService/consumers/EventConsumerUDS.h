/**============================================================================
Name        : EventConsumerUDS.h
Created on  : 29.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : EventConsumerUDS.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EVENTCONSUMERUDS_H
#define FINANCETECHNOLOGYPROJECTS_EVENTCONSUMERUDS_H

#include <string_view>
#include <thread>
#include "Event.h"
#include "Queue.h"

#include <sys/poll.h>

namespace EventConsumerUDS
{
    using Socket = int32_t;

    constexpr int32_t RESULT_SUCCESS = 0;
    constexpr int32_t INVALID_HANDLE = -1;
    constexpr int32_t SOCKET_ERROR = -1;

    struct UDSAsynchServer
    {
        constexpr static size_t BUFFER_SIZE { 10 * 1024 };
        constexpr static size_t MAX_DESCRIPTORS { 256 };
        constexpr static int32_t TIMEOUT { 3 * 60 * 1000 };

        Socket serverSocket { INVALID_HANDLE };
        std::string filePath ;

        std::array<char, BUFFER_SIZE> buffer {};
        std::array<pollfd, MAX_DESCRIPTORS> fds {};
        uint32_t handlesCount { 0 };

        std::jthread serverThread;
        Common::Queue<Common::DepthEvent>& eventQueue;

        explicit UDSAsynchServer(Common::Queue<Common::DepthEvent>& queue,
                                 std::string udmSockPath);
        ~UDSAsynchServer();

        void closeEvent(pollfd& pollEvent);
        void removeClosedHandles();

        // TODO: std::expected<R,E>
        [[nodiscard]]
        bool init() const;

        bool start();

        static bool setSocketToNonBlock(Socket socket) ;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EVENTCONSUMERUDS_H
