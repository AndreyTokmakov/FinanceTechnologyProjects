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


    void testInitialState()
    {
        constexpr Position position { INSTRUMENT };

        Assert(position.instrumentId() == INSTRUMENT,
            "invalid instrument id");
        Assert(position.quantity() == 0,
            "initial position quantity must be zero");
        Assert(position.averagePrice().isZero(),
            "initial average price must be zero");
        Assert(position.realizedPnl().isZero(),
            "initial realized PnL must be zero");
        Assert(!position.isLong(),
            "initial position must not be long");
        Assert(!position.isShort(),
            "initial position must not be short");
        Assert(position.isFlat(),
            "initial position must be flat");
    }

    void testBuyCreatesLongPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);

        Assert(position.quantity() == 100'000'000,
            "Buy must create long position");
        Assert(position.averagePrice() == BUY_PRICE,
            "invalid average price");
        Assert(position.isLong(),
            "position must be long");
        Assert(!position.isShort(),
            "position must not be short");
        Assert(!position.isFlat(),
            "position must not be flat");
    }

    void testSellCreatesShortPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);

        Assert(position.quantity() == -100'000'000,
            "Sell must create short position");
        Assert(position.averagePrice() == BUY_PRICE,
            "invalid average price");
        Assert(!position.isLong(),
            "position must not be long");
        Assert(position.isShort(),
            "position must be short");
        Assert(!position.isFlat(),
            "position must not be flat");
    }

    void testMultipleBuysIncreaseLongPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 200'000'000,
            "invalid accumulated long position");
        Assert(position.averagePrice() == BUY_PRICE,
            "invalid weighted average price");
        Assert(position.isLong(),
            "position must be long");
    }

    void testMultipleSellsIncreaseShortPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, LOWER_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == -200'000'000,
            "invalid accumulated short position");
        Assert(position.averagePrice() == BUY_PRICE,
            "invalid weighted average price");
        Assert(position.isShort(),
            "position must be short");
    }

    void testWeightedAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_100);
        position.applyTrade( Side::Buy, HIGHER_PRICE, QUANTITY_150);

        // (100 * 6000 + 150 * 7000) / 250 = 6600
        constexpr Price expectedAveragePrice { 6'600'000'000'000 };

        Assert(position.quantity() == 250'000'000,
            "invalid total position quantity");
        Assert(position.averagePrice() == expectedAveragePrice,
            "invalid weighted average price");
    }

    void testPartialLongPositionReductionPreservesAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_40);

        Assert(position.quantity() == 60'000'000,
            "invalid reduced long position");
        Assert(position.averagePrice() == BUY_PRICE,
            "average price must be preserved after partial reduction");
        Assert(position.isLong(),
            "position must remain long");
    }

    void testPartialShortPositionReductionPreservesAveragePrice()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE,QUANTITY_40);

        Assert(position.quantity() == -60'000'000,
            "invalid reduced short position");
        Assert(position.averagePrice() == BUY_PRICE,
            "average price must be preserved after partial reduction");
        Assert(position.isShort(),
            "position must remain short");
    }

    void testClosingLongPositionResetsState()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy,BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 0,
            "position must become flat");
        Assert(position.averagePrice().isZero(),
            "average price must reset when position becomes flat");
        Assert(position.isFlat(),
            "position must be flat");
        Assert(!position.isLong(),
            "flat position must not be long");
        Assert(!position.isShort(),
            "flat position must not be short");
    }

    void testClosingShortPositionResetsState()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy,LOWER_PRICE, QUANTITY_100);

        Assert(position.quantity() == 0,
            "position must become flat");
        Assert(position.averagePrice().isZero(),
            "average price must reset when position becomes flat");
        Assert(position.isFlat(),
            "position must be flat");
    }

    void testLongPositionReversal()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, QUANTITY_150);

        Assert(position.quantity() == -50'000'000,
            "invalid reversed short position");
        Assert(position.averagePrice() == HIGHER_PRICE,
            "reversed position must use execution price");
        Assert(position.isShort(),
            "reversed position must be short");
    }

    void testShortPositionReversal()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Sell, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Buy, LOWER_PRICE, QUANTITY_150);

        Assert(position.quantity() == 50'000'000,
            "invalid reversed long position");
        Assert(position.averagePrice() == LOWER_PRICE,
            "reversed position must use execution price");
        Assert(position.isLong(),
            "reversed position must be long");
    }

    void testZeroQuantityTradeOnFlatPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, Quantity {});

        Assert(position.quantity() == 0,
            "zero quantity trade must not change position");

        Assert(position.averagePrice().isZero(),
            "zero quantity trade must not set average price");
        Assert(position.isFlat(),
            "position must remain flat");
    }

    void testZeroQuantityTradeOnExistingPosition()
    {
        Position position { INSTRUMENT };
        position.applyTrade(Side::Buy, BUY_PRICE, QUANTITY_100);
        position.applyTrade(Side::Sell, HIGHER_PRICE, Quantity {});

        Assert(position.quantity() == 100'000'000,
            "zero quantity trade must not change position quantity");
        Assert(position.averagePrice() == BUY_PRICE,
            "zero quantity trade must not change average price");
    }

    void testRealizedPnlInitialValue()
    {
        constexpr Position position { INSTRUMENT };
        Assert(position.realizedPnl().isZero(), "initial realized PnL must be zero");
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

    testPartialLongPositionReductionPreservesAveragePrice();
    testPartialShortPositionReductionPreservesAveragePrice();

    testClosingLongPositionResetsState();
    testClosingShortPositionResetsState();

    testLongPositionReversal();
    testShortPositionReversal();

    testZeroQuantityTradeOnFlatPosition();
    testZeroQuantityTradeOnExistingPosition();

    testRealizedPnlInitialValue();

    std::cout << "All Position tests: OK\n";
}