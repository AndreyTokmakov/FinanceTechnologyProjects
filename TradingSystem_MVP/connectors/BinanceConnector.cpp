/**============================================================================
Name        : BinanceConnector.cpp
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BinanceConnector.cpp
============================================================================**/

#include <iostream>

#include "BinanceConnector.h"
#include "Utilities.h"
#include "Exchange.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/system/error_code.hpp>


namespace
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace http = beast::http;
    namespace ssl = boost::asio::ssl;
    using tcp = boost::asio::ip::tcp;

    using utilities::fast_modulo;
    using common::Exchange;
}

namespace connectors
{
    BinanceWsConnector::BinanceWsConnector(engine::PricingEngine& engine, const uint32_t cpuId):
        pricingEngine { engine }, coreId { cpuId }, buffers (bufferSize ) {
    }

    // TODO: Re-structure code -- Coroutines ??
    void BinanceWsConnector::start(const std::string& pair)
    {
        //const std::string subscription { R"({"method": "SUBSCRIBE","params": [")" + pair + R"(@ticker"], "id": 1})" };

        const std::string subscriptionTicker { R"({"method": "SUBSCRIBE","params": [
            "ethusdt@ticker@ticker", "btcusdt@ticker", "memeusdt@ticker"
        ], "id": 1})" };

        const std::string subscriptionDepth {
            R"({"method": "SUBSCRIBE","params": ["btcusdt@depth", "memeusdt@depth"], "id": 1})"
        };


        worker = std::jthread{[this, subscriptionDepth]
        {
            const auto threadId { std::this_thread::get_id() };
            if (!utilities::setThreadCore(coreId)) {
                std::cerr << "Failed to set core " << coreId << " for " << threadId << std::endl;
                return;
            }

            net::io_context ioCtx;
            ssl::context sslCtx { ssl::context::tlsv13_client };

            tcp::resolver resolver { ioCtx };
            const tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));
            websocket::stream<ssl::stream<tcp::socket>> wsStream { ioCtx, sslCtx };

            [[maybe_unused]]
            const asio::ip::basic_endpoint<tcp> endpoint = net::connect(beast::get_lowest_layer(wsStream), results);

            // Set SNI Hostname (many hosts need this to handshake successfully)
            if(! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.data()))
                throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()), "Failed to set SNI Hostname");

            std::cout << "---1---\n";

            // Perform the SSL handshake
            wsStream.next_layer().handshake(ssl::stream_base::client);

            std::cout << "---2---\n";

            // Set a decorator to change the User-Agent of the handshake
            wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent,std::string(BOOST_BEAST_VERSION_STRING) +" websocket-client-coro");
            }));

            wsStream.handshake(host, "/stream");

            std::cout << "---3---\n";

            [[maybe_unused]]
            const size_t bytesSend = wsStream.write(net::buffer(subscriptionDepth));
            size_t bytesRead = 0;
            while (true)
            {
                // TODO: Memory / Object Pool ???? (For MarkerData Events)
                try
                {
                    beast::flat_buffer& buffer = buffers[fast_modulo(counter, bufferSize)];
                    buffer.clear();
                    bytesRead = wsStream.read(buffer);
                    pricingEngine.push(Exchange::Binance, &buffer);
                    ++counter;
                }
                catch (const std::exception& exc) {
                    // FIXME: Logging ???
                    std::cerr << exc.what() << std::endl;
                }
            }
        }};
        worker.detach();
    }
}