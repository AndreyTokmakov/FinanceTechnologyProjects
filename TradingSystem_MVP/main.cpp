/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <iostream>
#include "BinanceConnector.h"


// TODO:       *************** MULTIPLEXER ************************
//  - Каждый сервер должен публиковать сообщения в очереь соответствующую для данной
//    Exchange или symbol
//  - Каждое такое кольцо живёт в отдельном Thread-e
//  - В этом же Thread-e живет OrderBook-и для всех Бирж что публикуются в данное кольцо


// TODO: Optimisations:
//  -  Pair [usdtbtc] --> uint64_t  [так как длина 'usdtbtc' максимум 8 байт]
//     Возможно использовать SIMD для корвертации в INT

// TODO: Next steps
//  1. Доделать мультиплексер
//  2. Убрать копирования прочитанных сообщений в очереди [типа Direct Memory Access.....]
//     - каждый сервер читает сообщения в свой кольцевой буффер
//     - после этого push-ит УКАЗАТЕЛЬ на даннный элемент буффера в соотвествующий кольцевой буффер BookKeeper-а?


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);


    engine::PricingEngine pricingEngine;
    connectors::BinanceWsConnector binanceWsConnectorBtc { pricingEngine, 1 };
    connectors::BinanceWsConnector binanceWsConnectorEth { pricingEngine, 1 };
    binanceWsConnectorBtc.start("btcusdt");
    binanceWsConnectorEth.start("ethusdt");
    pricingEngine.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100U));
    }

    return EXIT_SUCCESS;
}
