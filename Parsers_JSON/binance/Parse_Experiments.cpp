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
}


namespace parsing::mini_ticker
{
    using binance::market_data::MiniTicker;

    MiniTicker parseMiniTicker(const nlohmann::json& data)
    {
        MiniTicker ticker;
        data.at("E").get_to(ticker.timestamp);
        ticker.symbol   = data.value("s", "");
        ticker.close    = std::stod(data.value("c", "0"));
        ticker.open     = std::stod(data.value("o", "0"));
        ticker.high     = std::stod(data.value("h", "0"));
        ticker.low      = std::stod(data.value("l", "0"));
        ticker.volume   = std::stod(data.value("v", "0"));
        ticker.quantity = std::stod(data.value("q", "0"));
        return ticker;
    }

    MiniTicker parseMiniTicker2(const nlohmann::json& data)
    {
        MiniTicker ticker;
        data.at("E").get_to(ticker.timestamp);
        data.at("s").get_to(ticker.symbol);
        ticker.close    = std::stod(data.at("c").get_ref<const std::string&>());
        ticker.open     = std::stod(data.at("o").get_ref<const std::string&>());
        ticker.high     = std::stod(data.at("h").get_ref<const std::string&>());
        ticker.low      = std::stod(data.at("l").get_ref<const std::string&>());
        ticker.volume   = std::stod(data.at("v").get_ref<const std::string&>());
        ticker.quantity = std::stod(data.at("q").get_ref<const std::string&>());
        return ticker;
    }

    void parse()
    {
        const std::string content = FileUtilities::ReadFile(getDataDir() / "miniTicker.json");
        try {
            const nlohmann::json jsonData = nlohmann::json::parse(content);
            const nlohmann::json& data = jsonData["data"];
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

namespace binance::all_streams
{
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
        const std::string& stream = jsonData["stream"].get_ref<const std::string&>();
        std::cout << stream << std::endl;
        return  NoYetImplemented{};
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
    all_streams::allStreams();
    // parsing::mini_ticker::parse();
}
