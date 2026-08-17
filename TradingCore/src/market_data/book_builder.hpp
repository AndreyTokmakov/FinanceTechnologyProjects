/**============================================================================
Name        : book_builder.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : book_builder.hpp
============================================================================**/

/*
    BookBuilder is responsible for applying normalized BookUpdate events to
    an OrderBook and publishing a MarketEvent after each successfully applied
    update.

    BookBuilder sits between the normalized market data stream and consumers
    interested in the resulting order book state.

    Data Flow:

        MarketDataSource
               |
               | raw market data
               v
        MarketDataParser
               |
               | BookUpdate
               v
          BookBuilder
               |
               | apply update
               v
           OrderBook
               |
               | current top-of-book
               v
          MarketEvent
               |
               v
        MarketEventHandler
               |
               v
           Strategy

    Responsibilities:

        - receive normalized BookUpdate events;
        - verify that the update belongs to the configured instrument;
        - apply updates to OrderBook;
        - detect invalid or out-of-sequence updates through OrderBook;
        - obtain the resulting best bid and best ask;
        - create and publish MarketEvent.

    BookBuilder does not know the exchange-specific market data format and
    does not perform protocol parsing. Those responsibilities belong to the
    market data source and parser.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP

#include "book_update_handler.hpp"
#include "market_event_handler.hpp"
#include "order_book.hpp"
#include "timestamp.hpp"
#include "types.hpp"

namespace trading::market_data
{
    class BookBuilder final : public IBookUpdateHandler
    {
    public:
        BookBuilder(InstrumentId instrument,
                    OrderBook& orderBook,
                    IMarketEventHandler& eventHandler) noexcept;

        [[nodiscard]]
        bool applySnapshot(SequenceNumber sequence,
                           const OrderBook::Levels& bids,
                           const OrderBook::Levels& asks,
                           Timestamp exchangeTimestamp) const;

        void onBookUpdate(const BookUpdate& update) override;

    private:
        void publishMarketEvent(SequenceNumber sequence,
                                Timestamp exchangeTimestamp) const;

        InstrumentId instrument;
        OrderBook& orderBook;
        IMarketEventHandler& eventHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP