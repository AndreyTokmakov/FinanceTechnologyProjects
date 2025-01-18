/**============================================================================
Name        : Binance.cpp
Created on  : 15.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance.cpp
============================================================================**/

#include "BinanceWssStream.h"

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

namespace Demo
{
    void fail(beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    // Sends a WebSocket message and prints the response
    class session : public std::enable_shared_from_this<session>
    {
        tcp::resolver resolver_;
        websocket::stream<ssl::stream<beast::tcp_stream>> ws_;
        beast::flat_buffer buffer;
        std::string host;
        std::string text;

    public:
        // Resolver and socket require an io_context
        explicit session(net::io_context& ioc, ssl::context& ctx):
            resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc), ctx) {
        }

        // Start the asynchronous operation
        void run(char const* host,
                 char const* port,
                 char const* text)
        {
            this->host = host;
            this->text = text;

            // Look up the domain name
            resolver_.async_resolve(host, port,
                beast::bind_front_handler(&session::on_resolve, shared_from_this()));
        }

        void on_resolve(const beast::error_code& ec,
                        const tcp::resolver::results_type& results)
        {
            if(ec)
                return fail(ec, "resolve");

            // Set a timeout on the operation
            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30u));

            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(ws_).async_connect(results,
                beast::bind_front_handler(&session::on_connect, shared_from_this()));
        }

        void on_connect(beast::error_code ec,
                        const tcp::resolver::results_type::endpoint_type& ep)
        {
            if (ec)
                return fail(ec, "connect");

            // Set a timeout on the operation
            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30u));

            // Set SNI Hostname (many hosts need this to handshake successfully)
            if(! SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str()))
            {
                ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
                return fail(ec, "connect");
            }

            // Update the host_ string. This will provide the value of the Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            host += ':' + std::to_string(ep.port());

            // Perform the SSL handshake
            ws_.next_layer().async_handshake(ssl::stream_base::client,
                beast::bind_front_handler(&session::on_ssl_handshake,shared_from_this()));
        }

        void on_ssl_handshake(const beast::error_code& ec)
        {
            if(ec)
                return fail(ec, "ssl_handshake");

            // Turn off the timeout on the tcp_stream, because the websocket stream has its own timeout system.
            beast::get_lowest_layer(ws_).expires_never();

            // Set suggested timeout settings for the websocket
            ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

            // Set a decorator to change the User-Agent of the handshake
            ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async-ssl");
            }));

            // Perform the websocket handshake
            ws_.async_handshake(host, "/",
                                beast::bind_front_handler(&session::on_handshake,shared_from_this()));
        }

        void on_handshake(const beast::error_code& ec)
        {
            if (ec)
                return fail(ec, "handshake");

            // Send the message
            ws_.async_write(net::buffer(text),
                            beast::bind_front_handler(&session::on_write, shared_from_this()));
        }

        void on_write(const beast::error_code& ec,
                      std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if(ec)
                return fail(ec, "write");

            // Read a message into our buffer
            ws_.async_read(buffer, beast::bind_front_handler(&session::on_read, shared_from_this()));
        }

        void on_read(const beast::error_code& ec,
                     std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (ec)
                return fail(ec, "read");

            // Close the WebSocket connection
            ws_.async_close(websocket::close_code::normal,
                beast::bind_front_handler(&session::on_close, shared_from_this()));
        }

        void on_close(const beast::error_code& ec)
        {
            if(ec)
                return fail(ec, "close");

            // If we get here then the connection is closed gracefully
            // The make_printable() function helps print a ConstBufferSequence
            std::cout << beast::make_printable(buffer.data()) << std::endl;
        }
    };

    void run()
    {
        const std::string host { "testnet.binance.vision" };
        constexpr uint16_t port { 443 };

        net::io_context ioCtx;
        ssl::context sslCtx { ssl::context::tlsv13_client };

        // std::make_shared<session>(ioCtx, sslCtx)->run(host, port, text);
    }
}


namespace BinanceWssStream
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
        // std::cout << "Bytes send: " << bytesSend << std::endl;

        beast::flat_buffer buffer;
        while (true)
        {
            size_t bytesRead = wsStream.read(buffer);
            // std::cout << "Bytes read: " << bytesRead << std::endl;
            // std::cout << beast::make_printable(buffer.data()) << std::endl;

            nlohmann::json data = nlohmann::json::parse(beast::buffers_to_string(buffer.data()));
            std::cout << data << std::endl;

            buffer.clear();
        }

        wsStream.close(websocket::close_code::normal);
    }
}

void BinanceWssStream::TestAll()
{
    BinanceWssStream::test();
}