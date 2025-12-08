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

#include "RingBuffer.hpp"
#include "price_engine/PriceEngine.hpp"
#include "parser/Parser.hpp"
#include "connectors/Connector.hpp"
#include "utils/Utils.hpp"
#include "tests/Tests.hpp"

namespace demo
{
    using namespace ring_buffer;
    using price_engine::ParserType;

    template<ConnectorType ConnectorT, ParserType ParserT>
    struct Processor
    {
        ConnectorT& connector;
        ParserT& parser;

        two_phase_push::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        Processor(ConnectorT& connector, ParserT& parser)
            : connector { connector }, parser { parser } {
        }

        void run() {
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
                    parser.parse(*item);
                    item->clear();
                    misses = 0;
                    continue;
                }

                /** Sleep when do not have messages for some time **/
                if (misses++ > maxSessionBeforeSleep) {
                    std::this_thread::sleep_for(std::chrono::microseconds (10U));
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

        price_engine::PricerEngine priceEngine {};
        priceEngine.run();

        parser::DummyParser parser { priceEngine };  /** TODO: Rename **/
        Processor processor {connector, parser };
        processor.run();
    }
}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    demo::start();
    // tests::pricerTests();

    return EXIT_SUCCESS;
}
