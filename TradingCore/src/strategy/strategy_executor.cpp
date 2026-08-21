/**============================================================================
Name        : strategy_executor.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Strategy signal execution component implementation.
============================================================================**/

/*
    StrategyExecutor implementation.

    StrategyExecutor converts the result of strategy evaluation into an
    OrderRequest understood by OrderManager.

    The component deliberately contains no strategy-specific logic.

    For a Buy signal:

        Signal::Buy
             |
             v
        Side::Buy
             |
             v
        bestAsk
             |
             v
        OrderRequest

    For a Sell signal:

        Signal::Sell
             |
             v
        Side::Sell
             |
             v
        bestBid
             |
             v
        OrderRequest

    Signal::None is ignored and does not result in an order.

    The generated OrderRequest is passed to OrderManager::createOrder().
    OrderManager is then responsible for risk validation, order creation and
    submission through the configured execution gateway.

    StrategyExecutor therefore does not bypass the normal order-management
    path.
*/

#include "strategy_executor.hpp"


namespace trading::strategy
{
    using execution::OrderRequest;


    StrategyExecutor::StrategyExecutor(execution::OrderManager& orderManager,
                                       const Quantity orderQuantity) noexcept :
        orderManager { orderManager }, orderQuantity { orderQuantity }
    {
    }

    StrategyExecutionResult StrategyExecutor::execute(const Signal signal,
                                                      const market_data::MarketEvent& event) const
    {
        if (signal == Signal::None)
            return std::optional<OrderId> {};

        const Side side = signal == Signal::Buy ? Side::Buy : Side::Sell;
        const Price price = signal == Signal::Buy ? event.bestAsk : event.bestBid;

        const OrderRequest request {
            .instrument = event.instrument,
            .side = side,
            .type = OrderType::Limit,
            .price = price,
            .quantity = orderQuantity
        };

        const execution::OrderCreationResult result = orderManager.createOrder(request);
        if (!result)
            return std::unexpected(result.error());

        return *result;
    }
}