/**============================================================================
Name        : strategy_executor.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Strategy signal execution component.
============================================================================**/

/*
    StrategyExecutor converts a strategy Signal into an order request and
    submits the resulting order through OrderManager.

    StrategyExecutor is the integration layer between the strategy subsystem
    and the execution subsystem.

    A strategy is intentionally unaware of order management. It only evaluates
    market data and produces a Signal:

        MarketEvent
             |
             v
        IStrategy::evaluate()
             |
             v
          Signal
             |
             v
        StrategyExecutor
             |
             v
        OrderRequest
             |
             v
        OrderManager

    Responsibilities:

        - receive a strategy Signal;
        - ignore Signal::None;
        - determine the order side from the Signal;
        - select the appropriate market price;
        - use the configured order quantity;
        - construct an OrderRequest;
        - submit the order through OrderManager.

    Price selection:

        Signal::Buy
            -> best ask price

        Signal::Sell
            -> best bid price

    The strategy decides only the trading direction. It does not decide the
    order quantity or construct an OrderRequest.

    Order quantity is supplied to StrategyExecutor during construction.

    StrategyExecutor does not:

        - calculate trading signals;
        - perform risk checks;
        - validate order limits;
        - manage positions;
        - communicate directly with an exchange;
        - submit orders through IExecutionGateway.

    All order validation and execution remain the responsibility of
    OrderManager and the execution subsystem.

    A Signal::None does not create an order and returns OrderId { 0 }.

    The returned OrderId identifies the order created by OrderManager.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_STRATEGY_EXECUTOR_HPP
#define FINANCETECHNOLOGYPROJECTS_STRATEGY_EXECUTOR_HPP

#include "signal.hpp"
#include "types.hpp"
#include "quantity.hpp"
#include "order_manager.hpp"
#include "model/market_event.hpp"

namespace trading::strategy
{
    using StrategyExecutionResult = std::expected<std::optional<OrderId>, execution::OrderCreationError>;

    class StrategyExecutor final
    {
    public:
        StrategyExecutor(execution::OrderManager& orderManager,
                         Quantity orderQuantity) noexcept;

        [[nodiscard]]
        StrategyExecutionResult execute(Signal signal, const market_data::MarketEvent& event) const;

    private:
        execution::OrderManager& orderManager;
        Quantity orderQuantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_STRATEGY_EXECUTOR_HPP