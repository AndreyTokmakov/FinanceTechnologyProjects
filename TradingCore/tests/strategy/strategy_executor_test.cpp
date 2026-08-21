/**============================================================================
Name        : strategy_executor_test.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : StrategyExecutor unit tests.
============================================================================**/

#include "strategy_executor.hpp"
#include "test_support/testing.hpp"

#include "order_manager.hpp"
#include "order.hpp"
#include "execution_gateway.hpp"
#include "risk_manager.hpp"
#include "position.hpp"

#include <iostream>

#include "imbalance_strategy.hpp"

using trading::ExchangeOrderId;
using trading::ExecType;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::OrderType;

using trading::execution::ExecutionReport;
using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderManager;

using trading::market_data::MarketEvent;

using trading::position::Position;

using trading::risk::IRiskManager;
using trading::risk::RiskLimits;
using trading::risk::RiskManager;
using trading::risk::RiskReason;
using trading::risk::RiskResult;

using trading::strategy::ImbalanceStrategy;
using trading::strategy::Signal;
using trading::strategy::StrategyExecutor;
using trading::strategy::StrategyExecutionResult;

namespace
{
    using testing::Assert;


    class TestExecutionGateway final : public IExecutionGateway
    {
    public:
        void send(const Order& order) override
        {
            sentOrder = order;
            ++sendCount;
        }

        void cancel(const OrderId orderId) override
        {
            cancelledOrderId = orderId;
            ++cancelCount;
        }

        [[nodiscard]]
        std::size_t sendCountValue() const noexcept
        {
            return sendCount;
        }

        [[nodiscard]]
        const Order& sentOrderValue() const noexcept {
            return sentOrder;
        }

        [[nodiscard]]
        std::size_t cancelCountValue() const noexcept {
            return cancelCount;
        }

        [[nodiscard]]
        OrderId cancelledOrderIdValue() const noexcept {
            return cancelledOrderId;
        }

    private:
        Order sentOrder {};
        OrderId cancelledOrderId {};
        std::size_t sendCount { 0 };
        std::size_t cancelCount { 0 };
    };


    class RejectingRiskManager final : public IRiskManager
    {
    public:
        explicit RejectingRiskManager(const RiskReason reason) noexcept : reason { reason }{
        }

        [[nodiscard]]
        RiskResult checkOrder(const trading::execution::OrderRequest&, const Position&) override {
            return RiskResult::Rejected;
        }

        [[nodiscard]]
        RiskReason lastReason() const noexcept override{
            return reason;
        }

    private:
        RiskReason reason;
    };


    MarketEvent createMarketEvent()
    {
        MarketEvent event {};

        event.instrument = InstrumentId { 42 };
        event.bestBid = Price { 6'500'000'000'000 };
        event.bestBidQuantity = Quantity { 300'000'000 };
        event.bestAsk = Price { 6'500'001'000'000 };
        event.bestAskQuantity = Quantity { 200'000'000 };

        return event;
    }


    void testNoneSignalDoesNotCreateOrder()
    {
        // Input: Signal::None and a valid market event.
        // Expected: no order is created and gateway is not called.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::None, event);

        Assert(result.has_value(), "None signal must not produce an execution error");
        Assert(!result.value().has_value(), "None signal must not create an order");
        Assert(gateway.sendCountValue() == 0, "gateway must not be called for None signal");
    }


    void testBuySignalCreatesBuyLimitOrder()
    {
        // Input: Buy signal with best bid 6500 and best ask 6500.001.
        // Expected: Buy Limit order is created at best ask.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::Buy, event);

        Assert(result.has_value(), "Buy signal must create an order");
        Assert(result.value().has_value(), "Buy signal must return an order id");
        Assert(gateway.sendCountValue() == 1, "gateway must be called once");

        const Order& order = gateway.sentOrderValue();

        Assert(order.instrument == InstrumentId { 42 }, "invalid order instrument");
        Assert(order.side == Side::Buy, "invalid order side");
        Assert(order.type == OrderType::Limit, "invalid order type");
        Assert(order.price == event.bestAsk, "Buy order must use best ask price");
        Assert(order.quantity == orderQuantity, "invalid order quantity");
    }


