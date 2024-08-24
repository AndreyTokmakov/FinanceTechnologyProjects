/**============================================================================
Name        : MatchingEngineWithAlloc.cpp
Created on  : 12.08.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MatchingEngineWithAlloc.cpp
============================================================================**/

#include "PerfUtilities.h"
#include "Order.h"

#include <iostream>
#include <memory>
#include <numeric>
#include <list>
#include <forward_list>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>

#include <boost/container/flat_map.hpp>

namespace
{
    uint64_t getNextOrderID()
    {
        static uint64_t id { 10'000 };
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

        static constexpr size_type DEFAULT_CHUNK_SIZE { 5 };
        static constexpr size_type GROWTH_STRATEGY { 2 };

        size_type _new_block_size { DEFAULT_CHUNK_SIZE };
        size_type _size { 0 };
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
                --pool->_size;
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

            // Get a free object.
            const pointer objectPtr { available.back() };

            // Initialize, i.e. construct, an instance of T in an uninitialized block of memory
            // using placement new, and perfectly forward any provided arguments to the constructor.
            pointer obj = new (objectPtr) object_type { std::forward<Args>(args)... };

            // Remove the object from the list of free objects.
            available.pop_back();
            ++_size;

            // Wrap the initialized object and return it.
            return std::unique_ptr<object_type, Deleter> { objectPtr, Deleter{this}};
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return _size;
        }

        [[nodiscard]]
        size_type capacity() const noexcept {
            return _capacity;
        }
    };
}

namespace MatchingEngine_NO_WithAllocator
{
    using namespace Common;

    struct OrderMatchingEngine
    {
        using OrderIter = typename std::list<Order>::iterator;
        using PriceOrderList = std::list<OrderIter>;
        using PriceOrderIter = typename PriceOrderList::iterator;

        struct ReferencesBlock
        {
            OrderIter orderIter;
            PriceOrderIter priceOrderIter;
            PriceOrderList* priceLevelOrderList;
        };

        Memory::ObjectPool<Order> ordersPool;
        std::list<Order> orders {};

        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        boost::container::flat_map<Order::Price, PriceOrderList, std::less<>> buyOrders;
        boost::container::flat_map<Order::Price, PriceOrderList, std::greater<>> sellOrders;

        void processOrder(Order& order)
        {
            switch (order.action)
            {
                case OrderActionType::NEW:
                    return handleOrderNew(order);
                case OrderActionType::CANCEL:
                    return handleOrderCancel(order);
                case OrderActionType::AMEND:
                    return handleOrderAmend(order);
                default:
                    return;
            }
        }

        unsigned long long matchOrder(Order& order)
        {
            // TODO:
            return 1;
        }

        template<typename OrderSideMap>
        void matchOrder(Order& order, OrderSideMap& oppositeSideOrdersPriceMap)
        {
            // TODO:
        }


        void matchOrderList(Order& order,
                            PriceOrderList& matchedOrderList)
        {
            // TODO:
        }

        void handleOrderNew(Order& order)
        {
            if (0 == matchOrder(order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(
                    order.orderId, ReferencesBlock{});
            if (inserted)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] =
                        iterOrderMap->second;
                orderIter = orders.insert(orders.end(), order);
                priceLevelOrderList = (OrderSide::BUY == order.side) ?
                                      &buyOrders[order.price] : &sellOrders[order.price];
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
            }
        }

        void handleOrderCancel(Order& order)
        {
            // TODO:
        }

        void handleOrderAmend(Order& order)
        {
            // TODO:
        }

        void info(bool printTrades = true)
        {
            // TODO:
        }
    };
}

namespace MatchingEngine_WithAllocator
{
    using namespace Common;


    struct OrderMatchingEngine
    {
        using OrderPtr = std::unique_ptr<Order, Memory::ObjectPool<Order>::Deleter>;
        using OrderPtrIter = typename std::list<OrderPtr>::iterator;
        using PriceOrderList = std::list<OrderPtrIter>;
        using PriceOrderListPtr = std::unique_ptr<PriceOrderList>;
        using PriceOrderIter = typename PriceOrderList::iterator;

        template <class K, class T, class Compare>
        using PriceLevelsMapType = boost::container::flat_map<K, T, Compare>;
        // using PriceLevelsMapType = std::map<K, T, Compare>;

