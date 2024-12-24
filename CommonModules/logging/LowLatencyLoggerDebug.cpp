/**============================================================================
Name        : LowLatencyLoggerDebug.cpp
Created on  : 21.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : LowLatencyLoggerDebug.cpp
============================================================================**/

#include "LowLatencyLoggerDebug.h"

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
#include <utility>

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

namespace LowLatencyLoggerDebug::Utils
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

namespace LowLatencyLoggerDebug::Common
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

namespace LowLatencyLoggerDebug
{
    // TODO: Check for copies
    //      - when using RingBuffer::put()
    //      - when using RingBuffer::get()
    struct LongEntry
    {
        std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
        std::string text;
        Level level { Level::INFO };

        LongEntry() = default;

        explicit LongEntry(std::string txt) :
                text{std::move(txt)} {
        }

        LongEntry(const Level lvl, std::string txt) :
                text{std::move(txt)}, level{lvl} {
        }

        bool operator<(const LongEntry &rhs) const noexcept {
            return timestamp < rhs.timestamp;
        }
    };


    struct ILogHandler
    {
        virtual void handleEntry(const LongEntry &) noexcept = 0;
        virtual ~ILogHandler() = default;
    };


    // TODO: Use std::list<T> or std::deque<T>
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

        // using LogBundle = Common::RingBuffer<LongEntry>;
        using LogBundle = Common::RingBufferAtomic1<LongEntry>;
        // using LogBundle = Common::RingBufferAtomic2<LongEntry>;

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

        void consumeLogs(const std::stop_source &source)
        {
            Logger::LogBundle::value_type logEntry;
            std::vector<Logger::LogBundle::value_type> logsLocal;
            while (!source.stop_requested()) {
                std::this_thread::sleep_for(logsConsumerTimeout);

                {   /// Loop thought all Logger::LogBundle in the threadLogs (each Logger::LogBundle created per on thread)
                    /// and move all logs from each LogBundle into logsLocal collection
                    std::shared_lock<std::shared_mutex> lock{mutex};
                    for (Logger::LogBundle &logBundle: threadLogs) {
                        for (int32_t n = 0; n < consumeBlockSize && logBundle.get(logEntry); ++n) {
                            logsLocal.push_back(std::move(logEntry));
                        }
                    }
                }
                if (!logsLocal.empty()) {
                    std::lock_guard<std::mutex> lock{mtxLogs2Handle};
                    logsToHandle.emplace_back().swap(logsLocal);
                }
            }
        }

        void handleLogs(const std::stop_source &source)
        {
            std::vector<Logger::LogBundle::value_type> logsLocal;
            while (!source.stop_requested()) {
                std::this_thread::sleep_for(logsHandleTimeout);

                {   /// Trying to get first entry (std::vector or Logs) into the current thread
                    /// Swap()-ing first entry from logsToHandle into the local logs storage
                    std::lock_guard<std::mutex> lock{mtxLogs2Handle};
                    if (logsToHandle.empty()) {
                        continue;
                    }
                    logsLocal.swap(logsToHandle.front());
                    logsToHandle.pop_front();
                }

                /// Sort logs based on the Timestamp
                std::sort(logsLocal.begin(), logsLocal.end());

                for (const auto &entry: logsLocal) {
                    for (const std::shared_ptr<ILogHandler> &handler: handlers) {
                        handler->handleEntry(entry);
                    }
                }
                logsLocal.clear();
            }
        }
    };
}

namespace LowLatencyLoggerDebug::Handlers
{
    using namespace LowLatencyLoggerDebug;

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

        void handleEntry(const LongEntry&) noexcept override
        {
            ++logsProcessed;
            if (0 == logsProcessed % 1'000'000)
                std::cout << logsProcessed << std::endl;
        }
    };

    struct LogHandlerSink final : public ILogHandler
    {
        void handleEntry(const LongEntry&) noexcept override {
        }
    };
}



namespace Tests
{
    using namespace LowLatencyLoggerDebug;
    using namespace LowLatencyLoggerDebug::Handlers;

