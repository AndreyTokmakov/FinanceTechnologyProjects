/**============================================================================
Name        : market_event_handler_test.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market event handler tests.
============================================================================**/

#include "market_event_handler.hpp"
#include "strategy.hpp"
#include "strategy_executor.hpp"
#include "order_manager.hpp"
#include "position.hpp"
#include "risk_manager.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;

using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderManager;

using trading::market_data::MarketEvent;
using trading::market_data::MarketEventHandler;

using trading::position::Position;

using trading::recording::IRecorder;

using trading::risk::IRiskManager;
using trading::risk::RiskReason;
using trading::risk::RiskResult;

using trading::strategy::IStrategy;
using trading::strategy::Signal;
using trading::strategy::StrategyExecutor;
using trading::strategy::StrategyExecutionResult;

namespace
{
    using testing::Assert;

    constexpr InstrumentId INSTRUMENT { 42 };
    constexpr Price BEST_BID { 6'500'000'000'000 };
    constexpr Price BEST_ASK { 6'500'100'000'000 };

    constexpr Quantity BEST_BID_QUANTITY { 700'000'000 };
    constexpr Quantity BEST_ASK_QUANTITY { 300'000'000 };
    constexpr Quantity ORDER_QUANTITY { 100'000'000 };


    MarketEvent createMarketEvent()
    {
        return MarketEvent {
            .instrument = INSTRUMENT,
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp { 1'000 },
            .receiveTimestamp = Timestamp { 2'000 },
            .bestBid = BEST_BID,
            .bestBidQuantity = BEST_BID_QUANTITY,
            .bestAsk = BEST_ASK,
            .bestAskQuantity = BEST_ASK_QUANTITY
        };
    }


    class TestStrategy final : public IStrategy
    {
    public:
        explicit TestStrategy(const Signal signal) noexcept :
            signal { signal }
        {
        }

        [[nodiscard]]
        Signal evaluate(const MarketEvent& event) const override
        {
            ++evaluateCount;
            lastEvent = &event;
            return signal;
        }

        mutable std::size_t evaluateCount { 0 };
        mutable const MarketEvent* lastEvent { nullptr };

    private:
        Signal signal;
    };

    class TestRecorder final : public IRecorder
    {
    public:
        void record(const MarketEvent& event) override
        {
            ++marketEventCount;
            lastMarketEvent = &event;
        }

        void record(const trading::execution::ExecutionReport&) override
        {
            ++executionReportCount;
        }

        std::size_t marketEventCount { 0 };
        std::size_t executionReportCount { 0 };
        const MarketEvent* lastMarketEvent { nullptr };
    };

    class TestExecutionGateway final : public IExecutionGateway
    {
    public:
        void send(const Order& order) override
        {
            ++sendCount;
            lastOrder = order;
        }

        void cancel(const OrderId orderId) override
        {
            ++cancelCount;
            lastCancelledOrderId = orderId;
        }

        [[nodiscard]]
        std::size_t sendCountValue() const noexcept
        {
            return sendCount;
        }

        [[nodiscard]]
        const Order& sentOrder() const noexcept
        {
            return lastOrder;
        }

    private:
        std::size_t sendCount { 0 };
        std::size_t cancelCount { 0 };
        Order lastOrder {};
        OrderId lastCancelledOrderId {};
    };

    class TestRiskManager final : public IRiskManager
    {
    public:
        explicit TestRiskManager(const RiskResult result = RiskResult::Accepted,
                                 const RiskReason reason = RiskReason::None) noexcept :
            result { result },
            reason { reason }
        {
        }

        [[nodiscard]]
        RiskResult checkOrder(const trading::execution::OrderRequest&,
                              const Position&) override
        {
            ++checkCount;
            return result;
        }

        [[nodiscard]]
        RiskReason lastReason() const noexcept override
        {
            return reason;
        }

        std::size_t checkCount { 0 };

    private:
        RiskResult result;
        RiskReason reason;
    };


    void testMarketEventIsRecorded()
    {
        // Input: one market event.
        // Expected: recorder receives exactly the same event once.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::None };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(recorder.marketEventCount == 1, "market event must be recorded once");
        Assert(recorder.lastMarketEvent == &event, "recorder must receive original market event");
    }


    void testStrategyReceivesMarketEvent()
    {
        // Input: one market event and a Buy strategy.
        // Expected: strategy evaluates the exact event once.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::Buy };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(strategy.evaluateCount == 1, "strategy must evaluate event once");
        Assert(strategy.lastEvent == &event, "strategy must receive original market event");
    }


    void testBuySignalCreatesBuyOrder()
    {
        // Input: Buy signal generated for a market event.
        // Expected: one Buy limit order is sent at the best ask price.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        TestStrategy strategy { Signal::Buy };
        TestRecorder recorder;

        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(gateway.sendCountValue() == 1, "Buy signal must send one order");

        const Order& order = gateway.sentOrder();

        Assert(order.instrument == INSTRUMENT, "invalid Buy order instrument");
        Assert(order.side == Side::Buy, "invalid Buy order side");
        Assert(order.price == BEST_ASK, "Buy order must use best ask price");
        Assert(order.quantity == ORDER_QUANTITY, "invalid Buy order quantity");
    }


    void testSellSignalCreatesSellOrder()
    {
        // Input: Sell signal generated for a market event.
        // Expected: one Sell limit order is sent at the best bid price.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::Sell };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(gateway.sendCountValue() == 1, "Sell signal must send one order");

        const Order& order = gateway.sentOrder();

        Assert(order.instrument == INSTRUMENT, "invalid Sell order instrument");
        Assert(order.side == Side::Sell, "invalid Sell order side");
        Assert(order.price == BEST_BID, "Sell order must use best bid price");
        Assert(order.quantity == ORDER_QUANTITY, "invalid Sell order quantity");
    }


    void testNoneSignalDoesNotCreateOrder()
    {
        // Input: None signal generated for a market event.
        // Expected: event is recorded and evaluated, but no order is sent.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::None };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(recorder.marketEventCount == 1, "market event must be recorded");
        Assert(strategy.evaluateCount == 1, "strategy must evaluate event once");
        Assert(gateway.sendCountValue() == 0, "None signal must not send an order");
    }


    void testRiskRejectionDoesNotSendOrder()
    {
        // Input: Buy signal and RiskManager rejecting the order.
        // Expected: event is processed but execution gateway is not called.

        TestExecutionGateway gateway;
        TestRiskManager riskManager {
            RiskResult::Rejected,
            RiskReason::MaxPositionQuantity
        };

        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::Buy };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent event = createMarketEvent();

        handler.onMarketEvent(event);

        Assert(recorder.marketEventCount == 1, "market event must be recorded");
        Assert(strategy.evaluateCount == 1, "strategy must evaluate event once");
        Assert(riskManager.checkCount == 1, "risk manager must check the order");
        Assert(gateway.sendCountValue() == 0, "rejected order must not be sent");
    }


    void testMultipleMarketEventsAreProcessed()
    {
        // Input: two consecutive market events.
        // Expected: both events are recorded and evaluated, and two orders are sent.

        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { INSTRUMENT };
        OrderManager orderManager { gateway, riskManager, position };

        TestStrategy strategy { Signal::Buy };
        TestRecorder recorder;

        StrategyExecutor executor { orderManager, ORDER_QUANTITY };
        MarketEventHandler handler { strategy, executor, recorder };

        const MarketEvent firstEvent = createMarketEvent();
        const MarketEvent secondEvent = createMarketEvent();

        handler.onMarketEvent(firstEvent);
        handler.onMarketEvent(secondEvent);

        Assert(recorder.marketEventCount == 2, "both market events must be recorded");
        Assert(strategy.evaluateCount == 2, "both market events must be evaluated");
        Assert(gateway.sendCountValue() == 2, "both Buy signals must create orders");
    }
}


void market_event_handler_test()
{
    testMarketEventIsRecorded();
    testStrategyReceivesMarketEvent();
    testBuySignalCreatesBuyOrder();
    testSellSignalCreatesSellOrder();
    testNoneSignalDoesNotCreateOrder();
    testRiskRejectionDoesNotSendOrder();
    testMultipleMarketEventsAreProcessed();

    std::cout << "All MarketEventHandler tests: OK\n";
}