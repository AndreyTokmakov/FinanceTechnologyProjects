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


namespace price_engine
{
    using namespace market_data::binance;

    struct ExchangeDataProcessor
    {
        ring_buffer::basic::RingBuffer<BinanceMarketEvent, 1024> queue {};
        MarketDepthBook<Price, Quantity> marketDepthBook;
        std::jthread worker {};

        void run(uint32_t cpuId);
        void handleEvents(uint32_t cpuId);

        /** TODO: Support move **/
        void push(BinanceMarketEvent& event);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXCHANGE_DATA_PROCESSOR_HPP