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
#include <queue>

#include "FinalAction.hpp"
#include "Buffer.hpp"
#include "RingBuffer.hpp"

#include "DateTimeUtilities.hpp"

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

namespace
{
    using namespace DateTimeUtilities;

    bool setThreadCore(const uint32_t coreId) noexcept
    {
        cpu_set_t cpuSet {};
        CPU_ZERO(&cpuSet);
        CPU_SET(coreId, &cpuSet);
        return 0 == pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
    }

    int32_t getCpu() noexcept
    {
        return sched_getcpu();
    }
}

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

namespace lock_free_queue_polling
{
    struct Queue
    {
        int counter { 0 };

        bool get(int& result)
        {
            ++counter;
            if (0 == counter % 10) {
                result = counter;
                return true;
            }
            return false;
        }
    };

    void run()
    {
        Queue queue {};
        int result;
        uint32_t misses { 0 };
        while (true)
        {
            if (queue.get(result))
            {
                std::cout << getCurrentTime() << "  Got: " << result << std::endl;
                misses = 0;
                continue;
            }

            std::cout << getCurrentTime()  << "  No data" << std::endl;

            ++misses;
            if (misses > 5) {
                std::this_thread::sleep_for(std::chrono::milliseconds (500U));
            }
        }
    }
}

namespace ring_buffer_polling_demo
{
    void run()
    {
        ring_buffer::static_capacity::RingBuffer<int, 1024> queue {};

        auto consume = [&queue]
        {
            uint32_t misses { 0 };
            int32_t result { 0 };
            while (true)
            {
                if (queue.pop(result)) {
                    std::cout << getCurrentTime() << "  Got: " << result << std::endl;
                    misses = 0;
                    continue;
                }

                ++misses;
                if (misses > 5) {
                    std::cout << getCurrentTime() << "  No data - Sleeping\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds (100U));
                }
            }
        };

        auto produce = [&queue]
        {
            while (true)
            {
                int32_t value { 0 };
                while (100 > value) {
                    if (queue.add(value)) {
                        ++value;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds (900U));
            }
        };

        std::thread consumer { consume }, producer { produce };
        consumer.join();
        producer.join();
    }
}

namespace ring_buffer_polling_tcp
{
    struct ConnectorImpl
    {
        constexpr static uint32_t RECV_BUFFER_SIZE { 12 };
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

    constexpr static uint16_t port { 52525 };
    constexpr static std::string_view host {"0.0.0.0"};

    void run()
    {
        ring_buffer::static_capacity_with_commit_buffer::RingBuffer<1024> queue {};

        ConnectorImpl connector {};
        if (!connector.init()) {
            std::cerr << "Failed to init connector" << std::endl;
            return;
        }
        if (!connector.connectAndSubscribe(host, port)) {
            std::cerr << "Failed to connect and subscribe" << std::endl;
            return;
        }

        auto produce = [&queue, &connector]
        {
            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                connector.getData(*response);
                queue.commit();
            }
        };

        auto consume = [&queue]
        {
            uint64_t counter { 0 };
            buffer::Buffer* item { nullptr };
            while (true)
            {
                if ((item = queue.pop())) {
                    std::cout << ++counter << " (" << item->length() << ")" << std::endl;
                    item->clear();
                }
                else {
                    std::this_thread::sleep_for(std::chrono::microseconds (10U));
                }
            }
        };


        std::jthread producer { produce }, consumer { consume };
    }
}

namespace data_feeder_demo_ex
{
    struct ConnectorImpl
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

    /*

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
*/

    // TODO: use concepts ??
    template<typename Connector, typename Parser>
    struct DataFeederImpl
    {
        Connector& connector;
        Parser& parser;

        ring_buffer::static_capacity_with_commit_buffer::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        DataFeederImpl(Connector& connector, Parser& parser)
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
            std::cout << "Connector started" << std::endl;
            if (!setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            while (true) {
                std::cout << "Connector loop (CPU: " << getCpu() << ") : " << connector.getData() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds (1U));
            }

            /*
            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                connector.getData(*response);
                queue.commit();
            }*/
        }

        void runParser()
        {
            std::cout << "Parser started" << std::endl;
            if (!setThreadCore(2)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 2  << std::endl;
                return;
            }

            while (true) {
                std::cout << "Parser loop (CPU: " << getCpu() << ") : " << parser.parse("Market Data") << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds (1U));
            }
            /*
            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                connector.getData(*response);
                queue.commit();
            }*/
        }

    };

    struct DummyConnectorImpl
    {
        static std::string getData() {
            return "Feeder2-Data";
        }
    };

    struct DummyParserImpl
    {
        static std::string parse(const std::string& str) {
            return "{" + str + "}";
        }
    };


    void run()
    {
        DummyConnectorImpl connector {};
        DummyParserImpl parser {};

        DataFeederImpl feeder {connector, parser};
        feeder.run();
    }
}



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // data_feeder_demo::test();
    // tcp_connector_test::test();
    // lock_free_queue_polling::run();

    // ring_buffer_polling_demo::run();
    // ring_buffer_polling_tcp::run();

    data_feeder_demo_ex::run();

    return EXIT_SUCCESS;
}
