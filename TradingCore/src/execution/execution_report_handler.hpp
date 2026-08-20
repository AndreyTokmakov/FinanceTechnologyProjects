/**============================================================================
Name        : execution_report_handler.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler.hpp
============================================================================**/

/*
    ExecutionReportHandler is the entry point for normalized execution reports
    entering the trading core.

    It coordinates the processing of an ExecutionReport between the execution,
    position and recording subsystems.

    Data Flow:

        Exchange / Execution Venue
                  |
                  | exchange execution message
                  v
        Exchange-specific adapter
                  |
                  | ExecutionReport
                  v
        ExecutionReportHandler
                  |
          +-------+--------+----------------+
          |                |                |
          v                v                v
      IRecorder      OrderManager    PositionManager
          |                |                |
          v                v                v
       Event Log         Order          Position
                                             |
                                             v
                                            PnL


    Responsibilities:

        - receive normalized ExecutionReport objects;
        - record execution reports through IRecorder;
        - apply execution results to OrderManager;
        - apply execution results to PositionManager;
        - coordinate the inbound execution event flow.


    --------------------------------------------------------------------------
    ExecutionReport processing
    --------------------------------------------------------------------------

    onExecutionReport() is called after an exchange-specific execution
    message has been converted into the normalized ExecutionReport model.

    The method represents the boundary where an external execution event
    becomes an internal trading-core event.

    The processing sequence is:

        ExecutionReport
               |
               v
        IRecorder::record()
               |
               v
        OrderManager::applyExecution()
               |
               v
        PositionManager::applyExecution()

    The ExecutionReport is recorded before it is applied to the internal
    trading state.

    This preserves the original normalized execution event independently of
    the state changes performed by OrderManager and PositionManager.

    OrderManager::applyExecution() updates the lifecycle and execution state
    of the corresponding Order.

    PositionManager::applyExecution() updates the Position using the executed
    quantity represented by the ExecutionReport.

    Both methods return bool. Processing stops if OrderManager cannot apply
    the execution report successfully.


    --------------------------------------------------------------------------
    Recording
    --------------------------------------------------------------------------

    IRecorder receives the same normalized ExecutionReport that entered the
    execution pipeline.

    ExecutionReportHandler does not know how the report is serialized,
    stored or replayed.

    Those responsibilities belong to the concrete IRecorder implementation.

    The handler only forwards the event to IRecorder::record().


    ExecutionReportHandler does not:

        - parse exchange-specific execution messages;
        - communicate with an exchange;
        - create orders;
        - perform risk checks;
        - maintain Order state;
        - maintain Position state;
        - calculate PnL;
        - serialize execution reports.


    Its responsibility is orchestration of the inbound ExecutionReport event.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP

#include "execution_report.hpp"
#include "order_manager.hpp"
#include "position_manager.hpp"
#include "recorder.hpp"

namespace trading::execution
{
    class ExecutionReportHandler final
    {
    public:
        ExecutionReportHandler(OrderManager& orderManager,
                               position::PositionManager& positionManager,
                               recording::IRecorder& recorder) noexcept;

        [[nodiscard]]
        bool onExecutionReport(const ExecutionReport& report) const;

    private:
        OrderManager& orderManager;
        position::PositionManager& positionManager;
        recording::IRecorder& recorder;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP