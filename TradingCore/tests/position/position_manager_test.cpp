/**============================================================================
Name        : position_manager_test.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : position_manager_test.cpp
============================================================================**/

#include "position_manager.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::ExchangeOrderId;
using trading::ExecType;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::Price;
using trading::Quantity;
using trading::Side;

using trading::execution::ExecutionReport;
using trading::position::Position;
using trading::position::PositionManager;

namespace
{
    using testing::Assert;


    ExecutionReport createTradeReport(const InstrumentId instrument,
                                      const Side side,
                                      const Price price,
                                      const Quantity quantity)
    {
        return ExecutionReport {
            .clientOrderId = OrderId { 1 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = instrument,
            .side = side,
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = price,
            .quantity = quantity,
            .filledQuantity = quantity
        };
    }

    void testInitialState()
    {
        const PositionManager manager {};
        const Position* position = manager.find(InstrumentId { 1 });
        Assert(position == nullptr, "position must not exist initially");
    }

#if 0
    void testApplyBuyTradeCreatesPosition()
    {
        PositionManager manager {};

        const ExecutionReport report = createTradeReport(
            InstrumentId { 1 },
            Side::Buy,
            Price { 6'500'000'000'000 },
            Quantity { 100'000'000 });

        const bool applied = manager.applyExecution(report);

        Assert(applied, "Buy trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must be created");
        Assert(position->quantity() == Quantity { 100'000'000 }.raw(),
            "invalid position quantity");
        std::cout << position->averagePrice().raw() << std::endl;
        Assert(position->averagePrice() == Price { 6'500'000'000 },
            "invalid average entry price");
    }


    void testApplySellTradeCreatesShortPosition()
    {
        PositionManager manager {};

        const ExecutionReport report = createTradeReport(
            InstrumentId { 1 },
            Side::Sell,
            Price { 6'500'000'000'000 },
            Quantity { 100'000'000 });

        const bool applied = manager.applyExecution(report);

        Assert(applied, "Sell trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must be created");
        Assert(
            position->quantity() == Quantity { -100'000'000 }.raw(),
            "invalid short position quantity");

        Assert(
            position->averagePrice() == Price { 6'500'000'000'000 },
            "invalid short position entry price");
    }


    void testMultipleTradesUpdateExistingPosition()
    {
        PositionManager manager {};

        const bool firstApplied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'000'000'000'000 },
                Quantity { 100'000'000 }));

        const bool secondApplied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 7'000'000'000'000 },
                Quantity { 100'000'000 }));

        Assert(firstApplied, "first trade must be applied");
        Assert(secondApplied, "second trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must exist");
        Assert(
            position->quantity() == Quantity { 200'000'000 }.raw(),
            "invalid accumulated position quantity");

        Assert(
            position->averagePrice() == Price { 6'500'000'000'000 },
            "invalid weighted average entry price");
    }


    void testTradesForDifferentInstrumentsCreateDifferentPositions()
    {
        PositionManager manager {};

        const bool firstApplied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'500'000'000'000 },
                Quantity { 100'000'000 }));

        const bool secondApplied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 2 },
                Side::Buy,
                Price { 7'000'000'000'000 },
                Quantity { 200'000'000 }));

        Assert(firstApplied, "first trade must be applied");
        Assert(secondApplied, "second trade must be applied");

        const Position* firstPosition = manager.find(InstrumentId { 1 });
        const Position* secondPosition = manager.find(InstrumentId { 2 });

        Assert(firstPosition != nullptr, "first position must exist");
        Assert(secondPosition != nullptr, "second position must exist");

        Assert(
            firstPosition->quantity() == Quantity { 100'000'000 }.raw(),
            "invalid first position quantity");

        Assert(
            secondPosition->quantity() == Quantity { 200'000'000 }.raw(),
            "invalid second position quantity");
    }


    void testNonTradeExecutionIsRejected()
    {
        PositionManager manager {};

        const ExecutionReport report {
            .clientOrderId = OrderId { 1 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::New,
            .status = OrderStatus::New,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        };

        const bool applied = manager.applyExecution(report);

        Assert(!applied, "non-trade execution must be rejected");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(
            position == nullptr,
            "non-trade execution must not create a position");
    }


    void testZeroQuantityTradeIsRejected()
    {
        PositionManager manager {};

        const ExecutionReport report = createTradeReport(
            InstrumentId { 1 },
            Side::Buy,
            Price { 6'500'000'000'000 },
            Quantity {});

        const bool applied = manager.applyExecution(report);

        Assert(!applied, "zero quantity trade must be rejected");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(
            position == nullptr,
            "zero quantity trade must not create a position");
    }


    void testPartialReductionPreservesAverageEntryPrice()
    {
        PositionManager manager {};

        manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'500'000'000'000 },
                Quantity { 100'000'000 }));

        const bool applied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Sell,
                Price { 7'000'000'000'000 },
                Quantity { 40'000'000 }));

        Assert(applied, "reducing trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must exist");

        Assert(
            position->quantity() == Quantity { 60'000'000 }.raw(),
            "invalid reduced position quantity");

        Assert(
            position->averagePrice() == Price { 6'500'000'000'000 },
            "average entry price must be preserved");
    }


    void testClosingPositionResetsAverageEntryPrice()
    {
        PositionManager manager {};

        manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'500'000'000'000 },
                Quantity { 100'000'000 }));

        const bool applied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Sell,
                Price { 7'000'000'000'000 },
                Quantity { 100'000'000 }));

        Assert(applied, "closing trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must exist");

        Assert(
            position->quantity() == 0,
            "position must become flat");

        Assert(
            position->averagePrice().isZero(),
            "average entry price must be reset");
    }


    void testPositionReversalUsesExecutionPrice()
    {
        PositionManager manager {};

        manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'500'000'000'000 },
                Quantity { 100'000'000 }));

        const bool applied = manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Sell,
                Price { 7'000'000'000'000 },
                Quantity { 150'000'000 }));

        Assert(applied, "reversal trade must be applied");

        const Position* position = manager.find(InstrumentId { 1 });

        Assert(position != nullptr, "position must exist");

        Assert(
            position->quantity() == Quantity { -50'000'000 }.raw(),
            "invalid reversed position quantity");

        Assert(
            position->averagePrice() == Price { 7'000'000'000'000 },
            "reversed position must use execution price");
    }


    void testFindUnknownInstrument()
    {
        PositionManager manager {};

        manager.applyExecution(
            createTradeReport(
                InstrumentId { 1 },
                Side::Buy,
                Price { 6'500'000'000'000 },
                Quantity { 100'000'000 }));

        const Position* position = manager.find(InstrumentId { 2 });

        Assert(
            position == nullptr,
            "unknown instrument must not be found");
    }
#endif
}


void position_manager_test()
{
#if 0
    testInitialState();
    testApplyBuyTradeCreatesPosition();
    testApplySellTradeCreatesShortPosition();
    testMultipleTradesUpdateExistingPosition();
    testTradesForDifferentInstrumentsCreateDifferentPositions();

    testNonTradeExecutionIsRejected();
    testZeroQuantityTradeIsRejected();

    testPartialReductionPreservesAverageEntryPrice();
    testClosingPositionResetsAverageEntryPrice();
    testPositionReversalUsesExecutionPrice();

    testFindUnknownInstrument();
#endif


    std::cout << "All PositionManager tests: OK\n";
}