/**============================================================================
Name        : order.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

namespace trading::execution
{
    struct OrderRequest
    {
        InstrumentId instrument;
        Side side;
        OrderType type;
        Price price;
        Quantity quantity;
    };

    struct Order
    {
        OrderId clientOrderId { 0 };
        ExchangeOrderId exchangeOrderId { 0 };
        InstrumentId instrument { 0 };
        Side side { Side::Buy };
        OrderType type { OrderType::Limit };
        Price price {};
        Quantity quantity {};
        Quantity filledQuantity {};
        OrderStatus status { OrderStatus::New };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_HPP