        struct ReferencesBlock
        {
            OrderPtrIter orderIter;
            PriceOrderIter priceOrderIter;
            PriceOrderList* priceLevelOrderList;
        };

        Memory::ObjectPool<Order> ordersPool;
        std::list<OrderPtr> orders {};

        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        PriceLevelsMapType<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        PriceLevelsMapType<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;

        OrderPtr makeOrder() {
            return ordersPool.acquireObject();
        }

        void processOrder(OrderPtr&& order)
        {
            // TODO: Remove branching ???  - Fedor Pikus
            switch (order->action)
            {
                case OrderActionType::NEW:
                    return handleOrderNew(std::move(order));
                case OrderActionType::CANCEL:
                    return handleOrderCancel(order);
                case OrderActionType::AMEND:
                    return handleOrderAmend(std::move(order));
                default:
                    return;
            }
        }

        unsigned long long matchOrder(Order& order)
        {
            // TODO: Remove branch  - Fedor Pikus
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
                matchOrderList(order, matchedPriceLevelIter->second.get());
                ++matchedPriceLevelIter;
            }
        }

        void matchOrderList(Order& order,
                            PriceOrderList* matchedOrderList)
        {
            for (auto orderIter = matchedOrderList->begin(); matchedOrderList->end() != orderIter;)
            {
                // TODO: Check for performance: multiple dereferences
                Order& matchedOrder = **(*orderIter);

                // TODO: Remove branch - Fedor Pikus
                // Trade& trade = trades.addTrade();
                // trade.setQuantity(std::min(matchedOrder.quantity,order.quantity));
                if (OrderSide::SELL == order.side) {
                    // trade.setBuyOrder(matchedOrder).setSellOrder(order);
                } else {
                    // trade.setBuyOrder(order).setSellOrder(matchedOrder);
                }

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

        void handleOrderNew(OrderPtr&& orderIn)
        {
            Order& order = *orderIn.get();
            if (0 == matchOrder(order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(
                    order.orderId, ReferencesBlock{});
            if (inserted)
            {
                iterOrderMap->second.orderIter = orders.insert(orders.end(), std::move(orderIn));
                // TODO: Remove branch - Fedor Pikus
                PriceOrderListPtr& lvlListPtr = ((OrderSide::BUY == order.side) ? buyOrders.emplace(order.price, nullptr) :
                        sellOrders.emplace(order.price, nullptr)).first->second;
                if (!lvlListPtr) {
                    lvlListPtr = std::make_unique<PriceOrderList>();
                    iterOrderMap->second.priceLevelOrderList = lvlListPtr.get();
                }
                iterOrderMap->second.priceOrderIter = iterOrderMap->second.priceLevelOrderList->insert(
                        iterOrderMap->second.priceLevelOrderList->end(), iterOrderMap->second.orderIter);
            }
        }

        void handleOrderCancel(const OrderPtr& order)
        {
            // std::cout << "handleOrderCancel(id: " << order->orderId << ")\n";
            if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
                orderByIDMap.end() != orderByIDIter)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = orderByIDIter->second;

                std::cout << orderIter->get()->orderId << " to be CANCEL-ed\n";

                if (priceLevelOrderList) {
                    std::cout << "priceLevelOrderList size = " << priceLevelOrderList->size() << "\n";
                    //if (priceLevelOrderList->end() != orderIter{}
                }

                // priceLevelOrderList->erase(priceOrderIter);
                //orders.erase(orderIter);
                //orderByIDMap.erase(orderByIDIter);
            }
        }

        void handleOrderAmend(OrderPtr&& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                Order& orderOriginal = **(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order->side) {
                    return;
                } else if (orderOriginal.price != order->price) {
                    handleOrderCancel(order);
                    handleOrderNew(std::move(order));
                } else if (orderOriginal.price == order->price) {
                    // TODO: update order parameters
                    orderOriginal.quantity = order->quantity;
                }
            }
        }
    };


