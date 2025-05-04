/**============================================================================
Name        : WebSockerConnectorAsynch.cpp
Created on  : 04.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : WebSockerConnectorAsynch.cpp
============================================================================**/

#include "WebSockerConnectorAsynch.h"

#include <iostream>
#include <string_view>
#include <fstream>
#include <memory>
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

    [[maybe_unused]]
    void fail(const beast::error_code& errorCode,
              std::string_view message)
    {
        std::cerr << message << ": " << errorCode.message() << "\n";
    }
}

namespace
{
    class Session : public std::enable_shared_from_this<Session>
    {
        tcp::resolver resolver;
        websocket::stream<ssl::stream<beast::tcp_stream>> wsStream;
        beast::flat_buffer buffer;
        std::string host;
        std::string message;

    public:

        explicit Session(asio::io_context& ioc, ssl::context& ctx)
                : resolver { asio::make_strand(ioc) }, wsStream { asio::make_strand(ioc), ctx } {
        }

        void run(const std::string_view _host,
                 const int port,
                 const std::string_view text)
        {
            this->host.assign(_host);
            this->message.assign(text);

            resolver.async_resolve(host, std::to_string(port),
                beast::bind_front_handler(&Session::on_resolve, shared_from_this()));
        }

        void on_resolve(const beast::error_code& errorCode,
                        const tcp::resolver::results_type& results)
        {
            if (errorCode)
                return fail(errorCode, "resolve");

            beast::get_lowest_layer(wsStream).expires_after(std::chrono::seconds(30u));
            beast::get_lowest_layer(wsStream).async_connect(results,
                    beast::bind_front_handler(&Session::on_connect, shared_from_this()));
        }

        void on_connect(const beast::error_code& errorCode,
                        const tcp::resolver::results_type::endpoint_type& ep)
        {
            if (errorCode)
                return fail(errorCode, "connect");

            // Set a timeout on the operation
            beast::get_lowest_layer(wsStream).expires_after(std::chrono::seconds(30u));
            if (! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.c_str())) {
                const beast::error_code err = beast::error_code(static_cast<int>(::ERR_get_error()),
                                                                asio::error::get_ssl_category());
                return fail(err, "connect");
            }

            // Update the host_ string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake. See https://tools.ietf.org/html/rfc7230#section-5.4
            host += ':' + std::to_string(ep.port());

            wsStream.next_layer().async_handshake(ssl::stream_base::client,
                beast::bind_front_handler(&Session::on_ssl_handshake, shared_from_this()));
        }

        void on_ssl_handshake(const beast::error_code& errorCode)
        {
            if (errorCode)
                return fail(errorCode, "ssl_handshake");

            beast::get_lowest_layer(wsStream).expires_never();

            wsStream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async-ssl");
            }));

            wsStream.async_handshake(host, "/stream",
                beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
        }

        void on_handshake(const beast::error_code& errorCode)
        {
            if (errorCode)
                return fail(errorCode, "handshake");

            wsStream.async_write(asio::buffer(message),
                beast::bind_front_handler(&Session::on_write, shared_from_this()));
        }

        void on_write(const beast::error_code& errorCode,
                      std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (errorCode)
                return fail(errorCode, "write");

            wsStream.async_read(buffer, beast::bind_front_handler(&Session::on_read, shared_from_this()));
        }

        void on_read(const beast::error_code& errorCode,
                     std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (errorCode)
                return fail(errorCode, "read");

            std::cout << beast::make_printable(buffer.data()) << std::endl;
            buffer.clear();

            // Close the WebSocket connection
            // wsStream.async_close(websocket::close_code::normal, beast::bind_front_handler(&Session::on_close, shared_from_this()));
        }

        void enable_async_read()
        {
            wsStream.async_read(buffer,beast::bind_front_handler(&Session::on_read, shared_from_this()));
        }

        bool is_socket_open() {
            return wsStream.is_open();
        }

        void on_close(const beast::error_code& errorCode)
        {
            if (errorCode)
                return fail(errorCode, "close");
            std::cout << beast::make_printable(buffer.data()) << std::endl;
        }
    };
}


void WebSockerConnectorAsynch::TestAll()
{
    constexpr std::string_view host { "testnet.binance.vision" };
    constexpr uint16_t port { 443 };
    const std::string message { R"({"method": "SUBSCRIBE","params": ["btcusdt@ticker"], "id": 1})" };

    asio::io_context ioCtx;
    ssl::context ctx{ssl::context::tlsv13_client};
    std::shared_ptr<Session> session { std::make_shared<Session>(ioCtx, ctx)};

    session->run(host, port, message);
    ioCtx.run();

    while (session->is_socket_open()) {
        ioCtx.restart();
        session->enable_async_read();
        ioCtx.run();
    }
}