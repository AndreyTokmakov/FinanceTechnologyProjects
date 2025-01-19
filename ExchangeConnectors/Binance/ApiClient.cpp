/**============================================================================
Name        : ApiClient.cpp
Created on  : 18.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : ApiClient.cpp
============================================================================**/

#include "ApiClient.h"

#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <vector>
#include <expected>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/json.hpp>


#include "FileUtilities.h"

namespace
{
    using namespace std::string_view_literals;

    constexpr std::string_view testnetApiUrl { R"(testnet.binance.vision)"sv };
    constexpr std::string_view apiKey { "9FmOZl0CCPVkzipOv0kXMx0gaL1BSeCuUhzG0CKilr0yjS6mxf037UvqM2nhAuXf"sv };

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace asio = boost::asio;
    namespace ssl = boost::asio::ssl;
    namespace json = boost::json;
    namespace ip = boost::asio::ip;
    using tcp = asio::ip::tcp;
}

#define DEBUG_LINE  std::cout << __PRETTY_FUNCTION__ << '.' << __LINE__ << std::endl;

#if 0
namespace ApiClientOld
{
    template<typename Callback, class ObjectType>
    struct FinalAction
    {
        FinalAction(ObjectType* ptrObj, Callback callbackMethod) :
                objectPtr { ptrObj }, callback { callbackMethod }{
        }

        ~FinalAction()
        {
            std::invoke(callback, objectPtr);
        }

    private:
        ObjectType* objectPtr { nullptr };
        Callback callback {};
    };

    struct ApiClientOld
    {
        static constexpr int32_t version { 11 };
        static constexpr std::chrono::duration connectionTimeout { std::chrono::seconds(5u) };

        tcp::resolver resolver;
        ssl::stream<beast::tcp_stream> sslStream;
        tcp::resolver::results_type apiEndpoint;
        beast::flat_buffer buffer;

        explicit ApiClientOld(const asio::any_io_executor& executor,
                           ssl::context& sslContext,
                           std::string_view host):
                resolver { executor }, sslStream { executor, sslContext }
        {
            apiEndpoint = resolver.resolve(host.data(), "https");

            sslContext.set_default_verify_paths();
            sslStream.set_verify_mode(ssl::verify_none);
            sslStream.set_verify_callback([](bool, ssl::verify_context&) {
                return true; /** Accept any certificate **/
            });

            if (!SSL_set_tlsext_host_name(sslStream.native_handle(), testnetApiUrl.data())) {
                beast::error_code ec {static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category() };
                throw beast::system_error{ec};
            }
        }

        ~APIClient()
        {
            shutdown();
        }

        void shutdown()
        {
            DEBUG_LINE
            if (!sslStream.lowest_layer().is_open())
                return;
            beast::error_code errorCode;
            const boost_swap_impl::error_code result = sslStream.shutdown(errorCode);
            if (errorCode == asio::error::eof) {
                errorCode = {};
            }
            if (errorCode) {
                std::cerr << beast::system_error{errorCode}.what() << std::endl;
            }
            DEBUG_LINE
        }

        [[nodiscard]]
        bool connect()
        {
            DEBUG_LINE
            try {
                DEBUG_LINE
                get_lowest_layer(sslStream).connect(apiEndpoint);
                get_lowest_layer(sslStream).expires_after(connectionTimeout);
                sslStream.handshake(ssl::stream_base::client);
                DEBUG_LINE
                return true;
            }
            catch (const std::exception& exc) {
                DEBUG_LINE
                std::cerr << exc.what() << std::endl;
                return false;
            }
        }

        void closeConnection()
        {
            DEBUG_LINE
            sslStream.lowest_layer().close();
            //sslStream.shutdown();
            DEBUG_LINE
        }

        std::expected<std::string, std::string> sendRequest()
        {
            try
            {
                if (!connect())
                    return std::unexpected("Failed to connect");

                FinalAction closeConnection {this, &APIClient::closeConnection };

                DEBUG_LINE
                http::request<http::empty_body> request{http::verb::get, "/api/v3/exchangeInfo", version};
                request.set(http::field::host, testnetApiUrl.data());
                request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                request.set("X-MBX-APIKEY", apiKey.data());

                DEBUG_LINE
                http::write(sslStream, request);

                http::response<http::dynamic_body> response;
                buffer.clear();

                DEBUG_LINE
                http::read(sslStream, buffer, response);

                DEBUG_LINE

                return buffers_to_string(response.body().data());
            }
            catch (const std::exception& exc)
            {
                DEBUG_LINE
                std::cerr << exc.what() << std::endl;
                return std::unexpected(exc.what());
            }
        }
    };

    void test()
    {
        asio::io_context ioCtx;
        ssl::context sslContext { ssl::context::tlsv13_client };

        APIClient client (asio::make_strand(ioCtx), sslContext, testnetApiUrl);

        for (int i = 0; i < 2; ++i)
        {
            const std::expected<std::string, std::string > response = client.sendRequest();
            if (response) {
                std::cout << response.value().size() << std::endl;
            } else {
                std::cout << response.error() << std::endl;
            }
        }
    }
}
#endif

namespace tests
{
    struct StreamGuard
    {
        beast::ssl_stream<beast::tcp_stream>& stream;

        explicit StreamGuard(beast::ssl_stream<beast::tcp_stream>& sslStream): stream { sslStream } {
        }

