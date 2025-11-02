/**============================================================================
Name        : DemoOne.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>

#if 0
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
#endif


#include <ixwebsocket/IXWebSocket.h>


#if 0
using json = nlohmann::json;

struct MarketEvent
{
    std::string exchange;
    std::string symbol;
    std::string type;
    double price;
    double qty;
    int64_t ts;
};

struct MarketBus
{
    void push(MarketEvent e)
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(e));
    }

    bool pop(MarketEvent& e)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty())
            return false;
        e = std::move(queue.front());
        queue.pop();
        return true;
    }

private:
    std::mutex mtx;
    std::queue<MarketEvent> queue;
};

static std::shared_ptr<boost::asio::ssl::context> on_tls_init()
{
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    ctx->set_default_verify_paths();
    return ctx;
}

#endif

#if 0
void test()
{
     ix::WebSocket ws;

    // Binance spot trade stream (BTC/USDT)
    std::string url = "wss://stream.binance.com:9443/ws/btcusdt@trade";
    ws.setUrl(url);

    ws.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "✅ Connected to Binance WebSocket\n";
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            try
            {
                auto j = json::parse(msg->str);

                // Example payload:
                // {"e":"trade","E":1699925934404,"s":"BTCUSDT","p":"67321.12","q":"0.001","T":...}
                if (j["e"] == "trade")
                {
                    std::string symbol = j["s"];
                    double price = std::stod(j["p"].get<std::string>());
                    double qty   = std::stod(j["q"].get<std::string>());
                    int64_t ts   = j["T"];

                    auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));
                    std::time_t tt = std::chrono::system_clock::to_time_t(tp);

                    std::cout << std::put_time(std::gmtime(&tt), "%H:%M:%S")
                              << "  " << symbol
                              << "  price=" << price
                              << "  qty=" << qty << std::endl;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Parse error: " << e.what() << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::cout << "❌ Disconnected\n";
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "⚠️ Error: " << msg->errorInfo.reason << std::endl;
        }
    });

    // Enable automatic reconnection (optional)
    // ws.enableAutomaticReconnection(true);
    ws.start();

    // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1u));
    }
}
#endif
