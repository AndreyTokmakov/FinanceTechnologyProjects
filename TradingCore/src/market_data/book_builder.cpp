/**============================================================================
Name        : book_builder.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder.cpp
============================================================================**/

/*
    BookBuilder receives normalized BookUpdate events, applies them to the
    associated OrderBook and publishes a MarketEvent containing the resulting
    top-of-book state.

    Data Flow:

        BookUpdate
            |
            v
        BookBuilder
            |
            +----> validate instrument
            |
            +----> apply update
            |          |
            |          v
            |      OrderBook
            |
            v
        MarketEvent
            |
            v
        IMarketEventHandler

    An update that belongs to another instrument or cannot be applied because
    the OrderBook is invalid or the sequence is incorrect is ignored and does
    not produce a MarketEvent.
*/
#include "book_builder.hpp"

#include "book_builder.hpp"

namespace trading::market_data
{
    BookBuilder::BookBuilder(const InstrumentId instrument,
                             OrderBook& orderBook,
                             IMarketEventHandler& eventHandler) noexcept :
        instrument { instrument },
        orderBook { orderBook },
        eventHandler { eventHandler }
    {
    }

    bool BookBuilder::applySnapshot(const SequenceNumber sequence,
                                    const OrderBook::Levels& bids,
                                    const OrderBook::Levels& asks,
                                    const Timestamp exchangeTimestamp) const
    {
        orderBook.replace(sequence, bids, asks);

        publishMarketEvent(sequence, exchangeTimestamp);

        return true;
    }

    void BookBuilder::onBookUpdate(const BookUpdate& update)
    {
        if (update.instrument != instrument)
            return;

        if (!orderBook.applyUpdate(update))
            return;

        publishMarketEvent(update.sequence, update.exchangeTimestamp);
    }

    void BookBuilder::publishMarketEvent(const SequenceNumber sequence,
                                         const Timestamp exchangeTimestamp) const
    {
        const auto bestBid = orderBook.bestBid();
        const auto bestAsk = orderBook.bestAsk();

        eventHandler.onMarketEvent(MarketEvent {
            .instrument = instrument,
            .sequence = sequence,
            .exchangeTimestamp = exchangeTimestamp,
            .receiveTimestamp = Timestamp::now(),
            .bestBid = bestBid ? bestBid->price : Price {},
            .bestBidQuantity = bestBid ? bestBid->quantity : Quantity {},
            .bestAsk = bestAsk ? bestAsk->price : Price {},
            .bestAskQuantity = bestAsk ? bestAsk->quantity : Quantity {}
        });
    }
}