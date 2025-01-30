/**============================================================================
Name        : Gateway.h
Created on  : 22.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Gateway.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_GATEWAY_H
#define FINANCETECHNOLOGYPROJECTS_GATEWAY_H

#include <cstdint>

namespace Gateway
{
    using Socket = int32_t;

    constexpr int32_t RESULT_SUCCESS = 0;
    constexpr int32_t INVALID_HANDLE = -1;
    constexpr int32_t SOCKET_ERROR = -1;
}

#endif //FINANCETECHNOLOGYPROJECTS_GATEWAY_H