    void testSellSignalCreatesSellLimitOrder()
    {
        // Input: Sell signal with best bid 6500 and best ask 6500.001.
        // Expected: Sell Limit order is created at best bid.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::Sell, event);

        Assert(result.has_value(), "Sell signal must create an order");
        Assert(result.value().has_value(), "Sell signal must return an order id");
        Assert(gateway.sendCountValue() == 1, "gateway must be called once");

        const Order& order = gateway.sentOrderValue();

        Assert(order.instrument == InstrumentId { 42 }, "invalid order instrument");
        Assert(order.side == Side::Sell, "invalid order side");
        Assert(order.type == OrderType::Limit, "invalid order type");
        Assert(order.price == event.bestBid, "Sell order must use best bid price");
        Assert(order.quantity == orderQuantity, "invalid order quantity");
    }


    void testOrderIdIsReturned()
    {
        // Input: Buy signal with a valid market event.
        // Expected: StrategyExecutor returns the OrderId created by OrderManager.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::Buy, event);

        Assert(result.has_value(), "order creation must succeed");
        Assert(result.value().has_value(), "successful order creation must return OrderId");
        Assert(result.value().value() != OrderId {}, "returned OrderId must be valid");
        Assert(gateway.sendCountValue() == 1, "gateway must be called once");
    }


    void testConfiguredOrderQuantityIsUsed()
    {
        // Input: Buy signal and configured execution quantity of 250'000'000.
        // Expected: created order uses exactly the configured quantity.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 250'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::Buy, event);

        Assert(result.has_value(), "order creation must succeed");
        Assert(gateway.sendCountValue() == 1, "gateway must be called once");
        Assert(gateway.sentOrderValue().quantity == orderQuantity, "invalid configured order quantity");
    }


    void testEventInstrumentIsUsed()
    {
        // Input: Buy signal for instrument 777.
        // Expected: created order belongs to instrument 777.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 777 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };

        MarketEvent event = createMarketEvent();
        event.instrument = InstrumentId { 777 };

        const StrategyExecutionResult result = executor.execute(Signal::Buy, event);

        Assert(result.has_value(), "order creation must succeed");
        Assert(gateway.sentOrderValue().instrument == InstrumentId { 777 }, "invalid order instrument");
    }


    void testRiskRejectionIsReturned()
    {
        // Input: Buy signal and RiskManager rejecting the order because of position limit.
        // Expected: order creation fails and execution gateway is not called.

        TestExecutionGateway gateway;
        RejectingRiskManager riskManager { RiskReason::MaxPositionQuantity };
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult result = executor.execute(Signal::Buy, event);

        Assert(!result.has_value(), "risk rejection must fail order creation");
        Assert(result.error() == trading::execution::OrderCreationError::RiskRejected,
            "invalid order creation error");
        Assert(gateway.sendCountValue() == 0, "gateway must not be called after risk rejection");
    }

    void testBuyAndSellUseDifferentMarketPrices()
    {
        // Input: Buy and Sell signals for the same market event.
        // Expected: Buy uses best ask and Sell uses best bid.

        TestExecutionGateway gateway;
        RiskManager riskManager {};
        Position position { InstrumentId { 42 } };
        OrderManager orderManager { gateway, riskManager, position };

        constexpr Quantity orderQuantity { 100'000'000 };
        const StrategyExecutor executor { orderManager, orderQuantity };
        const MarketEvent event = createMarketEvent();

        const StrategyExecutionResult buyResult = executor.execute(Signal::Buy, event);
        const Price buyPrice = gateway.sentOrderValue().price;

        const StrategyExecutionResult sellResult = executor.execute(Signal::Sell, event);
        const Price sellPrice = gateway.sentOrderValue().price;

        Assert(buyResult.has_value(), "Buy order creation must succeed");
        Assert(sellResult.has_value(), "Sell order creation must succeed");
        Assert(buyPrice == event.bestAsk, "Buy must use best ask");
        Assert(sellPrice == event.bestBid, "Sell must use best bid");
        Assert(buyPrice != sellPrice, "Buy and Sell must use different market prices");
    }

}


void strategy_executor_test()
{
    testNoneSignalDoesNotCreateOrder();

    testBuySignalCreatesBuyLimitOrder();
    testSellSignalCreatesSellLimitOrder();

    testOrderIdIsReturned();
    testConfiguredOrderQuantityIsUsed();
    testEventInstrumentIsUsed();

    testRiskRejectionIsReturned();

    testBuyAndSellUseDifferentMarketPrices();

    std::cout << "All StrategyExecutor tests: OK\n";
}
