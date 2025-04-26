/**============================================================================
Name        : Types.h
Created on  : 23.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Types.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TYPES_H
#define FINANCETECHNOLOGYPROJECTS_TYPES_H

#include <string>
#include "BlockingQueuePtr.h"
#include <boost/beast/core.hpp>

namespace common
{
    using Pair = std::string;
    using FlatBuffer = boost::beast::flat_buffer;
    using FlatBufferQueue = common::BlockingQueuePtr<FlatBuffer>;


    using price_t = double;
    using quantity_t = double;
    using time_t = int64_t;
    using number_t = int64_t;
}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_H

