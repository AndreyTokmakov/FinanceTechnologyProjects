/**============================================================================
Name        : order.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order.hpp
============================================================================**/

/*
    This module contains the domain models used by the execution subsystem:

        - OrderRequest
        - Order

    OrderRequest represents an intention to create an order.

    Order represents an order that has been accepted into the internal
    execution lifecycle and is subsequently tracked by OrderManager.

    --------------------------------------------------------------------------
    OrderRequest
    --------------------------------------------------------------------------

    OrderRequest describes what the trading system wants to execute.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        RiskManager
           |
           | approved OrderRequest
           v
        OrderManager
           |
           | creates Order
           v
        Order
           |
           v
        IExecutionGateway
           |
           v
        Exchange / Execution Venue

    Responsibilities:

        - identify the instrument to trade;
        - specify the order side;
        - specify the order type;
        - specify the requested price;
        - specify the requested quantity.

    OrderRequest represents an instruction, not an execution result.

    It therefore does not contain:

        - client order identifier;
        - exchange order identifier;
        - filled quantity;
        - order status.

    Those values become meaningful only after the order enters the execution
    lifecycle.

    OrderRequest does not:

        - perform risk validation;
        - create an OrderId;
        - communicate with an exchange;
        - track execution state;
        - modify Position;
        - calculate PnL.

    RiskManager and OrderManager are responsible for processing an
    OrderRequest before it reaches the execution gateway.

    --------------------------------------------------------------------------
    Order
    --------------------------------------------------------------------------

    Order represents the internal state of an order tracked by the execution
    subsystem.

    Data Flow:

        OrderRequest
             |
             v
        OrderManager
             |
             | creates and tracks Order
             v
           Order
             |
             +----------------------+
             |                      |
             v                      v
      IExecutionGateway      ExecutionReport
             |                      |
             |                      v
             |              ExecutionReportHandler
             |                      |
             |                      v
             |                 OrderManager
             |                      |
             +----------------------+
                        |
                        v
                  updated Order

    Responsibilities:

        - identify the order inside the trading system;
        - identify the corresponding exchange order;
        - store the instrument and side;
        - store the order type;
        - store the requested price and quantity;
        - track the quantity already filled;
        - track the current order status.

    Order is mutable execution state owned and maintained by OrderManager.

    OrderManager is responsible for applying execution results to an Order.

    Order does not:

        - send itself to an exchange;
        - communicate with an execution gateway;
        - perform risk checks;
        - update Position directly;
        - calculate PnL;
        - parse ExecutionReport;
        - contain exchange-specific transport details.

    The distinction between OrderRequest and Order is intentional:

        OrderRequest
            -> "what the trading system wants to execute"

        Order
            -> "the order currently tracked by the trading system"

    An OrderRequest may result in the creation of an Order, while an Order
    continues to exist and change state as execution reports are received.

    The execution lifecycle is therefore:

        OrderRequest
             |
             v
        RiskManager
             |
             v
        OrderManager
             |
             v
           Order
             |
             v
        IExecutionGateway
             |
             v
        Execution Venue
             |
             v
        ExecutionReport
             |
             v
        OrderManager
             |
             v
        updated Order
*/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

namespace trading::execution
{
    /*
        OrderRequest represents an instruction to create an order.
        It contains only the information required to describe the requested
        trade and does not contain execution state.
    */
    struct OrderRequest
    {
        InstrumentId instrument;
        Side side;
        OrderType type;
        Price price;
        Quantity quantity;
    };

    /*
        Order represents an order tracked by the execution subsystem.
        OrderManager owns the lifecycle of the order and updates its execution
        state when ExecutionReport instances are received.
    */
    struct Order
    {
        OrderId clientOrderId { 0 };
        ExchangeOrderId exchangeOrderId { 0 };
        InstrumentId instrument { 0 };
        Side side { Side::Buy };
        OrderType type { OrderType::Limit };
        Price price {};
        Quantity quantity {};
        Quantity filledQuantity {};
        OrderStatus status { OrderStatus::New };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_HPP