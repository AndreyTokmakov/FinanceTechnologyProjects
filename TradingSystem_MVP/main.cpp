/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <iostream>
#include <string_view>
#include <vector>
#include <thread>
#include <format>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/basic_channel.hpp>
#include <boost/asio/experimental/basic_concurrent_channel.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/variant2/variant.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include "boost/url/scheme.hpp"

#include <nlohmann/json.hpp>

#include "common/Utils.h"
#include "common/BlockingQueue.h"

namespace
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace http = beast::http;
    namespace ssl = boost::asio::ssl;
    using tcp = boost::asio::ip::tcp;

    using MessagesQueue = Common::BlockingQueue<std::string>;
    using JsonMessagesQueue = Common::BlockingQueue<nlohmann::json>;

}

namespace SimpleDemo
{
    struct WsServer
    {
        MessagesQueue& queue;
        std::jthread worker;

        std::string name {};
        int counter = 0;
        uint32_t coreId = 0;

        explicit WsServer(MessagesQueue& q, std::string n, uint32_t cpuId):
                queue {q}, name { std::move(n) }, coreId { cpuId } {
        }

        void start()
        {
            const auto threadId { std::this_thread::get_id() };
            worker = std::jthread{ [this, threadId]{
                if (!Utils::setThreadCore(coreId)) {
                    std::cerr << "Failed to set core " << coreId << " for " << threadId << std::endl;
                    return;
                }

                while (true) {
                    queue.push(std::format("{} (CPU: {}): Message_{}",
                    name, Utils::getCpu(), ++counter));
                    std::this_thread::sleep_for(std::chrono::milliseconds (250U));
                }
            }};
            worker.detach();
        }

        ~WsServer() {
            std::cout << "WsServer" << std::endl;
        }
    };

    struct BookBuilder
    {
        MessagesQueue& queue;

        void start()
        {
            std::string message;
            std::jthread worker { [this, &message] {
                while (true) {
                    queue.pop(message);
                    std::cout << "Got message (CPU: " << Utils::getCpu() << ") : " << message << std::endl;
                }
            }};
        }
    };


    void runDemo()
    {
        MessagesQueue queue;

        BookBuilder builder {queue};
        WsServer server1 { queue, "Server-1" , 1};
        WsServer server2 { queue, "Server-2" , 2};

        server1.start();
        server2.start();

        builder.start();
    }
}

namespace Processing
{
    struct BookKeeper
    {
        JsonMessagesQueue queue;

        void start()
        {
            nlohmann::json message;
            std::jthread worker { [this, &message]
            {
                while (true)
                {
                    queue.pop(message);
                    std::cout << "Got message (CPU: " << Utils::getCpu() << ") : " << message << std::endl;
                }
            }};
        }

        // TODO: Add --MULTIPLEXER-- logic here
        //   ---->  Table [ Exchange-Symbol, RingBuffer && List[OrderBook] && Processor]
        void push(nlohmann::json&& jsonMessage)
        {
            // std::cout << "push (CPU: " << Utils::getCpu() << ") : " << jsonMessage << std::endl;
            queue.push(std::move(jsonMessage));
        }
    };
}


namespace Connectors
{
    struct BinanceWsConnector
    {
        static inline constexpr std::string_view host { "testnet.binance.vision" };
        static inline constexpr uint16_t port { 443 };

        Processing::BookKeeper& bookKeeper;
        std::jthread worker;
        uint32_t coreId = 0;

        explicit BinanceWsConnector(Processing::BookKeeper& keeper,
            const uint32_t cpuId): bookKeeper { keeper }, coreId { cpuId } {
        }

        // TODO: Re-structure code -- Coroutines ??
        void start(const std::string& pair)
        {
            const std::string subscription { R"({"method": "SUBSCRIBE","params": [")" + pair + R"(@ticker"], "id": 1})" };

            worker = std::jthread{ [this, subscription]
            {
                const auto threadId { std::this_thread::get_id() };
                if (!Utils::setThreadCore(coreId)) {
                    std::cerr << "Failed to set core " << coreId << " for " << threadId << std::endl;
                    return;
                }

                net::io_context ioCtx;
                ssl::context sslCtx { ssl::context::tlsv13_client };

                tcp::resolver resolver { ioCtx };
                const tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));
                websocket::stream<ssl::stream<tcp::socket>> wsStream { ioCtx, sslCtx };

                const asio::ip::basic_endpoint<tcp> endpoint = net::connect(beast::get_lowest_layer(wsStream), results);

                // Set SNI Hostname (many hosts need this to handshake successfully)
                if(! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.data()))
                    throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
                                                net::error::get_ssl_category()), "Failed to set SNI Hostname");
                // Perform the SSL handshake
                wsStream.next_layer().handshake(ssl::stream_base::client);

                // Set a decorator to change the User-Agent of the handshake
                wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                    req.set(http::field::user_agent,std::string(BOOST_BEAST_VERSION_STRING) +" websocket-client-coro");
                }));

                wsStream.handshake(host, "/stream");;

                const size_t bytesSend = wsStream.write(net::buffer(subscription));
                size_t bytesRead = 0;
                beast::flat_buffer buffer;
                while (true) {
                    bytesRead = wsStream.read(buffer);
                    // TODO: Rename push() ??
                    //  - Push only std::string to the Queue?
                    //  - beast::buffers_to_string(buffer.data()) ---> BAD: Create a copy
                    //  - Как то можно избежать копирований ???
                    //  - Memory / Object Pool ???? (For MarkerData Events)
                    bookKeeper.push( nlohmann::json::parse(beast::buffers_to_string(buffer.data())));
                    buffer.clear();
                }
            }};
            worker.detach();
        }
    };
}





// TODO:       *************** MULTIPLEXER ************************
//  - Каждый сервер должен публиковать сообщения в очереь соответствующую для данной
//    Exchange или symbol
//  - Каждое такое кольцо живёт в отдельном Thread-e
//  - В этом же Thread-e живет OrderBook-и для всех Бирж что публикуются в данное кольцо


// TODO: Next steps
//  1. Доделать мультиплексер
//  2. Убрать копирования прочитанных сообщений в очереди
//     - каждый сервер читает сообщения в свой кольцевой буффер
//     - после этого push-ит УКАЗАТЕЛЬ на даннный элемент буффера в соотвествующий кольцевой буффер BookKeeper-а?


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    Processing::BookKeeper bookBuilder;
    Connectors::BinanceWsConnector binanceWsConnectorBtc { bookBuilder, 1 };
    Connectors::BinanceWsConnector binanceWsConnectorEth { bookBuilder, 1 };
    binanceWsConnectorBtc.start("btcusdt");
    binanceWsConnectorEth.start("ethusdt");
    bookBuilder.start();

    return EXIT_SUCCESS;
}
