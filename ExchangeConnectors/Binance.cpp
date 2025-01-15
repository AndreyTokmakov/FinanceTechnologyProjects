/**============================================================================
Name        : Binance.cpp
Created on  : 15.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance.cpp
============================================================================**/

#include "Binance.h"

#include <iostream>
#include <string_view>
#include <utility>
#include <vector>
#include <thread>
#include <fstream>
#include <format>
#include <print>
#include <chrono>
#include <typeindex>

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

namespace
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace http = beast::http;
    namespace ssl = boost::asio::ssl;
    using tcp = boost::asio::ip::tcp;

}


namespace BinanceConnector
{
    const std::string host { "testnet.binance.vision" };
    constexpr uint16_t port { 443 };

    void test()
    {
        net::io_context ioCtx;
        ssl::context sslCtx { ssl::context::tlsv13_client };

        // This holds the root certificate used for verification load_root_certificates(sslCtx);
        tcp::resolver resolver { ioCtx };
        websocket::stream<ssl::stream<tcp::socket>> wsStream { ioCtx, sslCtx };

        const tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));
        const asio::ip::basic_endpoint<tcp> endpoint = net::connect(beast::get_lowest_layer(wsStream), results);
        std::cout << "endpoint: " << endpoint.address() << std::endl;


        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
                                                        net::error::get_ssl_category()), "Failed to set SNI Hostname");

        // Perform the SSL handshake
        wsStream.next_layer().handshake(ssl::stream_base::client);

        // Set a decorator to change the User-Agent of the handshake
        wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
            req.set(http::field::user_agent,std::string(BOOST_BEAST_VERSION_STRING) +" websocket-client-coro");
        }));

        wsStream.handshake(host, "/stream");
        constexpr std::string_view subscription { R"({"method": "SUBSCRIBE","params": ["btcusdt@ticker"], "id": 1})" };

        const size_t bytesSend = wsStream.write(net::buffer(subscription));
        std::cout << "Bytes send: " << bytesSend << std::endl;

        beast::flat_buffer buffer;
        while (true)
        {
            size_t bytesRead = wsStream.read(buffer);
            std::cout << "Bytes read: " << bytesRead << std::endl;
            std::cout << beast::make_printable(buffer.data()) << std::endl;
            buffer.clear();
        }

        wsStream.close(websocket::close_code::normal);
    }
}

void Binance::TestAll()
{
    BinanceConnector::test();
}