    void info(const OrderMatchingEngine& engine, bool printTrades = true)
    {
        for (const auto& [orderId, orderIter]: engine.orderByIDMap) {
            Order& orderOne = **orderIter.orderIter;
            Order& orderTwo = ***orderIter.priceOrderIter;
            if (orderId != orderOne.orderId || orderId != orderTwo.orderId) {
                std::cerr << "ERROR: ID: " << orderId << "!= " << orderOne.orderId << std::endl;
            }
        }

        auto printOrders = [](const auto& orderMap) {
            for (const auto& [price, ordersList]: orderMap) {
                std::cout << "\tPrice: [" << price << "]" << std::endl;
                for (const auto & orderIter: *ordersList) {
                    Common::printOrder(**orderIter);
                }
            }
        };

        // std::cout << engine.ordersPool.size() << std::endl;
        std::cout << "BUY:  " << std::endl; printOrders(engine.buyOrders);
        std::cout << "SELL: " << std::endl; printOrders(engine.sellOrders);

        /*
        std::cout << std::string(160, '=') << std::endl;
        if (!printTrades)
            return;
        for (const auto& trade: trades.trades)
        {
            std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                      << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                      << "quantity: " << trade.quantity << ")\n";
        }
        */
    }

    void info_validate(const OrderMatchingEngine& engine)
    {
        std::cout << std::string(160, '=') << std::endl;
        for (const auto& [orderId, orderDataIter]: engine.orderByIDMap)
        {
            auto& [orderIter, priceOrderIter, priceLevelOrderList] = orderDataIter;

            const Order& order = **orderIter;
            OrderMatchingEngine::PriceOrderList* priceLvlList = priceLevelOrderList;

            std::cout << "Order [id: " << order.orderId << ", price: " << order.price << "]\n";
            if (priceLvlList) {
                std::cout << "\tPrice lvl : [ptr: " << priceLvlList << ", size: " << priceLvlList->size() << "]\n";
            }
        }
    }
}


namespace MatchingEngine_NO_WithAllocator::Tests
{

    void Trade_SELL()
    {
        OrderMatchingEngine engine;

        PerfUtilities::ScopedTimer timer { "TEST"};
        for (int i = 0, price = 10; i < 8'000'000; ++i)
        {
            if (price > 16)
                price = 10;

            Order order;
            order.side = OrderSide::BUY;
            order.price = price+=2;
            order.quantity = 3;
            order.orderId = getNextOrderID();

            engine.processOrder(order);
        }

    }
}

namespace MatchingEngine_WithAllocator::Tests
{
    void ProcessOrders()
    {
        OrderMatchingEngine engine;

        for (int i = 0, price = 10; i < 10; ++i)
        {
            if (price > 16) price = 10;
            OrderMatchingEngine::OrderPtr order = engine.makeOrder();
            order->side = OrderSide::BUY;
            order->price = price+=2;
            order->quantity = 3;
            order->orderId = getNextOrderID();

            engine.processOrder(std::move(order));
        }

        info_validate(engine);
    }

    void ProcessOrders_SamePrice()
    {
        OrderMatchingEngine engine;

        for (int i = 0; i < 1; ++i)
        {
            OrderMatchingEngine::OrderPtr order = engine.makeOrder();
            order->side = OrderSide::BUY;
            order->price = 10;
            order->quantity = 3;
            order->orderId = getNextOrderID();

            engine.processOrder(std::move(order));
        }

        info_validate(engine);
    }

    void Trade_Cancel_Order()
    {
        OrderMatchingEngine engine;

        // Utilities::ScopedTimer timer { "TEST"};
        for (int i = 0, price = 10; i < 10; ++i)
        {
            if (price > 16)
                price = 10;

            OrderMatchingEngine::OrderPtr order = engine.makeOrder();
            order->side = OrderSide::BUY;
            order->price = price+=2;
            order->quantity = 3;
            order->orderId = getNextOrderID();

            engine.processOrder(std::move(order));
        }

        info(engine);

        for (Order::OrderID orderID: {1000})
        {
            OrderMatchingEngine::OrderPtr order = engine.makeOrder();
            order->action = OrderActionType::CANCEL;
            order->orderId = orderID;
            engine.processOrder(std::move(order));
        }
        info(engine);
    }
}


void MatchingEngine_WithAllocator_Tests()
{
    // using namespace MatchingEngine_NO_WithAllocator::Tests;
    using namespace MatchingEngine_WithAllocator::Tests;

    ProcessOrders();
    // ProcessOrders_SamePrice();

    // Trade_Cancel_Order();
}