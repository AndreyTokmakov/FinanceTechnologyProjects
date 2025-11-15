/**============================================================================
Name        : main.cpp
Created on  : 12.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "queues/LockFreeQueue.h"
#include "logging/LowLatencyLogger.h"
#include "logging/LowLatencyLoggerDebug.h"
#include "locking/SpinLock.h"
#include "cryptography/Certificates.h"
#include "consumers/UnixDomainSocketConsumer.h"

#include "buffer/Buffer.hpp"
#include "ring_buffer/RingBuffer_SPSC.h"
#include "ring_buffer_ex/RingBufferEx.h"
#include "ring_buffer_two/RingBuffer_SPSC_Two.hpp"
#include "ring_buffer_spsc_commit/RingBuffer_SpSc_Commit.hpp"
#include "ring_buffer_spsc_commit_buffer/RingBuffer_SpSc_Commit_Buffer.hpp"
#include "ring_buffer_spsc_pricer/RingBuffer_SpSc_Pricer.hpp"


// TODO:
//  -  Low Latency logger



int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // LockFreeQueue::TestAll();

    // LowLatencyLogger::TestAll();
    // LowLatencyLoggerDebug::TestAll();

    // RingBuffer_SPSC::TestAll();
    // RingBufferEx::TestAll();
    // RingBuffer_SPSC_Two::TestAll();
    // RingBuffer_SpSc_Commit::TestAll();
    // RingBuffer_SpSc_Commit_Buffer::TestAll();
    RingBuffer_SpSc_Pricer::TestAll();

    // SpinLock::TestAll();

    // Certificates::TestAll();

    // UnixDomainSocketConsumer::TestAll();

    // buffer::TestAll();

    return EXIT_SUCCESS;
}
