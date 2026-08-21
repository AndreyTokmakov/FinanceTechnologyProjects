/**============================================================================
Name        : position_test.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : position_test.cpp
============================================================================**/

#include "position.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::Side;

using trading::position::Position;

namespace
{
    using testing::Assert;

    constexpr InstrumentId INSTRUMENT { 1 };

    constexpr Price BUY_PRICE { 6'500'000'000'000 };
    constexpr Price HIGHER_PRICE { 7'000'000'000'000 };
    constexpr Price LOWER_PRICE { 6'000'000'000'000 };

    constexpr Quantity QUANTITY_100 { 100'000'000 };
    constexpr Quantity QUANTITY_40 { 40'000'000 };
    constexpr Quantity QUANTITY_50 { 50'000'000 };
    constexpr Quantity QUANTITY_60 { 60'000'000 };
    constexpr Quantity QUANTITY_150 { 150'000'000 };


    /*
        Input:
            New position for instrument 1.

        Expected:
            Quantity = 0
            Average price = 0
            Realized PnL = 0
            Position is flat.
    */
    void testInitialState()
    {
        constexpr Position position { INSTRUMENT };

        Assert(position.instrumentId() == INSTRUMENT,"invalid instrument id");
        Assert(position.quantity() == 0,"initial position quantity must be zero");
        Assert(position.averagePrice().isZero(),"initial average price must be zero");
        Assert(position.realizedPnl().isZero(),"initial realized PnL must be zero");
        Assert(!position.isLong(),"initial position must not be long");
        Assert(!position.isShort(),"initial position must not be short");
        Assert(position.isFlat(),"initial position must be flat");
    }

