/**============================================================================
Name        : Utilities.hpp
Created on  : 07.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_UTILITIES_HPP
#define FINANCETECHNOLOGYPROJECTS_UTILITIES_HPP

#include <vector>
#include <filesystem>

namespace utilities
{
    std::filesystem::path getDataDir() noexcept;
    std::vector<std::string> readFile(const std::filesystem::path &filePath);
}

#endif //FINANCETECHNOLOGYPROJECTS_UTILITIES_HPP