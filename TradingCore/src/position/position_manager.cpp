/**============================================================================
Name        : position_manager.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : position_manager.cpp
============================================================================**/

/*
    PositionManager implementation.

    PositionManager converts executed Trade events into changes of the
    corresponding instrument position.

    Data Flow:

        ExecutionReport
               |
               | ExecType::Trade
               v
        PositionManager::applyExecution()
               |
               | instrument / side / price / quantity
               v
        Position::applyTrade()
               |
               v
        Position
               |
               +------------------+
               |                  |
               v                  v
        PnLCalculator        RiskManager
        / Strategy

    Responsibilities:

        - reject execution reports that do not represent trades;
        - reject trades with zero execution quantity;
        - locate or create the position for the instrument;
        - forward the trade information to Position;
        - provide read-only access to stored positions.

    Position arithmetic is implemented by Position. PositionManager is
    responsible only for routing execution events to the correct position.

    PnL calculation is outside the responsibility of this module.
*/

#include "position_manager.hpp"

namespace trading::position
{
    bool PositionManager::applyExecution(const execution::ExecutionReport& report)
    {
        if (report.execType != ExecType::Trade || report.quantity.isZero())
            return false;

        auto [it, inserted] = positions.try_emplace(report.instrument, report.instrument);

        Position& position = it->second;
        position.applyTrade(report.side, report.price, report.quantity);

        return true;
    }

    const Position* PositionManager::find(const InstrumentId instrument) const noexcept
    {
        const auto it = positions.find(instrument);
        if (it == positions.end())
            return nullptr;

        return &it->second;
    }
}