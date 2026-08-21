/**============================================================================
Name        : binance_execution_gateway.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution gateway implementation.
============================================================================**/

#include "binance_execution_gateway.hpp"

namespace trading::exchanges::binance
{
    BinanceExecutionGateway::BinanceExecutionGateway(SendHandler sendHandler,
                                                     CancelHandler cancelHandler) noexcept :
                                                     sendHandler { std::move(sendHandler) },
                                                     cancelHandler { std::move(cancelHandler) }
    {
    }

    void BinanceExecutionGateway::setSendHandler(SendHandler handler) noexcept
    {
        sendHandler = std::move(handler);
    }

    void BinanceExecutionGateway::setCancelHandler(CancelHandler handler) noexcept
    {
        cancelHandler = std::move(handler);
    }

    void BinanceExecutionGateway::send(const execution::Order& order)
    {
        if (!sendHandler)
            return;
        sendHandler(order);
    }

    void BinanceExecutionGateway::cancel(const OrderId orderId)
    {
        if (!cancelHandler)
            return;

        cancelHandler(orderId);
    }
}