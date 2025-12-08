/**============================================================================
Name        : main.cpp
Created on  : 14.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <print>
#include <string_view>
#include <vector>
#include <thread>

#include "RingBuffer.hpp"
#include "price_engine/PriceEngine.hpp"
#include "parser/Parser.hpp"
#include "connectors/Connector.hpp"
#include "utils/Utils.hpp"
#include "tests/Tests.hpp"

namespace base
{
    using namespace ring_buffer;
    using price_engine::ParserType;

    // TODO: Rename >>>
    template<ConnectorType ConnectorT, ParserType ParserT>
    struct DataFeederBase
    {
        ConnectorT& connector;
        ParserT& parser;

        two_phase_push::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        DataFeederBase(ConnectorT& connector, ParserT& parser)
            : connector { connector }, parser { parser } {
        }

        void run()
        {
            connectorThread = std::jthread { [&] { runConnector(); } };
            parserThread    = std::jthread { [&] { runParser(); } };
        }

    private:

        void runConnector()
        {
            if (!utilities::setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                const auto _ = connector.getData(*response);
                // std::cout << "Connector [CPU: " << getCpu() << "] : " << response->length() << std::endl;
                queue.commit();
            }
        }

        void runParser()
        {
            if (!utilities::setThreadCore(2)) {
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

    void start()
    {
        connectors::FileData_DummyConnector connector;
        if (!connector.init()) {
            std::cerr << "Failed to init connector" << std::endl;
            return;
        }

        price_engine::PricerEngine priceEngine {};
        priceEngine.run();

        parser::DummyParser parser { priceEngine };
        DataFeederBase feeder {connector, parser};
        feeder.run();
    }
}

namespace demo
{
    using namespace ring_buffer;

    template<typename Connector, typename Parser>
    struct Processor
    {
        Connector& connector;
        Parser& parser;

        two_phase_push::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        Processor(Connector& connector, Parser& parser)
            : connector { connector }, parser { parser } {
        }

        void run()
        {
            connectorThread = std::jthread { [&] { runConnector(); } };
            parserThread    = std::jthread { [&] { runParser(); } };
        }

    private:

        void runConnector()
        {
            if (!utilities::setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            connector.run(queue);
        }

        void runParser()
        {
            if (!utilities::setThreadCore(2)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 2  << std::endl;
                return;
            }

            uint32_t misses { 0 };
            buffer::Buffer* item { nullptr };
            while (true)
            {
                if ((item = queue.pop()))
                {
                    std::cout << "Parsed: " << item->length() << std::endl;
                    parser.parse(*item);
                    item->clear();
                    misses = 0;
                    continue;
                }

                if (misses++ > maxSessionBeforeSleep) {
                    // std::this_thread::sleep_for(std::chrono::microseconds (10U));
                }
            }
        }
    };

    void start()
    {
        connectors::IxWsConnector connector;
        if (!connector.init()) {
            std::cerr << "Failed to init connector" << std::endl;
            return;
        }

        parser::EventParser parser;
        Processor processor {connector, parser };
        processor.run();
    }
}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    // base::start();
    demo::start();
    // tests::pricerTests();

    return EXIT_SUCCESS;
}
