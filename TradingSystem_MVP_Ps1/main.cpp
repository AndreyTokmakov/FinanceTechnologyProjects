/**============================================================================
Name        : main.cpp
Created on  : 14.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <vector>
#include <thread>

#include "common/Buffer.hpp"
#include "common/RingBuffer.hpp"
#include "market_data/Parser.hpp"

// #include "FinalAction.hpp"
// #include "DateTimeUtilities.hpp"

#include <nlohmann/json.hpp>

namespace
{
    constexpr std::filesystem::path getDataDir() noexcept
    {
        return std::filesystem::current_path() / "../../Parsers_JSON/data/binance/";
    }

    std::vector<std::string> readFile(const std::filesystem::path &filePath)
    {
        std::vector<std::string> lines;
        if (std::ifstream file(filePath); file.is_open() && file.good()){
            while (std::getline(file, lines.emplace_back())) { /** **/ }
        }
        lines.pop_back();
        return lines;
    }
}

namespace
{
    using namespace common;
    using namespace ring_buffer;
    using namespace market_data::binance;

    struct NoYetImplemented {};

    using BinanceMarketEvent = std::variant<
        BookTicker,
        MiniTicker,
        Trade,
        AggTrade,
        DepthUpdate,
        NoYetImplemented
    >;

    template<typename T>
    concept ConnectorType = requires(T& connector, buffer::Buffer& buffer) {
        { connector.getData(buffer) } -> std::same_as<bool>;
    };

    template<typename T>
    concept ParserType = requires(T& parser, const buffer::Buffer& buffer) {
        { parser.parse(buffer) } -> std::same_as<void>;
        // { parser.parse(buffer) } -> std::same_as<std::string>;
    };

    template<typename T>
    concept PricerType = requires(T& parser, BinanceMarketEvent& event) {
        { parser.push(event) } -> std::same_as<void>;
    };


    template<ConnectorType ConnectorT, ParserType ParserT>
    struct DataFeederBase
    {
        ConnectorT& connector;
        ParserT& parser;

        static_buffer::RingBuffer<1024> queue {};

        std::jthread connectorThread {};
        std::jthread parserThread {};

        constexpr static uint32_t maxSessionBeforeSleep { 10'000 };

        DataFeederBase(ConnectorT& connector, ParserT& parser)
            : connector { connector }, parser { parser } {
        }

        void run()
        {
            connectorThread = std::jthread { [&] { runConnector(); } };
            parserThread = std::jthread { [&] { runParser(); } };
        }

    private:

        void runConnector()
        {
            if (!setThreadCore(1)) {
                std::cerr << "Failed to pin Connector thread to  CPU " << 1  << std::endl;
                return;
            }

            buffer::Buffer* response { nullptr };
            while ((response = queue.getItem())) {
                const auto _ = connector.getData(*response);
                std::cout << "Connector [CPU: " << getCpu() << "] : " << response->length() << std::endl;
                queue.commit();
            }
        }

        void runParser()
        {
            if (!setThreadCore(2)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 2  << std::endl;
                return;
            }

            uint32_t misses { 0 };
            buffer::Buffer* item { nullptr };
            while (true)
            {
                if ((item = queue.pop())) {
                    parser.parse(*item);
                    item->clear();
                    misses = 0;
                    continue;
                }

                if (misses++ > maxSessionBeforeSleep) {
                    std::this_thread::sleep_for(std::chrono::microseconds (10U));
                }
            }
        }
    };

