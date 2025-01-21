/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <numeric>
#include <list>
#include <unordered_map>
#include <memory>
#include <thread>

#include <nlohmann/json.hpp>
#include "Order.h"
#include <boost/container/flat_map.hpp>

#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace
{
    constexpr int INVALID_SOCKET { -1 };
    constexpr int SOCKET_ERROR { -1 };

    using Socket = int32_t;
    using UserId = uint32_t;
    using HashType = size_t;
    using IdType   = size_t;
}


struct Server
{
    constexpr static uint32_t receiveBufferSize { 1024 };
    constexpr static std::string_view host {"0.0.0.0"};

    Socket serverSocket { -1 };
    uint16_t serverPort { 52525 };

    explicit Server(const uint16_t port = 52525) : serverPort { port }
    {
        serverSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (INVALID_SOCKET == serverSocket)
            throw std::runtime_error("Failed to create UDP socket");

        sockaddr_in server {PF_INET, htons(serverPort), {.s_addr = inet_addr(host.data())}, {}};
        if (SOCKET_ERROR == ::bind(serverSocket, reinterpret_cast<sockaddr*>(&server), sizeof(server))) {
            throw std::runtime_error("Failed to bind socket on port " + std::to_string(serverPort));
        }
    }

    ~Server()
    {
        ::close(serverSocket);
    }

    /// TODO: Use select/poll/epoll instead od ::recvfrom(....) ???
    [[noreturn]]
    void runServer()
    {
        sockaddr_in senderAddress {};
        socklen_t len = sizeof(senderAddress);
        std::array<uint8_t, receiveBufferSize> buffer {};
        std::string payload {};
        ssize_t bytesReceived { -1 };

        while (true)
        {
            bytesReceived = ::recvfrom(serverSocket,
                                       buffer.data() ,receiveBufferSize, 0,
                                       reinterpret_cast<sockaddr*>(&senderAddress), &len);
            if (SOCKET_ERROR != bytesReceived)
            {
                payload.assign(reinterpret_cast<const char*>(buffer.data()), bytesReceived);
                std::cout << payload << std::endl;

            } else  {
                std::cerr << "Failed to receive data from server" << std::endl;
            }
        }
    }
};


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    Server server;
    server.runServer();

    return EXIT_SUCCESS;
}
