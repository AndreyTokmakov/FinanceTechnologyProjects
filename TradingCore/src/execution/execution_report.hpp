/**============================================================================
Name        : execution_report.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report.hpp
============================================================================**/

/*
    ExecutionReport represents the normalized result of an order execution
    event received from an external execution venue.

    An ExecutionReport is an inbound execution-domain message. It carries the
    information required by the trading core to update the state of an order
    and the resulting position.

    Data Flow:

        Exchange / Execution Venue
                  |
                  | exchange execution message
                  v
        Exchange-specific Execution Adapter
                  |
                  | normalized ExecutionReport
                  v
        ExecutionReportHandler
                  |
                  +----------------------+
                  |                      |
                  v                      v
             OrderManager         PositionManager
                  |                      |
                  | order state          | position state
                  v                      v
                Order                Position
                                         |
                                         v
                                       PnL
    Responsibilities:

        - identify the affected client order;
        - identify the corresponding exchange order;
        - identify the traded instrument;
        - describe the order side;
        - describe the execution event type;
        - describe the resulting order status;
        - provide the order price;
        - provide the order quantity;
        - provide the quantity filled by the execution venue.

    ExecutionReport is a normalized domain model.

    Exchange-specific execution messages must be converted into
    ExecutionReport before they enter the trading core.

    ExecutionReport does not:

        - send orders to an exchange;
        - cancel orders;
        - modify Order directly;
        - modify Position directly;
        - perform risk checks;
        - calculate PnL;
        - parse exchange-specific messages;
        - contain transport-specific information.

    The report represents information about what happened at the execution venue.
    It is therefore consumed by components responsible for maintaining internal trading state.

    OrderManager uses ExecutionReport to update the lifecycle and execution state of an Order.
    PositionManager uses ExecutionReport to update the position resulting from executed quantity.

    PnL calculation is performed by the PnL module based on the resulting
    position and relevant market or execution prices.

    The separation between ExecutionReport and Order is intentional:
        Order
            -> internal state and intent maintained by the trading core

        ExecutionReport
            -> external execution result received from the venue
*/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

namespace trading::execution
{
    struct ExecutionReport
    {
        OrderId clientOrderId { 0 };
        ExchangeOrderId exchangeOrderId { 0 };
        InstrumentId instrument { 0 };
        Side side { Side::Buy };
        ExecType execType { ExecType::New };
        OrderStatus status { OrderStatus::New };
        Price price;
        Quantity quantity;
        Quantity filledQuantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP