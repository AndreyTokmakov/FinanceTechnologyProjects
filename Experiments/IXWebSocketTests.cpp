/**============================================================================
Name        : DemoOne.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <ixwebsocket/IXWebSocket.h>

void wsTest()
{
    // Our websocket object
    ix::WebSocket webSocket;

    // Connect to a server with encryption
    // See https://machinezone.github.io/IXWebSocket/usage/#tls-support-and-configuration
    //     https://github.com/machinezone/IXWebSocket/issues/386#issuecomment-1105235227 (self signed certificates)
    const std::string url("wss://echo.websocket.org");
    webSocket.setUrl(url);

    // ix::SocketTLSOptions::disable_hostname_validation = true;

    ix::SocketTLSOptions tlsOptions;
    // tlsOptions.disable_hostname_validation = true;

    tlsOptions.caFile = "/etc/ssl/certs/ca-certificates.crt"; // Debian/Ubuntu
    // tlsOptions.c = "/etc/ssl/certs";

    webSocket.setTLSOptions(tlsOptions);

    std::cout << "Connecting to " << url << "..." << std::endl;

    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
        {
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                std::cout << "received message: " << msg->str << std::endl;
                std::cout << "> " << std::flush;
            }
            else if (msg->type == ix::WebSocketMessageType::Open)
            {
                std::cout << "Connection established" << std::endl;
                std::cout << "> " << std::flush;
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                // Maybe SSL is not configured properly
                std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
                std::cout << "> " << std::flush;
            }
        }
    );

    webSocket.start();                     // Now that our callback is setup, we can start our background thread and receive messages
    webSocket.send("hello world"); // Send a message to the server (default to TEXT mode)

    // Display a prompt
    std::cout << "> " << std::flush;

    std::string text;
    // Read text from the console and send messages in text mode.
    // Exit with Ctrl-D on Unix or Ctrl-Z on Windows.
    while (std::getline(std::cin, text))
    {
        webSocket.send(text);
        std::cout << "> " << std::flush;
    }
}