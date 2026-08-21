/**============================================================================
Name        : binance_execution_gateway.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance implementation of the execution gateway.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_GATEWAY_HPP

#include "execution_gateway.hpp"

#include <functional>

/**
 * Binance implementation of IExecutionGateway.
 *
 * BinanceExecutionGateway is responsible for translating internal Order
 * and cancel requests into Binance-specific execution requests.
 *
 * The gateway belongs to the exchange integration layer. Higher-level
 * execution components such as OrderManager must depend only on the
 * IExecutionGateway interface and must not contain Binance-specific logic.
 *
 * Execution flow:
 *
 *     Strategy
 *         |
 *         v
 *     StrategyExecutor
 *         |
 *         v
 *     OrderManager
 *         |
 *         v
 *     IExecutionGateway
 *         |
 *         v
 *     BinanceExecutionGateway
 *         |
 *         v
 *     Binance execution API
 *
 * The actual network transport is intentionally injected through
 * callbacks. This keeps the gateway independent from a particular HTTP
 * or WebSocket implementation and makes the component easy to test.
 *
 * Execution reports travel in the opposite direction:
 *
 *     Binance execution API
 *         |
 *         v
 *     ExecutionReport
 *         |
 *         v
 *     ExecutionReportHandler
 *         |
 *         v
 *     OrderManager / PositionManager / Recorder
 */

namespace trading::exchanges::binance
{
    class BinanceExecutionGateway final : public execution::IExecutionGateway
    {
    public:
        using SendHandler   = std::function<void(const execution::Order&)>;
        using CancelHandler = std::function<void(OrderId)>;

        BinanceExecutionGateway() = default;

        BinanceExecutionGateway(SendHandler sendHandler,
                                CancelHandler cancelHandler) noexcept;

        void setSendHandler(SendHandler sendHandler) noexcept;

        void setCancelHandler(CancelHandler cancelHandler) noexcept;

        void send(const execution::Order& order) override;

        void cancel(OrderId orderId) override;

    private:
        SendHandler sendHandler;
        CancelHandler cancelHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_GATEWAY_HPP