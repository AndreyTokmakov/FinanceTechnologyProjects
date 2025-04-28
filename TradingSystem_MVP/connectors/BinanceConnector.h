/**============================================================================
Name        : BinanceConnector.h
Created on  : 24.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BinanceConnector.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCECONNECTOR_H
#define FINANCETECHNOLOGYPROJECTS_BINANCECONNECTOR_H

#include "Types.h"
#include "Engine.h"
#include <thread>

namespace connectors
{
    using common::FlatBuffer;

    struct BinanceWsConnector
    {
        static inline constexpr std::string_view host { "testnet.binance.vision" };
        static inline constexpr uint16_t port { 443 };
        static inline constexpr uint32_t bufferSize { 1024 };

        engine::PricingEngine& pricingEngine;
        std::jthread worker;
        uint32_t coreId = 0;

        uint32_t counter = 0;
        std::vector<FlatBuffer> buffers {};

        explicit BinanceWsConnector(engine::PricingEngine& engine, const uint32_t cpuId);

        // TODO: Re-structure code -- Coroutines ??
        void start(const std::string& pair);
    };
};

#endif //FINANCETECHNOLOGYPROJECTS_BINANCECONNECTOR_H
