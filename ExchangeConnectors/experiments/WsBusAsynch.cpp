/**============================================================================
Name        : WsBusAsynch.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include "Experiments.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

namespace
{
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    using json = nlohmann::json;
}

#if 0
namespace
{
    struct EventBus
    {
        using Handler = std::function<void(const json&)>;

        void subscribe(const std::string& stream, Handler handler) {
            handlers_[stream].push_back(std::move(handler));
        }

        void publish(const std::string& stream, const json& data) const {
            auto it = handlers_.find(stream);
            if (it != handlers_.end()) {
                for (auto& h : it->second)
                    h(data);
            }
        }

    private:
        std::unordered_map<std::string, std::vector<Handler>> handlers_;
    };


    class BinanceConnector : public std::enable_shared_from_this<BinanceConnector> {
    public:
        BinanceConnector(net::io_context& ioc,
                         net::ssl::context& ssl_ctx,
                         std::string combined_streams_url,
                         EventBus& bus)
            : resolver_(net::make_strand(ioc)),
              ws_(net::make_strand(ioc), ssl_ctx),
              bus_(bus),
              host_("stream.binance.com"),
              port_("9443"),
              target_(std::move(combined_streams_url))
        {}

        void run() {
            resolver_.async_resolve(host_, port_,
                beast::bind_front_handler(&BinanceConnector::on_resolve, shared_from_this()));
        }

    private:
        tcp::resolver resolver_;
        websocket::stream<beast::ssl_stream<tcp::socket>> ws_;
        beast::flat_buffer buffer_;
        EventBus& bus_;
        std::string host_, port_, target_;

        void on_resolve(const beast::error_code& errorCode,
                        const tcp::resolver::results_type& results)
        {
            if (errorCode)
                return fail(errorCode, "resolve");


            net::async_connect(ws_.next_layer().next_layer(), results.begin(), results.end(),
                beast::bind_front_handler(&BinanceConnector::on_connect, shared_from_this()));
        }

        void on_connect(beast::error_code ec, const tcp::resolver::results_type::endpoint_type&) {
            if (ec) return fail(ec, "connect");
            ws_.next_layer().async_handshake(net::ssl::stream_base::client,
                beast::bind_front_handler(&BinanceConnector::on_ssl_handshake, shared_from_this()));
        }

        void on_ssl_handshake(beast::error_code ec) {
            if (ec) return fail(ec, "ssl_handshake");
            ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            ws_.async_handshake(host_, target_,
                beast::bind_front_handler(&BinanceConnector::on_handshake, shared_from_this()));
        }

        void on_handshake(beast::error_code ec) {
            if (ec) return fail(ec, "handshake");
            std::cout << "Connected to Binance combined stream: " << target_ << "\n";
            do_read();
        }

        void do_read() {
            ws_.async_read(buffer_,
                beast::bind_front_handler(&BinanceConnector::on_read, shared_from_this()));
        }

        void on_read(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec) {
                fail(ec, "read");
                reconnect();
                return;
            }

            try {
                auto msg = beast::buffers_to_string(buffer_.data());
                buffer_.consume(buffer_.size());
                auto j = json::parse(msg);

                if (j.contains("stream") && j.contains("data")) {
                    std::string stream = j["stream"];
                    bus_.publish(stream, j["data"]);
                }
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << "\n";
            }

            do_read();
        }

        void reconnect() {
            std::cerr << "Reconnecting in 5s...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5u));
            run();
        }

        static void fail(beast::error_code ec, const char* what) {
            std::cerr << what << ": " << ec.message() << "\n";
        }
    };
}
#endif


void experiments::WsBusAsynch::TestAll()
{

}
