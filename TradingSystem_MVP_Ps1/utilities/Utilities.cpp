/**============================================================================
Name        : Utilities.cpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.cpp
============================================================================**/

#include "Utilities.hpp"

#include <fstream>

namespace utilities
{
    std::filesystem::path getDataDir() noexcept
    {
        return std::filesystem::current_path() / "../../Parsers_JSON/data/binance/";
    }

    std::vector<std::string> readFile(const std::filesystem::path &filePath)
    {
        std::vector<std::string> lines;
        if (std::ifstream file(filePath); file.is_open() && file.good()){
            while (std::getline(file, lines.emplace_back())) { /** **/ }
        }
        lines.pop_back();
        return lines;
    }
}