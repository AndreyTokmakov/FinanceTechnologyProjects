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

namespace Common
{
    using Pair = std::string;
    using FlatBuffer = boost::beast::flat_buffer;
    using FlatBufferQueue = Common::BlockingQueuePtr<FlatBuffer>;

}

#endif //FINANCETECHNOLOGYPROJECTS_TYPES_H

