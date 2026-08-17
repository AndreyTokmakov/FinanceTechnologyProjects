/**============================================================================
Name        : order_book.cpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_book.cpp
============================================================================**/

#include "order_book.hpp"

namespace trading::market_data
{
    bool OrderBook::isValid() const noexcept
    {
        return valid;
    }

    SequenceNumber OrderBook::sequence() const noexcept
    {
        return sequenceNumber;
    }

    void OrderBook::clear() noexcept
    {
        bids.clear();
        asks.clear();
        sequenceNumber = 0;
        valid = false;
    }

    void OrderBook::applySnapshot(const SequenceNumber sequence,
                                  const Levels& newBids,
                                  const Levels& newAsks)
    {
        bids = newBids;
        asks = newAsks;
        sequenceNumber = sequence;
        valid = true;
    }

    bool OrderBook::applyUpdate(const BookUpdate& update) noexcept
    {
        if (!valid)
            return false;

        if (update.sequence != sequenceNumber + 1)
        {
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

    std::optional<BookLevel> OrderBook::bestBid() const
    {
        if (bids.empty())
            return std::nullopt;
        const auto& [price, quantity] = *bids.rbegin();

        return BookLevel {
            .price = price,
            .quantity = quantity
        };
    }

    std::optional<BookLevel> OrderBook::bestAsk() const
    {
        if (asks.empty())
            return std::nullopt;
        const auto& [price, quantity] = *asks.begin();

        return BookLevel {
            .price = price,
            .quantity = quantity
        };
    }

    Quantity OrderBook::bidVolume(const Price price) const noexcept
    {
        const auto it = bids.find(price);
        if (it == bids.end())
            return {};

        return it->second;
    }

    Quantity OrderBook::askVolume(const Price price) const noexcept
    {
        const auto it = asks.find(price);
        if (it == asks.end())
            return {};

        return it->second;
    }
}