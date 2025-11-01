/**============================================================================
Name        : main.cpp
Created on  : 23.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <print>
#include <thread>

#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cerrno>
#include <netdb.h>
#include <queue>

#include "common/Buffer.hpp"
#include "common/RingBuffer.hpp"

#include "FinalAction.hpp"
#include "DateTimeUtilities.hpp"

// TODO:
//  -  Connector / Auth / Fetch data - Strategy
//     - Коннктится к Exchange + авторизуется
//     - Получает данных и кладёт их RingBuffer с GrowingBuffers
//     -
//  -  Parser - Strategy
//     - Конвертирует 'Growing Buffers' в MarketData внутрюнюю стукуру
//     -
//  -  RingBuffer
//     - Holds Growing Buffers
//     - (модифицировать при записи если буфер используется для парсинга)



void tests();
void working_example();


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    tests();
    // working_example();

    return EXIT_SUCCESS;
}
