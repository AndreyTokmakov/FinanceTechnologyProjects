/**============================================================================
Name        : json_config_loader.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : JSON configuration loader.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP
#define FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP

#include <expected>
#include <filesystem>

#include "config.hpp"


namespace trading::config
{
    enum class Error: uint8_t
    {
        FileOpenFailed,
        InvalidJson,
        InvalidConfiguration
    };

    class JsonConfigLoader final
    {
    public:
        [[nodiscard]]
        static std::expected<Config, Error> load(const std::filesystem::path& configPath);
    };
}

#endif // FINANCETECHNOLOGYPROJECTS_JSON_CONFIG_LOADER_HPP