/**============================================================================
Name        : book_update.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_update.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP

#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

#include <cstdint>

namespace trading::market_data
{
    struct BookUpdate
    {
        InstrumentId instrument;
        SequenceNumber sequence;
        Side side;
        Price price;
        Quantity quantity;
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP
