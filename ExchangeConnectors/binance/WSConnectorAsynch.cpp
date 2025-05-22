/**============================================================================
Name        : WSConnectorAsynch.cpp
Created on  : 04.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : WSConnectorAsynch.cpp
============================================================================**/

#include "WSConnectorAsynch.h"

#include <iostream>
#include <string_view>
#include <fstream>
#include <memory>
#include <format>
#include <source_location>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/stream_file.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace http = beast::http;
    namespace ssl = boost::asio::ssl;
    using tcp = boost::asio::ip::tcp;

    void error(const beast::error_code& errorCode,
               const std::source_location location = std::source_location::current())
    {
        std::cout << std::format("DEBUG [{}.{}] {} {}\n",
                                 location.function_name(), location.line(), errorCode.what(), errorCode.value());
    }

    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
}

namespace WSConnectorAsynch
{
    class Session : public std::enable_shared_from_this<Session>
    {
        static inline constexpr uint32_t bufferSize { 1024 };
        static inline constexpr std::string_view path { "/stream" };

        // FIXME: Move out
        tcp::resolver resolver;

        websocket::stream<ssl::stream<beast::tcp_stream>> wsStream;
        int_fast32_t readNum = 0;
        int_fast32_t buffWriteId = 0;
        std::vector<boost::beast::flat_buffer> bufferPool {};
        std::string host;
        std::string message;

    public:

        explicit Session(asio::io_context& ioc, ssl::context& ctx)
                : resolver { asio::make_strand(ioc) },
                  wsStream { asio::make_strand(ioc), ctx },
                  bufferPool (bufferSize ){
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
                return error(errorCode);

            beast::get_lowest_layer(wsStream).expires_after(std::chrono::seconds(30u));
            beast::get_lowest_layer(wsStream).async_connect(results,
                                                            beast::bind_front_handler(&Session::on_connect, shared_from_this()));
        }

        void on_connect(const beast::error_code& errorCode,
                        const tcp::resolver::results_type::endpoint_type& ep)
        {
            if (errorCode)
                return error(errorCode);

            // Set a timeout on the operation
            beast::get_lowest_layer(wsStream).expires_after(std::chrono::seconds(30u));
            if (! SSL_set_tlsext_host_name(wsStream.next_layer().native_handle(), host.c_str())) {
                const beast::error_code err = beast::error_code(static_cast<int>(::ERR_get_error()),
                                                                asio::error::get_ssl_category());
                return error(err);
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
                return error(errorCode);

            beast::get_lowest_layer(wsStream).expires_never();

            wsStream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            wsStream.set_option(websocket::stream_base::decorator([](websocket::request_type& req){
                req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async-ssl");
            }));

            wsStream.async_handshake(host, path,
                                     beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
        }

        void on_handshake(const beast::error_code& errorCode)
        {
            if (errorCode)
                return error(errorCode);

            wsStream.async_write(asio::buffer(message),
                                 beast::bind_front_handler(&Session::on_write, shared_from_this()));
        }

        void readAsync()
        {
            ++readNum;
            buffWriteId = fast_modulo(readNum, bufferSize);
            wsStream.async_read(bufferPool[buffWriteId],
                                beast::bind_front_handler(&Session::on_read, shared_from_this()));
        }

        void on_write(const beast::error_code& errorCode,
                      std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (errorCode)
                return error(errorCode);
            readAsync();
        }

        void on_read(const beast::error_code& errorCode,
                     std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if (errorCode)
                return error(errorCode);

            // FIXME: Remove
            if (buffWriteId != fast_modulo(readNum, bufferSize)) {
                std::cerr << "ERROR\n";
            }

            beast::flat_buffer& buffer = bufferPool[buffWriteId];
            std::cout << beast::make_printable(buffer.data()) << std::endl;
            buffer.clear();

            // Close the WebSocket connection
            // wsStream.async_close(websocket::close_code::normal, beast::bind_front_handler(&Session::on_close, shared_from_this()));
        }

        [[nodiscard]]
        bool isOpen() const {
            return wsStream.is_open();
        }

        void on_close(const beast::error_code& errorCode)
        {
            if (errorCode)
                return error(errorCode);
        }
    };
}


void WSConnectorAsynch::TestAll()
{
    constexpr std::string_view host { "testnet.binance.vision" };
    constexpr uint16_t port { 443 };
    const std::vector<std::string> params {
        R"({"method": "SUBSCRIBE","params": ["btcusdt@ticker", "btcusdt@trade", "btcusdt@depth"], "id": 1})"
        // R"({"method": "SUBSCRIBE","params": ["ethusdt@ticker", "ethusdt@trade", "ethusdt@depth"], "id": 2})",
        // R"({"method": "SUBSCRIBE","params": ["bnbusdt@ticker", "bnbusdt@trade", "bnbusdt@depth"], "id": 3})",
        // R"({"method": "SUBSCRIBE","params": ["xrpusdt@ticker", "xrpusdt@trade", "xrpusdt@depth"], "id": 4})",
        // R"({"method": "SUBSCRIBE","params": ["ltcusdt@ticker", "ltcusdt@trade", "ltcusdt@depth"], "id": 5})",
        // R"({"method": "SUBSCRIBE","params": ["pepeusdt@ticker", "pepeusdt@trade", "pepeusdt@depth"], "id": 5})"
    };

    asio::io_context ioCtx(1);
    ssl::context ctx{ssl::context::tlsv13_client};
    std::vector<std::shared_ptr<Session>> sessions;
    for (const auto& param: params) {
        sessions.emplace_back( std::make_shared<Session>(ioCtx, ctx))->run(host, port, param);
    }

    ioCtx.run();
    int counter = 0;
    while (true)
    {
        counter = 0;
        ioCtx.restart();
        for (const auto& session: sessions) {
            if (session->isOpen()) {
                session->readAsync();
                ++counter;
            }
        }
        if (0 == counter)
            break;
        ioCtx.run();
    }
}