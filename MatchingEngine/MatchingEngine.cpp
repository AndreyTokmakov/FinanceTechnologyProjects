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
#include <optional>
#include <list>
#include <ranges>
#include <vector>
#include <map>
#include <unordered_map>

#include <boost/container/flat_map.hpp>

#define ASSERT_TRUE(condition) \
    if (!(condition)) \
        std::cerr << "Assertation at line " << __LINE__ << " failed\n";


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
        /** BID's (BUY Orders) PriceLevels **/
        std::map<Order::Price, PriceOrderListPtr, std::greater<>> buyOrders;

        /** ASK's (SELL Orders) PriceLevels **/
        std::map<Order::Price, PriceOrderListPtr, std::less<>> sellOrders;
#else
        /** BID's (BUY Orders) PriceLevels **/
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::greater<>> bidPriceLevelMap;

        /** ASK's (SELL Orders) PriceLevels **/
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::less<>> askPriceLevelMap;
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
                matchOrder<decltype(bidPriceLevelMap), std::greater_equal<>>(order,bidPriceLevelMap);
            } else {
                matchOrder<decltype(askPriceLevelMap), std::less_equal<>>(order,askPriceLevelMap);
            }

            // Return remaining quantity
            return order.quantity;
        }

        template<typename OrderSideMap,
                 typename Comparator = std::less<typename OrderSideMap::key_type>>
        void matchOrder(Order& order,
                        OrderSideMap& oppositeSidePriceLvlMap)
        {
            constexpr Comparator comparator{};
            auto itBestLevel = oppositeSidePriceLvlMap.begin();

            while (oppositeSidePriceLvlMap.end() != itBestLevel &&
                   order.quantity > 0 &&
                   comparator(itBestLevel->first, order.price))
            {
                const bool levelEmpty = matchOrderList(order, itBestLevel->second);
                if (levelEmpty) {
                    oppositeSidePriceLvlMap.erase(itBestLevel);
                } else {
                    ++itBestLevel;
                }
            }
        }

        bool matchOrderList(Order& order,
                            PriceOrderList* priceLvlOrdersList)
        {
            for (auto orderIter = priceLvlOrdersList->begin(); priceLvlOrdersList->end() != orderIter;)
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
                    priceLvlOrdersList->erase(orderIter++);
                } else {
                    matchedOrder.quantity -= order.quantity;
                    order.quantity = 0;
                    ++orderIter;
                    break;
                }
            }
            return priceLvlOrdersList->empty();
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
                priceLevelOrderList = (OrderSide::BUY == order->side) ? getOrderPriceList(bidPriceLevelMap, order->price) :
                                      getOrderPriceList(askPriceLevelMap, order->price);
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
                *orderIter = std::move(order);
            }
        }

        void handleOrderCancel(const Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                auto& [orderIter, iterPriceLvlOrder, priceLevelOrderList] = orderByIDIter->second;

                priceLevelOrderList->erase(iterPriceLvlOrder);
                orders.erase(orderIter);
                orderByIDMap.erase(orderByIDIter);

                // FIXME: Performance impact
                // FIXME: BUG -- нужно использовать что-то другое а не PRICE входного Order-a
                if (priceLevelOrderList->empty())
                {
                    if (Common::OrderSide::BUY == order.side) {
                        bidPriceLevelMap.erase(order.price);
                    } else {
                        askPriceLevelMap.erase(order.price);
                    }
                }
            }
        }

        void handleOrderAmend(OrderPtr&& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order->orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                // TODO: Remove branching ???
                Order& orderOriginal = **(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order->side){
                    return;
                }
                else if (orderOriginal.price != order->price)
                {
                    // TODO: handleOrderCancel() --> need to create one functions to avoid second orderByIDMap.find(..)
                    //       extract common part from "handleOrderCancel(const Order& order)"
                    handleOrderCancel(*order);
                    handleOrderNew(std::move(order));
                }
                else if (orderOriginal.price == order->price)
                {
                    // TODO: update order parameters
                    orderOriginal.quantity = order->quantity;
                }
            }
        }
    };
}


