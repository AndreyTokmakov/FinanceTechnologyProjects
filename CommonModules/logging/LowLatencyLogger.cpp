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
#include <shared_mutex>
#include <thread>
#include <future>

#include "PerfUtilities.h"

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
    };

    template<typename T>
    struct RingBufferAtomic1
    {
        using size_type = size_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        std::atomic<size_type> idxRead { 0 };
        std::atomic<size_type> idxWrite { 0 };
        std::atomic<bool> overflow {false };
        collection_type buffer {};

        explicit RingBufferAtomic1(size_t size): idxRead { 0 }, idxWrite { 0 }, overflow { false } {
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
    };


    template<typename T>
    struct RingBufferAtomic2
    {
        using size_type = size_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        std::atomic<size_type> idxRead { 0 };
        std::atomic<size_type> idxWrite { 0 };

        // TODO: test with boolean flag?
        std::atomic<bool> overflow {false };

        collection_type buffer {};

        explicit RingBufferAtomic2(size_t size): idxRead { 0 }, idxWrite { 0 }, overflow { false } {
            buffer.resize(size);
        }

        void put(value_type&& value)
        {
            if (idxWrite.load(std::memory_order::relaxed) == buffer.size()) {
                idxWrite.store(0, std::memory_order::relaxed);
                overflow.store(true, std::memory_order::relaxed);
            }
            if (overflow.load(std::memory_order::relaxed) &&
                idxWrite.load(std::memory_order::relaxed) == idxRead.load(std::memory_order::relaxed)) {
                idxRead.fetch_add(1, std::memory_order::relaxed);
            }
            buffer[idxWrite.fetch_add(1, std::memory_order::relaxed)] = std::move(value);
        }

        bool get(value_type& value)
        {
            if (!overflow.load(std::memory_order::relaxed) &&
                idxWrite.load(std::memory_order::relaxed) == idxRead.load(std::memory_order::relaxed)) {
                return false;
            }

            if (idxRead.load(std::memory_order::relaxed) == buffer.size()) {
                idxRead.store(0, std::memory_order::relaxed);
                overflow.store(false, std::memory_order::relaxed);
            }

            value = std::move(buffer[idxRead.fetch_add(1, std::memory_order::relaxed)]);
            return true;
        }
    };
}

namespace LowLatencyLogger
{
    // TODO: Check for copies
    //      - when using RingBuffer::put()
    //      - when using RingBuffer::get()
    struct LongEntry
    {
        std::chrono::system_clock::time_point timestamp { std::chrono::system_clock::now() };
        std::string text;
        Level level { Level::INFO };

        LongEntry() = default;

        explicit LongEntry(std::string txt):
                text { std::move(txt) } {
        }

        LongEntry(const Level lvl, std::string txt):
            text { std::move(txt) }, level { lvl } {
        }

        bool operator<(const LongEntry& rhs) const noexcept {
            return timestamp < rhs.timestamp;
        }
    };


    struct ILogHandler
    {
        virtual void handleEntry(const LongEntry&) noexcept = 0;
        virtual ~ILogHandler() = default;
    };


    // TODO: Use std::list<T> or std::deque<T>
    struct Logger
    {
        constexpr static inline int32_t logBundleSize { 1024 };
        constexpr static inline int32_t consumeBlockSize { 1000 };
        constexpr static inline std::chrono::milliseconds logsConsumerTimeout {
                std::chrono::milliseconds(1UL)
        };
        constexpr static inline std::chrono::milliseconds logsHandleTimeout {
                std::chrono::milliseconds(1UL)
        };

        // using LogBundle = Common::RingBuffer<LongEntry>;
        // using LogBundle = Common::RingBufferAtomic1<LongEntry>;
        using LogBundle = Common::RingBufferAtomic2<LongEntry>;

        mutable std::shared_mutex mutex;
        std::list<LogBundle> threadLogs;

        mutable std::mutex mtxLogs2Handle;
        std::list<std::vector<Logger::LogBundle::value_type>> logsToHandle;

        std::vector<std::shared_ptr<ILogHandler>> handlers;

        std::jthread logProcessor;
        std::jthread logHandler;
        std::stop_source stopSource;

        Logger()
        {
            logProcessor = std::jthread(&Logger::consumeLogs, this, stopSource);
            logHandler = std::jthread(&Logger::handleLogs, this, stopSource);
        }

        bool addHandler(const std::shared_ptr<ILogHandler>& handler)
        {
            handlers.push_back(handler);
            return true;
        }

        [[nodiscard]]
        LogBundle& getThreadLocalLogs()
        {
            std::lock_guard<std::shared_mutex> lock { mutex };
            return threadLogs.emplace_back(logBundleSize);
        }

