/**============================================================================
Name        : MatchingEngine.cpp
Created on  : 03.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngine.cpp
============================================================================**/

#include "MatchingEngine.h"

using namespace Common;

void MatchingEngine::processOrder(OrderPtr&& order)
{
    // TODO: Remove branching ???
    switch (order->action)
    {
        case OrderActionType::NEW:
            return handleOrderNew(std::move(order));
        case OrderActionType::CANCEL:
            return handleOrderCancel(*order);
        case OrderActionType::AMEND:
            return handleOrderAmend(std::move(order));
        default:
            return;
    }
}

Common::Order::Quantity MatchingEngine::matchOrder(Order& order)
{
    // TODO: Remove branching ???
    if (OrderSide::SELL == order.side) {
        matchOrder<decltype(bidPriceLevelMap), std::greater_equal<>>(order,bidPriceLevelMap);
    } else {
        matchOrder<decltype(askPriceLevelMap), std::less_equal<>>(order,askPriceLevelMap);
    }

    // Return remaining quantity
    return order.quantity;
}

template<typename OrderSideMap,
        typename Comparator>
void MatchingEngine::matchOrder(Order& order,
                                OrderSideMap& oppositeSidePriceLvlMap)
{
    constexpr Comparator comparator{};
    auto bestLevelIter = oppositeSidePriceLvlMap.begin();

    while (oppositeSidePriceLvlMap.end() != bestLevelIter &&
           order.quantity > 0 &&
           comparator(bestLevelIter->first, order.price))
    {
        if (const bool levelEmpty = matchOrderList(order, bestLevelIter->second.get()); levelEmpty) {
            oppositeSidePriceLvlMap.erase(bestLevelIter);
        } else {
            ++bestLevelIter;
        }
    }
}

bool MatchingEngine::matchOrderList(Order& order, PriceLevel* priceLevel)
{
    for (auto orderIter = priceLevel->orders.begin(); priceLevel->orders.end() != orderIter;)
    {
        Order& matchedOrder = *(*orderIter);

        // FIXME : Required performance improvements: cause of ~35% CPU usage
#if 1
        Trade& trade = trades.addTrade();
        trade.setQuantity(std::min(matchedOrder.quantity,order.quantity));
        // TODO: Remove branching ???
        if (OrderSide::SELL == order.side) {
            trade.setBuyOrder(matchedOrder).setSellOrder(order);
        } else {
            trade.setBuyOrder(order).setSellOrder(matchedOrder);
        }
#endif

        if (order.quantity >= matchedOrder.quantity)
        {
            order.quantity -= matchedOrder.quantity;
            matchedOrder.quantity = 0;

            /** Deleting order **/
            orderByIDMap.erase(matchedOrder.orderId);
            priceLevel->orders.erase(orderIter++);
        } else {
            matchedOrder.quantity -= order.quantity;
            order.quantity = 0;
            ++orderIter;
            break;
        }
    }
    return priceLevel->orders.empty();
}

template<class OrderSideMap>
MatchingEngine::PriceLevel* MatchingEngine::getOrderPriceList(OrderSideMap& map, const Order::Price& price)
{
    const auto [iter, inserted] = map.emplace(price, nullptr);
    if (inserted) {
        iter->second = std::make_unique<PriceLevel>() ;
    }
    return iter->second.get();
};

void MatchingEngine::handleOrderNew(OrderPtr&& order)
{
    if (0 == matchOrder(*order)) {
        return;
    }

    const auto [iterOrderMap, inserted] = orderByIDMap.emplace(order->orderId, ReferencesBlock{});
    if (inserted)
    {
        auto& [ptrOrder, priceOrderIter, priceLevel] = iterOrderMap->second;

        ptrOrder = order.get();
        priceLevel = (OrderSide::BUY == order->side) ? getOrderPriceList(bidPriceLevelMap, order->price) :
                     getOrderPriceList(askPriceLevelMap, order->price);
        priceLevel->quantity += order->quantity;
        priceOrderIter = priceLevel->orders.insert(priceLevel->orders.end(), std::move(order));
    }
}

void MatchingEngine::cancelOrder(const OrderIdMap::iterator& orderByIDIter)
{
    auto& [ptrOrder, iterPriceLvlOrder, priceLevel] = orderByIDIter->second;
    const Order& order { *ptrOrder };

    priceLevel->orders.erase(iterPriceLvlOrder);
    priceLevel->quantity -= order.quantity;
    orderByIDMap.erase(orderByIDIter);

    // FIXME: Performance impact
    if (priceLevel->orders.empty())
    {
        if (Common::OrderSide::BUY == order.side) {
            bidPriceLevelMap.erase(order.price);
        } else {
            askPriceLevelMap.erase(order.price);
        }
    }
}

void MatchingEngine::handleOrderCancel(const Order& order)
{
    [[likely]]
    if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
            orderByIDMap.end() != orderByIDIter)
    {
        // If Order SIDE is the same as it was before
        if (order.side == orderByIDIter->second.order->side) {
            cancelOrder(orderByIDIter);
        }
    }
}

void MatchingEngine::handleOrderAmend(OrderPtr&& order)
{
    if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
            orderByIDMap.end() != orderByIDIter)
    {
        // TODO: Remove branching ???
        Order& orderOriginal = *(orderByIDIter->second.order);
        if (orderOriginal.side != order->side){
            return;
        }
        else if (orderOriginal.price != order->price)
        {
            cancelOrder(orderByIDIter);
            // TODO: Refactor 'handleOrderNew' - doing HashTable look-up once again
            handleOrderNew(std::move(order));
        }
        else if (orderOriginal.price == order->price)
        {
            // TODO: update order parameters
            PriceLevel& priceLevel = *orderByIDIter->second.priceLevel;
            priceLevel.quantity -= orderOriginal.quantity;
            priceLevel.quantity +=  order->quantity;

            orderOriginal.quantity = order->quantity;
        }
    }
}

std::optional<Order*> MatchingEngine::getBestBuyOrder() const noexcept
{
    if (bidPriceLevelMap.empty())
        return std::nullopt;
    return bidPriceLevelMap.begin()->second->orders.begin()->get();
}

std::optional<Order*> MatchingEngine::getBestSellOrder() const noexcept
{
    if (askPriceLevelMap.empty())
        return std::nullopt;
    return askPriceLevelMap.begin()->second->orders.begin()->get();
}

size_t MatchingEngine::getOrdersCount() const noexcept
{
    return orderByIDMap.size();
}

size_t MatchingEngine::getBuyPriceLevelCount() const noexcept
{
    return bidPriceLevelMap.size();
}

size_t MatchingEngine::getSellPriceLevelsCount() const noexcept
{
    return askPriceLevelMap.size();
}