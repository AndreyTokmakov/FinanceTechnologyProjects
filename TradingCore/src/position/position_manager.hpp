/**============================================================================
Name        : position_manager.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Position manager.
============================================================================**/

/*
Position manager.
Maintains the current position state for each trading instrument.

Event Flow :
    ExecutionGateway
          |
          | ExecutionReport
          v
    OrderManager
          |
          | ExecutionReport
          v
    PositionManager
          |
          | Trade execution
          v
    Position
          |
          | Position state
          v
    PnLCalculator / RiskEngine / Strategy

Responsibility :
    PositionManager is responsible for maintaining position state based on
    executed trades.

    It converts individual Trade execution reports into changes of the
    corresponding instrument position.

    PositionManager does not:
      - communicate with an exchange;
      - create or send orders;
      - perform risk checks;
      - calculate realized or unrealized PnL;
      - process market data;
      - manage order lifecycle.

    Only ExecutionReport messages with ExecType::Trade can change a position.

Position semantics :
    Positive quantity represents a long position.
    Negative quantity represents a short position.
    Zero quantity represents a flat position.

    The average entry price is maintained for the currently open position.

*/

#ifndef FINANCETECHNOLOGYPROJECTS_POSITION_MANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_POSITION_MANAGER_HPP

#include "execution_report.hpp"
#include "position.hpp"
#include "types.hpp"

#include <map>

namespace trading::position
{
    class PositionManager
    {
    public:
        [[nodiscard]]
        bool applyExecution(const trading::execution::ExecutionReport& report);

        [[nodiscard]]
        const Position* find(InstrumentId instrument) const noexcept;

    private:
        std::map<InstrumentId, Position> positions;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_POSITION_MANAGER_HPP