/**============================================================================
Name        : main.cpp
Created on  : 23.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
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

#include "FinalAction.hpp"
#include "Buffer.hpp"


// TODO:
//  -  Connector / Auth / Fetch data - Strategy
//     - Коннктится к Exchange + авторизуется
//     - Получает данных и кладёт их RingBuffer с GrowingBuffers
//     -
//  -  Parser - Strategy
//     - Конвертирует 'Growing Buffers' в MarketData внутрюнюю стукуру
//     -
//  -  RingBuffer
//     - Holds Growing Buffers
//     - (модифицировать при записи если буфер используется для парсинга)



namespace data_feeder_demo
{
    struct DataFeeder
    {
        template <typename Self>
        void run(this Self&& self)
        {
            while (true)
            {
                auto data = self.getData();
                auto parsed = self.parse(data);
                std::cout << parsed << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds (250U));
            }
        }
    };


    template<typename Connector, typename Parser>
    struct DataFeederImpl: DataFeeder
    {
        Connector& connector;
        Parser& parser;

        DataFeederImpl(Connector& connector, Parser& parser)
            : connector(connector), parser(parser) {
        }

        std::string getData() {
            return connector.getData();
        }

        std::string parse(const std::string& str) {
            return parser.parse(str);
        }
    };

    struct ConnectorImpl
    {
        static std::string getData() {
            return "Feeder2-Data";
        }
    };

    struct ParserImpl
    {
        static std::string parse(const std::string& str) {
            return "{" + str + "}";
        }
    };


    void test()
    {
        ConnectorImpl connector {};
        ParserImpl parser {};

        DataFeederImpl feeder {connector, parser};
        feeder.run();
    }
}

namespace tcp_connector_test
{
    constexpr uint32_t RECV_BUFFER_SIZE { 12 };
    constexpr int32_t INVALID_SOCKET { -1 };
    constexpr int32_t SOCKET_ERROR { -1 };
    constexpr uint16_t port { 52525 };
    constexpr std::string_view host {"0.0.0.0"};

    using Socket = int32_t;


    void test()
    {
        const Socket socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == socket) {
            std::cout << "Failed to create socket. Error = " << errno << std::endl;
            return;
        }

        final_action::ScopeExit cleanup { [&] { ::close(socket); }};

        const sockaddr_in server {PF_INET, htons(port), {.s_addr = inet_addr(host.data())}, {}};
        std::cout << "Connecting to server..." << std::endl;
        const int error = ::connect(socket, (sockaddr*)&server, sizeof(server));
        if (error == SOCKET_ERROR) {
            std::cout << "Connect function failed with error: " << errno << std::endl;
            ::close(socket);
            return;
        }
        else {
            std::cout << "Connected.\n";
        }

        const std::string httpRequest = "Subscribe: BTC/USDT\n";
        ssize_t bytes = ::send(socket, httpRequest.c_str(), httpRequest.length(), 0);

        /*
        bytes = RECV_BUFFER_SIZE;
        std::string response;
        std::array<char, RECV_BUFFER_SIZE> buffer {};
        while (true)
        {
            while (bytes == RECV_BUFFER_SIZE) {
                bytes = ::recv(socket, buffer.data(), buffer.size(), 0);
                response.append(buffer.data(), bytes);
            }
            std::cout << response << std::endl;
            bytes = RECV_BUFFER_SIZE;
            response.clear();
        }
        */

        bytes = RECV_BUFFER_SIZE;
        buffer::Buffer response;
        while (true)
        {
            std::cout << std::string(120, '-') << std::endl;
            while (bytes == RECV_BUFFER_SIZE) {
                bytes = ::recv(socket, response.tail(RECV_BUFFER_SIZE), RECV_BUFFER_SIZE, 0);
                response.incrementLength(bytes);

                std::cout << "Bytes: " << bytes << std::endl;
            }
            std::cout << std::string_view(response.head(), response.length()) << std::endl;
            bytes = RECV_BUFFER_SIZE;
            response.clear();
        }
    }
}

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // data_feeder_demo::test();
    tcp_connector_test::test();


    return EXIT_SUCCESS;
}
