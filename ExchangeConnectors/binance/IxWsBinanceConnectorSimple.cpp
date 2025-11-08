/**============================================================================
Name        : IxWsBinanceConnectorSimple.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : IxWsBinanceConnectorSimple.cpp
============================================================================**/

#include "Binance.hpp"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>

#include "FileUtilities.hpp"

using json = nlohmann::json;


namespace
{
    std::filesystem::path dataFile {
        R"(/home/andtokm/DiskS/ProjectsUbuntu/FinanceTechnologyProjects/Parsers_JSON/data/binance/allData.json)" };

    void run()
    {
        std::string buffer;
        buffer.reserve(10 * 1024 * 1024);

        ix::WebSocket webSocket;
        const std::string binanceWsUrl = "wss://stream.binance.com:9443/";

        std::string url1 = binanceWsUrl + "ws/btcusdt@trade";

        /*
        std::string url2 = binanceWsUrl + "stream?streams="
                                          "btcusdt@trade/"
                                          "ethusdt@bookTicker/"
                                          "ethusdt@depth@100ms/"
                                          "bnbusdt@ticker/bnbusdt@aggTrade";*/
        std::string url2 = binanceWsUrl + "stream?streams="
                                          "btcusdt@trade/"
                                          "btcusdt@aggTrade/"
                                          "btcusdt@ticker/"
                                          "btcusdt@bookTicker/"
                                          "btcusdt@depth@100ms/"
                                          "btcusdt@depth20@100ms/"
                                          "btcusdt@kline_1m/"
                                          "btcusdt@miniTicker";

        // webSocket.setUrl(url1);
        webSocket.setUrl(url2);

        ix::SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "/etc/ssl/certs/ca-certificates.crt"; // Debian/Ubuntu
        // tlsOptions.c = "/etc/ssl/certs";

        webSocket.setTLSOptions(tlsOptions);

        webSocket.setOnMessageCallback([&buffer](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "✅ Connected to Binance WebSocket\n";
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                try
                {
                    /*
                    const auto jsonData = json::parse(msg->str);
                    if (jsonData["e"] == "trade")
                    {
                        const std::string symbol = jsonData["s"];
                        const double price = std::stod(jsonData["p"].get<std::string>());
                        const double qty   = std::stod(jsonData["q"].get<std::string>());
                        const int64_t ts = jsonData["T"];

                        const auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));
                        const std::time_t tt = std::chrono::system_clock::to_time_t(tp);

                        std::cout << std::put_time(std::gmtime(&tt), "%H:%M:%S")
                                  << "  " << symbol << "  price=" << price << "  qty=" << qty << std::endl;
                    }
                    else {
                        std::cout << jsonData << std::endl;
                    }*/

                    // FileUtilities::AppendToFile()
                    if (buffer.size() > 128 * 1024) {
                        FileUtilities::AppendToFile(dataFile, buffer);
                        std::cout << "File size: " << buffer.size() << std::endl;
                        buffer.clear();
                    }
                    buffer.append(msg->str).append(1, '\n');

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
}


void IxWsBinanceConnectorSimple::TestAll()
{
    run();
}

/*
15:31:25  BTCUSDT  price=104388  qty=0.00133
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=0.00054
15:31:25  BTCUSDT  price=104388  qty=0.0007
15:31:25  BTCUSDT  price=104388  qty=0.00053
15:31:25  BTCUSDT  price=104388  qty=0.00123
15:31:25  BTCUSDT  price=104388  qty=0.00105
15:31:25  BTCUSDT  price=104388  qty=0.00103
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=0.00102
15:31:25  BTCUSDT  price=104388  qty=0.00102
15:31:25  BTCUSDT  price=104388  qty=0.00096
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=4e-05
15:31:25  BTCUSDT  price=104388  qty=9e-05
15:31:25  BTCUSDT  price=104388  qty=1e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=4e-05
15:31:25  BTCUSDT  price=104388  qty=0.00064
15:31:25  BTCUSDT  price=104388  qty=1e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=0.00048
15:31:25  BTCUSDT  price=104388  qty=0.00106
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=0.00097
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=3e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=3e-05
15:31:25  BTCUSDT  price=104388  qty=2e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=0.00043
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104388  qty=5e-05
15:31:25  BTCUSDT  price=104387  qty=5e-05
15:31:25  BTCUSDT  price=104387  qty=5e-05
15:31:25  BTCUSDT  price=104387  qty=5e-05
15:31:25  BTCUSDT  price=104387  qty=5e-05
15:31:25  BTCUSDT  price=104386  qty=6e-05
15:31:25  BTCUSDT  price=104385  qty=5e-05
15:31:25  BTCUSDT  price=104385  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=6e-05
15:31:25  BTCUSDT  price=104384  qty=6e-05
15:31:25  BTCUSDT  price=104384  qty=6e-05
15:31:25  BTCUSDT  price=104384  qty=0.00734
15:31:25  BTCUSDT  price=104384  qty=0.0128
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=0.00039
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104384  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=0.00096
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=0.00035
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=0.00032
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=2e-05
15:31:25  BTCUSDT  price=104383  qty=3e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=2e-05
15:31:25  BTCUSDT  price=104383  qty=3e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=0.00029
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104383  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=0.00026
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=0.00024
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104381  qty=5e-05
15:31:25  BTCUSDT  price=104380  qty=5e-05
15:31:25  BTCUSDT  price=104380  qty=4e-05
15:31:26  BTCUSDT  price=104380  qty=0.00133
15:31:26  BTCUSDT  price=104380  qty=2e-05
15:31:26  BTCUSDT  price=104380  qty=5e-05
15:31:26  BTCUSDT  price=104380  qty=5e-05
15:31:26  BTCUSDT  price=104380  qty=0.00021
15:31:26  BTCUSDT  price=104380  qty=5e-05
15:31:26  BTCUSDT  price=104380  qty=5e-05
15:31:26  BTCUSDT  price=104380  qty=0.09537
15:31:26  BTCUSDT  price=104380  qty=9e-05
15:31:27  BTCUSDT  price=104380  qty=0.00012
15:31:27  BTCUSDT  price=104380  qty=0.00038
15:31:27  BTCUSDT  price=104380  qty=0.00133
15:31:27  BTCUSDT  price=104380  qty=0.00354
15:31:27  BTCUSDT  price=104380  qty=0.002
15:31:27  BTCUSDT  price=104380  qty=8e-05
15:31:28  BTCUSDT  price=104380  qty=5e-05
15:31:28  BTCUSDT  price=104380  qty=5e-05
15:31:28  BTCUSDT  price=104380  qty=0.00047
15:31:28  BTCUSDT  price=104380  qty=0.00188
15:31:28  BTCUSDT  price=104380  qty=0.0016
15:31:28  BTCUSDT  price=104380  qty=0.00064
15:31:28  BTCUSDT  price=104380  qty=0.00383
15:31:28  BTCUSDT  price=104380  qty=0.00105
15:31:28  BTCUSDT  price=104380  qty=0.00028
15:31:28  BTCUSDT  price=104380  qty=0.0005
15:31:28  BTCUSDT  price=104380  qty=0.00106
15:31:28  BTCUSDT  price=104380  qty=0.00012
15:31:29  BTCUSDT  price=104380  qty=0.0005
15:31:29  BTCUSDT  price=104380  qty=0.00133
*/