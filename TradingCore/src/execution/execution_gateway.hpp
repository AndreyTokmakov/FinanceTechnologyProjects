/**============================================================================
Name        : execution_gateway.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_gateway.hpp
============================================================================**/

/*
    IExecutionGateway represents the execution boundary between the trading
    core and an external execution system.

    The gateway is responsible for sending orders and cancellation requests
    from the internal trading system to an external venue.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        RiskManager
           |
           | approved request
           v
        OrderManager
           |
           | Order
           v
        IExecutionGateway
           |
           | order / cancel request
           v
        Exchange / Execution Venue
           |
           | execution report
           v
        ExecutionReportHandler
           |
           +------------------+
           |                  |
           v                  v
        OrderManager     PositionManager

    Responsibilities:

        - provide an abstraction over the external execution venue;
        - send an Order to the external venue;
        - request cancellation of an existing order;
        - hide transport and exchange-specific execution details from the trading core.

    IExecutionGateway defines a boundary, not an execution implementation.

    Concrete implementations may use:

        - REST;
        - WebSocket;
        - FIX;
        - TCP;
        - exchange-specific APIs;
        - another transport mechanism.

    Transport-specific details must remain inside the concrete gateway  implementation.

    IExecutionGateway does not:

        - create Orders;
        - validate trading strategy decisions;
        - perform risk checks;
        - maintain Position;
        - calculate PnL;
        - update Order state from execution reports;
        - parse exchange-specific execution messages;
        - generate ExecutionReport objects.

    Execution state flows back into the trading core through execution reports.
    The gateway therefore represents the outbound direction of the execution
    pipeline, while ExecutionReportHandler represents the inbound direction.

    The separation between outbound commands and inbound execution reports
    keeps the trading core independent of a particular exchange or transport.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP

#include "order.hpp"

namespace trading::execution
{
    struct IExecutionGateway
    {
        virtual ~IExecutionGateway() = default;

        virtual void send(const Order& order) = 0;
        virtual void cancel(OrderId orderId) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_GATEWAY_HPP