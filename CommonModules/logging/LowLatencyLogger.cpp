/**============================================================================
Name        : LowLatencyLogger.cpp
Created on  : 13.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : LowLatencyLogger.cpp
============================================================================**/

#include "LowLatencyLogger.h"

#include <iostream>
#include <string_view>
#include <chrono>

namespace LowLatencyLogger
{
    /** SILENT <-- FATAL <-- ERROR <-- WARNING <-- INFO <-- DEBUG <-- TRACE
     *
     * The higher the logging level is set, the fewer messages will be logged
     * Examples:
     *  1. The SILENT level is set -> nothing is printed
     *  2. The TRACE  level is set -> all TRACE - FATAL levels are logged
     *  3. The INFO   level is set -> levels with INFO - FATAL are logged
    **/
    enum class Level: uint8_t
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        SILENT
    };

    std::string toString(const Level level)
    {
        switch (level) {
            case Level::TRACE: return std::string { "TRACE" };
            case Level::DEBUG: return std::string { "DEBUG" };
            case Level::INFO:  return std::string { "INFO" };
            case Level::WARN:  return std::string { "WARN" };
            case Level::ERROR: return std::string { "ERROR" };
            case Level::FATAL: return std::string { "FATAL" };
            case Level::SILENT:return std::string { "SILENT" };
            default: return std::string { "Unknown" };
        }
    }

    struct LongEntry
    {
        // TODO: Sequence number
        std::chrono::system_clock::time_point timestamp { std::chrono::system_clock::now() };
        std::string text ;

        explicit LongEntry(std::string txt): text { std::move(txt) } {
        }
    };


    struct LogHandler
    {
        virtual void handleEntry(const LongEntry&) const noexcept = 0;
        virtual ~LogHandler() = default;
    };


    template<typename T>
    struct RingBuffer
    {
        using size_type = size_t;

        template<typename Type>
        using collection_type = std::vector<Type>;

        size_type idxRead { 0 };
        size_type idxWrite { 0 };
        collection_type<T> buffer;

        explicit RingBuffer(size_t size) {
            buffer.resize(size);
        }
    };
}


// TODO:
//  1. need to have DEFINES to enable/disable Logging based of Logger level ???
//  2. Each thread stores elements int THREAD LOCAL RING BUFFER

// TODO:
//  1. RingBuffer ?

// TODO:
//  1. Что с точки зрения производительности эффективнее
//     При каждом добавлении LongEntry проверять не достигли мы конца RingBuffer и ставить счетчик --> 0
//     или же сделать его uint16_t и просто использовать переполнее?

// TODO:
//  - Need to check multiple values of Level with once IF condition ??

void LowLatencyLogger::TestAll()
{
    // uint64_t size = std::numeric_limits<uint16_t>::max() * sizeof(LongEntry);
    // std::cout << size << std::endl;


    uint16_t counter = std::numeric_limits<uint16_t>::max();

    std::cout << counter << std::endl;
    std::cout << ++counter << std::endl;

}