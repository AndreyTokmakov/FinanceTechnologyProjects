/**============================================================================
Name        : risk_manager_test.cpp
Created on  : 18.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : risk_manager_test.cpp
============================================================================**/

/*
    Tests for RiskManager.

    Data Flow:

        OrderRequest + Position
                  |
                  v
             RiskManager
                  |
          +-------+-------+
          |               |
       Accepted         Rejected
                          |
                          v
                      RiskReason

    The tests cover:

        - accepted orders;
        - maximum order quantity;
        - maximum resulting position;
        - maximum notional;
        - long positions;
        - short positions;
        - disabled limits.
*/

#include "risk_manager.hpp"
#include "test_support/testing.hpp"

#include <cstdlib>
#include <iostream>

using trading::InstrumentId;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;

using trading::execution::OrderRequest;
using trading::position::Position;
using trading::risk::RiskLimits;
using trading::risk::RiskManager;
using trading::risk::RiskReason;
using trading::risk::RiskResult;

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
    constexpr OrderRequest limitOrder(const Side side,
                                      const Price orderPrice,
                                      const Quantity orderQuantity)
    {
        return OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = side,
            .type = OrderType::Limit,
            .price = orderPrice,
            .quantity = orderQuantity
        };
    }

    void testAcceptOrder()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(100),
                .maxPositionQuantity = quantity(1'000),
                .maxNotional = price(1'000'000)
            }
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr auto request = limitOrder(Side::Buy, price(100), quantity(10));
        const RiskResult result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Accepted, "valid order must be accepted");
        Assert(manager.lastReason() == RiskReason::None, "reason must be None");
    }

    void testRejectMaxOrderQuantity()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(100),
                .maxPositionQuantity = quantity(1'000),
                .maxNotional = price(1'000'000)
            }
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr auto request = limitOrder(Side::Buy, price(100), quantity(101));
        const RiskResult result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Rejected, "oversized order must be rejected");
        Assert(manager.lastReason() == RiskReason::MaxOrderQuantity, "invalid rejection reason");
    }

    void testRejectMaxLongPosition()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(100),
                .maxNotional = price(1'000'000)
            }
        };

        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(80));

        constexpr auto request = limitOrder(Side::Buy, price(100), quantity(30));
        const RiskResult result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Rejected, "position limit must reject order");
        Assert(manager.lastReason() == RiskReason::MaxPositionQuantity, "invalid position rejection reason");
    }

    void testAcceptReducingLongPosition()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(100),
                .maxNotional = price(1'000'000)
            }
        };

        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Buy, price(100), quantity(80));
        constexpr auto request = limitOrder(Side::Sell, price(100), quantity(30));
        const RiskResult result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Accepted, "position reduction must be accepted");
    }

    void testRejectMaxShortPosition()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(100),
                .maxNotional = price(1'000'000)
            }
        };

        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(80));

        constexpr OrderRequest request = limitOrder(Side::Sell, price(100), quantity(30));
        const RiskResult result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Rejected, "short position limit must reject order");
        Assert(manager.lastReason() == RiskReason::MaxPositionQuantity,
            "invalid short position rejection reason");
    }

    void testAcceptReducingShortPosition()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(100),
                .maxNotional = price(1'000'000)
            }
        };

        Position position { InstrumentId { 1 } };
        position.applyTrade(Side::Sell, price(100), quantity(80));

        constexpr OrderRequest request = limitOrder(Side::Buy, price(100), quantity(30));
        const auto result = manager.checkOrder(request, position);
        Assert(result == RiskResult::Accepted, "position reduction must be accepted");
    }

    void testRejectMaxNotional()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(1'000),
                .maxNotional = price(10'000)
            }
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr OrderRequest request = limitOrder(Side::Buy, price(200), quantity(60));
        const auto result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Rejected, "large notional must be rejected");
        Assert(manager.lastReason() == RiskReason::MaxNotional,"invalid notional rejection reason");
    }

    void testAcceptOrderAtNotionalLimit()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(1'000),
                .maxPositionQuantity = quantity(1'000),
                .maxNotional = price(10'000)
            }
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr OrderRequest request = limitOrder(Side::Buy, price(100), quantity(100));
        const auto result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Accepted, "order at limit must be accepted");
    }

    void testDisabledLimits()
    {
        RiskManager manager {
            RiskLimits {}
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr OrderRequest request = limitOrder(Side::Buy, price(1'000'000), quantity(1'000'000));
        const auto result = manager.checkOrder(request, position);

        Assert(result == RiskResult::Accepted, "disabled limits must accept order");
        Assert(manager.lastReason() == RiskReason::None, "reason must be None");
    }

    void testReasonIsResetAfterAcceptedOrder()
    {
        RiskManager manager {
            RiskLimits {
                .maxOrderQuantity = quantity(100),
                .maxPositionQuantity = quantity(1'000),
                .maxNotional = price(1'000'000)
            }
        };

        constexpr Position position { InstrumentId { 1 } };
        constexpr auto rejected =limitOrder(Side::Buy, price(100), quantity(101));

        Assert(manager.checkOrder(rejected, position) == RiskResult::Rejected,
            "first order must be rejected");
        Assert(manager.lastReason() == RiskReason::MaxOrderQuantity,
            "invalid first rejection reason");

        constexpr auto accepted =
            limitOrder(Side::Buy, price(100), quantity(10));

        Assert(manager.checkOrder(accepted, position) == RiskResult::Accepted,
            "second order must be accepted");
        Assert(manager.lastReason() == RiskReason::None,
            "reason must be reset after accepted order");
    }
}

void risk_manager_test()
{
    testAcceptOrder();
    testRejectMaxOrderQuantity();

    testRejectMaxLongPosition();
    testAcceptReducingLongPosition();

    testRejectMaxShortPosition();
    testAcceptReducingShortPosition();

    testRejectMaxNotional();
    testAcceptOrderAtNotionalLimit();

    testDisabledLimits();
    testReasonIsResetAfterAcceptedOrder();

    std::cout << "All RiskManager tests: OK\n";
}