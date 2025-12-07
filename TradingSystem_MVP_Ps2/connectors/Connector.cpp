/**============================================================================
Name        : Connector.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Connector.cpp
============================================================================**/

#include "Connector.hpp"

#include <cstring>
#include <iostream>
#include <print>
#include <thread>
#include "Utils.hpp"

#include <ixwebsocket/IXWebSocket.h>

namespace connectors
{
    bool FileData_DummyConnector::init()
    {
        // data =  utilities::readFile(utilities::getDataDir() / "allData.json");
        data = utilities::readFile(utilities::getDataDir() / "depth.json");
        return !data.empty();
    }

    bool FileData_DummyConnector::getData(buffer::Buffer& response)
    {
        if (readPost == data.size()) {
            std::println(std::cerr, "No more data to read");
            std::terminate();
        }

        const std::string& entry = data[readPost % data.size()];
        const size_t bytes = entry.size();

        std::memcpy(response.tail(bytes), entry.data(), bytes);
        response.incrementLength(bytes);

        std::this_thread::sleep_for(std::chrono::microseconds (250U));
        ++readPost;
        return true;
    };
}

namespace connectors
{
    void run()
    {
        ix::WebSocket webSocket;
        const std::string binanceWsUrl = "wss://stream.binance.com:9443/";
        const std::string url2 = binanceWsUrl + "stream?streams=btcusdt@depth@100ms";

        webSocket.setUrl(url2);

        ix::SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "/etc/ssl/certs/ca-certificates.crt"; // Debian/Ubuntu
        // tlsOptions.c = "/etc/ssl/certs";

        webSocket.setTLSOptions(tlsOptions);

        webSocket.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "✅ Connected to Binance WebSocket\n";
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                try
                {
                    auto data = msg->str;
                    std::cout << data << std::endl;
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

        // Keep the main thread alive
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1u));
        }
    }
}