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
#include <list>

#include <mutex>
#include <thread>
#include <future>


namespace
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

    [[nodiscard]]
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
}

namespace Utils
{
    using namespace std::chrono;
    constexpr std::string_view FORMAT { "%d-%02d-%02d %02d:%02d:%02d.%06ld" };

    [[nodiscard]]
    std::string getCurrentTime(const time_point<system_clock>& timestamp = system_clock::now()) noexcept
    {
        const time_t time { std::chrono::system_clock::to_time_t(timestamp) };
        std::tm tm {};
        ::localtime_r(&time, &tm);

        std::string buffer(64, '\0');
        const int32_t size = std::sprintf(buffer.data(), FORMAT.data(),
                                          tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                                          duration_cast<microseconds>(timestamp - time_point_cast<seconds>(timestamp)).count());
        buffer.resize(size);
        buffer.shrink_to_fit();
        return buffer;
    }
}

namespace Common
{
    template<typename T>
    struct RingBuffer
    {
        using size_type = size_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        size_type idxRead { 0 };
        size_type idxWrite { 0 };
        collection_type buffer {};
        bool overflow { false };
        // TODO: add consumer lost counter ???

        explicit RingBuffer(size_t size): idxRead { 0 }, idxWrite { 0 }, overflow { false } {
            buffer.resize(size);
        }

        void put(value_type&& value)
        {
            if (idxWrite == buffer.size()) {
                idxWrite = 0;
                overflow = true;
            }
            if (overflow && idxWrite == idxRead) {
                ++idxRead;
            }
            buffer[idxWrite++] = std::move(value);
        }

        bool get(value_type& value)
        {
            if (!overflow && idxWrite == idxRead) {
                return false;
            }

            if (idxRead == buffer.size()) {
                idxRead = 0;
                overflow = false;
            }

            value = std::move(buffer[idxRead++]);
            return true;
        }

        /*
        size_type size() const noexcept {
            return idxWrite - idxRead;
        }*/
    };
}

namespace LowLatencyLogger
{

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


    struct Logger
    {
        using LogBundle = std::vector<std::string>;

        static inline std::mutex mutex;
        static inline std::vector<std::vector<std::string>*> threadLogs;

        static inline thread_local std::vector<std::string> logs = []
        {
            std::lock_guard<std::mutex> lock { mutex };
            std::vector<std::string> logs(1024);
            threadLogs.push_back(&logs);
            return logs;
        }();


        void log(std::string&& info)
        {
            logs.push_back(std::move(info));
        }
    };


    // TODO: Use std::list<T> or std::deque<T>
    // TODO: ThreadLocalLogBundle --> RingBuffer[SIZE]
    struct LoggerEx
    {
        using LogBundle = Common::RingBuffer<std::string>;

        constexpr static inline size_t logBundleSize { 128 };

        mutable std::mutex mutex;
        std::list<LogBundle> threadLogs;

        std::jthread logProcessor;
        std::stop_source stopSource;

        LoggerEx()
        {
            logProcessor = std::jthread(&LoggerEx::processor, this, stopSource);
        }

        [[nodiscard]]
        LogBundle& getThreadLocalLogs()
        {
            std::lock_guard<std::mutex> lock { mutex };
            return threadLogs.emplace_back(logBundleSize);
        }

        void log(std::string&& info)
        {
            static thread_local LogBundle& bundle = getThreadLocalLogs();
            bundle.put(std::move(info));
        }

        // TODO: Rename
        void processor(const std::stop_source& source)
        {
            constexpr int32_t maxRecords { 10 };

            // TODO
            //  1. Get N records from each LogBundle
            //  2. Store in the local collection
            //  3. Sort by Timestamp

            LoggerEx::LogBundle::value_type logEntry;
            while (!source.stop_requested())
            {
                // TODO: Lock 'threadLogs'
                std::this_thread::sleep_for(std::chrono::seconds (1U));
                for (LoggerEx::LogBundle& logBundle : threadLogs)
                {
                    for (int32_t n = 0; n < maxRecords && logBundle.get(logEntry); ++n) {
                        std::cout  << logEntry << std::endl;
                    }
                }
            }
        }
    };

    void testLogs()
    {
        // Logger logger;
        LoggerEx logger;

        auto f1 = std::async([&] {
            for (int i = 0; i < 10; ++i) {
                logger.log("log_1");
            }
        });
        auto f2 = std::async([&] {
            for (int i = 0; i < 10; ++i) {
                logger.log("log_2");
            }
        });

        f1.wait();
        f2.wait();

        /*
        LoggerEx::LogBundle::value_type value;
        for (LoggerEx::LogBundle& logBundle : logger.threadLogs)
        {
            bool result { true };
            while (result) {
                result = logBundle.get(value);
                std::cout << value << std::endl;
            }
            std::cout << Utils::getCurrentTime() << std::endl;
        }*/
    }
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

    LowLatencyLogger::testLogs();
}