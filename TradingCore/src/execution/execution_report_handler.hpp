/**============================================================================
Name        : execution_report_handler.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler.hpp
============================================================================**/

/*
    ExecutionReportHandler distributes ExecutionReport events to the
    components interested in execution state.

    Data Flow:

        ExecutionGateway
               |
               | ExecutionReport
               v
        ExecutionReportHandler
               |
               +--------------------+
               |                    |
               v                    v
        OrderManager         PositionManager
               |                    |
               v                    v
             Order              Position
                                    |
                                    v
                              PnLCalculator

    Responsibilities:

        - receive ExecutionReport events;
        - forward reports to OrderManager;
        - forward reports to PositionManager.

    ExecutionReportHandler does not:

        - create or send orders;
        - perform risk validation;
        - modify Order directly;
        - modify Position directly;
        - calculate PnL;
        - contain exchange-specific logic.

    The handler acts as an execution-event dispatcher. Domain state changes
    remain the responsibility of the corresponding domain managers.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP

#include "execution_report.hpp"
#include "position_manager.hpp"
#include "order_manager.hpp"

namespace trading::execution
{
    class ExecutionReportHandler final
    {
    public:
        ExecutionReportHandler(OrderManager& orderManager,
                               position::PositionManager& positionManager) noexcept;

        bool onExecutionReport(const ExecutionReport& report) const;

    private:
        OrderManager& orderManager;
        position::PositionManager& positionManager;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HANDLER_HPP