    /*
        Input:
            Buy 100 @ 6500.

        Expected:
            Long position of 100.
            Average price = 6500.
    */
    void testBuyCreatesLongPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);

        Assert(position.quantity() == 100'000'000, "Buy must create long position");
        Assert(position.averagePrice() == BUY_PRICE, "invalid average price");
        Assert(position.isLong(), "position must be long");
        Assert(!position.isShort(), "position must not be short");
        Assert(!position.isFlat(), "position must not be flat");
    }

    /*
        Input:
            Sell 100 @ 6500.

        Expected:
            Short position of 100.
            Average price = 6500.
    */
    void testSellCreatesShortPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);

        Assert(position.quantity() == -100'000'000, "Sell must create short position");
        Assert(position.averagePrice() == BUY_PRICE, "invalid average price");
        Assert(!position.isLong(), "position must not be long");
        Assert(position.isShort(), "position must be short");
        Assert(!position.isFlat(), "position must not be flat");
    }

    /*
        Input:
            Buy 100 @ 6000.
            Buy 100 @ 7000.

        Expected:
            Long position of 200.
            Average price = 6500.
    */
    void testMultipleBuysIncreaseLongPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 200'000'000, "invalid accumulated long position");
        Assert(position.averagePrice() == BUY_PRICE, "invalid weighted average price");
        Assert(position.isLong(), "position must be long");
    }

    /*
        Input:
            Sell 100 @ 6000.
            Sell 100 @ 7000.

        Expected:
            Short position of 200.
            Average price = 6500.
    */
    void testMultipleSellsIncreaseShortPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == -200'000'000, "invalid accumulated short position");
        Assert(position.averagePrice() == BUY_PRICE, "invalid weighted average price");
        Assert(position.isShort(), "position must be short");
    }

    /*
        Input:
            Buy 100 @ 6000.
            Buy 150 @ 7000.

        Expected:
            Long position of 250.
            Weighted average price = 6600.
    */
    void testWeightedAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_150);

        // (100 * 6000 + 150 * 7000) / 250 = 6600
        constexpr Price expectedAveragePrice { 6'600'000'000'000 };

        Assert(position.quantity() == 250'000'000, "invalid total position quantity");
        Assert(position.averagePrice() == expectedAveragePrice, "invalid weighted average price");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 40 @ 7000.

        Expected:
            Long position of 60.
            Average price remains 6500.
    */
    void testPartialLongPositionReductionPreservesAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_40);

        Assert(position.quantity() == 60'000'000, "invalid reduced long position");
        Assert(position.averagePrice() == BUY_PRICE, "average price must be preserved after partial reduction");
        Assert(position.isLong(), "position must remain long");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 40 @ 6000.

        Expected:
            Short position of 60.
            Average price remains 6500.
    */
    void testPartialShortPositionReductionPreservesAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_40);

        Assert(position.quantity() == -60'000'000, "invalid reduced short position");
        Assert(position.averagePrice() == BUY_PRICE, "average price must be preserved after partial reduction");
        Assert(position.isShort(), "position must remain short");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 100 @ 7000.

        Expected:
            Position becomes flat.
            Average price resets to zero.
    */
    void testClosingLongPositionResetsState()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 0, "position must become flat");
        Assert(position.averagePrice().isZero(), "average price must reset when position becomes flat");
        Assert(position.isFlat(), "position must be flat");
        Assert(!position.isLong(), "flat position must not be long");
        Assert(!position.isShort(), "flat position must not be short");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 100 @ 6000.

        Expected:
            Position becomes flat.
            Average price resets to zero.
    */
    void testClosingShortPositionResetsState()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 0, "position must become flat");
        Assert(position.averagePrice().isZero(), "average price must reset when position becomes flat");
        Assert(position.isFlat(), "position must be flat");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 150 @ 7000.

        Expected:
            Long position is closed.
            New short position of 50 is opened.
            Short average price = 7000.
    */
    void testLongPositionReversal()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_150);

        Assert(position.quantity() == -50'000'000, "invalid reversed short position");
        Assert(position.averagePrice() == HIGHER_PRICE, "reversed position must use execution price");
        Assert(position.isShort(), "reversed position must be short");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 150 @ 6000.

        Expected:
            Short position is closed.
            New long position of 50 is opened.
            Long average price = 6000.
    */
    void testShortPositionReversal()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_150);

        Assert(position.quantity() == 50'000'000,"invalid reversed long position");
        Assert(position.averagePrice() == LOWER_PRICE,"reversed position must use execution price");
        Assert(position.isLong(),"reversed position must be long");
    }

    /*
        Input:
            Buy 0 @ 6500 on a flat position.

        Expected:
            Position remains flat.
            Average price remains zero.
    */
    void testZeroQuantityTradeOnFlatPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, Quantity {});

        Assert(position.quantity() == 0,"zero quantity trade must not change position");
        Assert(position.averagePrice().isZero(),"zero quantity trade must not set average price");
        Assert(position.isFlat(),"position must remain flat");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 0 @ 7000.

        Expected:
            Position remains long 100.
            Average price remains 6500.
    */
    void testZeroQuantityTradeOnExistingPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, Quantity {});

        Assert(position.quantity() == 100'000'000, "zero quantity trade must not change position quantity");
        Assert(position.averagePrice() == BUY_PRICE, "zero quantity trade must not change average price");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Buy 50 @ 7000.

        Expected:
            Long position of 150.
            Average price = 6666.666666...
            Fixed-point integer arithmetic is used.
    */
    void testWeightedAveragePriceWithUnequalQuantities()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_50);

        constexpr Price expectedAveragePrice { 6'666'666'666'666 };

        Assert(position.quantity() == 150'000'000,"invalid position quantity");
        Assert(position.averagePrice() == expectedAveragePrice,"invalid weighted average price");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 40 @ 7000.
            Buy 50 @ 6000.

        Expected:
            Existing long position is increased again.
            Final quantity = 110.
            Average price is recalculated using the remaining position.
    */
    void testIncreaseLongPositionAfterPartialReduction()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Buy,BUY_PRICE,QUANTITY_100);
        position.applyTrade(Side::Sell,HIGHER_PRICE,QUANTITY_40);
        position.applyTrade(Side::Buy,LOWER_PRICE,QUANTITY_50);

        // (60 * 6500 + 50 * 6000) / 110 = 6272.727272...
        constexpr Price expectedAveragePrice { 6'272'727'272'727 };

        Assert(position.quantity() == 110'000'000, "invalid final long position");
        Assert(position.averagePrice() == expectedAveragePrice, "invalid average price after increasing reduced position");
        Assert(position.isLong(), "position must be long");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 40 @ 6000.
            Sell 50 @ 7000.

        Expected:
            Existing short position is increased again.
            Final quantity = -110.
            Average price is recalculated.
    */
    void testIncreaseShortPositionAfterPartialReduction()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell,BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_40);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_50);

        // (60 * 6500 + 50 * 7000) / 110 = 6727.272727...
        constexpr Price expectedAveragePrice { 6'727'272'727'272 };

        Assert(position.quantity() == -110'000'000,"invalid final short position");
        Assert(position.averagePrice() == expectedAveragePrice,"invalid average price after increasing reduced position");
        Assert(position.isShort(),"position must be short");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 60 @ 7000.

        Expected:
            Long position of 40 remains.
            Average price remains 6500.
    */
    void testLongPositionReductionLeavesRemainingPosition()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_60);

        Assert(position.quantity() == 40'000'000,"invalid remaining long position");
        Assert(position.averagePrice() == BUY_PRICE,"average price must remain unchanged");
        Assert(position.isLong(),"position must remain long");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 60 @ 6000.

        Expected:
            Short position of 40 remains.
            Average price remains 6500.
    */
    void testShortPositionReductionLeavesRemainingPosition()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_60);

        Assert(position.quantity() == -40'000'000,"invalid remaining short position");
        Assert(position.averagePrice() == BUY_PRICE,"average price must remain unchanged");
        Assert(position.isShort(),"position must remain short");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 100 @ 7000.
            Buy 50 @ 6000.

        Expected:
            Position is closed first.
            New long position of 50 is created.
            Average price of the new position = 6000.
    */
    void testOpenNewLongPositionAfterClosingLong()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_50);

        Assert(position.quantity() == 50'000'000,"invalid new long position");
        Assert(position.averagePrice() == LOWER_PRICE,"new long position must use execution price");
        Assert(position.isLong(),"new position must be long");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 100 @ 6000.
            Sell 50 @ 7000.

        Expected:
            Position is closed first.
            New short position of 50 is created.
            Average price of the new position = 7000.
    */
    void testOpenNewShortPositionAfterClosingShort()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_50);

        Assert(position.quantity() == -50'000'000,"invalid new short position");
        Assert(position.averagePrice() == HIGHER_PRICE,"new short position must use execution price");
        Assert(position.isShort(),"new position must be short");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Sell 150 @ 7000.

        Expected:
            Long position is completely closed.
            Remaining 50 opens a short position.
            Average price = 7000.
    */
    void testLongToShortReversalWithExactRemainingQuantity()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_150);

        Assert(position.quantity() == -50'000'000,"invalid reversed quantity");
        Assert(position.averagePrice() == HIGHER_PRICE,"invalid reversed average price");
        Assert(position.isShort(),"position must be short");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Buy 150 @ 6000.

        Expected:
            Short position is completely closed.
            Remaining 50 opens a long position.
            Average price = 6000.
    */
    void testShortToLongReversalWithExactRemainingQuantity()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_150);

        Assert(position.quantity() == 50'000'000,"invalid reversed quantity");
        Assert(position.averagePrice() == LOWER_PRICE,"invalid reversed average price");
        Assert(position.isLong(),"position must be long");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Buy 50 @ 7000.
            Sell 150 @ 6000.

        Expected:
            Entire long position is closed.
            Position becomes flat.
            Average price resets to zero.
    */
    void testCloseAccumulatedLongPosition()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_50);
        position.applyTrade(Side::Sell, LOWER_PRICE, QUANTITY_150);

        Assert(position.quantity() == 0,"position must become flat");
        Assert(position.averagePrice().isZero(),"average price must reset");
        Assert(position.isFlat(),"position must be flat");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Sell 50 @ 6000.
            Buy 150 @ 7000.

        Expected:
            Entire short position is closed.
            Position becomes flat.
            Average price resets to zero.
    */
    void testCloseAccumulatedShortPosition()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, LOWER_PRICE,QUANTITY_50);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_150);

        Assert(position.quantity() == 0,"position must become flat");
        Assert(position.averagePrice().isZero(),"average price must reset");
        Assert(position.isFlat(),"position must be flat");
    }

    /*
        Input:
            Buy 100 @ 6500.
            Buy 50 @ 7000.
            Sell 50 @ 6000.

        Expected:
            Long position is reduced from 150 to 100.
            Average price remains unchanged.
    */
    void testPartialReductionOfAccumulatedLongPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_50);

        const Price averagePriceBeforeReduction = position.averagePrice();

        position.applyTrade(Side::Sell, LOWER_PRICE,QUANTITY_50);

        Assert(position.quantity() == 100'000'000,"invalid remaining long position");
        Assert(position.averagePrice() == averagePriceBeforeReduction,"average price must remain unchanged");
        Assert(position.isLong(),"position must remain long");
    }

    /*
        Input:
            Sell 100 @ 6500.
            Sell 50 @ 7000.
            Buy 50 @ 6000.

        Expected:
            Short position is reduced from 150 to 100.
            Average price remains unchanged.
    */
    void testPartialReductionOfAccumulatedShortPosition()
    {
        Position position { INSTRUMENT };

        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_50);

        const Price averagePriceBeforeReduction = position.averagePrice();

        position.applyTrade(Side::Buy, LOWER_PRICE,QUANTITY_50);

        Assert(position.quantity() == -100'000'000, "invalid remaining short position");
        Assert(position.averagePrice() == averagePriceBeforeReduction, "average price must remain unchanged");
        Assert(position.isShort(), "position must remain short");
    }

    /*
        Input:
            New position.
            No trades.

        Expected:
            Realized PnL remains zero.
    */
    void testRealizedPnlInitialValue()
    {
        constexpr Position position { INSTRUMENT };

        Assert(position.realizedPnl().isZero(),"initial realized PnL must be zero");
    }
}

