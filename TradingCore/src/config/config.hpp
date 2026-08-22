/**============================================================================
Name        : config.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Application configuration model.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_CONFIG_HPP
#define FINANCETECHNOLOGYPROJECTS_CONFIG_HPP

#include "risk_limits.hpp"
#include "types.hpp"

namespace trading::config
{
    struct Config
    {
        InstrumentId instrument { 1 };

        Quantity strategyOrderQuantity { 100'000'000 };
        int64_t strategyThresholdNumerator { 7 };
        int64_t strategyThresholdDenominator { 10 };

        risk::RiskLimits riskLimits {};
    };
}

#endif // FINANCETECHNOLOGYPROJECTS_CONFIG_HPP