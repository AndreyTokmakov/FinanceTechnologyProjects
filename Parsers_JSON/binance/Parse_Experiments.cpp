/**============================================================================
Name        : Parse_Experiments.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include "MarketData.hpp"
#include "Parsers.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <variant>
#include <filesystem>

#include "FileUtilities.hpp"
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

    // TODO: Replace 'stod' to std::from_str | Check performance
    constexpr inline double asDouble(const nlohmann::json& data){
        return std::stod(data.get_ref<const std::string&>());
    };
}


namespace parsing::mini_ticker
{
    using binance::market_data::JsonParams;
    using binance::market_data::MiniTicker;

    MiniTicker parseMiniTicker(const nlohmann::json& data)
    {
        MiniTicker ticker;
        data.at("E").get_to(ticker.timestamp);
        ticker.symbol   = data.value(JsonParams::symbol, "");
        ticker.close    = std::stod(data.value("c", "0"));
        ticker.open     = std::stod(data.value("o", "0"));
        ticker.high     = std::stod(data.value("h", "0"));
        ticker.low      = std::stod(data.value("l", "0"));
        ticker.volume   = std::stod(data.value("v", "0"));
        ticker.quantity = std::stod(data.value(JsonParams::quantity, "0"));
        return ticker;
    }

    MiniTicker parseMiniTicker2(const nlohmann::json& data)
    {
        MiniTicker ticker;
        data.at(JsonParams::eventTime).get_to(ticker.timestamp);
        data.at(JsonParams::symbol).get_to(ticker.symbol);
        ticker.close    = std::stod(data.at("c").get_ref<const std::string&>());
        ticker.open     = std::stod(data.at("o").get_ref<const std::string&>());
        ticker.high     = std::stod(data.at("h").get_ref<const std::string&>());
        ticker.low      = std::stod(data.at("l").get_ref<const std::string&>());
        ticker.volume   = std::stod(data.at("v").get_ref<const std::string&>());
        ticker.quantity = std::stod(data.at("q").get_ref<const std::string&>());
        return ticker;
    }

    void test()
    {
        const std::string content = FileUtilities::ReadFile(getDataDir() / "miniTicker.json");
        try {
            const nlohmann::json jsonData = nlohmann::json::parse(content);
            const nlohmann::json& data = jsonData[JsonParams::data];
            {
                MiniTicker ticker = parseMiniTicker(data);
                std::cout << ticker << std::endl;
            }
            {
                MiniTicker ticker = parseMiniTicker2(data);
                std::cout << ticker << std::endl;
            }
        }
        catch (const std::exception& exc) {
            std::cout << exc.what() << std::endl;
        }
    }
}

namespace parsing::book_ticker
{
    using binance::market_data::JsonParams;
    using binance::market_data::BookTicker;

    BookTicker parseBookTicker(const nlohmann::json& data)
    {
        BookTicker ticker;
        data.at(JsonParams::symbol).get_to(ticker.symbol);
        ticker.bidPrice    = std::stod(data.at(JsonParams::BookTicker::bestBuyPrice).get_ref<const std::string&>());
        ticker.bidQuantity = std::stod(data.at(JsonParams::BookTicker::bestBuyQuantity).get_ref<const std::string&>());
        ticker.askPrice    = std::stod(data.at(JsonParams::BookTicker::bestAskPrice).get_ref<const std::string&>());
        ticker.askQuantity = std::stod(data.at(JsonParams::BookTicker::bestBuyQuantity).get_ref<const std::string&>());
        data.at(JsonParams::BookTicker::orderBookUpdateId).get_to(ticker.updateId);
        return ticker;
    }

    void test()
    {
        const std::string content = FileUtilities::ReadFile(getDataDir() / "bookTicker.json");
        try {
            const nlohmann::json jsonData = nlohmann::json::parse(content);
            const nlohmann::json& data = jsonData[JsonParams::data];
            std::cout << data << std::endl;
            const BookTicker ticker = parseBookTicker(data);
            std::cout << ticker << std::endl;
        }
        catch (const std::exception& exc) {
            std::cout << exc.what() << std::endl;
        }
    }
}

namespace parsing::book_depth_updates
{
    using binance::market_data::JsonParams;
    using binance::market_data::DepthUpdate;

    DepthUpdate parseDepthUpdate(const nlohmann::json& data)
    {
        DepthUpdate update;
        data.at(JsonParams::symbol).get_to(update.symbol);
        data.at(JsonParams::eventTime).get_to(update.eventTime);
        data.at(JsonParams::DepthUpdate::firstUpdateId).get_to(update.firstUpdateId);
        data.at(JsonParams::DepthUpdate::finalUpdateId).get_to(update.finalUpdateId);

        const nlohmann::json& bids = data[JsonParams::DepthUpdate::bids];
        update.bids.reserve(bids.size());
        for (const auto& lvl: bids) {
            update.bids.emplace_back(std::stod(lvl[0].get_ref<const std::string&>()),
                    std::stod(lvl[1].get_ref<const std::string&>()));
        }

        const nlohmann::json& asks = data[JsonParams::DepthUpdate::asks];
        update.asks.reserve(asks.size());
        for (const auto& lvl: asks) {
            update.asks.emplace_back(std::stod(lvl[0].get_ref<const std::string&>()),
                    std::stod(lvl[1].get_ref<const std::string&>()));
        }

        return update;
    }

    void test()
    {
        const std::string content = FileUtilities::ReadFile(getDataDir() / "depthUpdate.json");
        try {
            const nlohmann::json jsonData = nlohmann::json::parse(content);
            const nlohmann::json& data = jsonData[JsonParams::data];
            // std::cout << data << std::endl;

            const DepthUpdate update = parseDepthUpdate(data);
            std::cout << update << std::endl;
        }
        catch (const std::exception& exc) {
            std::cout << exc.what() << std::endl;
        }
    }
}

namespace parsing::ticker
{
    using binance::market_data::JsonParams;
    using binance::market_data::Ticker;

    Ticker parseTicker(const nlohmann::json& data)
    {
        Ticker ticker;

        data.at(JsonParams::symbol).get_to(ticker.symbol);
        data.at(JsonParams::eventTime).get_to(ticker.eventTime);

        /*
        t.event_time = j["E"].get<uint64_t>();

        auto asDouble = [](const json& v) -> double {
            return std::stod(v.get_ref<const std::string&>());
        };

        t.price_change         = asDouble(j["p"]);
        t.price_change_percent = asDouble(j["P"]);
        t.weighted_avg_price   = asDouble(j["w"]);
        t.prev_close_price     = asDouble(j["x"]);
        t.last_price           = asDouble(j["c"]);
        t.last_qty             = asDouble(j["Q"]);
        t.best_bid             = asDouble(j["b"]);
        t.best_bid_qty         = asDouble(j["B"]);
        t.best_ask             = asDouble(j["a"]);
        t.best_ask_qty         = asDouble(j["A"]);
        t.open_price           = asDouble(j["o"]);
        t.high_price           = asDouble(j["h"]);
        t.low_price            = asDouble(j["l"]);
        t.volume               = asDouble(j["v"]);
        t.quote_volume         = asDouble(j["q"]);

        t.open_time       = j["O"].get<uint64_t>();
        t.close_time      = j["C"].get<uint64_t>();
        t.first_trade_id  = j["F"].get<uint64_t>();
        t.last_trade_id   = j["L"].get<uint64_t>();
        t.num_trades      = j["n"].get<uint64_t>();*/

        return ticker;
    }


    void test()
    {
        const std::string content = FileUtilities::ReadFile(getDataDir() / "ticker.json");
        try {
            const nlohmann::json jsonData = nlohmann::json::parse(content);
            const nlohmann::json& data = jsonData[JsonParams::data];
            std::cout << data << std::endl;

            const Ticker ticker = parseTicker(data);
            std::cout << ticker << std::endl;
        }
        catch (const std::exception& exc) {
            std::cout << exc.what() << std::endl;
        }
    }
}


