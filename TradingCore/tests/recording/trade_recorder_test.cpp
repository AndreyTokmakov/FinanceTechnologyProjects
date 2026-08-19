/**============================================================================
Name        : trade_recorder_test.cpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_recorder_test.cpp
============================================================================**/

#include "trade_recorder.hpp"
#include "test_support/testing.hpp"

#include <iostream>
#include <string_view>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::SequenceNumber;
using trading::Side;
using trading::Timestamp;
using trading::ExecType;

using trading::execution::ExecutionReport;
using trading::market_data::MarketEvent;
using trading::recording::TradeRecorder;

namespace
{
    using testing::Assert;

    void testEmptyRecorder()
    {
        const TradeRecorder recorder;

        Assert(recorder.marketEventCount() == 0, "new recorder must contain no market events");
        Assert(recorder.executionReportCount() == 0, "new recorder must contain no execution reports");
        Assert(recorder.marketEvents().empty(), "market events must be empty");
        Assert(recorder.executionReports().empty(), "execution reports must be empty");
    }

    void testRecordMarketEvent()
    {
        TradeRecorder recorder;

        constexpr MarketEvent event {
            .instrument = InstrumentId { 1 },
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp {},
            .receiveTimestamp = Timestamp {},
            .bestBid = Price { 6'500'000'000'000 },
            .bestBidQuantity = Quantity { 100'000'000 },
            .bestAsk = Price { 6'500'001'000'000 },
            .bestAskQuantity = Quantity { 200'000'000 }
        };

        recorder.record(event);

        Assert(recorder.marketEventCount() == 1,"one market event must be recorded");

        const auto& events = recorder.marketEvents();

        Assert(events.size() == 1, "market event collection must contain one event");
        Assert(events[0].instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(events[0].sequence == SequenceNumber { 100 }, "invalid sequence");
        Assert(events[0].bestBid == Price { 6'500'000'000'000 },"invalid best bid");
        Assert(events[0].bestBidQuantity == Quantity { 100'000'000 },"invalid best bid quantity");
        Assert(events[0].bestAsk == Price { 6'500'001'000'000 },"invalid best ask");
        Assert(events[0].bestAskQuantity == Quantity { 200'000'000 },"invalid best ask quantity");
    }

    void testRecordExecutionReport()
    {
        TradeRecorder recorder;

        constexpr ExecutionReport report {
            .clientOrderId = OrderId { 1 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        };

        recorder.record(report);

        Assert(recorder.executionReportCount() == 1,"one execution report must be recorded");

        const auto& reports = recorder.executionReports();

        Assert(reports.size() == 1,"execution report collection must contain one report");
        Assert(reports[0].clientOrderId == OrderId { 1 },"invalid client order id");
        Assert(reports[0].exchangeOrderId == ExchangeOrderId { 1001 },"invalid exchange order id");
        Assert(reports[0].execType == ExecType::Trade,"invalid execution type");
        Assert(reports[0].status == OrderStatus::Filled,"invalid order status");
        Assert(reports[0].price == Price { 6'500'000'000'000 },"invalid execution price");
        Assert(reports[0].quantity == Quantity { 100'000'000 },"invalid execution quantity");
    }

    void testMultipleEvents()
    {
        TradeRecorder recorder;

        constexpr MarketEvent first {
            .instrument = InstrumentId { 1 },
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp {},
            .receiveTimestamp = Timestamp {},
            .bestBid = Price { 6'500'000'000'000 },
            .bestBidQuantity = Quantity { 100'000'000 },
            .bestAsk = Price { 6'500'001'000'000 },
            .bestAskQuantity = Quantity { 200'000'000 }
        };

        constexpr MarketEvent second {
            .instrument = InstrumentId { 1 },
            .sequence = SequenceNumber { 101 },
            .exchangeTimestamp = Timestamp {},
            .receiveTimestamp = Timestamp {},
            .bestBid = Price { 6'500'002'000'000 },
            .bestBidQuantity = Quantity { 300'000'000 },
            .bestAsk = Price { 6'500'003'000'000 },
            .bestAskQuantity = Quantity { 400'000'000 }
        };

        recorder.record(first);
        recorder.record(second);

        Assert(recorder.marketEventCount() == 2,"two market events must be recorded");
        Assert(recorder.marketEvents()[0].sequence == SequenceNumber { 100 },"first event must preserve order");
        Assert(recorder.marketEvents()[1].sequence == SequenceNumber { 101 },"second event must preserve order");
    }

    void testClear()
    {
        TradeRecorder recorder;

        recorder.record(MarketEvent {
            .instrument = InstrumentId { 1 },
            .sequence = SequenceNumber { 100 },
            .exchangeTimestamp = Timestamp {},
            .receiveTimestamp = Timestamp {},
            .bestBid = Price { 6'500'000'000'000 },
            .bestBidQuantity = Quantity { 100'000'000 },
            .bestAsk = Price { 6'500'001'000'000 },
            .bestAskQuantity = Quantity { 200'000'000 }
        });

        recorder.clear();

        Assert(recorder.marketEventCount() == 0, "market events must be removed by clear");
        Assert(recorder.executionReportCount() == 0, "execution reports must be removed by clear");
    }
}

void trade_recorder_test()
{
    testEmptyRecorder();
    testRecordMarketEvent();
    testRecordExecutionReport();
    testMultipleEvents();
    testClear();

    std::cout << "All TradeRecorder tests: OK\n";
}