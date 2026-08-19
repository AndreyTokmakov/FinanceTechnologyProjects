/**============================================================================
Name        : pnl_calculator_test.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : pnl_calculator_test.cpp
============================================================================**/

/*
    Tests for PnLCalculator.

    Data Flow under test:

        Position + ExecutionReport
                   |
                   v
             PnLCalculator
                   |
                   +----> Realized PnL
                   |
                   +----> Unrealized PnL
                   |
                   v
                  PnL

    The tests verify:

        - realized PnL for long positions;
        - realized PnL for short positions;
        - partial closes;
        - position reversals;
        - unrealized PnL for long positions;
        - unrealized PnL for short positions;
        - flat positions;
        - total PnL;
        - ignored non-Trade execution reports.
*/

#include "pnl_calculator.hpp"
#include "test_support/testing.hpp"

#include <cstdlib>
#include <iostream>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::ExecType;

using trading::execution::ExecutionReport;
using trading::market_data::MarketEvent;
using trading::pnl::PnLCalculator;
using trading::position::Position;

namespace
{
    using testing::Assert;

    [[nodiscard]]
    constexpr Price price(const int64_t value)
    {
        return Price { value * Price::Scale };
    }

    [[nodiscard]]
    constexpr Quantity quantity(const int64_t value)
    {
        return Quantity { value * Quantity::Scale };
    }

    [[nodiscard]]
    constexpr ExecutionReport trade(const InstrumentId instrument,
                                    const Side side,
                                    const Price executionPrice,
                                    const Quantity executionQuantity)
    {
        return ExecutionReport {
            .clientOrderId = OrderId { 1 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = instrument,
            .side = side,
            .execType = ExecType::Trade,
            .status = OrderStatus::PartiallyFilled,
            .price = executionPrice,
            .quantity = executionQuantity,
            .filledQuantity = executionQuantity
        };
    }

    [[nodiscard]]
    MarketEvent market(const InstrumentId instrument,
                       const Price bid,
                       const Quantity bidQuantity,
                       const Price ask,
                       const Quantity askQuantity)
    {
        return MarketEvent {
            .instrument = instrument,
            .sequence = 100,
            .bestBid = bid,
            .bestBidQuantity = bidQuantity,
            .bestAsk = ask,
            .bestAskQuantity = askQuantity
        };
    }

    void testLongRealizedPnl()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Sell, price(110), quantity(100));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(1'000), "long realized PnL must be 1000");
    }

    void testShortRealizedPnl()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Buy, price(90), quantity(100));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(1'000), "short realized PnL must be 1000");
    }

    void testLongPartialClose()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Sell, price(110), quantity(40));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(400), "partial long close must realize 400");
    }

    void testShortPartialClose()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Buy, price(90), quantity(40));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(400), "partial short close must realize 400");
    }

    void testLongPositionReversal()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Sell, price(110), quantity(150));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(1'000), "long reversal must realize 1000");
    }

    void testShortPositionReversal()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr auto report = trade(1, Side::Buy, price(90), quantity(150));
        const Price pnl = calculator.calculateRealized(position, report);

        Assert(pnl == price(1'000), "short reversal must realize 1000");
    }

    void testLongUnrealizedPnl()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        const auto marketEvent = market(
                1,
                price(110),
                quantity(100),
                price(111),
                quantity(100));
        constexpr PnLCalculator calculator;
        const Price pnl = calculator.calculateUnrealized(position, marketEvent);

        Assert(pnl == price(1'000), "long unrealized PnL must be 1000");
    }

    void testShortUnrealizedPnl()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(100));

        const auto marketEvent = market(
                1,
                price(89),
                quantity(100),
                price(90),
                quantity(100));

        constexpr PnLCalculator calculator;
        const Price pnl = calculator.calculateUnrealized(position, marketEvent);

        Assert(pnl == price(1'000), "short unrealized PnL must be 1000");
    }

    void testFlatPosition()
    {
        constexpr Position position { InstrumentId { 1 } };
        const auto marketEvent =
            market(
                1,
                price(110),
                quantity(100),
                price(111),
                quantity(100));

        constexpr PnLCalculator calculator;
        const Price pnl = calculator.calculateUnrealized(position, marketEvent);

        Assert(pnl.isZero(), "flat position must have zero unrealized PnL");
    }

    void testTotalPnl()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        constexpr auto report = trade(1, Side::Sell, price(110), quantity(50));
        const auto marketEvent =
            market(
                1,
                price(115),
                quantity(100),
                price(116),
                quantity(100));

        constexpr PnLCalculator calculator;
        const auto pnl = calculator.calculate(position, report, marketEvent);

        Assert(pnl.realized == price(500), "invalid realized PnL");
        Assert(pnl.unrealized == price(1'500), "invalid unrealized PnL");
        Assert(pnl.total() == price(2'000), "invalid total PnL");
    }

    void testNonTradeExecution()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(100));

        constexpr PnLCalculator calculator;
        constexpr ExecutionReport report {
            .clientOrderId = OrderId { 1 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Sell,
            .execType = ExecType::New,
            .status = OrderStatus::New,
            .price = price(110),
            .quantity = quantity(100),
            .filledQuantity = quantity(0)
        };

        const Price pnl = calculator.calculateRealized(position, report);
        Assert(pnl.isZero(), "non-trade execution must have zero realized PnL");
    }

    void testLongUsesBidForMarking()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(10));

        const auto marketEvent =
            market(
                1,
                price(105),
                quantity(10),
                price(110),
                quantity(10));

        constexpr PnLCalculator calculator;
        const Price pnl = calculator.calculateUnrealized(position, marketEvent);

        Assert(pnl == price(50), "long position must use bid price");
    }

    void testShortUsesAskForMarking()
    {
        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(10));

        const auto marketEvent =
            market(
                1,
                price(90),
                quantity(10),
                price(95),
                quantity(10));

        constexpr PnLCalculator calculator;
        const Price pnl = calculator.calculateUnrealized(position, marketEvent);
        Assert(pnl == price(50), "short position must use ask price");
    }
}

void pnl_calculator_test()
{
    testLongRealizedPnl();
    testShortRealizedPnl();
    testLongPartialClose();
    testShortPartialClose();
    testLongPositionReversal();
    testShortPositionReversal();

    testLongUnrealizedPnl();
    testShortUnrealizedPnl();
    testFlatPosition();

    testTotalPnl();
    testNonTradeExecution();

    testLongUsesBidForMarking();
    testShortUsesAskForMarking();

    std::cout << "All PnLCalculator tests: OK\n";
}