/**============================================================================
Name        : trade_recorder.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_recorder.cpp
============================================================================**/

/*
    TradeRecorder implementation.

    Data Flow:

        MarketEvent
             |
             v
        record()
             |
             v
        marketEvents

        ExecutionReport
             |
             v
        record()
             |
             v
        executionReports

    This implementation stores events in memory. It intentionally does not
    serialize or persist them externally.
*/

#include "trade_recorder.hpp"

namespace trading::recording
{
    void TradeRecorder::record(const market_data::MarketEvent& event)
    {
        marketEventsVec.push_back(event);
    }

    void TradeRecorder::record(const execution::ExecutionReport& report)
    {
        executionReportsVec.push_back(report);
    }

    const std::vector<market_data::MarketEvent>&
    TradeRecorder::marketEvents() const noexcept
    {
        return marketEventsVec;
    }

    const std::vector<execution::ExecutionReport>& TradeRecorder::executionReports() const noexcept
    {
        return executionReportsVec;
    }

    std::size_t TradeRecorder::marketEventCount() const noexcept
    {
        return marketEventsVec.size();
    }

    std::size_t TradeRecorder::executionReportCount() const noexcept
    {
        return executionReportsVec.size();
    }

    void TradeRecorder::clear() noexcept
    {
        marketEventsVec.clear();
        executionReportsVec.clear();
    }
}