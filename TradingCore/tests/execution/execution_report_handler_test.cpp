/**============================================================================
Name        : execution_report_handler_test.cpp
Created on  : 20.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler_test.cpp
============================================================================**/

#include "trade_recorder.hpp"
#include "execution_report_handler.hpp"
#include "test_support/testing.hpp"

#include <iostream>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::ExecType;

using trading::market_data::MarketEvent;
using trading::execution::ExecutionReport;
using trading::execution::ExecutionReportHandler;
using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderManager;
using trading::execution::OrderRequest;
using trading::risk::IRiskManager;
using trading::risk::RiskReason;
using trading::risk::RiskResult;
using trading::position::Position;
using trading::position::PositionManager;

using trading::recording::IRecorder;
using trading::recording::TradeRecorder;


namespace
{
    using testing::Assert;

    class TestRiskManager final : public IRiskManager
    {
    public:
        RiskResult checkOrder(const OrderRequest&,
                              const Position&) override
        {
            return RiskResult::Accepted;
        }

        [[nodiscard]]
        RiskReason lastReason() const noexcept override
        {
            return RiskReason::None;
        }
    };


    class TestExecutionGateway final : public IExecutionGateway
    {
    public:
        void send(const Order& order) override
        {
            sentOrder_ = order;
            sendCount_++;
        }

        void cancel(const OrderId orderId) override
        {
            cancelledOrderId_ = orderId;
            cancelCount_++;
        }

        [[nodiscard]]
        uint32_t sendCount() const noexcept
        {
            return sendCount_;
        }

    private:
        Order sentOrder_;
        OrderId cancelledOrderId_ { 0 };
        uint32_t sendCount_ { 0 };
        uint32_t cancelCount_ { 0 };
    };


    class TestRecorder final : public IRecorder
    {
    public:
        void record(const MarketEvent&) override
        {
            marketEventRecordCount_++;
        }

        void record(const ExecutionReport& report) override
        {
            executionReport_ = report;
            executionReportRecordCount_++;
        }

        [[nodiscard]]
        uint32_t executionReportRecordCount() const noexcept
        {
            return executionReportRecordCount_;
        }

        [[nodiscard]]
        const ExecutionReport& executionReport() const noexcept
        {
            return executionReport_;
        }

    private:
        ExecutionReport executionReport_;
        uint32_t executionReportRecordCount_ { 0 };
        uint32_t marketEventRecordCount_ { 0 };
    };


    ExecutionReport createExecutionReport(const OrderId orderId)
    {
        return ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        };
    }


    void testApplyExecution()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { InstrumentId { 1 } };
        OrderManager orderManager { gateway, riskManager, position };

        TestRecorder recorder;
        PositionManager positionManager { };
        const ExecutionReportHandler handler {orderManager,positionManager,recorder};

        const OrderId orderId = orderManager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const ExecutionReport report = createExecutionReport(orderId);
        const bool applied = handler.onExecutionReport(report);
        Assert(applied, "execution report must be applied");
    }


    void testExecutionReportIsRecorded()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { InstrumentId { 1 } };
        OrderManager orderManager { gateway, riskManager, position };

        TestRecorder recorder;
        PositionManager positionManager { };
        const ExecutionReportHandler handler {orderManager, positionManager,recorder};

        const OrderId orderId = orderManager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const ExecutionReport report = createExecutionReport(orderId);

        const auto _ = handler.onExecutionReport(report);
        Assert(recorder.executionReportRecordCount() == 1, "execution report must be recorded once");
    }


    void testExactExecutionReportIsRecorded()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { InstrumentId { 1 } };
        OrderManager orderManager { gateway, riskManager, position };

        TestRecorder recorder;
        PositionManager positionManager { };

        const ExecutionReportHandler handler {orderManager, positionManager, recorder};
        const OrderId orderId = orderManager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const ExecutionReport report = createExecutionReport(orderId);
        const auto _ = handler.onExecutionReport(report);

        const ExecutionReport& recorded = recorder.executionReport();
        Assert(recorded.clientOrderId == report.clientOrderId,
            "invalid recorded client order id");
        Assert(recorded.exchangeOrderId == report.exchangeOrderId,
            "invalid recorded exchange order id");
        Assert(recorded.instrument == report.instrument,
            "invalid recorded instrument");
        Assert(recorded.side == report.side,
            "invalid recorded side");
        Assert(recorded.execType == report.execType,
            "invalid recorded execution type");
        Assert(recorded.status == report.status,
            "invalid recorded status");
        Assert(recorded.price == report.price,
            "invalid recorded price");
        Assert(recorded.quantity == report.quantity,
            "invalid recorded quantity");
        Assert(recorded.filledQuantity == report.filledQuantity,
            "invalid recorded filled quantity");
    }

    void testOrderIsUpdated()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { InstrumentId { 1 } };
        OrderManager orderManager { gateway, riskManager, position };

        TestRecorder recorder;
        PositionManager positionManager { };

        const ExecutionReportHandler handler {orderManager,positionManager,recorder};
        const OrderId orderId = orderManager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const auto _ = handler.onExecutionReport(createExecutionReport(orderId));
        const Order* order = orderManager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->exchangeOrderId == ExchangeOrderId { 1001 },
            "exchange order id must be updated");
        Assert(order->status == OrderStatus::Filled,
            "order status must be Filled");
        Assert(order->filledQuantity == Quantity { 100'000'000 },
            "filled quantity must be updated");
    }

    void testUnknownOrderExecutionFails()
    {
        TestExecutionGateway gateway;
        TestRiskManager riskManager;
        Position position { InstrumentId { 1 } };
        OrderManager orderManager { gateway, riskManager, position };

        TestRecorder recorder;
        PositionManager positionManager { };

        constexpr OrderId unknownOrderId = OrderId { 42 };
        const ExecutionReportHandler handler { orderManager,positionManager,recorder};
        const ExecutionReport report = createExecutionReport(unknownOrderId);
        const bool applied = handler.onExecutionReport(report);

        const Position* resPosition = positionManager.find(unknownOrderId);

        Assert(!applied, "unknown order execution must fail");
        Assert(recorder.executionReportRecordCount() == 1,"execution report must still be recorded");
        Assert(resPosition == nullptr, "Position for unknown order execution must not exist");
    }
}


void execution_report_handler_test()
{
    testApplyExecution();
    testExecutionReportIsRecorded();
    testExactExecutionReportIsRecorded();
    testOrderIsUpdated();
    testUnknownOrderExecutionFails();

    std::cout << "All ExecutionReportHandler tests: OK\n";
}