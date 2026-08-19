/**============================================================================
Name        : recorder.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : recorder.hpp
============================================================================**/

/*
    IRecorder defines the recording boundary of the trading system.

    The recorder receives domain events and persists them for later analysis,
    debugging, replay or backtesting.

    Data Flow:

        MarketData
             |
             v
        MarketEvent
             |
             +----------------------+
                                    |
        ExecutionReport             |
             |                      |
             +----------------------+
                                    |
                                    v
                                IRecorder
                                    |
                                    v
                         recording implementation
                                    |
                                    v
                              storage/file

    Responsibilities:
        - accept MarketEvent instances;
        - accept ExecutionReport instances;
        - provide a persistence boundary for the trading system.

    IRecorder does not:
        - create market events;
        - create execution reports;
        - modify orders;
        - modify positions;
        - calculate PnL;
        - perform risk validation.

    The interface intentionally does not prescribe a storage format.
    Implementations may write to memory, a file, a binary log, a database or another storage system.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RECORDER_HPP
#define FINANCETECHNOLOGYPROJECTS_RECORDER_HPP

#include "execution_report.hpp"
#include "market_event.hpp"

namespace trading::recording
{
    struct IRecorder
    {
        virtual ~IRecorder() = default;

        virtual void record(const market_data::MarketEvent& event) = 0;
        virtual void record(const execution::ExecutionReport& report) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDER_HPP