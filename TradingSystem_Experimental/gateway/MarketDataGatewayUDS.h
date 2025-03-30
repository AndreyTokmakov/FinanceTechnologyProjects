/**============================================================================
Name        : MarketDataGatewayUDS.h
Created on  : 29.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MarketDataGatewayUDS.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MARKETDATAGATEWAYUDS_H
#define FINANCETECHNOLOGYPROJECTS_MARKETDATAGATEWAYUDS_H


#include <string_view>
#include <expected>
#include <thread>

#include "Gateway.h"
#include "Event.h"
#include "Queue.h"

#include <sys/poll.h>

namespace Gateway::UDS
{
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
        Common::Queue<std::string>& eventQueue;

        explicit UDSAsynchServer(Common::Queue<std::string>& queue,
                                 std::string udmSockPath);
        ~UDSAsynchServer();

        void closeEvent(pollfd& pollEvent);
        void removeClosedHandles();

        [[nodiscard]]
        std::expected<bool, std::string> init() const;

        bool start();

        [[nodiscard]]
        static std::expected<bool, std::string> setSocketToNonBlock(Socket socket) ;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_MARKETDATAGATEWAYUDS_H
