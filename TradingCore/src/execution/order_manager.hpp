/**============================================================================
Name        : order_manager.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : order_manager.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP
#define FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP


#include "execution_gateway.hpp"
#include "execution_report.hpp"
#include "order.hpp"

#include <map>

namespace trading::execution
{

    class OrderManager
    {
    public:
        explicit OrderManager(IExecutionGateway& gateway) noexcept;

        [[nodiscard]]
        OrderId createOrder(const OrderRequest& request);

        [[nodiscard]]
        bool applyExecutionReport(const ExecutionReport& report);

        [[nodiscard]]
        const Order* find(OrderId orderId) const noexcept;

        [[nodiscard]]
        bool cancel(OrderId orderId);

    private:
        IExecutionGateway& gateway;
        std::map<OrderId, Order> orders;
        OrderId nextOrderId { 1 };
    };

}

#endif //FINANCETECHNOLOGYPROJECTS_ORDER_MANAGER_HPP
