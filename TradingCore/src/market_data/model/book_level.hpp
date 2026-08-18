/**============================================================================
Name        : book_level.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_level.hpp
============================================================================**/

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