namespace MatchingEngine
{
    struct OrderMatchingEngineTester : OrderMatchingEngine
    {
        void info([[maybe_unused]] bool printTrades = true)
        {
            auto printOrders = [](const auto& orderMap)
            {
                for (const auto& [price, ordersList]: orderMap) {
                    std::cout << "\tPrice Level : [" << price << "]" << std::endl;
                    for (const auto & orderIter: *ordersList) {
                        Common::printOrder(**orderIter);
                    }
                }
            };


            std::cout << "BUY:  " << std::endl; printOrders(bidPriceLevelMap);
            std::cout << "SELL: " << std::endl; printOrders(askPriceLevelMap);
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

        std::optional<Order*> getBestBuyOrder() const noexcept
        {
            if (bidPriceLevelMap.empty())
                return std::nullopt;
            return (*bidPriceLevelMap.begin()->second->begin())->get();
        }

        std::optional<Order*> getBestSellOrder() const noexcept
        {
            if (askPriceLevelMap.empty())
                return std::nullopt;
            return (*askPriceLevelMap.begin()->second->begin())->get();
        }

        size_t getOrdersCount() const noexcept
        {
            return orderByIDMap.size();
        }

        // TODO: Add MetaData [quantity, ...] to each askPriceLevelMap
        size_t getBuyOrdersCount() const noexcept
        {
            size_t count { 0 };
            for (const auto& [price, orderList]: bidPriceLevelMap)
                count += orderList->size();
            return count;
        }

        // TODO: Add MetaData [quantity, ...] to each askPriceLevelMap
        size_t getSellOrdersCount() const noexcept
        {
            size_t count { 0 };
            for (const auto& [price, orderList]: askPriceLevelMap)
                count += orderList->size();
            return count;
        }

        size_t getBuyPriceLevelCount() const noexcept
        {
            return bidPriceLevelMap.size();
        }

        size_t getSellPriceLevelsCount() const noexcept
        {
            return askPriceLevelMap.size();
        }
    };
}


namespace Testsing
{
    using namespace MatchingEngine;
    using namespace Common;
    using OrderPtr = MatchingEngine::OrderMatchingEngineTester::OrderPtr;

    Memory::ObjectPool<Order> ordersPool;

    OrderPtr createOrder(const Common::OrderSide orderSide,
                         const Common::OrderActionType action = Common::OrderActionType::NEW,
                         const Common::Order::Price price = 1,
                         unsigned long long quantity = 0,
                         const  Common::Order::OrderID orderId = getNextOrderID())
    {
        OrderPtr order { ordersPool.acquireObject() };
        order->action = action;
        order->side = orderSide;
        order->price = price;
        order->quantity = quantity;
        order->orderId = orderId;
        return order;
    }

    void PostOrders(OrderMatchingEngineTester& engine,
                    const std::vector<Order::Price>& prices,
                    const Common::OrderSide orderSide = OrderSide::BUY,
                    const Common::OrderActionType action = Common::OrderActionType::NEW,
                    const uint32_t orderPerPrice = 1,
                    const uint32_t quantity = 1)
    {
        for (uint32_t n = 0, priceIdx = 0; n < prices.size() * orderPerPrice; ++n)
        {
            OrderPtr order = createOrder(orderSide,  action, prices[priceIdx++], quantity);
            engine.processOrder(std::move(order));

            if (priceIdx >= prices.size())
                priceIdx = 0;
        }
    }
}


namespace Testsing::MatchingEngine_Utilities_Tests
{
    /// * * * * * * TEST BIDS * * * * * * *

    void Test_getBestBuyOrder()
    {
        OrderMatchingEngineTester engine;
        PostOrders(engine, { 5, 6, 7, 8, 9 }, OrderSide::BUY, OrderActionType::NEW,  5, 3);

        const std::optional<Order*> bestBuy = engine.getBestBuyOrder();
        ASSERT_TRUE(bestBuy.has_value());
        ASSERT_TRUE(bestBuy.value()->price == 9);
    }

    void Test_getBestBuyOrder_No_BUY_Orders()
    {
        OrderMatchingEngineTester engine;

        const std::optional<Order*> bestBuy = engine.getBestBuyOrder();
        ASSERT_TRUE(not bestBuy.has_value());
    }

    void Test_getBestBuyOrder_SELL_Orders_Only()
    {
        OrderMatchingEngineTester engine;
        PostOrders(engine, { 5, 6, 7, 8, 9 }, OrderSide::SELL, OrderActionType::NEW, 5, 3);

        const std::optional<Order*> bestBuy = engine.getBestBuyOrder();
        ASSERT_TRUE(not bestBuy.has_value());

        const std::optional<Order*> bestSell = engine.getBestSellOrder();
        ASSERT_TRUE(bestSell.has_value());
    }

    /// * * * * * * TEST AKS's * * * * * * *

    void Test_getBestSellOrder()
    {
        OrderMatchingEngineTester engine;
        PostOrders(engine, { 5, 6, 7, 8, 9 }, OrderSide::SELL, OrderActionType::NEW,  5, 3);

        const std::optional<Order*> bestSell = engine.getBestSellOrder();
        ASSERT_TRUE(bestSell.has_value());
        ASSERT_TRUE(bestSell.value()->price == 5);
    }

    void Test_getBestSellOrder_No_SELL_Orders()
    {
        OrderMatchingEngineTester engine;

        const std::optional<Order*> bestSell = engine.getBestSellOrder();
        ASSERT_TRUE(not bestSell.has_value());
    }

    void Test_getBestSellOrder_SELL_Orders_Only()
    {
        OrderMatchingEngineTester engine;
        PostOrders(engine, { 5, 6, 7, 8, 9 }, OrderSide::BUY, OrderActionType::NEW, 5, 3);

        const std::optional<Order*> bestSell = engine.getBestSellOrder();
        ASSERT_TRUE(not bestSell.has_value());

        const std::optional<Order*> bestBuy = engine.getBestBuyOrder();
        ASSERT_TRUE(bestSell.has_value());
    }

    void Test_getBestBuyOrder_Cancel_CheckBID_Updated ()
    {
        OrderMatchingEngineTester engine;
        const std::vector<Common::Order::Price> prices { 5, 6, 7, 8, 9 };

        std::unordered_map<Common::Order::Price, uint64_t> priceIdMap;
        for (Common::Order::Price price : prices ) {
            const auto& [iter, ok] = priceIdMap.emplace(price, getNextOrderID());
            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW,
                                            price, 3, iter->second));
        }

