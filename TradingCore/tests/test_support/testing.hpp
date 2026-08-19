/**============================================================================
Name        : testing.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : testing.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TESTING_HPP
#define FINANCETECHNOLOGYPROJECTS_TESTING_HPP

#include <source_location>
#include <iostream>
#include <string_view>

namespace testing
{
    constexpr void printLocation(const std::source_location& location)
    {
        std::println(std::cerr, "\tFile: {}\n\tFunction: {}\n\tLine: {}",
            location.file_name(), location.function_name(), location.line());
    }

    inline void Assert(const bool condition,
                       const std::string_view message,
                       const std::source_location& location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            printLocation(location);
            std::terminate();
        }
    }
}

#endif //FINANCETECHNOLOGYPROJECTS_TESTING_HPP