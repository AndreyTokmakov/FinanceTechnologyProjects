/**============================================================================
Name        : ExchangeDataProcessor.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : ExchangeDataProcessor.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXCHANGE_DATA_PROCESSOR_HPP
#define FINANCETECHNOLOGYPROJECTS_EXCHANGE_DATA_PROCESSOR_HPP

#include <thread>

#include "RingBuffer.hpp"
#include "MarketData.hpp"
#include "MarketDepthBook.hpp"
#include "Parser.hpp"

namespace price_engine
{
    using namespace market_data::binance;
    using ring_buffer::two_phase_push::RingBuffer;

    // TODO: Create ::parse(buffer) concepts for Parser
    using Parser = std::variant<
         parser::BinanceParser,
         parser::ByBitParser
     >;

    struct ExchangeDataProcessor
    {
        void run(uint32_t cpuId);

        /** TODO: Support move ??? **/
        void push(const std::string& eventData);

        explicit ExchangeDataProcessor(Parser parser);

    private:

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };
        constexpr static uint32_t bufferSize { 1024 };

        std::jthread worker {};

        RingBuffer<bufferSize> queue;
        MarketDepthBook<Price, Quantity> marketDepthBook;
        Parser parser;

        void handleEvents(uint32_t cpuId);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXCHANGE_DATA_PROCESSOR_HPP