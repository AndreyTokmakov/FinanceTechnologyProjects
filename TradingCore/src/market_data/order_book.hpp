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

#include "book_update.hpp"
#include "book_level.hpp"
#include "price.hpp"
#include "quantity.hpp"

#include <cstdint>
#include <map>
#include <optional>

namespace trading::market_data
{
    class OrderBook
    {
    public:
        using Levels = std::map<Price, Quantity>;

        [[nodiscard]]
        bool isValid() const noexcept {
            return valid;
        }

        [[nodiscard]]
        SequenceNumber sequence() const noexcept {
            return sequenceNumber;
        }

        void clear() noexcept
        {
            bids.clear();
            asks.clear();
            sequenceNumber = 0;
            valid = false;
        }

        void applySnapshot(const SequenceNumber sequence,
                           const Levels& spanBids,
                           const Levels& spanAsks)
        {
            bids = spanBids;
            asks = spanAsks;
            sequenceNumber = sequence;
            valid = true;
        }

        [[nodiscard]]
        bool applyUpdate(const BookUpdate& update) noexcept
        {
            if (!valid)
                return false;

            if (update.sequence != sequenceNumber + 1) {
                valid = false;
                return false;
            }

            auto& levels = update.side == Side::Buy ? bids : asks;
            if (update.quantity.isZero())
                levels.erase(update.price);
            else
                levels[update.price] = update.quantity;

            sequenceNumber = update.sequence;
            return true;
        }

        [[nodiscard]]
        std::optional<BookLevel> bestBid() const
        {
            if (bids.empty())
                return std::nullopt;
            const auto& [price, quantity] = *bids.rbegin();
            return BookLevel { .price = price, .quantity = quantity };
        }

        [[nodiscard]]
        std::optional<BookLevel> bestAsk() const
        {
            if (asks.empty())
                return std::nullopt;
            const auto& [price, quantity] = *asks.begin();
            return BookLevel { price, quantity };
        }

        [[nodiscard]]
        Quantity bidVolume(const Price price) const noexcept
        {
            const auto it = bids.find(price);
            if (it == bids.end())
                return {};
            return it->second;
        }

        [[nodiscard]]
        Quantity askVolume(const Price price) const noexcept
        {
            const auto it = asks.find(price);
            if (it == asks.end())
                return {};
            return it->second;
        }

    private:
        Levels bids;
        Levels asks;
        SequenceNumber sequenceNumber { 0 };
        bool valid { false };
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_BOOK_HPP
