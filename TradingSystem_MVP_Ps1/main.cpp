/**============================================================================
Name        : main.cpp
Created on  : 14.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <string_view>
#include <vector>
#include <thread>

#include "common/Buffer.hpp"
#include "common/RingBuffer.hpp"

#include "FinalAction.hpp"
#include "DateTimeUtilities.hpp"

namespace
{
    using namespace common;

    template<typename T>
    concept ConnectorType = requires(T& connector, buffer::Buffer& buffer) {
        { connector.getData(buffer) } -> std::same_as<void>;
    };

    template<typename T>
    concept ParserType = requires(T& parser, const buffer::Buffer& buffer) {
        { parser.parse(buffer) } -> std::same_as<std::string>;
    };

    template<ConnectorType Connector, ParserType Parser>
    struct DataFeederBase
    {
        Connector& connector;
        Parser& parser;

        ring_buffer::static_capacity_with_commit_buffer::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        DataFeederBase(Connector& connector, Parser& parser)
            : connector(connector), parser(parser) {
        }

        void run()
        {
            connectorThread = std::jthread { [&] { runConnector(); } };
            parserThread = std::jthread { [&] { runParser(); } };
        }

    private:

        void runConnector()
        {
            if (!setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                connector.getData(*response);
                std::cout << "Connector [CPU: " << getCpu() << ") : " << response->length() << std::endl;
                queue.commit();
            }
        }

        void runParser()
        {
            if (!setThreadCore(2)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 2  << std::endl;
                return;
            }

            uint32_t misses { 0 };
            buffer::Buffer* item { nullptr };
            while (true)
            {
                if ((item = queue.pop())) {
                    parser.parse(*item);
                    item->clear();
                    misses = 0;
                    continue;
                }

                if (misses++ > maxSessionBeforeSleep) {
                    std::this_thread::sleep_for(std::chrono::microseconds (10U));
                }
            }
        }
    };

}



int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);


    return EXIT_SUCCESS;
}
