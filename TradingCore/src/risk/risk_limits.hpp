/**============================================================================
Name        : risk_limits.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : risk_limits.hpp
============================================================================**/

/*
    RiskLimits contains static limits used by RiskManager.

    Data Flow:

        Application configuration
                  |
                  v
             RiskLimits
                  |
                  v
             RiskManager
                  |
                  v
             OrderRequest

    Responsibilities:

        - define the maximum quantity of a single order;
        - define the maximum absolute position;
        - define the maximum order notional.

    A zero limit means that the corresponding limit is disabled.

    RiskLimits is configuration only. It does not perform validation.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RISK_LIMITS_HPP
#define FINANCETECHNOLOGYPROJECTS_RISK_LIMITS_HPP

#include "price.hpp"
#include "quantity.hpp"

namespace trading::risk
{
    struct RiskLimits
    {
        Quantity maxOrderQuantity {};
        Quantity maxPositionQuantity {};
        Price maxNotional {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RISK_LIMITS_HPP