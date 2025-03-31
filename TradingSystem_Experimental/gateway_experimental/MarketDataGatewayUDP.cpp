/**============================================================================
Name        : EventConsumer.cpp
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : EventConsumer.cpp
============================================================================**/

#include "MarketDataGatewayUDP.h"
#include "Utils.h"

#include <iostream>
#include <string_view>
#include <array>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <nlohmann/json.hpp>

namespace
{
    constexpr int INVALID_SOCKET { -1 };
    constexpr int SOCKET_ERROR { -1 };

    void addPriceLevel(const nlohmann::json& lvl,
                       std::vector<Common::PriceLevel>& prices)
    {
        static std::string buffer;
        Common::PriceLevel& level = prices.emplace_back();

        lvl[0].get_to(buffer);
        level.price = Utils::priceToLong(buffer);

        lvl[1].get_to(buffer);
        level.quantity = Utils::priceToLong(buffer);
    }
}

namespace Gateway_Experimental::UDP
{
    Server::Server(Common::Queue<Common::DepthEvent>& queue,
                   const uint16_t port) : eventQueue {queue}, serverPort { port }
    {
        serverSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (INVALID_SOCKET == serverSocket)
            throw std::runtime_error("Failed to create UDP socket");

        sockaddr_in server {PF_INET, htons(serverPort), {.s_addr = inet_addr(host.data())}, {}};
        if (SOCKET_ERROR == ::bind(serverSocket, reinterpret_cast<sockaddr*>(&server), sizeof(server))) {
            throw std::runtime_error("Failed to bind socket on port " + std::to_string(serverPort));
        }
    }

    Server::~Server()
    {
        ::close(serverSocket);
    }

    /// TODO: Use select/poll/epoll instead od ::recvfrom(....) ???
    void Server::consumeEvents()
    {
        sockaddr_in senderAddress {};
        socklen_t len = sizeof(senderAddress);
        std::array<uint8_t, receiveBufferSize> buffer {};
        std::string message {};
        ssize_t bytesReceived { -1 };

        message.reserve(32 * 1024);
        while (true)
        {
            bytesReceived = ::recvfrom(serverSocket,
                                       buffer.data() ,receiveBufferSize, 0,
                                       reinterpret_cast<sockaddr*>(&senderAddress), &len);
            if (SOCKET_ERROR != bytesReceived) {
                const nlohmann::json data = nlohmann::json::parse(buffer.begin(), buffer.begin() + bytesReceived);
                const uint32_t partsLeft = data["p"].get<uint32_t>();
                const uint32_t type = data["t"].get<uint32_t>();
                message.append(data["d"]);
                if (0 == partsLeft) {
                    processMessage(message, type);
                    message.clear();
                }
            }
            else  {
                std::cout << "[Failed to receive data from server]" << std::endl;
                return;
            }
        }
    }

    void Server::runServer()
    {
        // FIXME
        serverThread = std::jthread(&Server::consumeEvents, this);
    }

    void Server::processMessage(const std::string& message,
                                const uint32_t type )
    {
        using namespace Common;
        DepthEvent event;
        event.type = type == 1 ? EventType::DepthSnapshot : EventType::DepthUpdate;

        nlohmann::json asks, bids;
        const nlohmann::json jsonMessage = nlohmann::json::parse(message);
        const nlohmann::json& data = jsonMessage["data"];
        if (EventType::DepthSnapshot == event.type)
        {
            asks = data["asks"];
            bids = data["bids"];
            event.lastUpdateId = data["lastUpdateId"];
            event.symbol = jsonMessage["s"].get<std::string>();
        }
        else if (EventType::DepthUpdate == event.type)
        {
            asks = data["a"];
            bids = data["b"];
            event.id = data["E"].get<uint64_t>();
            event.lastUpdateId = data["u"].get<uint64_t>();
            event.firstUpdateId = data["U"].get<uint64_t>();
            event.symbol = data["s"].get<std::string>();
        }

        for (const auto& lvl: asks) {
            addPriceLevel(lvl, event.akss);
        }
        for (const auto& lvl: bids) {
            addPriceLevel(lvl, event.bids);
        }

        eventQueue.push(std::move(event));
    }
}