/**============================================================================
Name        : execution_report_handler.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler.cpp
============================================================================**/

/*
    ExecutionReportHandler implementation.

    Data Flow:

        ExecutionReport
               |
               v
        ExecutionReportHandler
               |
               +-----> OrderManager
               |
               +-----> PositionManager

    The handler does not interpret the execution report. Each consumer decides
    how the report affects its own domain state.
*/

#include "execution_report_handler.hpp"

#include "order_manager.hpp"
#include "position_manager.hpp"

namespace trading::execution
{
    ExecutionReportHandler::ExecutionReportHandler(OrderManager& orderManager,
                                                   position::PositionManager& positionManager) noexcept
        : orderManager { orderManager },
          positionManager { positionManager }
    {
    }

    bool ExecutionReportHandler::onExecutionReport(const ExecutionReport& report) const
    {
        if (!orderManager.applyExecution(report))
            return false;
        return positionManager.applyExecution(report);
    }
}