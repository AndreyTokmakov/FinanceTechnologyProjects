/**============================================================================
Name        : imbalance_strategy_test.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : imbalance_strategy_test.cpp
============================================================================**/

#include "imbalance_strategy.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::InstrumentId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::strategy::Signal;
using trading::Timestamp;

using trading::market_data::MarketEvent;
using trading::strategy::ImbalanceStrategy;

namespace
{
    using testing::Assert;

    constexpr InstrumentId INSTRUMENT { 1 };
    constexpr Price BID_PRICE { 6'500'000'000'000 };
    constexpr Price ASK_PRICE { 6'500'100'000'000 };

    MarketEvent createMarketEvent(const Quantity bestBidQuantity,
                                  const Quantity bestAskQuantity)
    {
        return MarketEvent {
            .instrument = INSTRUMENT,
            .sequence = SequenceNumber { 1 },
            .exchangeTimestamp = Timestamp {},
            .receiveTimestamp = Timestamp {},
            .bestBid = BID_PRICE,
            .bestBidQuantity = bestBidQuantity,
            .bestAsk = ASK_PRICE,
            .bestAskQuantity = bestAskQuantity
        };
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 100.
            Ask quantity = 0.

        Expected:
            Bid-side imbalance is 100%, therefore the strategy returns Buy.
    */
    void testStrongBuyImbalance()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 100 }, Quantity {});

        Assert(strategy.evaluate(event) == Signal::Buy, "strong bid imbalance must produce Buy");
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 0.
            Ask quantity = 100.

        Expected:
            Ask-side imbalance is 100%, therefore the strategy returns Sell.
    */
    void testStrongSellImbalance()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity {}, Quantity { 100 });

        Assert(strategy.evaluate(event) == Signal::Sell, "strong ask imbalance must produce Sell");
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 85.
            Ask quantity = 15.

            Imbalance = (85 - 15) / (85 + 15) = 0.70.

        Expected:
            Imbalance exactly reaches the threshold, therefore the strategy
            returns Buy.
    */
    void testBuyAtExactThreshold()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 85 }, Quantity { 15 });

        Assert(strategy.evaluate(event) == Signal::Buy, "imbalance at threshold must produce Buy");
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 84.
            Ask quantity = 16.

            Imbalance = (84 - 16) / (84 + 16) = 0.68.

        Expected:
            Imbalance is below the threshold, therefore the strategy returns None.
    */
    void testBuyBelowThreshold()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 84 }, Quantity { 16 });

        Assert(strategy.evaluate(event) == Signal::None, "imbalance below threshold must return None");
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 15.
            Ask quantity = 85.

            Imbalance = (85 - 15) / (85 + 15) = 0.70.

        Expected:
            Imbalance exactly reaches the threshold, therefore the strategy
            returns Sell.
    */
    void testSellAtExactThreshold()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 15 }, Quantity { 85 });

        Assert(strategy.evaluate(event) == Signal::Sell, "imbalance at threshold must produce Sell");
    }

    /*
        Input:
            Default threshold = 7 / 10.
            Bid quantity = 16.
            Ask quantity = 84.

            Imbalance = (84 - 16) / (84 + 16) = 0.68.

        Expected:
            Imbalance is below the threshold, therefore the strategy returns None.
    */
    void testSellBelowThreshold()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 16 }, Quantity { 84 });

        Assert(strategy.evaluate(event) == Signal::None, "imbalance below threshold must return None");
    }

    /*
        Input:
            Bid quantity = 100.
            Ask quantity = 100.

        Expected:
            There is no imbalance, therefore the strategy returns None.
    */
    void testEqualQuantities()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 100 }, Quantity { 100 });

        Assert(strategy.evaluate(event) == Signal::None, "equal quantities must return None");
    }

    /*
        Input:
            Bid quantity = 0.
            Ask quantity = 0.

        Expected:
            Total quantity is zero, therefore the strategy returns None.
    */
    void testZeroTotalQuantity()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity {}, Quantity {});

        Assert(strategy.evaluate(event) == Signal::None, "zero total quantity must return None");
    }

    /*
        Input:
            Bid quantity = 100.
            Ask quantity = 0.

        Expected:
            Zero ask liquidity produces a valid 100% bid imbalance and Buy.
    */
    void testZeroAskQuantity()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity { 100 }, Quantity {});

        Assert(strategy.evaluate(event) == Signal::Buy, "zero ask quantity must produce Buy");
    }

    /*
        Input:
            Bid quantity = 0.
            Ask quantity = 100.

        Expected:
            Zero bid liquidity produces a valid 100% ask imbalance and Sell.
    */
    void testZeroBidQuantity()
    {
        const ImbalanceStrategy strategy {};
        const MarketEvent event = createMarketEvent(Quantity {}, Quantity { 100 });

        Assert(strategy.evaluate(event) == Signal::Sell, "zero bid quantity must produce Sell");
    }

    /*
        Input:
            Custom threshold = 1 / 2.
            Bid quantity = 75.
            Ask quantity = 25.

            Imbalance = 0.50.

        Expected:
            Imbalance exactly reaches the custom threshold, therefore Buy.
    */
    void testCustomThresholdBuy()
    {
        constexpr ImbalanceStrategy::Value thresholdNumerator { 1 };
        constexpr ImbalanceStrategy::Value thresholdDenominator { 2 };

        const ImbalanceStrategy strategy { thresholdNumerator, thresholdDenominator };
        const MarketEvent event = createMarketEvent(Quantity { 75 }, Quantity { 25 });

        Assert(strategy.evaluate(event) == Signal::Buy, "custom threshold must be applied for Buy");
    }

    /*
        Input:
            Custom threshold = 1 / 2.
            Bid quantity = 25.
            Ask quantity = 75.

            Imbalance = 0.50.

        Expected:
            Imbalance exactly reaches the custom threshold, therefore Sell.
    */
    void testCustomThresholdSell()
    {
        constexpr ImbalanceStrategy::Value thresholdNumerator { 1 };
        constexpr ImbalanceStrategy::Value thresholdDenominator { 2 };

        const ImbalanceStrategy strategy { thresholdNumerator, thresholdDenominator };
        const MarketEvent event = createMarketEvent(Quantity { 25 }, Quantity { 75 });

        Assert(strategy.evaluate(event) == Signal::Sell, "custom threshold must be applied for Sell");
    }

    /*
        Input:
            Custom threshold = 1 / 2.
            Bid quantity = 60.
            Ask quantity = 40.

            Imbalance = 0.20.

        Expected:
            Imbalance is below the custom threshold, therefore None.
    */
    void testCustomThresholdBelowLimit()
    {
        constexpr ImbalanceStrategy::Value thresholdNumerator { 1 };
        constexpr ImbalanceStrategy::Value thresholdDenominator { 2 };

        const ImbalanceStrategy strategy { thresholdNumerator, thresholdDenominator };
        const MarketEvent event = createMarketEvent(Quantity { 60 }, Quantity { 40 });

        Assert(strategy.evaluate(event) == Signal::None, "imbalance below custom threshold must return None");
    }

    /*
        Input:
            Bid quantity = 85.
            Ask quantity = 15.

            Positive imbalance = 0.70.

        Expected:
            Strategy returns Buy.

            Swapping bid and ask produces the same magnitude of imbalance
            with opposite direction, therefore Sell.
    */
    void testBuySellSymmetry()
    {
        const ImbalanceStrategy strategy {};

        const MarketEvent buyEvent = createMarketEvent(Quantity { 85 }, Quantity { 15 });
        const MarketEvent sellEvent = createMarketEvent(Quantity { 15 }, Quantity { 85 });

        Assert(strategy.evaluate(buyEvent) == Signal::Buy, "positive imbalance must produce Buy");
        Assert(strategy.evaluate(sellEvent) == Signal::Sell, "negative imbalance must produce Sell");
    }

    /*
        Input:
            Very large bid and ask quantities.

        Expected:
            Calculation must remain correct without overflow in the
            intermediate multiplication used by the threshold comparison.
    */
    void testLargeQuantities()
    {
        const ImbalanceStrategy strategy {};

        constexpr Quantity bidQuantity { 8'500'000'000'000'000'000LL };
        constexpr Quantity askQuantity { 1'500'000'000'000'000'000LL };

        const MarketEvent event = createMarketEvent(bidQuantity, askQuantity);

        Assert(strategy.evaluate(event) == Signal::Buy, "large quantities must produce correct Buy signal");
    }
}

void imbalance_strategy_test()
{
    testStrongBuyImbalance();
    testStrongSellImbalance();

    testBuyAtExactThreshold();
    testBuyBelowThreshold();
    testSellAtExactThreshold();
    testSellBelowThreshold();

    testEqualQuantities();
    testZeroTotalQuantity();
    testZeroAskQuantity();
    testZeroBidQuantity();

    testCustomThresholdBuy();
    testCustomThresholdSell();
    testCustomThresholdBelowLimit();

    testBuySellSymmetry();
    testLargeQuantities();

    std::cout << "All ImbalanceStrategy tests: OK\n";
}