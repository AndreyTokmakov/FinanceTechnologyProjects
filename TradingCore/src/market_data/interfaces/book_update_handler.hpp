/**============================================================================
Name        : book_update_handler.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_update_handler.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HANDLER_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HANDLER_HPP

#include "../model/book_update.hpp"

namespace trading::market_data
{
    struct IBookUpdateHandler
    {
        virtual ~IBookUpdateHandler() = default;

        virtual void onBookUpdate(const BookUpdate& update) = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HANDLER_HPP
