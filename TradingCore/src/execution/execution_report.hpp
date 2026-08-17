/**============================================================================
Name        : execution_report.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

namespace trading::execution
{
    struct ExecutionReport
    {
        OrderId clientOrderId { 0 };
        ExchangeOrderId exchangeOrderId { 0 };
        ExecType execType { ExecType::New };
        OrderStatus status { OrderStatus::New };
        Price price;
        Quantity quantity;
        Quantity filledQuantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_HPP
