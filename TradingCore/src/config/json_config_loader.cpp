/**============================================================================
Name        : json_config_loader.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : JSON configuration loader implementation.
============================================================================**/

#include "json_config_loader.hpp"


#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace trading::config
{
    std::expected<Config, Error> JsonConfigLoader::load(const std::filesystem::path& configPath)
    {
        std::ifstream file { configPath };
        if (!file)
            return std::unexpected(Error::FileOpenFailed);

        try
        {
            const nlohmann::json json = nlohmann::json::parse(file);

            Config config {};

            if (json.contains("instrument")) {
                config.instrument = InstrumentId {json.at("instrument").get<InstrumentId>()};
            }
            if (json.contains("strategy"))
            {
                const auto& strategy = json.at("strategy");

                if (strategy.contains("orderQuantity")) {
                    config.strategyOrderQuantity = Quantity {strategy.at("orderQuantity").get<Quantity::Value>()};
                }
                if (strategy.contains("thresholdNumerator")) {
                    config.strategyThresholdNumerator = strategy.at("thresholdNumerator").get<int64_t>();
                }
                if (strategy.contains("thresholdDenominator")) {
                    config.strategyThresholdDenominator = strategy.at("thresholdDenominator").get<int64_t>();
                }
            }
            if (json.contains("risk"))
            {
                [[maybe_unused]]
                const auto& risk = json.at("risk");
                // RiskLimits fields will be mapped here.
            }

            return config;
        }
        catch (const nlohmann::json::parse_error&)
        {
            return std::unexpected(Error::InvalidJson);
        }
        catch (const nlohmann::json::type_error&)
        {
            return std::unexpected(Error::InvalidConfiguration);
        }
    }
}