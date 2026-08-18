/**============================================================================
Name        : risk.hpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : risk.hpp
============================================================================**/

/*
    Risk model used by the order validation layer.

    Data Flow:

        Strategy
           |
           | OrderRequest
           v
        OrderManager
           |
           v
        IRiskManager
           |
           v
        RiskManager
           |
           +---- Accepted ----> OrderManager -> IExecutionGateway
           |
           +---- Rejected ---> caller

    Responsibilities:

        - describe the result of a risk check;
        - provide a reason when an order is rejected.

    Risk does not:

        - send orders to an exchange;
        - modify Position;
        - modify Order;
        - process market data;
        - calculate PnL.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RISK_HPP
#define FINANCETECHNOLOGYPROJECTS_RISK_HPP

#include <cstdint>


namespace trading::risk
{
    enum class RiskResult: uint8_t
    {
        Accepted,
        Rejected
    };

    enum class RiskReason: uint8_t
    {
        None,
        MaxOrderQuantity,
        MaxPositionQuantity,
        MaxNotional
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_RISK_HPP