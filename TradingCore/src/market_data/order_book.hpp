/**============================================================================
Name        : order_book.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_book.hpp
============================================================================**/
/**============================================================================
Name        : order_book.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_book.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP

#include "book_level.hpp"
#include "book_update.hpp"
#include "price.hpp"
#include "quantity.hpp"
#include "types.hpp"

#include <map>
#include <optional>

namespace trading::market_data
{
    class OrderBook
    {
    public:
        using Levels = std::map<Price, Quantity>;

        [[nodiscard]]
        bool isValid() const noexcept;

        [[nodiscard]]
        SequenceNumber sequence() const noexcept;

        void clear() noexcept;

        void applySnapshot(SequenceNumber sequence,
                           const Levels& newBids,
                           const Levels& newAsks);

        [[nodiscard]]
        bool applyUpdate(const BookUpdate& update) noexcept;

        [[nodiscard]]
        std::optional<BookLevel> bestBid() const;

        [[nodiscard]]
        std::optional<BookLevel> bestAsk() const;

        [[nodiscard]]
        Quantity bidVolume(Price price) const noexcept;

        [[nodiscard]]
        Quantity askVolume(Price price) const noexcept;

    private:
        Levels bids;
        Levels asks;
        SequenceNumber sequenceNumber { 0 };
        bool valid { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP