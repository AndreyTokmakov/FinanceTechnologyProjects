/**============================================================================
Name        : WebSockerConnector.cpp
Created on  : 15.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : WebSockerConnector.cpp
============================================================================**/

#include "Binance.hpp"

#include <iostream>
#include <string_view>
#include <fstream>
#include <format>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <nlohmann/json.hpp>

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

namespace WebSockerConnector
{
    constexpr std::string_view testnet { "testnet.binance.vision" };
    constexpr std::string_view binanceReal { "stream.binance.com" };

    const std::string host { binanceReal };
    constexpr uint16_t port { 443 };

    void test()
    {
        try {
            net::io_context ioCtx;
            ssl::context sslCtx { ssl::context::tlsv13_client };

            // This holds the root certificate used for verification load_root_certificates(sslCtx);
            tcp::resolver resolver { ioCtx };
            websocket::stream<ssl::stream<tcp::socket>> wsStream { ioCtx, sslCtx };

            const tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));
            const asio::ip::basic_endpoint<tcp> endpoint = net::connect(beast::get_lowest_layer(wsStream), results);
            std::cout << "Endpoint: " << endpoint.address() << std::endl;

            // Set SNI Hostname (many hosts need this to handshake successfully)
            if(! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.c_str()))
                throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
                                                            net::error::get_ssl_category()), "Failed to set SNI Hostname");

            std::cout << "Performing the SSL handshake ... "<< std::endl;
            wsStream.next_layer().handshake(ssl::stream_base::client);

            std::cout << "Set a decorator to change the User-Agent of the handshake "<< std::endl;
            wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent,std::string(BOOST_BEAST_VERSION_STRING) +" websocket-client-coro");
            }));

            std::cout << "Performing the WS handshake ... "<< std::endl;
            wsStream.handshake(host, "/stream");
            constexpr std::string_view subscriptionTicker { R"({"method": "SUBSCRIBE","params": ["btcusdt@ticker"], "id": 1})" };
            constexpr std::string_view subscriptionDepth { R"({"method": "SUBSCRIBE","params": ["btcusdt@depth"], "id": 1})" };

            [[maybe_unused]]
            const size_t bytesSend = wsStream.write(net::buffer(subscriptionDepth));
            // std::cout << "Bytes send: " << bytesSend << std::endl;

            beast::flat_buffer buffer;
            while (true)
            {
                [[maybe_unused]]
                size_t bytesRead = wsStream.read(buffer);

                // std::cout << "Bytes read: " << bytesRead << std::endl;
                // std::cout << beast::make_printable(buffer.data()) << std::endl;

                nlohmann::json data = nlohmann::json::parse(beast::buffers_to_string(buffer.data()));
                std::cout << data << std::endl;

                buffer.clear();
            }

            wsStream.close(websocket::close_code::normal);
        }
        catch (const std::exception& exc) {
            std::cerr << exc.what() << std::endl;
        }
    }
}

void WebSockerConnector::TestAll()
{
    WebSockerConnector::test();
}