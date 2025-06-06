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

    // TODO: Price -> pair<int, int> ?
    using Price     = double;

    // TODO: Price -> pair<int, int> ?
    using Quantity  = double;

    using Timestamp = int64_t;
    using Number    = int64_t;

    // TODO: ---> implement StaticString (stack only)
    using String = std::string;

}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_H

