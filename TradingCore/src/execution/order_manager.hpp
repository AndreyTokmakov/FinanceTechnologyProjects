/**============================================================================
Name        : order_manager.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.hpp
============================================================================**/

/*
    OrderManager is responsible for maintaining the lifecycle and execution
    state of orders inside the trading core.

    OrderManager is the central component of the execution subsystem that
    coordinates order creation, risk validation, order tracking, execution
    requests, cancellation requests and application of execution results.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        OrderManager
           |
           | risk validation
           v
        IRiskManager
           |
           | approved
           v
        OrderManager
           |
           | Order
           v
        IExecutionGateway
           |
           v
        Exchange / Execution Venue
           |
           | ExecutionReport
           v
        ExecutionReportHandler
           |
           v
        OrderManager::applyExecution()
           |
           +-------------------+
           |                   |
           v                   v
         Order              Position
                               |
                               v
                              PnL


    Responsibilities:

        - validate OrderRequest through IRiskManager;
        - create and track internal Order objects;
        - assign client order identifiers;
        - send approved orders through IExecutionGateway;
        - maintain the current state of tracked orders;
        - apply execution results to tracked Orders;
        - request cancellation of existing orders;
        - provide access to tracked order state.


    --------------------------------------------------------------------------
    Order creation
    --------------------------------------------------------------------------

    createOrder() is the entry point for creating a new order inside the execution subsystem.
    It receives an OrderRequest representing the trading intent.
    Before an Order is created and sent to the execution gateway, the request must pass through IRiskManager.

    The execution flow is:

        Strategy
           |
           | OrderRequest
           v
        OrderManager::createOrder()
           |
           v
        IRiskManager
           |
           | approved
           v
        Order
           |
           v
        IExecutionGateway
           |
           v
        Exchange / Execution Venue

    OrderManager therefore acts as the coordinator between risk validation,
    internal order state and the external execution gateway.


    --------------------------------------------------------------------------
    OrderManager::applyExecution
    --------------------------------------------------------------------------

    applyExecution() is the inbound execution-state transition of OrderManager.

    It applies an ExecutionReport received from the external execution venue
    to the corresponding internally tracked Order.

    An Order initially represents an execution request created by the trading
    system. Its actual execution state becomes known only after the external
    venue sends an execution result.

    ExecutionReport contains information such as:

        - client order identifier;
        - exchange order identifier;
        - execution type;
        - order status;
        - execution price;
        - order quantity;
        - filled quantity.

    applyExecution() uses this information to update the corresponding Order, including its
    exchange order identifier, execution status and filled quantity.

    The method is called by ExecutionReportHandler after an execution report
    has been normalized and entered the trading core.

    The inbound execution flow is:

        Exchange / Execution Venue
                  |
                  | execution message
                  v
        Exchange-specific adapter
                  |
                  | ExecutionReport
                  v
        ExecutionReportHandler
                  |
                  | applyExecution(report)
                  v
        OrderManager
                  |
                  | update order state
                  v
                Order
                  |
                  v
        PositionManager::applyExecution()

    applyExecution() does not initiate execution.

    It does not:

        - send an order to the exchange;
        - request an order cancellation;
        - perform risk validation;
        - parse exchange-specific execution messages;
        - update Position directly;
        - calculate PnL.

    Its responsibility is limited to applying the normalized execution result
    to the internal Order state.

    The method returns bool to indicate whether the execution report was
    successfully applied.

    A failure may occur when the referenced order is not known to OrderManager
    or when the execution report cannot be applied to the current order state.


    --------------------------------------------------------------------------
    OrderManager::cancel
    --------------------------------------------------------------------------

    cancel() represents an outbound cancellation request.

    It requests cancellation of an existing order through IExecutionGateway.

    A successful cancellation request does not by itself prove that the order
    has been cancelled at the execution venue.

    The actual resulting order state is confirmed through a subsequent
    ExecutionReport, which is eventually processed by
    OrderManager::applyExecution().

    Therefore:

        cancel()
            -> sends cancellation request

        applyExecution()
            -> applies confirmed execution state


    --------------------------------------------------------------------------
    Order state
    --------------------------------------------------------------------------

    OrderManager owns the collection of internally tracked Orders.

    The Order object represents the current internal execution state of an
    order, while OrderRequest represents the original trading intent.

    The distinction is:

        OrderRequest
            -> what the trading system wants to execute

        Order
            -> the order currently tracked by the trading system


    OrderManager does not contain exchange-specific execution logic.

    Exchange-specific protocols, transport mechanisms and message formats
    belong to concrete implementations of IExecutionGateway and the
    corresponding exchange adapters.

    OrderManager is therefore a domain-level execution component that
    coordinates:

        Risk
          |
          v
        Order
          |
          v
        Execution Gateway
          |
          v
        Execution Report
          |
          v
        Order state
*/


#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP

#include <expected>
#include <map>

#include "execution_gateway.hpp"
#include "execution_report.hpp"
#include "order.hpp"
#include "risk_manager.hpp"
#include "position.hpp"

namespace trading::execution
{
    enum class OrderCreationError: uint8_t
    {
        RiskRejected,
        InvalidRequest
    };

    using OrderCreationResult = std::expected<OrderId, OrderCreationError>;

    class OrderManager
    {
    public:
        OrderManager(IExecutionGateway& gateway,
                     risk::IRiskManager& riskManager,
                     position::Position& position) noexcept;

        [[nodiscard]]
        OrderCreationResult createOrder(const OrderRequest& request);

        [[nodiscard]]
        bool applyExecution(const ExecutionReport& report);

        [[nodiscard]]
        const Order* find(OrderId orderId) const noexcept;

        [[nodiscard]]
        bool cancel(OrderId orderId);

    private:
        IExecutionGateway& gateway;
        risk::IRiskManager& riskManager;
        position::Position& position;
        std::map<OrderId, Order> orders;
        OrderId nextOrderId { 1 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP