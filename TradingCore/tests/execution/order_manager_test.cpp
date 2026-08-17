/**============================================================================
Name        : order_manager_test.cpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager_test.cpp
============================================================================**/

#include "order_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

using trading::ExchangeOrderId;
using trading::InstrumentId;
using trading::OrderId;
using trading::OrderStatus;
using trading::OrderType;
using trading::Price;
using trading::Quantity;
using trading::Side;
using trading::ExecType;

using trading::execution::ExecutionReport;
using trading::execution::IExecutionGateway;
using trading::execution::Order;
using trading::execution::OrderManager;
using trading::execution::OrderRequest;

namespace
{
    void Assert(const bool condition, const std::string_view message)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            std::terminate();
        }
    }

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
        const Order& sentOrder() const noexcept
        {
            return sentOrder_;
        }

        [[nodiscard]]
        OrderId cancelledOrderId() const noexcept
        {
            return cancelledOrderId_;
        }

        [[nodiscard]]
        uint32_t sendCount() const noexcept
        {
            return sendCount_;
        }

        [[nodiscard]]
        uint32_t cancelCount() const noexcept
        {
            return cancelCount_;
        }

    private:
        Order sentOrder_;
        OrderId cancelledOrderId_ { 0 };
        uint32_t sendCount_ { 0 };
        uint32_t cancelCount_ { 0 };
    };

    void testCreateOrder()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderId orderId = manager.createOrder(request);

        Assert(orderId == 1, "first order id must be one");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "created order must exist");
        Assert(order->clientOrderId == orderId, "invalid client order id");
        Assert(order->instrument == InstrumentId { 1 }, "invalid instrument");
        Assert(order->side == Side::Buy, "invalid side");
        Assert(order->type == OrderType::Limit, "invalid order type");
        Assert(order->price == Price { 6'500'000'000'000 }, "invalid price");
        Assert(order->quantity == Quantity { 100'000'000 }, "invalid quantity");
        Assert(order->filledQuantity.isZero(), "new order must have zero filled quantity");
        Assert(order->status == OrderStatus::New, "new order must have New status");
    }

    void testOrderIdsAreUnique()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderId first = manager.createOrder(request);
        const OrderId second = manager.createOrder(request);
        const OrderId third = manager.createOrder(request);

        Assert(first == 1, "first order id must be one");
        Assert(second == 2, "second order id must be two");
        Assert(third == 3, "third order id must be three");

        Assert(manager.find(first) != nullptr, "first order must exist");
        Assert(manager.find(second) != nullptr, "second order must exist");
        Assert(manager.find(third) != nullptr, "third order must exist");
    }

    void testOrderIsSentToGateway()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 42 },
            .side = Side::Sell,
            .type = OrderType::Limit,
            .price = Price { 6'500'001'000'000 },
            .quantity = Quantity { 200'000'000 }
        };

        const OrderId orderId = manager.createOrder(request);

        Assert(gateway.sendCount() == 1, "gateway send must be called once");

        const Order& sentOrder = gateway.sentOrder();

        Assert(sentOrder.clientOrderId == orderId, "invalid sent order id");
        Assert(sentOrder.instrument == InstrumentId { 42 }, "invalid sent instrument");
        Assert(sentOrder.side == Side::Sell, "invalid sent side");
        Assert(sentOrder.type == OrderType::Limit, "invalid sent order type");
        Assert(sentOrder.price == Price { 6'500'001'000'000 }, "invalid sent price");
        Assert(sentOrder.quantity == Quantity { 200'000'000 }, "invalid sent quantity");
    }

    void testFindUnknownOrder()
    {
        TestExecutionGateway gateway;
        const OrderManager manager { gateway };
        const Order* order = manager.find(OrderId { 42 });

        Assert(order == nullptr, "unknown order must not be found");
    }

    void testApplyNewExecutionReport()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        constexpr OrderRequest request {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        };

        const OrderId orderId = manager.createOrder(request);

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::New,
            .status = OrderStatus::New,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "New execution report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(
            order->exchangeOrderId == ExchangeOrderId { 1001 },
            "exchange order id must be updated");
        Assert(order->status == OrderStatus::New, "invalid order status");
        Assert(order->filledQuantity.isZero(), "filled quantity must be zero");
    }

    void testApplyPartialFill()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const OrderId orderId = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Trade,
            .status = OrderStatus::PartiallyFilled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 40'000'000 },
            .filledQuantity = Quantity { 40'000'000 }
        });

        Assert(applied, "partial fill report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(
            order->status == OrderStatus::PartiallyFilled,
            "order must be partially filled");
        Assert(
            order->filledQuantity == Quantity { 40'000'000 },
            "invalid filled quantity");
    }

    void testApplyFilled()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const OrderId orderId = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        });

        Assert(applied, "filled report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Filled, "order must be filled");
        Assert(
            order->filledQuantity == Quantity { 100'000'000 },
            "invalid filled quantity");
    }

    void testApplyCancelled()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const OrderId orderId = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Cancel,
            .status = OrderStatus::Cancelled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "cancel report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Cancelled, "order must be cancelled");
    }

    void testApplyRejected()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const OrderId orderId = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = orderId,
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Reject,
            .status = OrderStatus::Rejected,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity {}
        });

        Assert(applied, "reject report must be applied");

        const Order* order = manager.find(orderId);

        Assert(order != nullptr, "order must exist");
        Assert(order->status == OrderStatus::Rejected, "order must be rejected");
    }

    void testUnknownExecutionReport()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const bool applied = manager.applyExecutionReport(ExecutionReport {
            .clientOrderId = OrderId { 42 },
            .exchangeOrderId = ExchangeOrderId { 1001 },
            .execType = ExecType::Trade,
            .status = OrderStatus::Filled,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 },
            .filledQuantity = Quantity { 100'000'000 }
        });

        Assert(!applied, "report for unknown order must be rejected");
    }

    void testCancelOrder()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const OrderId orderId = manager.createOrder(OrderRequest {
            .instrument = InstrumentId { 1 },
            .side = Side::Buy,
            .type = OrderType::Limit,
            .price = Price { 6'500'000'000'000 },
            .quantity = Quantity { 100'000'000 }
        });

        const bool cancelled = manager.cancel(orderId);

        Assert(cancelled, "cancel must succeed");
        Assert(gateway.cancelCount() == 1, "gateway cancel must be called once");
        Assert(
            gateway.cancelledOrderId() == orderId,
            "gateway must receive correct order id");
    }

    void testCancelUnknownOrder()
    {
        TestExecutionGateway gateway;
        OrderManager manager { gateway };

        const bool cancelled = manager.cancel(OrderId { 42 });

        Assert(!cancelled, "cancel of unknown order must fail");
        Assert(gateway.cancelCount() == 0, "gateway cancel must not be called");
    }

}

void order_manager_test()
{
    testCreateOrder();
    testOrderIdsAreUnique();
    testOrderIsSentToGateway();
    testFindUnknownOrder();

    testApplyNewExecutionReport();
    testApplyPartialFill();
    testApplyFilled();
    testApplyCancelled();
    testApplyRejected();
    testUnknownExecutionReport();

    testCancelOrder();
    testCancelUnknownOrder();

    std::cout << "All OrderManager tests: OK\n";
}