/**============================================================================
Name        : working_example.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <print>
#include <thread>

#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cerrno>
#include <netdb.h>
#include <queue>

#include "common/Buffer.hpp"
#include "common/RingBuffer.hpp"

#include "FinalAction.hpp"
#include "DateTimeUtilities.hpp"

namespace
{
    using namespace common;

    struct TcpTestConnector
    {
        constexpr static uint32_t RECV_BUFFER_SIZE { 128 };
        constexpr static int32_t INVALID_SOCKET { -1 };
        constexpr static int32_t SOCKET_ERROR { -1 };
        using Socket = int32_t;

        Socket socket { INVALID_SOCKET };
        ssize_t bytes { 0 };

        [[nodiscard]]
        bool init()
        {
            socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (INVALID_SOCKET == socket) {
                std::cout << "Failed to create socket. Error = " << errno << std::endl;
                return false;
            }
            return true;
        }

        void close()
        {
            if (INVALID_SOCKET != socket) {
                ::close(socket);
                socket = INVALID_SOCKET;
            }
        }

        [[nodiscard]]
        bool connectAndSubscribe(const std::string_view host, const uint16_t port)
        {
            const sockaddr_in server {PF_INET, htons(port), {.s_addr = inet_addr(host.data())}, {}};
            std::cout << "Connecting to server..." << std::endl;
            const int error = ::connect(socket, reinterpret_cast<const sockaddr*>(&server), sizeof(server));
            if (error == SOCKET_ERROR) {
                std::cout << "Connect function failed with error: " << errno << std::endl;
                return false;
            }

            std::cout << "Connected.\n";

            // FIXME:
            const std::string httpRequest = "Subscribe: BTC/USDT\n";
            bytes = ::send(socket, httpRequest.c_str(), httpRequest.length(), 0);

            return true;
        }

        void getData(buffer::Buffer& response)
        {
            bytes = RECV_BUFFER_SIZE;
            while (bytes == RECV_BUFFER_SIZE) {
                bytes = ::recv(socket, response.tail(RECV_BUFFER_SIZE), RECV_BUFFER_SIZE, 0);
                response.incrementLength(bytes);
            }
            bytes = RECV_BUFFER_SIZE;
        }
    };


    template<typename T>
    concept ConnectorType = requires(T& connector, buffer::Buffer& buffer) {
        { connector.getData(buffer) } -> std::same_as<void>;
    };

    template<typename T>
    concept ParserType = requires(T& parser, const buffer::Buffer& buffer) {
        { parser.parse(buffer) } -> std::same_as<std::string>;
    };

    template<ConnectorType Connector, ParserType Parser>
    struct DataFeederBase
    {
        Connector& connector;
        Parser& parser;

        ring_buffer::static_capacity_with_commit_buffer::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        DataFeederBase(Connector& connector, Parser& parser)
            : connector(connector), parser(parser) {
        }

        void run()
        {
            connectorThread = std::jthread { [&] { runConnector(); } };
            parserThread = std::jthread { [&] { runParser(); } };
        }

    private:

        void runConnector()
        {
            if (!setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                connector.getData(*response);
                std::cout << "Connector [CPU: " << getCpu() << ") : " << response->length() << std::endl;
                queue.commit();
            }
        }

        void runParser()
        {
            if (!setThreadCore(2)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 2  << std::endl;
                return;
            }

            uint32_t misses { 0 };
            buffer::Buffer* item { nullptr };
            while (true)
            {
                if ((item = queue.pop())) {
                    parser.parse(*item);
                    item->clear();
                    misses = 0;
                    continue;
                }

                if (misses++ > maxSessionBeforeSleep) {
                    std::this_thread::sleep_for(std::chrono::microseconds (10U));
                }
            }
        }
    };

    struct DummyParser
    {
        static std::string parse(const buffer::Buffer& buffer) {
            std::cout << "Parser [CPU: " << getCpu() << ") : " << buffer.length() << std::endl;
            return "";
        }
    };
}


void working_example()
{
    constexpr static uint16_t port { 52525 };
    constexpr static std::string_view host {"0.0.0.0"};

    DummyParser parser {};

    TcpTestConnector connector {};
    if (!connector.init()) {
        std::cerr << "Failed to init connector" << std::endl;
        return;
    }
    if (!connector.connectAndSubscribe(host, port)) {
        std::cerr << "Failed to connect and subscribe" << std::endl;
        return;
    }

    DataFeederBase feeder {connector, parser};
    feeder.run();
}