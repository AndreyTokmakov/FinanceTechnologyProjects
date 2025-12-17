/**============================================================================
Name        : Types.hpp
Created on  : 01.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TYPES_HPP
#define FINANCETECHNOLOGYPROJECTS_TYPES_HPP

#include "RingBuffer.hpp"
#include "MarketData.hpp"
#include "Exchange.hpp"

namespace types
{
    using namespace ring_buffer::two_phase_push;
    using BinanceMarketEvent = market_data::binance::BinanceMarketEvent;

    template<typename T>
    concept ParserType = requires(T& parser,
                                  const buffer::Buffer& buffer) {
        { parser.parse(buffer) } -> std::same_as<BinanceMarketEvent>;
    };


    template<typename T>
    concept ConnectorType = requires(T& connector,
                                     RingBuffer<1024>& queue) {
        { connector.run(queue) } -> std::same_as<void>;
    };

    template<typename T>
    concept MarketEventProcessor = requires(T& eventProcessor,
                                            common::Exchange exchange,
                                            BinanceMarketEvent& event)
    {
        { eventProcessor.push(exchange, event) } -> std::same_as<void>;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_HPP