namespace binance::all_streams
{
    using market_data::JsonParams;
    using market_data::StreamNames;

    using market_data::BookTicker;
    using market_data::MiniTicker;

    struct NoYetImplemented {};

    using BinanceMarketEvent = std::variant<BookTicker, MiniTicker, NoYetImplemented>;

    struct EventHandler
    {
        void operator()(const BookTicker& ticker) const {
            std::cout << ticker << std::endl;
        }
        void operator()(const MiniTicker& ticker) const {
            std::cout << ticker << std::endl;
        }
        void operator()(const NoYetImplemented&) const {
            std::cout << "NoYetImplemented" << std::endl;
        }
    };

    void visit_test()
    {
        /*
        template <typename ...Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        std::visit(overloaded{
            [](const auto& x) {std::cout <<  x << "\n";},
            [](const Other& x) {std::cout << "Other\n";},
        }, event);
        */
    }

    BinanceMarketEvent parse(const nlohmann::json& jsonData)
    {
        std::string_view stream = jsonData[JsonParams::stream].get<std::string_view>();
        const size_t pos = stream.find('@');
        const std::string_view symbol ( stream.data(), pos);
        stream.remove_prefix(pos + 1);

        const nlohmann::json& data = jsonData[JsonParams::data];
        if (stream.starts_with(StreamNames::miniTicker))
            return parsing::mini_ticker::parseMiniTicker2(data);
        if (stream.starts_with(StreamNames::bookTicker))
            return BookTicker{};

        return NoYetImplemented{};
    }

    void allStreams()
    {
        EventHandler handler;
        for (const auto data = readFile(getDataDir() / "allData.json"); const std::string &line : data) {
            try {
                const nlohmann::json jsonData = nlohmann::json::parse(line);
                const BinanceMarketEvent event = parse(jsonData);
                std::visit(handler, event);
            }
            catch (const std::exception& exc) {
                std::cout << exc.what() << std::endl;
            }
        }
    }
}


void binance::Experiments::TestAll()
{
    // all_streams::allStreams();

    // parsing::mini_ticker::test();
    // parsing::book_ticker::test();
    // parsing::book_depth_updates::test();
    parsing::ticker::test();
}
