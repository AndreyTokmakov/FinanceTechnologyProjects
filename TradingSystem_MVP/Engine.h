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

#include <boost/beast/core.hpp>
#include <boost/container/flat_map.hpp>

namespace Engine
{
    using namespace std::string_literals;
    using Common::Pair;
    using Common::FlatBuffer;
    using Common::FlatBufferQueue;

    struct OrderBook
    {
        Pair pair;

        // TODO: Rename method
        void processEvent(FlatBuffer* message) const;
    };

    struct ExchangeBookKeeper
    {
        boost::container::flat_map<Pair, std::unique_ptr<OrderBook>> orderBooksByTicker;
        FlatBufferQueue queue;
        std::jthread worker;

        explicit ExchangeBookKeeper();

        // TODO: Rename??
        void push(FlatBuffer* message);
        void start(uint32_t cpuId);
    };

    struct PricingEngine
    {
        ExchangeBookKeeper binanceBookKeeper;
        ExchangeBookKeeper byBitBookKeeper;
        std::array<ExchangeBookKeeper*, 2> books { &binanceBookKeeper, &byBitBookKeeper };

        void start();
        void push(Common::Exchange exchange, FlatBuffer* message);
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_ENGINE_H