    struct EventHandler
    {
        void operator()(const BookTicker& ticker) const {
            // std::cout << ticker << std::endl;
            std::cout << "Pricer [CPU: " << getCpu() << "] : BookTicker" << std::endl;
        }
        void operator()(const MiniTicker& ticker) const {
            // std::cout << ticker << std::endl;
            std::cout << "Pricer [CPU: " << getCpu() << "] : MiniTicker" << std::endl;
        }
        void operator()(const Trade& trade) const {
            // std::cout << trade << std::endl;
            std::cout << "Pricer [CPU: " << getCpu() << "] : Trade" << std::endl;
        }
        void operator()(const AggTrade& aggTrade) const {
            // std::cout << aggTrade << std::endl;
            std::cout << "Pricer [CPU: " << getCpu() << "] : AggTrade" << std::endl;
        }
        void operator()(const DepthUpdate& depthUpdate) const {
            // std::cout << depthUpdate << std::endl;
            std::cout << "Pricer [CPU: " << getCpu() << "] : DepthUpdate" << std::endl;
        }
        void operator()(const NoYetImplemented&) const {
            std::cerr << "NoYetImplemented" << std::endl;
        }
    };

    template<PricerType PricerT>
    struct DummyParser
    {
        using JsonParams = JsonParams;
        using StreamNames = StreamNames;

        PricerT& pricer;

        explicit DummyParser(PricerT& pricer): pricer { pricer } {
        }

        static BinanceMarketEvent parseEventData(const nlohmann::json& jsonData)
        {
            std::string_view stream = jsonData[JsonParams::stream].get<std::string_view>();
            const size_t pos = stream.find('@');
            // const std::string_view symbol ( stream.data(), pos);
            stream.remove_prefix(pos + 1);

            const nlohmann::json& data = jsonData[JsonParams::data];
            if (stream.starts_with(StreamNames::miniTicker))
                return BinanceParserJson::parseMiniTicker(data);
            if (stream.starts_with(StreamNames::bookTicker))
                return BinanceParserJson::parseBookTicker(data);
            if (stream.starts_with(StreamNames::trade))
                return BinanceParserJson::parseTrade(data);
            if (stream.starts_with(StreamNames::aggTrade))
                return BinanceParserJson::parseAggTrade(data);
            if (stream.starts_with(StreamNames::depth))
                return BinanceParserJson::parseDepthUpdate(data);

            std::cerr << stream << std::endl;
            return NoYetImplemented{};
        }

        void parse(const buffer::Buffer& buffer)
        {
            std::cout << "Parser [CPU: " << getCpu() << "] : " << buffer.length() << std::endl;
            const std::string_view data = std::string_view(buffer.head(), buffer.length());
            const nlohmann::json jsonData = nlohmann::json::parse(data);
            BinanceMarketEvent event = parseEventData(jsonData);
            pricer.push(event);
        }
    };

    struct TestConnector
    {
        [[nodiscard]]
        bool init()
        {
            data = readFile(getDataDir() / "allData2.json");
            return !data.empty();
        }

        [[nodiscard]]
        bool getData(buffer::Buffer& response)
        {
            const std::string& entry = data[readPost % data.size()];
            const size_t bytes = entry.size();

            std::memcpy(response.tail(bytes), entry.data(), bytes);
            response.incrementLength(bytes);

            std::this_thread::sleep_for(std::chrono::milliseconds (1000U));
            ++readPost;
            return true;
        }

    private:

        std::vector<std::string> data;
        size_t readPost { 0 };
    };

    struct Pricer
    {
        pricer::RingBuffer<BinanceMarketEvent, 1024> queue {};
        std::jthread worker {};

        void run() {
            worker = std::jthread { [&] { handleEvents(); } };
        }

        void handleEvents()
        {
            if (!setThreadCore(3)) {
                std::cerr << "Failed to pin Parser thread to  CPU " << 3  << std::endl;
                return;
            }

            EventHandler eventHandler;
            BinanceMarketEvent event;
            while (true) {
                if (queue.pop(event)) {
                    std::visit(eventHandler, event);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds (1u));
            }
        }

        void push(BinanceMarketEvent& event)
        {
            const auto _ = queue.put(event);
        }
    };
}


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    TestConnector connector;
    if (!connector.init()) {
        std::cerr << "Failed to init connector" << std::endl;
        return EXIT_FAILURE;
    }

    Pricer pricer {};
    pricer.run();

    DummyParser parser {pricer};
    DataFeederBase feeder {connector, parser};
    feeder.run();

    return EXIT_SUCCESS;
}