        /// Backward loop the 'prices' collection:
        for (unsigned long price : std::ranges::reverse_view(prices))
        {
            const uint64_t orderId = priceIdMap[price];
            const std::optional<Order*> bidOrder = engine.getBestBuyOrder();

            ASSERT_TRUE(bidOrder.has_value());
            ASSERT_TRUE(bidOrder.value()->price == price);

            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::CANCEL, price, 3, orderId));
        }

        /// No BUY orders shall be in the OrderBook at this moment
        ASSERT_TRUE(!engine.getBestBuyOrder().has_value());
    }

    void Test_getBestSellOrder_Cancel_CheckBID_Updated ()
    {
        OrderMatchingEngineTester engine;
        const std::vector<Common::Order::Price> prices { 5, 6, 7, 8, 9 };

        std::unordered_map<Common::Order::Price, uint64_t> priceIdMap;
        for (Common::Order::Price price : prices ) {
            const auto& [iter, ok] = priceIdMap.emplace(price, getNextOrderID());
            engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW,
                                            price, 3, iter->second));
        }

        /// Backward loop the 'prices' collection:
        for (Common::Order::Price price : prices )
        {
            const uint64_t orderId = priceIdMap[price];
            const std::optional<Order*> askOrder = engine.getBestSellOrder();

            ASSERT_TRUE(askOrder.has_value());
            ASSERT_TRUE(askOrder.value()->price == price);

            engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::CANCEL, price, 3, orderId));
        }

        /// No SELL orders shall be in the OrderBook at this moment
        ASSERT_TRUE(!engine.getBestSellOrder().has_value());
    }

    void Test_CANCEL_Order_PriceLevel_Deleted()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getOrdersCount() == 1);


        //OrderPtr cancelOrder = createOrder(OrderSide::BUY, OrderActionType::CANCEL, 3, 3, orderId);
        //engine.processOrder(std::move(cancelOrder));
        //engine.info();
    }

    void Test_CANCEL_Order_TotalOrderCount()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getOrdersCount() == 1);

        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::CANCEL, 3, 3, orderId));
        ASSERT_TRUE(engine.getOrdersCount() == 0);
    }

    void Test_CANCEL_Order_PriceLevels_Count_BUY()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 1);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);

        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::CANCEL, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
    }

    void Test_CANCEL_Order_PriceLevels_Count_SELL()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 1);

        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::CANCEL, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
    }

    void Test_CANCEL_Order_PriceLevels_Count_BUY_Multiple()
    {
        OrderMatchingEngineTester engine;
        constexpr size_t ordersToSend { 100 };

        std::vector<uint64_t> iDs;
        iDs.reserve(ordersToSend);

        for (size_t idx = 0; idx < ordersToSend; ++idx) {
            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW,
                                            3, 3, iDs.emplace_back(getNextOrderID())));
        }

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 1);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
        ASSERT_TRUE(engine.getBuyOrdersCount() == 100);
        ASSERT_TRUE(engine.getSellOrdersCount() == 0);

        for (uint64_t orderId: iDs) {
            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::CANCEL,3, 3, orderId));
        }

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
        ASSERT_TRUE(engine.getBuyOrdersCount() == 0);
        ASSERT_TRUE(engine.getSellOrdersCount() == 0);
    }

    void Test_CANCEL_Order_PriceLevels_Count_SELL_Multiple()
    {
        OrderMatchingEngineTester engine;
        constexpr size_t ordersToSend { 100 };

        std::vector<uint64_t> iDs;
        iDs.reserve(ordersToSend);

        for (size_t idx = 0; idx < ordersToSend; ++idx) {
            engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW,
                                            3, 3, iDs.emplace_back(getNextOrderID())));
        }

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 1);
        ASSERT_TRUE(engine.getBuyOrdersCount() == 0);
        ASSERT_TRUE(engine.getSellOrdersCount() == 100);

        for (uint64_t orderId: iDs) {
            engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::CANCEL,3, 3, orderId));
        }

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
        ASSERT_TRUE(engine.getBuyOrdersCount() == 0);
        ASSERT_TRUE(engine.getSellOrdersCount() == 0);
    }

    void Test_CANCEL_Order_PriceLevels_Count_NotMatchingActions()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 1);

        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::CANCEL, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 0);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 1);
    }

    void Test_CANCEL_Order_PriceLevels_Count_NotMatchingActions_1()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 1);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);

        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::CANCEL, 3, 3, orderId));

        ASSERT_TRUE(engine.getBuyPriceLevelCount() == 1);
        ASSERT_TRUE(engine.getSellPriceLevelsCount() == 0);
    }

    void TestAll()
    {
        Test_getBestBuyOrder();
        Test_getBestBuyOrder_No_BUY_Orders();
        Test_getBestBuyOrder_SELL_Orders_Only();
        Test_getBestBuyOrder_Cancel_CheckBID_Updated();
        Test_getBestSellOrder_Cancel_CheckBID_Updated();
        ///  - Проверка BID - после Amend
        ///  - Проверка BID - после добавления Order-ов с одной ценой
        ///  - Проверка BID - после Trade

        Test_getBestSellOrder();
        Test_getBestSellOrder_No_SELL_Orders();
        Test_getBestSellOrder_SELL_Orders_Only();


        Test_CANCEL_Order_TotalOrderCount();
        Test_CANCEL_Order_PriceLevels_Count_BUY();
        Test_CANCEL_Order_PriceLevels_Count_SELL();
        Test_CANCEL_Order_PriceLevels_Count_BUY_Multiple();
        Test_CANCEL_Order_PriceLevels_Count_SELL_Multiple();
        Test_CANCEL_Order_PriceLevels_Count_NotMatchingActions();
        Test_CANCEL_Order_PriceLevels_Count_NotMatchingActions_1();

        Test_CANCEL_Order_PriceLevel_Deleted();

        std::cout << "All tests passed\n";
    }
}

