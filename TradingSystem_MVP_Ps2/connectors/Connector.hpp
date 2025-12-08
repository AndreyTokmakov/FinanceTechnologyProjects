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
#include <vector>

#include "Buffer.hpp"
#include "RingBuffer.hpp"

template<typename T>
concept ConnectorType = requires(T& connector, buffer::Buffer& buffer) {
    { connector.getData(buffer) } -> std::same_as<bool>;
};

namespace connectors
{
    struct FileData_DummyConnector
    {
        [[nodiscard]]
        bool init();

        [[nodiscard]]
        bool getData(buffer::Buffer& response);

    private:

        std::vector<std::string> data;
        size_t readPost { 0 };
    };

    struct IxWsConnector
    {
        [[nodiscard]]
        bool init();

        void run(ring_buffer::two_phase_push::RingBuffer<1024>& queue);
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_CONNECTOR_HPP