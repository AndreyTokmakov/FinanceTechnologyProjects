/**============================================================================
Name        : trade_recorder.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_recorder.hpp
============================================================================**/

/*
    TradeRecorder is the initial in-memory implementation of IRecorder.
    It is intended primarily as the first recording implementation and as a foundation for tests.

    Data Flow:

        MarketEvent
             |
             v
        TradeRecorder
             |
             v
        stored market events

        ExecutionReport
             |
             v
        TradeRecorder
             |
             v
        stored execution reports

    Responsibilities:
        - receive market events;
        - receive execution reports;
        - retain received events in memory;
        - expose recorded events for inspection.

    TradeRecorder does not:
        - write files;
        - serialize events;
        - modify domain state;
        - process market data;
        - process executions.

    A persistent recorder can later implement IRecorder independently without changing the producers of recording events.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_TRADE_RECORDER_HPP
#define FINANCETECHNOLOGYPROJECTS_TRADE_RECORDER_HPP

#include "recorder.hpp"

#include <vector>

namespace trading::recording
{
    class TradeRecorder final : public IRecorder
    {
    public:
        void record(const market_data::MarketEvent& event) override;
        void record(const execution::ExecutionReport& report) override;

        [[nodiscard]]
        const std::vector<market_data::MarketEvent>& marketEvents() const noexcept;

        [[nodiscard]]
        const std::vector<execution::ExecutionReport>& executionReports() const noexcept;

        [[nodiscard]]
        std::size_t marketEventCount() const noexcept;

        [[nodiscard]]
        std::size_t executionReportCount() const noexcept;

        void clear() noexcept;

    private:
        std::vector<market_data::MarketEvent> marketEventsVec;
        std::vector<execution::ExecutionReport> executionReportsVec;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TRADE_RECORDER_HPP