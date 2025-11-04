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
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>

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


void demoOne()
{
    ix::WebSocket webSocket;

    // Binance spot trade stream (BTC/USDT)
    std::string url = "wss://stream.binance.com:9443/ws/btcusdt@trade";
    webSocket.setUrl(url);

    ix::SocketTLSOptions tlsOptions;
    tlsOptions.caFile = "/etc/ssl/certs/ca-certificates.crt"; // Debian/Ubuntu
    // tlsOptions.c = "/etc/ssl/certs";

    webSocket.setTLSOptions(tlsOptions);

    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "✅ Connected to Binance WebSocket\n";
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            try {
                const auto jsonData = json::parse(msg->str);

                // Example payload: {"e":"trade","E":1699925934404,"s":"BTCUSDT","p":"67321.12","q":"0.001","T":...}
                if (jsonData["e"] == "trade")
                {
                    const std::string symbol = jsonData["s"];
                    const double price = std::stod(jsonData["p"].get<std::string>());
                    const double qty   = std::stod(jsonData["q"].get<std::string>());
                    const int64_t ts = jsonData["T"];

                    const auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));
                    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);

                    std::cout << std::put_time(std::gmtime(&tt), "%H:%M:%S")
                              << "  " << symbol
                              << "  price=" << price
                              << "  qty=" << qty << std::endl;
                }
            }
            catch (const std::exception& exc) {
                std::cerr << "Parse error: " << exc.what() << std::endl;
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            std::cout << "❌ Disconnected\n";
        }
        else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "⚠️ Error: " << msg->errorInfo.reason << std::endl;
        }
    });

    // Enable automatic reconnection (optional)
    // ws.enableAutomaticReconnection(true);
    webSocket.start();

    // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1u));
    }
}
