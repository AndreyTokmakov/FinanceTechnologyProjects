/**============================================================================
Name        : PriceEngine.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceEngine.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP
#define FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP

#include <concepts>
#include <thread>
#include "Exchange.hpp"
#include "RingBuffer.hpp"
#include "MarketData.hpp"
#include "MarketDepthBook.hpp"


namespace price_engine
{
    using namespace market_data::binance;

    template<typename T>
    concept ParserType = requires(T& parser, const buffer::Buffer& buffer) {
        { parser.parse(buffer) } -> std::same_as<void>;
    };

    // TODO: Rename
    struct ExchangeBookKeeper
    {
        ring_buffer::basic::RingBuffer<BinanceMarketEvent, 1024> queue {};
        MarketDepthBook<Price, Quantity> book;
        std::jthread worker {};

        void run(uint32_t cpuId);
        void handleEvents(uint32_t cpuId);

        /** TODO: Support move **/
        void push(BinanceMarketEvent& event);
    };

    struct PricerEngine
    {
        /** For Exchange::Binance **/
        ExchangeBookKeeper binanceBookKeeper;

        /** For Exchange::ByBit **/
        ExchangeBookKeeper byBitBookKeeper;

        std::array<ExchangeBookKeeper*, 2> books { &binanceBookKeeper, &byBitBookKeeper };

        void run() const;
        void push(common::Exchange exchange, BinanceMarketEvent& event) const;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_PRICEENGINE_HPP