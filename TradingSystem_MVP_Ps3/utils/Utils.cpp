/**============================================================================
Name        : Utilities.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.cpp
============================================================================**/

#include "Utils.hpp"

#include <fstream>

namespace utilities
{
    std::filesystem::path getDataDir() noexcept
    {
        return std::filesystem::path(TEST_DATA_PATH) / "binance";
    }

    std::vector<std::string> readFile(const std::filesystem::path &filePath)
    {
        std::vector<std::string> lines;
        if (std::ifstream file(filePath); file.is_open() && file.good()){
            while (std::getline(file, lines.emplace_back())) { /** **/ }
        }
        if (!lines.empty()) {
            lines.pop_back();
        }
        return lines;
    }

    bool setThreadCore(const uint32_t coreId) noexcept
    {
        cpu_set_t cpuSet {};
        CPU_ZERO(&cpuSet);
        CPU_SET(coreId, &cpuSet);
        return 0 == pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
    }

    int32_t getCpu() noexcept
    {
        return sched_getcpu();
    }
}