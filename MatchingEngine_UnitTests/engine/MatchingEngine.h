/**============================================================================
Name        : MatchingEngine.h
Created on  : 03.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngine.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_H
#define FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_H

#include <vector>
#include <list>
#include <unordered_map>
#include <boost/container/flat_map.hpp>

#include "ObjectPool.h"
#include "Order.h"

struct Trade
{
    using Order = Common::Order;

    struct OrderInfo
    {
        Order::OrderID id = 0;
        Order::OrderID price = 0;
    };

    OrderInfo buyOrderInfo;
    OrderInfo sellOrderInfo;
    uint32_t  quantity = 0;

    static void addOrder(const Order& order, OrderInfo& orderInfo)
    {
        orderInfo.id = order.orderId;
        orderInfo.price = order.price;
    }

    Trade& setBuyOrder(const Order& order) {
        addOrder(order, buyOrderInfo);
        return *this;
    }

    Trade& setSellOrder(const Order& order) {
        addOrder(order, sellOrderInfo);
        return *this;
    }

    Trade& setQuantity(const uint32_t qnty) {
        quantity = qnty;
        return *this;
    }
};

struct Trades
{
    std::vector<Trade> trades {};

    Trades()
    {
        trades.reserve(10'000'000);
    }

    Trade& addTrade() {
        return trades.emplace_back();
    }
};


namespace Testing {
    struct OrderMatchingEngineTester;
}


class MatchingEngine
{
    using Order = Common::Order;
    using OrderPtr = std::unique_ptr<Order, Memory::ObjectPool<Order>::Deleter>;

    struct PriceLevel
    {
        std::list<OrderPtr> orders;
        uint32_t quantity { 0 };
    };

    struct ReferencesBlock
    {
        Order *order { nullptr };
        decltype(PriceLevel::orders)::iterator iterPriceLevelOrder;
        PriceLevel *priceLevel { nullptr };
    };

    using OrderIdMap = std::unordered_map<Order::IDType, ReferencesBlock>;
    OrderIdMap orderByIDMap;

    /** BID's (BUY Orders) PriceLevels **/
    boost::container::flat_map<Order::Price, std::unique_ptr<PriceLevel>, std::greater<>> bidPriceLevelMap;

    /** ASK's (SELL Orders) PriceLevels **/
    boost::container::flat_map<Order::Price, std::unique_ptr<PriceLevel>, std::less<>> askPriceLevelMap;

    Trades trades;

public:

    void processOrder(OrderPtr &&order);

public:

    [[maybe_unused]] [[nodiscard]]
    std::optional<Order*> getBestBuyOrder() const noexcept;

    [[maybe_unused]] [[nodiscard]]
    std::optional<Order*> getBestSellOrder() const noexcept;

    [[maybe_unused]] [[nodiscard]]
    size_t getOrdersCount() const noexcept;

    [[maybe_unused]] [[nodiscard]]
    size_t getBuyPriceLevelCount() const noexcept;

    [[maybe_unused]] [[nodiscard]]
    size_t getSellPriceLevelsCount() const noexcept;

private:

    [[nodiscard]]
    Common::Order::Quantity matchOrder(Order &order);

    template<typename OrderSideMap,
            typename Comparator = std::less<typename OrderSideMap::key_type>>
    void matchOrder(Order &order,
                    OrderSideMap &oppositeSidePriceLvlMap);

    bool matchOrderList(Order &order, PriceLevel *priceLevel);

    template<class OrderSideMap> [[nodiscard]]
    PriceLevel *getOrderPriceList(OrderSideMap &map, const Order::Price &price);

    void handleOrderNew(OrderPtr &&order);
    void handleOrderCancel(const Order &order);
    void handleOrderAmend(OrderPtr &&order);

    void cancelOrder(const OrderIdMap::iterator &orderByIDIter);

    /** For testing: **/
    friend struct Testing::OrderMatchingEngineTester;
};

#endif //FINANCETECHNOLOGYPROJECTS_MATCHINGENGINE_H