        void log(std::string&& info)
        {
            // TODO: Check if compiler adds a 'if static variable created' check in the Assembly code generated
            static thread_local LogBundle& bundle = getThreadLocalLogs();
            bundle.put(LongEntry{std::move(info)});
        }

        void log(const Level level, std::string&& info)
        {
            // TODO: Check if compiler adds a 'if static variable created' check in the Assembly code generated
            static thread_local LogBundle& bundle = getThreadLocalLogs();
            bundle.put(LongEntry{level, std::move(info)});
        }

        void consumeLogs(const std::stop_source& source)
        {
            Logger::LogBundle::value_type logEntry;
            std::vector<Logger::LogBundle::value_type> logsLocal;
            while (!source.stop_requested())
            {
                std::this_thread::sleep_for(logsConsumerTimeout);

                {   /// Loop thought all Logger::LogBundle in the threadLogs (each Logger::LogBundle created per on thread)
                    /// and move all logs from each LogBundle into logsLocal collection
                    std::shared_lock<std::shared_mutex> lock { mutex };
                    for (Logger::LogBundle & logBundle: threadLogs)
                    {
                        for (int32_t n = 0; n < consumeBlockSize && logBundle.get(logEntry); ++n) {
                            logsLocal.push_back(std::move(logEntry));
                        }
                    }
                }
                if (!logsLocal.empty())
                {
                    std::lock_guard<std::mutex> lock { mtxLogs2Handle };
                    logsToHandle.emplace_back().swap(logsLocal);
                }
            }
        }

        void handleLogs(const std::stop_source& source)
        {
            std::vector<Logger::LogBundle::value_type> logsLocal;
            while (!source.stop_requested())
            {
                std::this_thread::sleep_for(logsHandleTimeout);

                {   /// Trying to get first entry (std::vector or Logs) into the current thread
                    /// Swap()-ing first entry from logsToHandle into the local logs storage
                    std::lock_guard<std::mutex> lock { mtxLogs2Handle };
                    if (logsToHandle.empty()) {
                        continue;
                    }
                    logsLocal.swap(logsToHandle.front());
                    logsToHandle.pop_front();
                }

                /// Sort logs based on the Timestamp
                std::sort(logsLocal.begin(), logsLocal.end());

                for (const auto& entry: logsLocal)
                {
                    for (const std::shared_ptr<ILogHandler>& handler: handlers) {
                        handler->handleEntry(entry);
                    }
                }
                logsLocal.clear();
            }
        }
    };

    struct StdOutLogHandler final : public ILogHandler
    {
        void handleEntry(const LongEntry& entry) noexcept override
        {
            // TODO: std::format
            std::cout << Utils::getCurrentTime(entry.timestamp) << " [" << std::left <<
                std::setw(6) << toString(entry.level) << "] " << entry.text << std::endl;
        }
    };

    struct LogHandlerCounter final : public ILogHandler
    {
        uint64_t logsProcessed { 0 };

        void handleEntry(const LongEntry& entry) noexcept override
        {
            ++logsProcessed;
            if (0 == logsProcessed % 1'000'000)
                std::cout << logsProcessed << std::endl;
        }
    };

    struct LogHandlerSink final : public ILogHandler
    {
        void handleEntry(const LongEntry& entry) noexcept override {
        }
    };

    void testLogs()
    {
        Logger logger;

        auto producer = [&logger](std::string text, std::chrono::duration<double> duration) {
            while (true) {
                for (int i = 0; i < 10; ++i) {
                    logger.log(std::string (text));
                }
                std::this_thread::sleep_for(duration);
            }
        };

        auto f1 = std::async(producer, "log_1", std::chrono::seconds (2U));
        auto f2 = std::async(producer, "log_2", std::chrono::seconds (2U));

        f1.wait();
        f2.wait();
    }



    void testLogHandler()
    {
        Logger logger;
        std::shared_ptr<ILogHandler> printer { std::make_shared<StdOutLogHandler>() };
        logger.addHandler(printer);

        auto producer = [&logger](const std::chrono::duration<double> duration,
                                  const uint64_t logsToSend)
        {
            for (uint64_t n = 0; n < logsToSend; ++n)
            {
                logger.log(Level::INFO, "Message_" + std::to_string (n));
                logger.log(Level::DEBUG, "Message_" + std::to_string (n));
                logger.log(Level::WARN, "Message_" + std::to_string (n));
                logger.log(Level::SILENT, "Message_" + std::to_string (n));


                std::this_thread::sleep_for(duration);
            }
        };

        auto f1 = std::async(producer, std::chrono::milliseconds (10U), 100);
        f1.wait();
        std::cout << "Producer done!\n";
    }

    void loadTest()
    {
        Logger logger;
        std::shared_ptr<ILogHandler> logCounter { std::make_shared<LogHandlerCounter>() };
        logger.addHandler(logCounter);

        auto producer = [&logger](std::string text,
                                  std::chrono::duration<double> duration,
                                  uint64_t logsToSend)
        {
            constexpr int32_t N = 25;
            for (uint64_t n = 0; n < logsToSend / N; ++n)
            {
                for (int i = 0; i < N; ++i) {
                    logger.log(std::string (text));
                }
                std::this_thread::sleep_for(duration);
            }
        };

        std::vector<std::future<void>> producers;
        for (int i = 0; i < 8; ++i) {
            producers.emplace_back(std::async(producer, "log_" + std::to_string(i), std::chrono::nanoseconds (1U), 10'000'000));
        }
        for (const auto& F: producers) {
            F.wait();
        }

        std::cout << "Done\n";
    }

    void loadTest_Sink()
    {
        Logger logger;
        std::shared_ptr<ILogHandler> logCounter { std::make_shared<LogHandlerSink>() };
        logger.addHandler(logCounter);

        auto producer = [&logger](std::string text,
                                  std::chrono::duration<double> duration,
                                  uint64_t logsToSend)
        {
            constexpr int32_t N = 25;
            for (uint64_t n = 0; n < logsToSend / N; ++n)
            {
                for (int i = 0; i < N; ++i) {
                    logger.log(std::string (text));
                }
                std::this_thread::sleep_for(duration);
            }
        };

        PerfUtilities::ScopedTimer timer { "loadTest_Sink"};
        std::vector<std::future<void>> producers;
        for (int i = 0; i < 8; ++i) {
            producers.emplace_back(std::async(producer, "log_" + std::to_string(i),
                                              std::chrono::nanoseconds (1U), 1'000'000));
        }
        for (const auto& F: producers) {
            F.wait();
        }

        std::cout << "Done\n";
        timer.~ScopedTimer();
    }
}


namespace Experiments
{
    struct String
    {
        std::string str;

        String()
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        explicit String(const char* s) : str {s}
        {
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";
        }

        ~String()
        {
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";
        }

        String(const String&)
        {
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";
        }

        String(String&&) noexcept
        {
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";
        }

        String& operator=(const String& rhs)
        {
            str = rhs.str;
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";;
            return *this;
        }

        String& operator=(String&& rhs) noexcept
        {
            str = std::move(rhs.str);
            std::cout << __PRETTY_FUNCTION__ << "(" << str << ")\n";
            return *this;
        }
    };

    struct Entry
    {
        String str1;
        String str2 {"Default"};

        explicit Entry(const char* s1): str1 { s1 }{}
        Entry(const char* s1, const char* s2): str1 { s1 }, str2 { s2 }{}
    };

    void demoTest()
    {
        // Entry entry1 = Entry { "111", "222"};
        Entry entry1 = Entry { "111"};
        Entry entry2 = std::move(entry1);
    }

    using namespace LowLatencyLogger;

    struct TestLogger
    {
        Common::RingBuffer<LongEntry> buffer { 1024 };
        void log(std::string&& info)
        {
            buffer.put(LongEntry {std::move(info)});
        }
    };

    struct TestLoggerEx
    {
        constexpr static inline int32_t logBundleSize { 1024 };
        using LogBundle = Common::RingBuffer<LongEntry>;

        mutable std::mutex mutex;
        std::list<LogBundle> threadLogs;

        [[nodiscard]]
        LogBundle& getThreadLocalLogs()
        {
            std::lock_guard<std::mutex> lock { mutex };
            return threadLogs.emplace_back(logBundleSize);
        }

        void log(std::string&& info)
        {
            // TODO: Check if compiler adds a 'if static variable created' check in the Assembly code generated
            static thread_local LogBundle& bundle = getThreadLocalLogs();
            bundle.put(LongEntry{std::move(info)});
        }
    };

    void Logger_RingBuffer_PerfTest()
    {
        TestLogger logger;
        PerfUtilities::ScopedTimer timer { "Logger_RingBuffer_PerfTest"};
        for (uint64_t i = 0; i < 30'000'000; ++i)
        {
            logger.log("12323");
            //std::this_thread::sleep_for(std::chrono::nanoseconds (1U));
        }
    }

    void LoggerEx_RingBuffer_PerfTest()
    {
        TestLoggerEx logger;
        PerfUtilities::ScopedTimer timer { "LoggerEx_RingBuffer_PerfTest"};
        for (uint64_t i = 0; i < 30'000'000; ++i)
        {
            logger.log("12323");
            //std::this_thread::sleep_for(std::chrono::nanoseconds (1U));
        }
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

    // LowLatencyLogger::testLogs();
    LowLatencyLogger::testLogHandler();
    // LowLatencyLogger::loadTest();
    // LowLatencyLogger::loadTest_Sink();

    // Experiments::demoTest();
    // Experiments::Logger_RingBuffer_PerfTest();
    // Experiments::LoggerEx_RingBuffer_PerfTest();
}