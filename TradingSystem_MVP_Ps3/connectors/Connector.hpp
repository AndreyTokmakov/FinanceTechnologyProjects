/**============================================================================
Name        : Connector.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Connector.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_CONNECTOR_HPP
#define FINANCETECHNOLOGYPROJECTS_CONNECTOR_HPP

#include <concepts>
#include "RingBuffer.hpp"


namespace connectors
{
    struct IxWsConnector
    {
        [[nodiscard]]
        bool init();

        void run(ring_buffer::two_phase_push::RingBuffer<1024>& queue);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_CONNECTOR_HPP