namespace Testsing::MatchingEngine_Tests
{
    void PostOrder_Single_BUY()
    {
        OrderMatchingEngineTester engine;
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 123, 3));

        engine.info();
    }

    void PostOrder_Multiple_BUY()
    {
        OrderMatchingEngineTester engine;

        const std::array<Order::Price, 5> prices { 5, 6, 7, 8, 9 };
        for (uint32_t n = 0, priceIdx = 0; n < prices.size() * 5; ++n)
        {
            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, prices[priceIdx++], 3));
            if (priceIdx >= prices.size())
                priceIdx = 0;
        }

        engine.info();
    }


    void PostOrder_Single_BUY_and_Cancel()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, 3, 3, orderId));

        OrderPtr cancelOrder = createOrder(OrderSide::BUY, OrderActionType::CANCEL, 3, 3, orderId);
        engine.processOrder(std::move(cancelOrder));
        engine.info();
    }

    void PostOrder_Single_SELL_and_Cancel()
    {
        OrderMatchingEngineTester engine;

        const uint64_t orderId = getNextOrderID();
        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW, 3, 3, orderId));

        std::cout << "ASK's Count: " << engine.getSellPriceLevelsCount()
                  << ", BID's Count: " << engine.getBuyPriceLevelCount() << std::endl;

        engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::CANCEL, 3, 3, orderId));

        // engine.info();
        std::cout << "ASK's Count: " << engine.getSellPriceLevelsCount()
                  << ", BID's Count: " << engine.getBuyPriceLevelCount() << std::endl;
    }

    void Trade_SELL()
    {
        OrderMatchingEngineTester engine;

        const std::vector<Order::Price> prices { 5, 6, 7 };
        for (uint32_t n = 0, priceIdx = 0; n < prices.size() * 1; ++n)
        {
            engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, prices[priceIdx++], 2));
            if (priceIdx >= prices.size())
                priceIdx = 0;
        }
        engine.info();

        {
            OrderPtr order = createOrder(OrderSide::SELL, OrderActionType::NEW, 7, 2);

            printOrder(*order.get());
            std::cout << std::string(160, '=') << std::endl;

            engine.processOrder(std::move(order));
        }

        std::cout << std::string(160, '=') << std::endl;
        engine.info();
    }

    void Trade_BUY()
    {
        OrderMatchingEngineTester engine;

        const std::vector<Order::Price> prices { 5, 6 };
        for (uint32_t n = 0, priceIdx = 0; n < prices.size() * 3; ++n)
        {
            engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW, prices[priceIdx++], 2));
            if (priceIdx >= prices.size())
                priceIdx = 0;
        }
        engine.info();

        {
            OrderPtr order = createOrder(OrderSide::BUY, OrderActionType::NEW, 6, 11);

            printOrder(*order.get());
            std::cout << std::string(160, '=') << std::endl;

            engine.processOrder(std::move(order));
        }

        std::cout << std::string(160, '=') << std::endl;
        engine.info();
    }

    void Trade_BUY_vs_SELL_EqualNum()
    {
        OrderMatchingEngineTester engine;

        constexpr uint32_t pricesCount { 500 }, initialPrice { 10 };
        constexpr uint32_t buyOrders { 2000 }, sellOrders { buyOrders };

        std::vector<int32_t> prices(pricesCount);
        std::iota(prices.begin(), prices.end(), initialPrice);

        for (const uint32_t price: prices) {
            for (uint32_t n = 0; n < buyOrders; ++n) {
                engine.processOrder(createOrder(OrderSide::SELL, OrderActionType::NEW, price, 2));
            }
        }

        for (const uint32_t price: prices) {
            for (uint32_t n = 0; n < sellOrders; ++n) {
                engine.processOrder(createOrder(OrderSide::BUY, OrderActionType::NEW, price, 2));
            }
        }
        engine.info();
    }

    void Load_Test()
    {
        constexpr uint32_t pricesCount { 50  }, initialPrice { 10 };
        constexpr uint32_t buyOrders { 100 }, sellOrders { 100 };

        OrderMatchingEngineTester engine;

        std::vector<int32_t> prices(pricesCount);
        std::iota(prices.begin(), prices.end(), initialPrice);

        std::vector<uint64_t> iDs;
        iDs.reserve(pricesCount * buyOrders * 10);


        PerfUtilities::ScopedTimer timer { "Tests_OrderAsPtr_Alloc"};
        uint64_t count = 0;

        for (int i = 0; i < 400; ++i)
        {
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::BUY;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::SELL;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < sellOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::SELL;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            for (uint32_t price: prices) {
                for (uint32_t n = 0; n < buyOrders; ++n) {
                    iDs.push_back(getNextOrderID());

                    OrderPtr order { ordersPool.acquireObject() };
                    order->side = OrderSide::BUY;
                    order->price = price;
                    order->quantity = 10;
                    order->orderId = iDs.back();

                    engine.processOrder(std::move(order));
                    ++count;
                }
            }
            iDs.clear();
        }
        std::cout << count << std::endl;
    }
}



