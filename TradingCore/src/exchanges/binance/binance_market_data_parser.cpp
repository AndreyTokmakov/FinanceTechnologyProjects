/**============================================================================
Name        : binance_market_data_parser.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : binance_market_data_parser.cpp
============================================================================**/

/*
    BinanceMarketDataParser implementation.

    Data Flow:

        Binance raw message
               |
               v
        BinanceMarketDataParser
               |
               +----------------------+
               |                      |
               v                      v
        vector<BookUpdate>        ParseError
               |
               v
        MarketDataMessageHandler
               |
               v
        IBookUpdateHandler

    Binance-specific JSON parsing remains entirely inside this class.
*/

#include "binance_market_data_parser.hpp"

// #include <nlohmann/json.hpp>

namespace trading::exchanges::binance
{
    market_data::ParseResult
    BinanceMarketDataParser::parse([[maybe_unused]] std::string_view message,
                                    market_data::BookUpdates& bookUpdates) const
    {
        bookUpdates.clear();

        /*
            Existing Binance parsing logic goes here.

            Every place where the previous implementation created or returned
            a local vector must now write directly into bookUpdates.

            Example:
                bookUpdates.emplace_back(...);

            Successful parsing:
                return market_data::ParseResult::Success;

            Parsing failure:
                return market_data::ParseResult::InvalidMessage;

            The concrete error values should correspond to the actual
            validation failure.
        */

        return market_data::ParseResult::Success;;


#if 0
        try
        {
            const auto json = nlohmann::json::parse(message);

            if (!json.is_object())
                return std::unexpected { market_data::ParseError::InvalidMessage };
            /*
                Binance depthUpdate example:

                {
                    "e": "depthUpdate",
                    "E": 1672515782136,
                    "s": "BTCUSDT",
                    "U": 157,
                    "u": 160,
                    "b": [
                        ["0.10000000", "1.00000000"]
                    ],
                    "a": [
                        ["0.20000000", "2.00000000"]
                    ]
                }

                The exact conversion to InstrumentId, Price and Quantity
                should use the existing project-specific conversion logic.
            */

            if (!json.contains("e"))
                return std::unexpected { market_data::ParseError::MissingField };

            if (json.at("e") != "depthUpdate")
                return std::unexpected {
                    market_data::ParseError::UnsupportedMessage
                };

            if (!json.contains("E") ||
                !json.contains("U") ||
                !json.contains("u") ||
                !json.contains("b") ||
                !json.contains("a"))
            {
                return std::unexpected {
                    market_data::ParseError::MissingField
                };
            }

            const auto sequence = json.at("u").get<SequenceNumber>();
            const auto timestamp = json.at("E").get<Timestamp::Value>();

            constexpr InstrumentId instrument {
                // Use the existing Binance symbol -> InstrumentId mapping.
                0
            };

            std::vector<market_data::BookUpdate> updates;

            for (const auto& level : json.at("b"))
            {
                if (!level.is_array() || level.size() != 2)
                    return std::unexpected {
                        market_data::ParseError::InvalidField
                    };

                market_data::BookUpdate update;
                update.instrument = instrument;
                update.sequence = sequence;
                update.exchangeTimestamp = Timestamp { timestamp };
                update.side = Side::Buy;

                // Use the existing Price / Quantity construction logic here.
                update.price = Price { level.at(0).get<double>() };
                update.quantity = Quantity { level.at(1).get<double>() };

                updates.push_back(update);
            }

            for (const auto& level : json.at("a"))
            {
                if (!level.is_array() || level.size() != 2)
                    return std::unexpected {
                        market_data::ParseError::InvalidField
                    };

                market_data::BookUpdate update;
                update.instrument = instrument;
                update.sequence = sequence;
                update.exchangeTimestamp = Timestamp { timestamp };
                update.side = Side::Sell;

                update.price = Price { level.at(0).get<double>() };
                update.quantity = Quantity { level.at(1).get<double>() };

                updates.push_back(update);
            }

            return updates;
        }
        catch (const nlohmann::json::exception&)
        {
            return std::unexpected {
                market_data::ParseError::InvalidJson
            };
        }
#endif
    }
}