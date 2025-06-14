/**============================================================================
Name        : Engine.h
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Engine.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ENGINE_H
#define FINANCETECHNOLOGYPROJECTS_ENGINE_H

#include <string>
#include <thread>

#include "BlockingQueuePtr.h"
#include "Exchange.h"
#include "Types.h"
#include "MarketData.h"
#include "PriceLevelBook.h"

#include <boost/beast/core.hpp>
#include <boost/container/flat_map.hpp>

namespace engine
{
    using namespace std::string_literals;
    using common::Pair;
    using common::FlatBuffer;
    using common::FlatBufferQueue;


    struct ExchangeBookKeeper
    {
        boost::container::flat_map<Pair, std::unique_ptr<PriceLevelBook>> orderBooksByTicker;
        FlatBufferQueue queue;
        std::jthread worker;

        explicit ExchangeBookKeeper();

        // TODO: Rename??
        void push(FlatBuffer* message);
        void start(uint32_t cpuId);
    };

    struct PricingEngine
    {
        /** For Exchange::Binance **/
        ExchangeBookKeeper binanceBookKeeper;
        /** For Exchange::ByBit **/
        ExchangeBookKeeper byBitBookKeeper;

        std::array<ExchangeBookKeeper*, 2> books { &binanceBookKeeper, &byBitBookKeeper };

        void start();
        void push(common::Exchange exchange, FlatBuffer* message);
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_ENGINE_H
