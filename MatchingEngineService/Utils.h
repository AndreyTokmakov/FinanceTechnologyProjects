/**============================================================================
Name        : Utils.h
Created on  : 23.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utils.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_UTILS_H
#define FINANCETECHNOLOGYPROJECTS_UTILS_H

#include <string>
#include <cstdint>

namespace Utils
{
    int64_t priceToLong(const std::string& strPrice);
    int64_t priceToLong(const char* strPrice, uint32_t size);
}

#endif //FINANCETECHNOLOGYPROJECTS_UTILS_H
