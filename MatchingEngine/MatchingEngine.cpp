/**============================================================================
Name        : MatchingEngine.cpp
Created on  : 10.08.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngine.cpp
============================================================================**/

#include "Includes.h"
#include "PerfUtilities.h"
#include "Order.h"

#include <iostream>
#include <numeric>
#include <list>
#include <vector>
#include <unordered_map>

#include <boost/container/flat_map.hpp>

namespace
{
    uint64_t getNextOrderID()
    {
        static uint64_t id { 1'000 };
        return id++;
    }
}

namespace Memory
{
    template <typename Ty, typename Allocator = std::allocator<Ty>>
    class ObjectPool final
    {
        using object_type = Ty;
        using pointer = object_type*;
        using size_type = typename std::vector<pointer>::size_type;

        static_assert(!std::is_same_v<object_type, void>,
                      "Type of the Objects in the pool can not be void");

    private:
        std::vector<pointer> pool;
        std::vector<pointer> available;

        static constexpr size_type DEFAULT_CHUNK_SIZE { 10 };
        static constexpr size_type GROWTH_STRATEGY { 4 };

        size_type _new_block_size { DEFAULT_CHUNK_SIZE };
        size_type _capacity { 0 };

        void addChunk()
        {
            // Allocate a new chunk of uninitialized memory
            pointer newBlock { m_allocator.allocate(_new_block_size) };

            // Keep all allocated blocks in 'pool' to delete them later:
            pool.push_back(newBlock);

            available.resize(_new_block_size);
            std::iota(std::begin(available), std::end(available), newBlock);

            _capacity += _new_block_size;
            _new_block_size *= GROWTH_STRATEGY;
        }

        // The allocator to use for allocating and deallocating chunks.
        Allocator m_allocator;

    public:

        struct Deleter final
        {
            ObjectPool* pool {nullptr};

            void operator()(pointer object) const noexcept
            {
                std::destroy_at(object);

                // Return object mem pointer back to pool
                pool->available.push_back(object);
            }
        };

    public:
        using ObjectPtr = std::unique_ptr<object_type, Deleter>;

    public:
        ObjectPool() = default;

        explicit ObjectPool(const Allocator& allocator) : m_allocator{ allocator } {
            // Trivial
        }

        virtual ~ObjectPool()
        {   // Note: this implementation assumes that all objects handed out by this
            // pool have been returned to the pool before the pool is destroyed.
            // The following statement asserts if that is not the case.
            assert(available.size() == DEFAULT_CHUNK_SIZE * (std::pow(2, pool.size()) - 1));

            // Deallocate all allocated memory.
            size_t chunkSize{ DEFAULT_CHUNK_SIZE };
            for (auto* chunk : pool) {
                m_allocator.deallocate(chunk, chunkSize);
                chunkSize *= GROWTH_STRATEGY;
            }
        }

        // Allow move construction and move assignment.
        ObjectPool(ObjectPool&& src) noexcept = default;
        ObjectPool& operator=(ObjectPool&& rhs) noexcept = default;

        // Prevent copy construction and copy assignment.
        ObjectPool(const ObjectPool& src) = delete;
        ObjectPool& operator=(const ObjectPool& rhs) = delete;

        // Reserves and returns an object from the pool. Arguments can be
        // provided which are perfectly forwarded to a constructor of T.
        template<typename... Args>
        std::unique_ptr<object_type, Deleter> acquireObject(Args... args)
        {
            // If there are no free objects, allocate a new chunk.
            if (available.empty()) {
                addChunk();
            }

            // Initialize, i.e. construct, an instance of T in an uninitialized block of memory
            // using placement new, and perfectly forward any provided arguments to the constructor.
            pointer objectPtr = new (available.back()) object_type { std::forward<Args>(args)... };

            // Remove the object from the list of free objects.
            available.pop_back();

            // Wrap the initialized object and return it.
            return std::unique_ptr<object_type, Deleter> { objectPtr, Deleter{this}};
        }


        [[nodiscard]]
        size_type capacity() const noexcept {
            return _capacity;
        }
    };
}

namespace MatchingEngine
{
    using namespace Common;

    struct Trade
    {
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


    /*
    struct Trades
    {
        // std::vector<Trade> trades {};
        Trade dummyTread;

        Trade& addTrade() {
            // return trades.emplace_back();
            return dummyTread;
        }
    };
     */

    struct OrderMatchingEngine
    {
        // using OrderPtr = std::unique_ptr<Order>;
        using OrderPtr = std::unique_ptr<Order, Memory::ObjectPool<Order>::Deleter>;

        using OrderIter = typename std::list<OrderPtr>::iterator;
        using PriceOrderList = std::list<OrderIter>;
        using PriceOrderListPtr = PriceOrderList*;
        using PriceOrderListIter = typename PriceOrderList::iterator;

