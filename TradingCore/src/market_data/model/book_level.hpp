 /**============================================================================
Name        : book_level.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_level.hpp
============================================================================**/

/*
    BookLevel represents a single price level of the local OrderBook.

    A BookLevel contains the normalized price and aggregate quantity available
    at that price.

    Data Flow:

        Exchange
           |
           | raw market data
           v
        MarketDataParser
           |
           | BookUpdate
           v
        BookBuilder
           |
           | updates price level
           v
        OrderBook
           |
           v
        BookLevel

    Responsibilities:

        - represent a single price level;
        - store the price associated with the level;
        - store the aggregate quantity available at the price.

    BookLevel is a domain model and is independent of any particular exchange
    or transport protocol.

    BookLevel does not:

        - contain exchange-specific data;
        - contain sequence numbers;
        - contain timestamps;
        - validate market-data messages;
        - apply updates to OrderBook;
        - generate MarketEvent;
        - manage orders;
        - calculate PnL.

    The lifecycle of a BookLevel is controlled by OrderBook. BookBuilder is
    responsible for converting BookUpdate objects into changes of the local
    OrderBook state.

    The quantity may represent either an existing aggregated level or a
    quantity supplied by a BookUpdate, depending on the context in which the
    BookLevel is used.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_LEVEL_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_LEVEL_HPP

#include "price.hpp"
#include "quantity.hpp"

namespace trading::market_data
{
    struct BookLevel
    {
        Price price;
        Quantity quantity;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_LEVEL_HPP