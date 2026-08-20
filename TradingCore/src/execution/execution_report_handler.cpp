/**============================================================================
Name        : execution_report_handler.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler.cpp
============================================================================**/

/*
    ExecutionReportHandler implementation.

    This module coordinates the inbound ExecutionReport processing pipeline.

    Event Flow:

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

    ExecutionReportHandler does not contain execution-domain business logic.
    It only coordinates the components responsible for recording and applying
    the execution event.
*/

#include "execution_report_handler.hpp"

namespace trading::execution
{
    ExecutionReportHandler::ExecutionReportHandler(OrderManager& orderManager,
                                                   position::PositionManager& positionManager,
                                                   recording::IRecorder& recorder) noexcept:
        orderManager { orderManager },
        positionManager { positionManager },
        recorder { recorder }
    {
    }

    bool ExecutionReportHandler::onExecutionReport(const ExecutionReport& report) const
    {
        recorder.record(report);
        if (!orderManager.applyExecution(report))
            return false;
        return positionManager.applyExecution(report);
    }
}