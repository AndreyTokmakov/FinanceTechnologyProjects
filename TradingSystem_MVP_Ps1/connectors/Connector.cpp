/**============================================================================
Name        : Connector.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Connector.cpp
============================================================================**/

#include "Connector.hpp"

#include <cstring>
#include <iostream>
#include <print>
#include <thread>
#include "../utilities/Utilities.hpp"

namespace connectors
{
    bool FileData_DummyConnector::init()
    {
        // data =  utilities::readFile(utilities::getDataDir() / "allData.json");
        data = utilities::readFile(utilities::getDataDir() / "depth.json");
        return !data.empty();
    }

    bool FileData_DummyConnector::getData(buffer::Buffer& response)
    {
        if (readPost == data.size()) {
            std::println(std::cerr, "No more data to read");
            std::terminate();
        }

        const std::string& entry = data[readPost % data.size()];
        const size_t bytes = entry.size();

        std::memcpy(response.tail(bytes), entry.data(), bytes);
        response.incrementLength(bytes);

        std::this_thread::sleep_for(std::chrono::microseconds (250U));
        ++readPost;
        return true;
    };
}