    void testLogs()
    {
        Logger logger;

        auto producer = [&logger](std::string text, std::chrono::duration<double> duration) {
            while (true) {
                for (int i = 0; i < 10; ++i) {
                    logger.info(std::string (text));
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
                logger.info("Message_" + std::to_string (n));
                logger.debug( "Message_" + std::to_string (n));
                logger.warning( "Message_" + std::to_string (n));

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
                    logger.info(std::string (text));
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
                    logger.info(std::string (text));
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

        timer.~ScopedTimer();
    }
}


namespace LowLatencyLoggerDebug::Experiments
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

    void func3(String&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        {
            String s = std::move(str);
        }
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    void func2(String&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func3(std::move(str));
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    void func1(String&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func2(std::move(str));
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    void Move_String_Test()
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func1(String {"TEST"});
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    template<typename Str>
    void func3_fwd(int type,Str&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        {
            String s = std::forward<Str>(str);
        }
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    template<typename Str>
    void func2_fwd(int type,Str&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func3_fwd(type, std::forward<Str>(str));
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    template<typename Str>
    void func1_fwd(int type, Str&& str)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func2_fwd(type, std::forward<Str>(str));
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }

    void Forward_String_Test()
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
        func1_fwd(1 , String {"TEST"});
        std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    }


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

namespace LowLatencyLoggerDebug::HandlersTests
{
    using namespace LowLatencyLoggerDebug;

    struct ConsoleLogHandler final : public ILogHandler
    {
        static inline constexpr std::string_view format{"{:} [{:6s}] {:}\n"};

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

    // TODO:
    //  - Rotate log
    //  - FileName format
    struct FileRotatedLogHandler final : public ILogHandler
    {
        static inline constexpr size_t bundleSizeMax { 1024 };
        static inline constexpr std::string_view format { "{:} [{:6s}] {:}\n" };

        // TODO: Move Level --> HandlerBase
        Level level { Level::INFO };
        std::filesystem::path filePath {};
        std::string buffer;
        // std::ofstream file {};

        explicit FileRotatedLogHandler(std::filesystem::path logFilePath, const Level level = Level::INFO) :
                level { level }, filePath { std::move( logFilePath) }
        {
            //file.open(filePath.c_str(), std::ios_base::app);
            buffer.reserve(1024);
        }

        ~FileRotatedLogHandler() override
        {
            //file.close();
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

    void writeLogs()
    {
        const std::shared_ptr<ILogHandler> fileHandler {
                std::make_shared<FileLogHandler>(R"(/tmp/trace.log)", Level::INFO) };
        const std::shared_ptr<ILogHandler> consoleHandler {
                std::make_shared<ConsoleLogHandler>(Level::INFO) };

        Logger logger;
        logger.addHandler(fileHandler);
        logger.addHandler(consoleHandler);

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
    }
}


namespace CollectionExperiments
{
    struct TestLongEntry
    {
        std::chrono::system_clock::time_point timestamp { std::chrono::system_clock::now() };
        std::string text;
        Level level { Level::INFO };

        TestLongEntry() = default;

        explicit TestLongEntry(std::string txt) :
                text{std::move(txt)} {
        }

        TestLongEntry(const Level lvl, std::string txt) :
                text { std::move(txt) }, level { lvl } {
        }

        bool operator<(const TestLongEntry &rhs) const noexcept {
            return timestamp < rhs.timestamp;
        }
    };



    void List_Concurrent_Modification()
    {
        using Bundle = std::vector<TestLongEntry>;
        std::list<Bundle> logsToHandle;
        std::mutex mtx;

        auto produce = [&logsToHandle, &mtx](const uint64_t iterCount)
        {
            Bundle logsLocal;
            for (uint64_t n = 0; n < iterCount; ++n)
            {
                for (int i = 0; i < 128; ++i)
                    logsLocal.emplace_back( "2222222222222222222222222222222222222222222222222");

                std::lock_guard<std::mutex> lock { mtx };
                logsToHandle.emplace_back().swap(logsLocal);
            }
        };

        auto consume = [&logsToHandle, &mtx](const uint64_t iterCount)
        {
            Bundle logsLocal;
            for (uint64_t n = 0; n < iterCount; ++n)
            {
                std::lock_guard<std::mutex> lock { mtx };
                if (logsToHandle.empty()) {
                    continue;
                }

                logsLocal.swap(logsToHandle.front());
                logsToHandle.pop_front();
            }
        };

        auto producer = std::async(produce, 1'000'000);
        auto consumer = std::async(consume, 1'000'000);

        producer.wait();
        consumer.wait();

        std::cout << "Done\n";
    }
}

namespace SimplifiedLogger
{
    struct Logger
    {
        constexpr static inline int32_t logBundleSize { 1024 };
        constexpr static inline int32_t consumeBlockSize { 1024 };
        constexpr static inline std::chrono::milliseconds logsConsumerTimeout { std::chrono::milliseconds(1UL) };
        constexpr static inline std::chrono::milliseconds logsHandleTimeout { std::chrono::milliseconds(1UL)};

        using LogBundle = LowLatencyLoggerDebug::Common::RingBufferAtomic1<std::string>;

        mutable std::shared_mutex mutex;
        std::list<LogBundle> threadLogs;

        mutable std::mutex mtxLogs2Handle;
        std::list<std::vector<Logger::LogBundle::value_type>> logsToHandle;

        std::jthread logProcessor;
        std::jthread logHandler;
        std::stop_source stopSource;

        [[nodiscard]]
        LogBundle &getThreadLocalLogs()
        {
            std::lock_guard<std::shared_mutex> lock { mutex };
            return threadLogs.emplace_back(logBundleSize);
        }

        Logger()
        {
            logProcessor = std::jthread(&Logger::consumeLogs, this, stopSource);
            logHandler = std::jthread(&Logger::handleLogs, this, stopSource);
        }

        template<class Str>
        void log(Str&& info, const Level level = Level::INFO)
        {
            static thread_local LogBundle &bundle = getThreadLocalLogs();
            bundle.put(std::forward<Str>(info));
        }

        void consumeLogs(const std::stop_source &source)
        {
            Logger::LogBundle::value_type logEntry;
            std::vector<Logger::LogBundle::value_type> logsLocal;
            while (!source.stop_requested())
            {
                {
                    std::shared_lock<std::shared_mutex> lock {mutex };
                    for (Logger::LogBundle &logBundle: threadLogs) {
                        for (int32_t n = 0; n < consumeBlockSize && logBundle.get(logEntry); ++n) {
                            logsLocal.push_back(std::move(logEntry));
                        }
                    }
                }
                if (!logsLocal.empty())
                {
                    std::lock_guard<std::mutex> lock { mtxLogs2Handle };
                    // logsToHandle.emplace_back().swap(logsLocal);
                    logsToHandle.push_back(std::move(logsLocal));
                    logsLocal.clear();
                }
                std::this_thread::sleep_for(logsConsumerTimeout);
            }
        }

        void handleLogs(const std::stop_source &source)
        {
            while (!source.stop_requested())
            {
                {
                    std::lock_guard<std::mutex> lock {mtxLogs2Handle };
                    if (logsToHandle.empty()) {
                        continue;
                    }
                    std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;

                    std::vector<Logger::LogBundle::value_type> logsLocal = std::move(logsToHandle.front());
                    std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;

                    logsToHandle.pop_front();
                    std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
                }
                std::this_thread::sleep_for(logsHandleTimeout);
            }
        }
    };

    void benchmark()
    {
        Logger logger;
        for (uint64_t i = 0; i < 1'000'000; ++i) {
            logger.log(std::string(128, '2'));
        }
        std::cout << "Done\n";
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

void LowLatencyLoggerDebug::TestAll()
{
    // Tests::testLogs();
    // Tests::testLogHandler();
    // Tests::loadTest();
    // Tests::loadTest_Sink();

    // Experiments::demoTest();
    // Experiments::Move_String_Test();
    // Experiments::Forward_String_Test();

    // Experiments::Logger_RingBuffer_PerfTest();
    // Experiments::LoggerEx_RingBuffer_PerfTest();

    // HandlersTests::writeLogs();

    // CollectionExperiments::List_Concurrent_Modification();

    SimplifiedLogger::benchmark();
}