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

#include "Types.hpp"
#include "MarketStateManager.hpp"
#include "Parser.hpp"
#include "Connector.hpp"
#include "utils/Utils.hpp"
#include "tests/Tests.hpp"

namespace demo
{

    void start()
    {
        price_engine::MarketStateManager marketStateMgr {};

        // connectors::IxWsConnector connector;
        connectors::DataFileDummyConnector2 connector { marketStateMgr };

        if (!connector.init()) {
            std::cerr << "Failed to init connector" << std::endl;
            return;
        }

        marketStateMgr.run();
        connector.run();
    }


    template<types::ParserType __Pt>
    struct Wrapper
    {
        __Pt& parser;

        void parse(const buffer::Buffer& buffer)
        {
            parser.parse(buffer);
        }
    };

    struct DummyParserEx
    {
        static market_data::binance::BinanceMarketEvent parse(const buffer::Buffer& buffer)
        {

        }
    };


    void test()
    {
        DummyParserEx exParser;
        Wrapper<DummyParserEx> w { exParser };
        w.parse({});
    }
}

// TODO:
//  1. Выделить Connector в более Generic интерфейс --> Что бы легко заменять с DataFileDummyConnector
//  2. Concepts [нужно как-то доработать]
//  3. Parser --> combined with ExchangeDataProcessor
//  4. SPD Logger

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    // demo::start();
    demo::test();
    // tests::pricerTests();

    return EXIT_SUCCESS;
}
