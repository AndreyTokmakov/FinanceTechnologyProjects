/**============================================================================
Name        : book_builder_test.cpp
Created on  : 17.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BookBuilder unit tests.
============================================================================**/

#include "book_builder.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;

using trading::market_data::BookBuilder;
using trading::market_data::BookUpdate;
using trading::market_data::IMarketEventHandler;
using trading::market_data::MarketEvent;
using trading::market_data::OrderBook;

namespace
{
    void Assert(const bool condition, const std::string_view message)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            std::terminate();
        }
    }

    class TestMarketEventHandler final : public IMarketEventHandler
    {
    public:
        void onMarketEvent(const MarketEvent& event) override
        {
            events.push_back(event);
        }

        [[nodiscard]]
        std::size_t eventCount() const noexcept {
            return events.size();
        }

        [[nodiscard]]
        const MarketEvent& lastEvent() const noexcept {
            return events.back();
        }

        void clear() {
            events.clear();
        }

        bool isEmpty() {
            return events.empty();
        }

    private:
        std::vector<MarketEvent> events;
    };

    void testApplySnapshot()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;

        constexpr InstrumentId instrument { 42 };
        const BookBuilder builder { instrument, orderBook,eventHandler };
        constexpr Timestamp exchangeTimestamp { 1'000'000 };

        const bool applied = builder.applySnapshot(
            SequenceNumber { 100 },
            {
                { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } },
                { Price { 6'499'999'000'000 }, Quantity { 250'000'000 } }
            },
            {
                { Price { 6'500'001'000'000 }, Quantity { 90'000'000 } },
                { Price { 6'500'002'000'000 }, Quantity { 310'000'000 } }
            },
            exchangeTimestamp);

        Assert(applied, "snapshot must be applied");

        Assert(orderBook.isValid(), "order book must be valid");
        Assert(orderBook.sequence() == 100, "invalid order book sequence");

        const auto bestBid = orderBook.bestBid();
        const auto bestAsk = orderBook.bestAsk();

        Assert(bestBid.has_value(), "best bid must exist");
        Assert(bestAsk.has_value(), "best ask must exist");
        Assert(bestBid->price == Price { 6'500'000'000'000 },
            "invalid best bid price");
        Assert(bestBid->quantity == Quantity { 120'000'000 },
            "invalid best bid quantity");
        Assert(bestAsk->price == Price { 6'500'001'000'000 },
            "invalid best ask price");
        Assert(bestAsk->quantity == Quantity { 90'000'000 },
            "invalid best ask quantity");
    }

    void testSnapshotPublishesMarketEvent()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;

        constexpr InstrumentId instrument { 42 };
        constexpr Timestamp exchangeTimestamp { 1'000'000 };
        const BookBuilder builder { instrument, orderBook,eventHandler };

        const bool applied = builder.applySnapshot(
            SequenceNumber { 100 },
            {{ Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }},
            {{ Price { 6'500'001'000'000 }, Quantity { 90'000'000 } }},
            exchangeTimestamp
        );

        Assert(applied, "snapshot must be applied");
        Assert(eventHandler.eventCount() == 1,
            "snapshot must publish one market event");

        const MarketEvent& event = eventHandler.lastEvent();

        Assert(event.instrument == instrument,
            "invalid event instrument");
        Assert(event.sequence == SequenceNumber { 100 },
            "invalid event sequence");
        Assert(event.exchangeTimestamp == exchangeTimestamp,
            "invalid exchange timestamp");
        Assert(event.bestBid == Price { 6'500'000'000'000 },
            "invalid event best bid");
        Assert(event.bestBidQuantity == Quantity { 120'000'000 },
            "invalid event best bid quantity");
        Assert(event.bestAsk == Price { 6'500'001'000'000 },
            "invalid event best ask");
        Assert(event.bestAskQuantity == Quantity { 90'000'000 },
            "invalid event best ask quantity");
        Assert(event.receiveTimestamp > Timestamp {},
            "receive timestamp must be set");
    }

    void testBookUpdatePublishesMarketEvent()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;
        constexpr InstrumentId instrument { 42 };
        BookBuilder builder { instrument, orderBook,eventHandler };

        constexpr Timestamp exchangeTimestamp { 2'000'000 };
        const bool _ = builder.applySnapshot(SequenceNumber { 100 },
        {
            { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }
        },
        {
        { Price { 6'500'001'000'000 }, Quantity { 90'000'000 } }
        }, Timestamp { 1'000 });

        builder.onBookUpdate(BookUpdate {
            .instrument = instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = exchangeTimestamp,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 200'000'000 }
        });

        Assert(eventHandler.eventCount() == 2,
            "book update must publish 2 market events");

        const MarketEvent& event = eventHandler.lastEvent();

        Assert(event.instrument == instrument, "invalid event instrument");
        Assert(event.sequence == SequenceNumber { 101 }, "invalid event sequence");
        Assert(event.exchangeTimestamp == exchangeTimestamp, "invalid exchange timestamp");
        Assert(event.bestBid == Price { 6'500'000'000'000 }, "invalid best bid");
        Assert(event.bestBidQuantity == Quantity { 200'000'000 }, "invalid best bid quantity");
        Assert(event.bestAsk == Price { 6'500'001'000'000 }, "invalid best ask");
        Assert(event.bestAskQuantity == Quantity { 90'000'000 }, "invalid best ask quantity");
    }

    void testAskUpdate()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;
        constexpr InstrumentId instrument { 42 };
        BookBuilder builder { instrument, orderBook,eventHandler };

        constexpr Timestamp exchangeTimestamp { 2'000'000 };

        const bool _ = builder.applySnapshot(
            SequenceNumber { 100 },
            {{ Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }},
            {{ Price { 6'500'001'000'000 }, Quantity { 90'000'000 } }},
            exchangeTimestamp
        );

        builder.onBookUpdate(BookUpdate {
            .instrument = instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = exchangeTimestamp,
            .side = Side::Sell,
            .price = Price { 6'500'001'000'000 },
            .quantity = Quantity { 150'000'000 }
        });

        Assert(eventHandler.eventCount() == 2, "ask update must publish two market events");

        const MarketEvent& event = eventHandler.lastEvent();

        Assert(event.bestBid == Price { 6'500'000'000'000 }, "best bid must remain unchanged");
        Assert(event.bestBidQuantity == Quantity { 120'000'000 }, "best bid quantity must remain unchanged");
        Assert(event.bestAsk == Price { 6'500'001'000'000 }, "invalid best ask");
        Assert(event.bestAskQuantity == Quantity { 150'000'000 }, "invalid best ask quantity");
    }

    void testSequenceGapDoes_PublishOneEvent()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;

        constexpr InstrumentId instrument { 42 };

        BookBuilder builder { instrument, orderBook,eventHandler };
        const bool _ = builder.applySnapshot(
            SequenceNumber { 100 },
            {{ Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }},
            { },
            Timestamp { 1'000 }
        );

        builder.onBookUpdate(BookUpdate {
            .instrument = instrument,
            .sequence = SequenceNumber { 102 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 200'000'000 }
        });

        Assert(eventHandler.eventCount() == 1,"invalid update must publish 1 market event");
        Assert(!orderBook.isValid(),"order book must become invalid after sequence gap");
    }

    void testUpdateAfterSnapshot()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;
        constexpr InstrumentId instrument { 42 };
        BookBuilder builder {instrument,orderBook,eventHandler};

        const bool snapshotApplied = builder.applySnapshot(
            SequenceNumber { 100 },
            {{ Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }},
            {{ Price { 6'500'001'000'000 }, Quantity { 90'000'000 } }},
            Timestamp { 1'000'000 }
        );

        Assert(snapshotApplied, "snapshot must be applied");
        Assert(eventHandler.eventCount() == 1,"snapshot must publish one event");

        builder.onBookUpdate(BookUpdate {
            .instrument = instrument,
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp { 2'000'000 },
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 200'000'000 }
        });

        Assert(eventHandler.eventCount() == 2,"update must publish second event");

        const MarketEvent& event = eventHandler.lastEvent();

        Assert(event.sequence == SequenceNumber { 101 },"invalid final sequence");
        Assert(event.exchangeTimestamp == Timestamp { 2'000'000 },"invalid final exchange timestamp");
        Assert(event.bestBidQuantity == Quantity { 200'000'000 },"invalid final best bid quantity");
    }

    void testEmptySnapshot()
    {
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;

        constexpr InstrumentId instrument { 42 };
        const BookBuilder builder { instrument, orderBook,eventHandler };

        const bool applied = builder.applySnapshot(
            SequenceNumber { 100 },
            {},
            {},
            Timestamp { 1'000'000 });

        Assert(applied, "empty snapshot must be applied");
        Assert(eventHandler.eventCount() == 1,"empty snapshot must publish one event");

        const MarketEvent& event = eventHandler.lastEvent();

        Assert(event.bestBid.isZero(),"empty book must have zero best bid");
        Assert(event.bestBidQuantity.isZero(),"empty book must have zero best bid quantity");
        Assert(event.bestAsk.isZero(),"empty book must have zero best ask");
        Assert(event.bestAskQuantity.isZero(),"empty book must have zero best ask quantity");
    }

    void testBookUpdateWithWrongInstrumentIsIgnored()
    {
        constexpr InstrumentId instrument { 1 };
        OrderBook orderBook;
        TestMarketEventHandler eventHandler;
        BookBuilder builder {instrument, orderBook, eventHandler };

        const bool _ = builder.applySnapshot(100,{},{},Timestamp { 1'000 });
        eventHandler.clear();

        builder.onBookUpdate(BookUpdate {
            .instrument = InstrumentId { 2 },
            .sequence = 101,
            .exchangeTimestamp = Timestamp { 2'000 },
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        Assert(eventHandler.isEmpty(),"update for another instrument must be ignored");
        Assert(orderBook.sequence() == 100, "order book sequence must not change");
    }

}

void book_builder_test()
{
    testApplySnapshot();
    testSnapshotPublishesMarketEvent();
    testBookUpdatePublishesMarketEvent();
    testAskUpdate();
    testSequenceGapDoes_PublishOneEvent();
    testUpdateAfterSnapshot();
    testEmptySnapshot();
    testBookUpdateWithWrongInstrumentIsIgnored();

    std::cout << "All BookBuilder tests: OK\n";
}