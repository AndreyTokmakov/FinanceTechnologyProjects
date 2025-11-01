/**============================================================================
Name        : MinimalSynchronousClient.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include "Experiments.hpp"

#include <iostream>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>


#include <nlohmann/json.hpp>

namespace
{
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    using json = nlohmann::json;

}

void experiments::MinimalSynchronousClient::TestAll()
{
    constexpr std::string_view host = "stream.binance.com";
    constexpr uint16_t port  { 9443 };
    constexpr std::string_view target = "/ws/btcusdt@trade";

    net::io_context ioc;
    net::ssl::context ctx(net::ssl::context::tlsv12_client);
    ctx.set_default_verify_paths();

    tcp::resolver resolver{ioc};
    const tcp::resolver::results_type results = resolver.resolve(host, std::to_string(port));

    websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

    // Connect and SSL handshake
    net::connect(ws.next_layer().next_layer(), results.begin(), results.end());
    ws.next_layer().handshake(net::ssl::stream_base::client);
    ws.handshake(host, target); // WS handshake

    beast::flat_buffer buffer;
    while (true) {
        ws.read(buffer);
        auto msg = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());
        try {
            auto j = json::parse(msg);
            std::cout << j.dump() << std::endl;
        } catch (...) {
            std::cout << msg << std::endl;
        }
    }
}

/**
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518451}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518452}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518453}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518454}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518455}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518456}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518457}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518458}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00045000","s":"BTCUSDT","t":5416518459}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518460}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.06961000","s":"BTCUSDT","t":5416518461}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518462}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518463}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518464}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00091000","s":"BTCUSDT","t":5416518465}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00091000","s":"BTCUSDT","t":5416518466}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518467}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00005000","s":"BTCUSDT","t":5416518468}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00112000","s":"BTCUSDT","t":5416518469}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.01470000","s":"BTCUSDT","t":5416518470}
{"E":1762006840035,"M":true,"T":1762006840035,"e":"trade","m":true,"p":"109849.68000000","q":"0.00118000","s":"BTCUSDT","t":5416518471}
**/