/**============================================================================
Name        : recording_event.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : recording_event.hpp
============================================================================**/

/*
    RecordingEvent contains the common metadata associated with a recorded trading-system event.
    Recording is intended to preserve events required for debugging, replay, analysis and later backtesting.

    Data Flow:

        MarketDataSource
               |
               v
        MarketData processing
               |
               v
          MarketEvent
               |
               +-------------------+
                                   |
        ExecutionGateway           |
               |                   |
               v                   |
        ExecutionReport            |
               |                   |
               +-------------------+
                                   |
                                   v
                              Recorder
                                   |
                                   v
                            Recorded Event

    Responsibilities:

        - identify the type of recorded event;
        - preserve event timestamps;
        - provide common metadata for recorded events.

    RecordingEvent does not contain the complete domain object. Domain-specific
    data remains in the corresponding event structures.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP
#define FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP

#include "timestamp.hpp"

#include <cstdint>

namespace trading::recording
{
    enum class EventType : uint8_t
    {
        MarketEvent,
        ExecutionReport
    };

    struct RecordingEvent
    {
        EventType type { EventType::MarketEvent };
        Timestamp timestamp {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP