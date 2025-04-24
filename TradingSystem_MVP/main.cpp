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
#include <boost/container/flat_map.hpp>

#include <nlohmann/json.hpp>

#include "common/Utils.h"
#include "common/BlockingQueue.h"
#include "common/BlockingQueuePtr.h"

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
    using FlatBufferQueue = Common::BlockingQueuePtr<beast::flat_buffer>;

}

namespace
{
    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & value - 1));
    }
}

#if 0
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
                    if (queue.pop(message)) {
                        std::cout << "Got message (CPU: " << Utils::getCpu() << ") : " << message << std::endl;
                    }
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

#endif

namespace
{
    enum class Exchange
    {
        Binance,
        ByBit,
        Deribit,
        GateIO,
        OKX,
    };
}

namespace Engine
{
    using namespace std::string_literals;

    // using Ticker = std::string;
    using Pair = std::string;


    struct OrderBook
    {
        Pair pair;

        // TODO: Rename method
        void processEvent(beast::flat_buffer* message) const {
            std::cout << "Processing event: " << nlohmann::json::parse(beast::buffers_to_string(message->data())) << std::endl;
        }
    };

    // TODO: Rename
    struct ExchangeBookKeeper
    {
        boost::container::flat_map<Pair, std::unique_ptr<OrderBook>> orderBooksByTicker;
        FlatBufferQueue queue;
        std::jthread worker;

        explicit ExchangeBookKeeper()
        {
            for (const auto& ticker: { "APPL"s, "TEST"s }){
                orderBooksByTicker.emplace(ticker, std::make_unique<OrderBook>(ticker));
            }
        }

        // TODO: Rename??
        void push(beast::flat_buffer* buffer)
        {
            queue.push(buffer);
        }

        void start(const uint32_t cpuId)
        {
            beast::flat_buffer* message;
            worker = std::jthread{ [this, &message, cpuId]
            {
                const auto threadId { std::this_thread::get_id() };
                if (!Utils::setThreadCore(cpuId)) {
                    std::cerr << "Failed to set core " << cpuId << " for " << threadId << std::endl;
                    return;
                } else {
                    std::cout << "Starting ExchangeBookKeeper on CPU: " << Utils::getCpu() << std::endl;
                }

                while (true) {
                    if (queue.pop(message))
                    {
                        Pair pair = "APPL";  // TODO: Get TICKER/PAIR here ??
                        const auto& book = orderBooksByTicker[pair];
                        // TODO: Rename method
                        book->processEvent(message);
                    }
                }
            }};
            worker.detach();
        }
    };


    struct PricingEngine
    {
        ExchangeBookKeeper binanceBookKeeper;
        ExchangeBookKeeper byBitBookKeeper;
        std::array<ExchangeBookKeeper*, 2> books { &binanceBookKeeper, &byBitBookKeeper };

        void start()
        {
            for (uint32_t cpuId = 2; ExchangeBookKeeper* bookKeeper: books)
                bookKeeper->start(cpuId++);
        }

        void push(Exchange exchange, beast::flat_buffer* buffer)
        {
            // std::cout << "push (CPU: " << Utils::getCpu() << ") : " << jsonMessage << std::endl;
            books[static_cast<uint32_t>(exchange)]->push(buffer);
        }
    };
}


namespace Connectors
{
    struct BinanceWsConnector
    {
        static inline constexpr std::string_view host { "testnet.binance.vision" };
        static inline constexpr uint16_t port { 443 };

        Engine::PricingEngine& pricingEngine;
        std::jthread worker;
        uint32_t coreId = 0;

        uint32_t counter = 0;
        std::vector<beast::flat_buffer> buffers {};

        explicit BinanceWsConnector(Engine::PricingEngine& engine, const uint32_t cpuId):
                pricingEngine { engine }, coreId { cpuId }, buffers (1024) {
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

                while (true)
                {   // TODO: Rename push() ??
                    //  - Push only std::string to the Queue?
                    //  - beast::buffers_to_string(buffer.data()) ---> BAD: Create a copy
                    //  - Как то можно избежать копирований ???
                    //  - Memory / Object Pool ???? (For MarkerData Events)

                    beast::flat_buffer& buffer = buffers[fast_modulo(counter, 1024)];
                    buffer.clear();
                    bytesRead = wsStream.read(buffer);
                    pricingEngine.push(Exchange::Binance, &buffer);
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


// TODO: Optimisations:
//  -  Pair [usdtbtc] --> uint64_t  [так как длина 'usdtbtc' максимум 8 байт]
//     Возможно использовать SIMD для корвертации в INT

// TODO: Next steps
//  1. Доделать мультиплексер
//  2. Убрать копирования прочитанных сообщений в очереди
//     - каждый сервер читает сообщения в свой кольцевой буффер
//     - после этого push-ит УКАЗАТЕЛЬ на даннный элемент буффера в соотвествующий кольцевой буффер BookKeeper-а?





int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);


    Engine::PricingEngine pricingEngine;
    Connectors::BinanceWsConnector binanceWsConnectorBtc { pricingEngine, 1 };
    Connectors::BinanceWsConnector binanceWsConnectorEth { pricingEngine, 1 };
    binanceWsConnectorBtc.start("btcusdt");
    binanceWsConnectorEth.start("ethusdt");
    pricingEngine.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100U));
    }

    return EXIT_SUCCESS;
}
