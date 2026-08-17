/**============================================================================
Name        : execution_gateway.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_gateway.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP

#include "order.hpp"

namespace trading::execution
{
    struct  IExecutionGateway
    {
        virtual ~IExecutionGateway() = default;
        virtual void send(const Order& order) = 0;
        virtual void cancel(OrderId orderId) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP
