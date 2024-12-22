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
#include <fstream>
#include <filesystem>

#include <mutex>
#include <shared_mutex>
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

        std::atomic<size_type> idxRead { 0 };
        std::atomic<size_type> idxWrite { 0 };
        std::atomic<bool> overflow {false };
        collection_type buffer {};

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

    class Logger
    {
        constexpr static inline int32_t logBundleSize { 1024 };
        constexpr static inline int32_t consumeBlockSize { 1000 };
        constexpr static inline std::chrono::milliseconds logsConsumerTimeout {
                std::chrono::milliseconds(1UL)
        };
        constexpr static inline std::chrono::milliseconds logsHandleTimeout {
                std::chrono::milliseconds(1UL)
        };

        using LogBundle = Common::RingBuffer<LongEntry>;

        mutable std::shared_mutex mutex;
        std::list<LogBundle> threadLogs;

        mutable std::mutex mtxLogs2Handle;
        std::list<std::vector<Logger::LogBundle::value_type>> logsToHandle;

        std::vector<std::shared_ptr<ILogHandler>> handlers;

        std::jthread logProcessor;
        std::jthread logHandler;
        std::stop_source stopSource;

        [[nodiscard]]
        LogBundle &getThreadLocalLogs()
        {
            std::lock_guard<std::shared_mutex> lock{mutex};
            return threadLogs.emplace_back(logBundleSize);
        }

        template<class Str>
        void log(const Level level, Str&& info)
        {
            static thread_local LogBundle &bundle = getThreadLocalLogs();
            bundle.put(LongEntry{level, std::forward<Str>(info)});
        }

    public:

        Logger()
        {
            logProcessor = std::jthread(&Logger::consumeLogs, this, stopSource);
            logHandler = std::jthread(&Logger::handleLogs, this, stopSource);
        }

        bool addHandler(const std::shared_ptr<ILogHandler> &handler)
        {
            handlers.push_back(handler);
            return true;
        }

        void trace(std::string&& text) {
            log(Level::TRACE, std::move(text));
        }

        void debug(std::string&& text) {
            log(Level::DEBUG, std::move(text));
        }

        void info(std::string&& text) {
            log(Level::INFO, std::move(text));
        }

        void warning(std::string&& text) {
            log(Level::WARN, std::move(text));
        }

        void error(std::string&& text) {
            log(Level::ERROR, std::move(text));
        }

        void fatal(std::string&& text) {
            log(Level::FATAL, std::move(text));
        }

    private:

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
}


namespace Handlers
{
    using namespace LowLatencyLogger;

    struct ConsoleLogHandler final : public ILogHandler
    {
        static inline constexpr std::string_view format {"{:} [{:6s}] {:}\n" };

        // TODO: Move Level --> HandlerBase
        Level level { Level::INFO };

        explicit ConsoleLogHandler(const Level level = Level::INFO) : level { level } {
        }

        void handleEntry(const LongEntry &entry) noexcept override
        {
            if (level > entry.level) {
                return;
            }
            std::cout << std::format(format,
                          Utils::getCurrentTime(entry.timestamp), toString(entry.level), entry.text);
        }
    };

    struct FileLogHandler final : public ILogHandler
    {
        static inline constexpr size_t bundleSizeMax { 1024 };
        static inline constexpr std::string_view format { "{:} [{:6s}] {:}\n" };

        // TODO: Move Level --> HandlerBase
        Level level { Level::INFO };
        std::filesystem::path filePath {};
        std::string buffer;

        explicit FileLogHandler(std::filesystem::path logFilePath, const Level level = Level::INFO) :
                level { level }, filePath { std::move( logFilePath) }
        {
            buffer.reserve(1024);
        }

        void handleEntry(const LongEntry& entry) noexcept override
        {
            if (level > entry.level) {
                return;
            }

            std::format_to(std::back_inserter(buffer), format,
                           Utils::getCurrentTime(entry.timestamp),
                           toString(entry.level),
                           entry.text);
            if (buffer.size() > bundleSizeMax)
            {
                if (std::ofstream file(filePath,  std::ios_base::app); file.is_open() && file.good())
                {
                    file.write(buffer.data(), std::ssize(buffer));
                }
                buffer.clear();
            }
        }
    };
}


namespace Tests
{
    using namespace LowLatencyLogger;

    void test()
    {
        Logger logger;
        const std::shared_ptr<ILogHandler> printer { std::make_shared<Handlers::ConsoleLogHandler>() };
        logger.addHandler(printer);

        auto producer = [&logger](const std::chrono::duration<double> duration,
                                  const uint64_t logsToSend)
        {
            for (uint64_t n = 0; n < logsToSend; ++n)
            {
                logger.info("Message_" + std::to_string (n));
                logger.debug("Message_" + std::to_string (n));
                logger.warning("Message_" + std::to_string (n));
                logger.error("Message_" + std::to_string (n));

                std::this_thread::sleep_for(duration);
            }
        };

        auto f1 = std::async(producer, std::chrono::milliseconds (10U), 100);
        f1.wait();
        std::cout << "Producer done!\n";
    }
}


// TODO:
//  - need to have DEFINES to enable/disable Logging based of Logger level ???
//  + Each thread stores elements int THREAD LOCAL RING BUFFER
//  + RingBuffer

// TODO:
//  1. Что с точки зрения производительности эффективнее
//     При каждом добавлении LongEntry проверять не достигли мы конца RingBuffer и ставить счетчик --> 0
//     или же сделать его uint16_t и просто использовать переполнее?

// TODO:
//  - Need to check multiple values of Level with once IF condition ??

void LowLatencyLogger::TestAll()
{
    Tests::test();
}