// TODO:
//  - use std::source_location for UnitTests


// TODO: Unit tests && Test Methods
//  + Проверка BID
//  + Проверка BID - пустой список BID
//  + Проверка BID - пустой список BID | SELL's only
//  + Проверка BID - после Cancel
//  - Проверка BID - после Amend
//  - Проверка BID - после добавления Order-ов с одной ценой
//  - Проверка BID - после Trade
//  -
//  - Проверка ASK
//  - Проверка ASK - пустой список ASK
//  - Проверка ASK - пустой список ASK | BUY's only
//  + Проверка ASK - после Cancel
//  - Проверка ASK - после Amend
//  - Проверка ASK - после добавления Order-ов с одной ценой
//  - Проверка ASK - после Trade
//  -
//  - Проверка количества PriceLevels BID
//  - Проверка количества PriceLevels ASK
//  - Проверка количества Order-ов в PriceLevel - BID
//  - Проверка количества Order-ов в PriceLevel - ASK
//  - Проверка Qunantiry  PriceLevel - BID
//  - Проверка Qunantiry  PriceLevel - ASK
//  -
//  + Проверка Cancel: Cancel 1 BUY
//  + Проверка Cancel: Cancel 1 SELL
//  + Проверка Cancel: Cancel 100 BUY
//  + Проверка Cancel: Cancel 100 SELL
//  + Проверка Cancel: Cancel Not-existing SELL
//  + Проверка Cancel: Cancel Not-existing BUY
//  - Проверка Cancel: Check total order count
//  - Проверка Cancel: Check BID's orders count ( SELL cancel order )
//  - Проверка Cancel: Check ASK's orders count ( BUY cancel order )
//  - Проверка Cancel: Check price level deleted



// TODO: Order Matching Rules ************** CREATE PARAMETRIZED_TESTS ****************
//  - Проверка Trade: 1 BUY - 1 SELL
//  - Проверка Trade: SELL {5,2},{5,2},{5,2},{6,2},{6,2},{6,2} | BUY {6,11} ---> BUY {6,1}


void MatchingEngine::TestAll()
{
    using namespace Testsing;

    MatchingEngine_Utilities_Tests::TestAll();


    // MatchingEngine_Tests::PostOrder_Single_BUY_and_Cancel();
    // MatchingEngine_Tests::PostOrder_Single_SELL_and_Cancel();
    // MatchingEngine_Tests::PostOrder_Single_BUY();
    // MatchingEngine_Tests::PostOrder_Multiple_BUY();

    // MatchingEngine_Tests::Trade_SELL();
    // MatchingEngine_Tests::Trade_BUY();
    // MatchingEngine_Tests::Trade_BUY_vs_SELL_EqualNum();

    // MatchingEngine_Tests::Load_Test();
}


void MatchingEngine::LoadTest()
{
    using namespace Testsing;
    MatchingEngine_Tests::Load_Test();
}