void position_test()
{
    testInitialState();

    testBuyCreatesLongPosition();
    testSellCreatesShortPosition();

    testMultipleBuysIncreaseLongPosition();
    testMultipleSellsIncreaseShortPosition();
    testWeightedAveragePrice();
    testWeightedAveragePriceWithUnequalQuantities();

    testPartialLongPositionReductionPreservesAveragePrice();
    testPartialShortPositionReductionPreservesAveragePrice();
    testLongPositionReductionLeavesRemainingPosition();
    testShortPositionReductionLeavesRemainingPosition();

    testPartialReductionOfAccumulatedLongPosition();
    testPartialReductionOfAccumulatedShortPosition();

    testClosingLongPositionResetsState();
    testClosingShortPositionResetsState();
    testCloseAccumulatedLongPosition();
    testCloseAccumulatedShortPosition();

    testLongPositionReversal();
    testShortPositionReversal();
    testLongToShortReversalWithExactRemainingQuantity();
    testShortToLongReversalWithExactRemainingQuantity();

    testOpenNewLongPositionAfterClosingLong();
    testOpenNewShortPositionAfterClosingShort();

    testIncreaseLongPositionAfterPartialReduction();
    testIncreaseShortPositionAfterPartialReduction();

    testZeroQuantityTradeOnFlatPosition();
    testZeroQuantityTradeOnExistingPosition();

    testRealizedPnlInitialValue();

    std::cout << "All Position tests: OK\n";
}