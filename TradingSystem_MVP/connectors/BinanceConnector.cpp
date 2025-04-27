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
        pricingEngine { engine }, coreId { cpuId }, buffers (1024) {
    }

    // TODO: Re-structure code -- Coroutines ??
    void BinanceWsConnector::start(const std::string& pair)
    {
        //const std::string subscription { R"({"method": "SUBSCRIBE","params": [")" + pair + R"(@ticker"], "id": 1})" };
        const std::string subscription { R"({"method": "SUBSCRIBE","params": [
            "ethusdt@ticker@ticker", "btcusdt@ticker", ,"memeusdt@ticker"
        ], "id": 1})" };

        worker = std::jthread{[this, subscription]
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

            // Perform the SSL handshake
            wsStream.next_layer().handshake(ssl::stream_base::client);

            // Set a decorator to change the User-Agent of the handshake
            wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent,std::string(BOOST_BEAST_VERSION_STRING) +" websocket-client-coro");
            }));

            wsStream.handshake(host, "/stream");;

            [[maybe_unused]]
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
                ++counter;
                bytesRead = wsStream.read(buffer);
                pricingEngine.push(Exchange::Binance, &buffer);
            }
        }};
        worker.detach();
    }
}