        ~StreamGuard()
        {
            beast::error_code errorCode;
            try
            {
                stream.shutdown(errorCode);
                if (errorCode == asio::error::eof) {
                    errorCode = {};
                }
                if (errorCode) {
                    std::cerr << errorCode.what() << std::endl;
                }
            }
            catch (const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    };

    std::expected<std::string, std::string>
    exec_get_request(asio::io_context& executor,
                     const tcp::resolver::results_type& endpoint,
                     std::string_view path)
    {
        try
        {
            ssl::context sslContext { ssl::context::tlsv13_client };
            sslContext.set_default_verify_paths();

            beast::ssl_stream<beast::tcp_stream> sslStream {executor, sslContext };
            sslStream.set_verify_mode(ssl::verify_none);
            sslStream.set_verify_callback([](bool, ssl::verify_context &) {
                return true; /** Accept any certificate **/
            });

            const StreamGuard guard { sslStream };
            if (!SSL_set_tlsext_host_name(sslStream.native_handle(), endpoint->host_name().data())) {
                beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
                throw beast::system_error{ec};
            }

            get_lowest_layer(sslStream).connect(endpoint);
            get_lowest_layer(sslStream).expires_after(std::chrono::seconds(30u));

            http::request<http::empty_body> request {http::verb::get, path, 11 };
            request.set(http::field::host, endpoint->host_name());
            request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            request.set("X-MBX-APIKEY", apiKey.data());

            // Send the request
            sslStream.handshake(ssl::stream_base::client);
            http::write(sslStream, request);

            // Receive the response
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(sslStream, buffer, res);

            return buffers_to_string(res.body().data());
        }
        catch (const std::exception& exc) {
            return std::unexpected { exc.what() };
        }
    }

    void exchangeInfo()
    {
        asio::io_context ioCtx;
        tcp::resolver resolver { ioCtx };
        const tcp::resolver::results_type apiEndpoint = resolver.resolve(testnetApiUrl.data(), "https");

        for (int i = 0; i < 5; ++i)
        {
            const std::expected<std::string, std::string> response = exec_get_request(
                    ioCtx, apiEndpoint, "/api/v3/exchangeInfo"sv);
            if (response) {
                std::cout << response.value().size() << std::endl;
            } else {
                std::cout << response.error() << std::endl;
            }
        }
    }
}

namespace ApiClient
{
    struct StreamGuard
    {
        beast::ssl_stream<beast::tcp_stream>& stream;

        explicit StreamGuard(beast::ssl_stream<beast::tcp_stream>& sslStream): stream { sslStream } {
        }

        ~StreamGuard()
        {
            try {
                beast::error_code errorCode;
                stream.shutdown(errorCode);
                if (errorCode == asio::error::eof) {
                    errorCode = {};
                }
                if (errorCode) {
                    std::cerr << errorCode.what() << std::endl;
                }
            }
            catch (const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
            }
        }
    };


    struct APIClient
    {
        static constexpr int32_t version { 11 };
        static constexpr std::chrono::duration connectionTimeout { std::chrono::seconds(5u) };
        static constexpr std::string_view API_KEY_HEADER { "X-MBX-APIKEY" };

        std::string hostname;

        asio::io_context ioCtx;
        tcp::resolver resolver;
        ssl::context sslContext;
        tcp::resolver::results_type endpoint;

        explicit APIClient(const std::string_view host):
                resolver { ioCtx }, sslContext { ssl::context::tlsv13_client }
        {
            endpoint = resolver.resolve(host.data(), "https");
            sslContext.set_default_verify_paths();
        }

        std::expected<std::string, std::string> getRequest(const std::string_view path)
        {
            try
            {
                beast::ssl_stream<beast::tcp_stream> sslStream { ioCtx, sslContext };
                sslStream.set_verify_mode(ssl::verify_none);
                sslStream.set_verify_callback([](bool, ssl::verify_context &) {
                    return true; /** Accept any certificate **/
                });

                const StreamGuard guard { sslStream };
                if (!SSL_set_tlsext_host_name(sslStream.native_handle(), endpoint->host_name().data())) {
                    beast::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
                    throw beast::system_error{ec};
                }

                get_lowest_layer(sslStream).connect(endpoint);
                get_lowest_layer(sslStream).expires_after(connectionTimeout);
                sslStream.handshake(ssl::stream_base::client);

                http::request<http::empty_body> request {http::verb::get, path, version };
                request.set(http::field::host, endpoint->host_name());
                request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                request.set(API_KEY_HEADER, apiKey.data());

                // Send the request
                http::write(sslStream, request);

                // Receive the response
                beast::flat_buffer buffer;
                http::response<http::dynamic_body> response;
                http::read(sslStream, buffer, response);

                return buffers_to_string(response.body().data());
            }
            catch (const std::exception& exc) {
                return std::unexpected { exc.what() };
            }
        }
    };

    void apiTests()
    {
        APIClient client { testnetApiUrl };
        auto callEndpoint = [&client](const std::string_view path)
        {
            const std::expected<std::string, std::string> response = client.getRequest(path);
            if (response) {
                std::cout << response.value() << std::endl;
            } else {
                std::cout << response.error() << std::endl;
            }
        };

        // callEndpoint("/api/v3/exchangeInfo"sv);
        callEndpoint("/api/v3/ping"sv);
        callEndpoint("/api/v3/time"sv);
    }
}

void ApiClient::TestAll()
{
    // tests::exchangeInfo();

    // ApiClient::test();
    ApiClient::apiTests();
}