/**============================================================================
Name        : Tests.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TESTS_HPP
#define FINANCETECHNOLOGYPROJECTS_TESTS_HPP

#include <thread>
#include "RingBuffer.hpp"
#include "MarketStateManager.hpp"

namespace tests
{
    void pricerTests();
}

namespace connectors
{
    struct DataFileDummyConnector
    {
        [[nodiscard]]
        bool init();

        void run(ring_buffer::two_phase_push::RingBuffer<1024>& queue);

    private:

        void produceTestEvents(ring_buffer::two_phase_push::RingBuffer<1024>& queue);

        std::vector<std::string> data;
        size_t readPost { 0 };
        std::jthread worker {};
    };

    struct DataFileDummyConnector2
    {
        explicit DataFileDummyConnector2(price_engine::MarketStateManager& marketStateManager);

        [[nodiscard]]
        bool init();

        void run();

    private:

        price_engine::MarketStateManager& marketStateManager;

        void produceTestEvents();

        std::vector<std::string> data;
        size_t readPost { 0 };
        std::jthread worker {};
    };
}


#endif //FINANCETECHNOLOGYPROJECTS_TESTS_HPP