        struct ReferencesBlock
        {
            OrderIter orderIter;
            PriceOrderListIter priceOrderIter;
            PriceOrderListPtr priceLevelOrderList;
        };

        std::list<OrderPtr> orders {};
        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        // TODO: Test replace std::map --> boost::flat_map [std::list --> shall be pointer?]
        //       Since look performance of this lookup is more critical one

#if 0
        std::map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        std::map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
#else
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
#endif

        Trades trades;

        void processOrder(OrderPtr&& order)
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

        unsigned long long matchOrder(Order& order)
        {
            // TODO: Remove branching ???
            if (OrderSide::SELL == order.side) {
                matchOrder(order, buyOrders);
            } else {
                matchOrder(order, sellOrders);
            }

            // Return remaining quantity
            return order.quantity;
        }

        template<typename OrderSideMap>
        void matchOrder(Order& order, OrderSideMap& oppositeSideOrdersPriceMap)
        {
            auto matchedPriceLevelIter = oppositeSideOrdersPriceMap.lower_bound(order.price);
            while (oppositeSideOrdersPriceMap.end() != matchedPriceLevelIter && order.quantity > 0)
            {
                matchOrderList(order, matchedPriceLevelIter->second);
                ++matchedPriceLevelIter;
            }
        }

        void matchOrderList(Order& order,
                            PriceOrderList* matchedOrderList)
        {
            for (auto orderIter = matchedOrderList->begin(); matchedOrderList->end() != orderIter;)
            {
                Order& matchedOrder = **(*orderIter);

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
                    matchedOrderList->erase(orderIter++);
                } else {
                    matchedOrder.quantity -= order.quantity;
                    order.quantity = 0;
                    ++orderIter;
                    return;;
                }
            }
        }

        template<class Map>
        PriceOrderListPtr getOrderPriceList(Map& map, const Order::Price& price)
        {
            const auto [iter, inserted] = map.emplace(price, nullptr);
            if (inserted) {
                iter->second = new PriceOrderList() ;
            }
            return iter->second;
        };

        void handleOrderNew(OrderPtr&& order)
        {
            if (0 == matchOrder(*order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(order->orderId, ReferencesBlock{});
            if (inserted)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = iterOrderMap->second;
                orderIter = orders.insert(orders.end(), nullptr);
                priceLevelOrderList = (OrderSide::BUY == order->side) ? getOrderPriceList(buyOrders, order->price) :
                                      getOrderPriceList(sellOrders, order->price);
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
                *orderIter = std::move(order);
            }
        }

        void handleOrderCancel(const Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = orderByIDIter->second;
                priceLevelOrderList->erase(priceOrderIter);
                orders.erase(orderIter);
                orderByIDMap.erase(orderByIDIter);
            }
        }

        void handleOrderAmend(OrderPtr&& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                // TODO: Remove branching ???
                Order& orderOriginal = **(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order->side) {
                    return;
                } else if (orderOriginal.price != order->price) {
                    handleOrderCancel(*order);
                    handleOrderNew(std::move(order));
                } else if (orderOriginal.price == order->price) {
                    // TODO: update order parameters
                    orderOriginal.quantity = order->quantity;
                }
            }
        }

        void info([[maybe_unused]] bool printTrades = true)
        {
            auto printOrders = [](const auto& orderMap) {
                for (const auto& [price, ordersList]: orderMap) {
                    std::cout << "\tPrice: [" << price << "]" << std::endl;
                    for (const auto & orderIter: *ordersList) {
                        Common::printOrder(**orderIter);
                    }
                }
            };


            std::cout << "BUY:  " << std::endl; printOrders(buyOrders);
            std::cout << "SELL: " << std::endl; printOrders(sellOrders);
            std::cout << std::string(160, '=') << std::endl;

            /*
            if (!printTrades)
                return;
            for (const auto& trade: trades.trades)
            {
                std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                          << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                          << "quantity: " << trade.quantity << ")\n";
            }*/
        }
    };
}

namespace MatchingEngine_Tests
{
    using namespace MatchingEngine;
    using OrderPtr = MatchingEngine::OrderMatchingEngine::OrderPtr;

    Memory::ObjectPool<Order> ordersPool;

    void PostOrder_BUY()
    {
        OrderMatchingEngine engine;

        OrderPtr order { ordersPool.acquireObject() };
        order->side = OrderSide::BUY;
        order->price = 123;
        order->quantity = 3;
        order->orderId = getNextOrderID();

        engine.processOrder(std::move(order));

        engine.info();
    }
}


void MatchingEngine::TestAll()
{
    MatchingEngine_Tests::PostOrder_BUY();
}