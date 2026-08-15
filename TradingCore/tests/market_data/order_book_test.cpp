#include "order_book.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace
{

using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::market_data::BookUpdate;
using trading::market_data::OrderBook;

void Assert(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::terminate();
    }
}

void testEmptyBook()
{
    OrderBook book;

    Assert(!book.isValid(), "new book must be invalid");
    Assert(book.sequence() == 0, "new book sequence must be zero");
    Assert(!book.bestBid().has_value(), "empty book must have no best bid");
    Assert(!book.bestAsk().has_value(), "empty book must have no best ask");
}

void testApplySnapshot()
{
    OrderBook book;

    OrderBook::Levels bids {
        { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } },
        { Price { 6'499'999'000'000 }, Quantity { 250'000'000 } }
    };

    OrderBook::Levels asks {
        { Price { 6'500'001'000'000 }, Quantity { 90'000'000 } },
        { Price { 6'500'002'000'000 }, Quantity { 310'000'000 } }
    };

    book.applySnapshot(100, bids, asks);

    Assert(book.isValid(), "book must be valid after snapshot");
    Assert(book.sequence() == 100, "snapshot sequence must be stored");

    const auto bestBid = book.bestBid();

    Assert(bestBid.has_value(), "best bid must exist");
    Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid best bid price");
    Assert(bestBid->quantity == Quantity { 120'000'000 }, "invalid best bid quantity");

    const auto bestAsk = book.bestAsk();

    Assert(bestAsk.has_value(), "best ask must exist");
    Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid best ask price");
    Assert(bestAsk->quantity == Quantity { 90'000'000 }, "invalid best ask quantity");
}

void testAddBid()
{
    OrderBook book;

    book.applySnapshot(100, {}, {});

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 101,
        .side = Side::Buy,
        .price = Price { 6'500'000'000'000 },
        .quantity = Quantity { 120'000'000 }
    });

    Assert(applied, "bid update must be applied");
    Assert(book.sequence() == 101, "sequence must be updated");

    const auto bestBid = book.bestBid();

    Assert(bestBid.has_value(), "best bid must exist");
    Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid bid price");
    Assert(bestBid->quantity == Quantity { 120'000'000 }, "invalid bid quantity");
}

void testAddAsk()
{
    OrderBook book;

    book.applySnapshot(100, {}, {});

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 101,
        .side = Side::Sell,
        .price = Price { 6'500'001'000'000 },
        .quantity = Quantity { 90'000'000 }
    });

    Assert(applied, "ask update must be applied");
    Assert(book.sequence() == 101, "sequence must be updated");

    const auto bestAsk = book.bestAsk();

    Assert(bestAsk.has_value(), "best ask must exist");
    Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid ask price");
    Assert(bestAsk->quantity == Quantity { 90'000'000 }, "invalid ask quantity");
}

void testUpdateExistingLevel()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {
            { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }
        },
        {});

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 101,
        .side = Side::Buy,
        .price = Price { 6'500'000'000'000 },
        .quantity = Quantity { 200'000'000 }
    });

    Assert(applied, "level update must be applied");
    Assert(
        book.bidVolume(Price { 6'500'000'000'000 }) == Quantity { 200'000'000 },
        "level quantity must be updated");
}

void testRemoveLevel()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {
            { Price { 6'500'000'000'000 }, Quantity { 120'000'000 } }
        },
        {});

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 101,
        .side = Side::Buy,
        .price = Price { 6'500'000'000'000 },
        .quantity = Quantity {}
    });

    Assert(applied, "remove update must be applied");
    Assert(
        book.bidVolume(Price { 6'500'000'000'000 }).isZero(),
        "removed level must have zero volume");
    Assert(!book.bestBid().has_value(), "book must have no best bid");
}

void testBestBid()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {
            { Price { 6'499'999'000'000 }, Quantity { 100'000'000 } },
            { Price { 6'500'000'000'000 }, Quantity { 200'000'000 } },
            { Price { 6'499'998'000'000 }, Quantity { 300'000'000 } }
        },
        {});

    const auto bestBid = book.bestBid();

    Assert(bestBid.has_value(), "best bid must exist");
    Assert(bestBid->price == Price { 6'500'000'000'000 }, "invalid best bid");
}

