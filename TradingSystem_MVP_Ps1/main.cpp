/**============================================================================
Name        : main.cpp
Created on  : 14.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <iostream>
#include <print>
#include <string_view>
#include <vector>
#include <thread>

#include "Buffer.hpp"
#include "RingBuffer.hpp"
#include "Parser.hpp"
#include "price_engine/MarketDepthBook.hpp"
#include "tests/Tests.hpp"
#include "utilities/Utilities.hpp"

namespace
{
    using namespace common;
    using namespace ring_buffer;
    using namespace market_data::binance;

    struct NoYetImplemented {
        std::string streamName {};
    };

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


    // TODO: Rename >>>
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
            parserThread    = std::jthread { [&] { runParser(); } };
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
                // std::cout << "Connector [CPU: " << getCpu() << "] : " << response->length() << std::endl;
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
}

namespace parser_local
{
    template<PricerType PricerT>
    struct DummyParser
    {
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

            return NoYetImplemented { std::string(stream) };
        }

        void parse(const buffer::Buffer& buffer)
        {
            // std::cout << "Parser [CPU: " << getCpu() << "] : " << buffer.length() << std::endl;
            const std::string_view data = std::string_view(buffer.head(), buffer.length());
            const nlohmann::json jsonData = nlohmann::json::parse(data);
            BinanceMarketEvent event = parseEventData(jsonData);
            pricer.push(event);
        }
    };
}

namespace connector_local
{
    struct FileData_DummyConnector
    {
        [[nodiscard]]
        bool init()
        {
            // data =  utilities::readFile(utilities::getDataDir() / "allData.json");
            data =  utilities::readFile(utilities::getDataDir() / "depth.json");
            return !data.empty();
        }

        [[nodiscard]]
        bool getData(buffer::Buffer& response)
        {
            // std::cout << data.size() << std::endl;
            if (readPost == data.size()) {
                std::println(std::cerr, "No more data to read");
                std::terminate();
            }

            const std::string& entry = data[readPost % data.size()];
            const size_t bytes = entry.size();

            std::memcpy(response.tail(bytes), entry.data(), bytes);
            response.incrementLength(bytes);

            std::this_thread::sleep_for(std::chrono::microseconds (250U));
            ++readPost;
            return true;
        }

    private:

        std::vector<std::string> data;
        size_t readPost { 0 };
    };
}

namespace pricer_local
{
    template<class P, class Q>
    struct EventHandler
    {
        price_engine::MarketDepthBook<P, Q>& marketDepthBook;

        void operator()([[maybe_unused]] const BookTicker& ticker) const {
            // debug(ticker);
        }
        void operator()([[maybe_unused]] const MiniTicker& ticker) const {
            // debug(ticker);
        }
        void operator()([[maybe_unused]] const Trade& trade) const {
            // debug(trade);
        }
        void operator()([[maybe_unused]] const AggTrade& aggTrade) const {
            // debug(aggTrade);
        }

        void operator()([[maybe_unused]] const DepthUpdate& depthUpdate) const
        {
            // debug(depthUpdate);
            for (const auto&[price, quantity]: depthUpdate.bids) {
                marketDepthBook.buyUpdate(price, quantity);
            }
            for (const auto&[price, quantity]: depthUpdate.asks) {
                marketDepthBook.askUpdate(price, quantity);
            }

            std::cout << "DepthUpdate { bids: " << depthUpdate.bids.size() << ". asks: " << depthUpdate.asks.size() << "}  "
                    << " Spread: " << marketDepthBook.getSpread()
                    << ", Market Price: " << marketDepthBook.getMarketPrice().value_or(0)
                    << ", Book [bids: " << marketDepthBook.bids.size() << ", asks: " << marketDepthBook.asks.size() << "]"
                    << ", Best [bid: " << marketDepthBook.getBestBid().value_or({}).first
                         << ", ask: " << marketDepthBook.getBestAsk().value_or({}).first << "]"
                    << std::endl;
        }

        void operator()(const NoYetImplemented& nonImpl) const {
            // std::println(std::cerr, "NoYetImplemented(strean: {})", nonImpl.streamName);
        }

    private:

        template<typename Event>
        static void debug(const Event& event)
        {
            std::cout << "Pricer [CPU: " << getCpu() << "] : " << typeid(event).name() << std::endl;
            std::cout << event << std::endl;
        }
    };


    struct Pricer
    {
        pricer::RingBuffer<BinanceMarketEvent, 1024> queue {};
        price_engine::MarketDepthBook<Price, Quantity> book;

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

            EventHandler eventHandler { book } ;
            BinanceMarketEvent event;
            while (true) {
                if (queue.pop(event)) {
                    std::visit(eventHandler, event);
                }
                //std::this_thread::sleep_for(std::chrono::milliseconds (1u));
            }
        }

        void push(BinanceMarketEvent& event)
        {
            const auto _ = queue.put(event);
        }
    };
}

void startService()
{
    connector_local::FileData_DummyConnector connector;
    if (!connector.init()) {
        std::cerr << "Failed to init connector" << std::endl;
        return;
    }

    pricer_local::Pricer pricer {};
    pricer.run();

    parser_local::DummyParser parser {pricer};
    DataFeederBase feeder {connector, parser};
    feeder.run();

}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    startService();
    tests::pricerTests();

    return EXIT_SUCCESS;
}