void testBestAsk()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {},
        {
            { Price { 6'500'002'000'000 }, Quantity { 100'000'000 } },
            { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } },
            { Price { 6'500'003'000'000 }, Quantity { 300'000'000 } }
        });

    const auto bestAsk = book.bestAsk();

    Assert(bestAsk.has_value(), "best ask must exist");
    Assert(bestAsk->price == Price { 6'500'001'000'000 }, "invalid best ask");
}

void testSequentialUpdates()
{
    OrderBook book;

    book.applySnapshot(100, {}, {});

    Assert(
        book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        }),
        "sequence 101 must be accepted");

    Assert(
        book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 102,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 200'000'000 }
        }),
        "sequence 102 must be accepted");

    Assert(
        book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 103,
            .side = Side::Sell,
            .price = Price { 6'500'001'000'000 },
            .quantity = Quantity { 300'000'000 }
        }),
        "sequence 103 must be accepted");

    Assert(book.isValid(), "book must remain valid");
    Assert(book.sequence() == 103, "invalid final sequence");
}

void testSequenceGap()
{
    OrderBook book;

    book.applySnapshot(100, {}, {});

    Assert(
        book.applyUpdate(BookUpdate {
            .instrument = 1,
            .sequence = 101,
            .side = Side::Buy,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        }),
        "sequence 101 must be accepted");

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 103,
        .side = Side::Buy,
        .price = Price { 6'500'000'000'000 },
        .quantity = Quantity { 200'000'000 }
    });

    Assert(!applied, "sequence gap must be rejected");
    Assert(!book.isValid(), "book must become invalid after sequence gap");
    Assert(book.sequence() == 101, "sequence must not advance after gap");
}

void testUpdatesRejectedWhenBookInvalid()
{
    OrderBook book;

    const bool applied = book.applyUpdate(BookUpdate {
        .instrument = 1,
        .sequence = 1,
        .side = Side::Buy,
        .price = Price { 6'500'000'000'000 },
        .quantity = Quantity { 100'000'000 }
    });

    Assert(!applied, "update must be rejected for invalid book");
}

void testSnapshotReplacesBook()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {
            { Price { 6'500'000'000'000 }, Quantity { 100'000'000 } }
        },
        {
            { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } }
        });

    book.applySnapshot(
        200,
        {
            { Price { 6'600'000'000'000 }, Quantity { 300'000'000 } }
        },
        {
            { Price { 6'600'001'000'000 }, Quantity { 400'000'000 } }
        });

    Assert(book.isValid(), "book must be valid");
    Assert(book.sequence() == 200, "snapshot sequence must replace old sequence");

    Assert(
        book.bidVolume(Price { 6'500'000'000'000 }).isZero(),
        "old bid must be removed");

    Assert(
        book.askVolume(Price { 6'500'001'000'000 }).isZero(),
        "old ask must be removed");

    Assert(
        book.bidVolume(Price { 6'600'000'000'000 }) == Quantity { 300'000'000 },
        "new bid must exist");

    Assert(
        book.askVolume(Price { 6'600'001'000'000 }) == Quantity { 400'000'000 },
        "new ask must exist");
}

void testClear()
{
    OrderBook book;

    book.applySnapshot(
        100,
        {
            { Price { 6'500'000'000'000 }, Quantity { 100'000'000 } }
        },
        {
            { Price { 6'500'001'000'000 }, Quantity { 200'000'000 } }
        });

    book.clear();

    Assert(!book.isValid(), "cleared book must be invalid");
    Assert(book.sequence() == 0, "cleared book sequence must be zero");
    Assert(!book.bestBid().has_value(), "cleared book must have no bid");
    Assert(!book.bestAsk().has_value(), "cleared book must have no ask");
}

}

void order_book_test()
{
    testEmptyBook();
    testApplySnapshot();
    testAddBid();
    testAddAsk();
    testUpdateExistingLevel();
    testRemoveLevel();
    testBestBid();
    testBestAsk();
    testSequentialUpdates();
    testSequenceGap();
    testUpdatesRejectedWhenBookInvalid();
    testSnapshotReplacesBook();
    testClear();

    std::cout << "All OrderBook tests: OK\n";
}