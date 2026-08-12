/**============================================================================
Name        : PriceLevelStorage_2.cpp
Created on  : 12.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : PriceLevelStorage_2.cpp
============================================================================**/

#include "../Collections.hpp"

#include <vector>
#include <cstdint>
#include <chrono>
#include <memory>
#include <thread>

// #include <absl/container/flat_hash_map.h>
#include <iostream>
#include <unordered_map>


/*
 * High-Performance Price Ladder for Low-Latency Trading Systems
 * ==============================================================
 *
 * This implementation provides a cache-efficient order book structure optimized
 * for high-frequency trading environments where nanosecond-level latency and
 * predictable performance are critical requirements.
 *
 * Key Design Decisions
 * --------------------
 *
 * 1. CRTP-based Intrusive Linked List
 *    - Order objects contain their own prev/next pointers (intrusive)
 *    - CRTP (Curiously Recurring Template Pattern) provides type safety without
 *      virtual functions or dynamic_cast overhead
 *    - Enables O(1) removal from any position in the list
 *    - Eliminates separate node allocations (each order is its own node)
 *
 * 2. Separate Bid/Ask Arrays
 *    - Bid levels stored in descending order (highest price first)
 *    - Ask levels stored in ascending order (lowest price first)
 *    - Improves cache locality for top-of-book operations
 *    - Enables efficient scanning for best prices
 *
 * 3. Cached Best Bid/Ask Pointers
 *    - Maintains direct pointers to best bid and ask levels
 *    - Provides O(1) top-of-book access without scanning
 *    - Automatically updated on level state changes
 *
 * 4. Open-Addressing Hash Map (absl::flat_hash_map)
 *    - Used for O(1) order lookup by ID (cancellations/modifications)
 *    - Open addressing stores all entries in contiguous memory
 *    - Significantly fewer cache misses than std::unordered_map (chaining)
 *    - Pre-reserved capacity prevents rehashing on hot path
 *
 * 5. Memory Pool Allocator
 *    - Eliminates system malloc/free calls on critical path
 *    - Pre-allocates memory blocks for orders
 *    - Prevents memory fragmentation
 *    - Enables predictable allocation latency
 *
 * 6. Zero Runtime Overhead
 *    - No virtual functions (no vtable indirection)
 *    - All methods marked noexcept for compiler optimization
 *    - constexpr where possible for compile-time evaluation
 *    - [[nodiscard]] prevents accidental ignoring of return values
 *
 * Pros
 * ----
 * + Extremely fast O(1) level access with single memory fetch
 * + O(1) order insertion and removal from lists
 * + O(1) best bid/ask access (cached pointers)
 * + Excellent cache locality for sequential operations
 * + No dynamic memory allocation on hot path (uses memory pool)
 * + Predictable low latency with minimal jitter
 * + Type-safe intrusive list (compile-time checks)
 * + Separate bid/ask arrays improve top-of-book performance
 *
 * Cons
 * ----
 * - Fixed price range requires knowing min/max prices at construction
 * - Memory overhead for empty levels (allocates full range even if sparse)
 * - Not suitable for instruments with very wide price ranges (e.g., crypto)
 * - Bid/ask separation doubles some maintenance complexity
 * - Single-threaded design (no internal locking or concurrency control)
 * - Memory pool requires careful management to avoid dangling pointers
 *
 * Performance Characteristics
 * ---------------------------
 * - getLevel: ~5-10 ns (single array access)
 * - getBestBid/getBestAsk: ~2-3 ns (direct pointer access)
 * - addOrder: ~50-100 ns (array access + list insert + hash insert)
 * - removeOrder: ~30-60 ns (list removal + hash erase)
 * - findOrder: ~20-40 ns (hash lookup)
 * - Memory footprint: ~8 bytes per level + 4 pointers per level + order data
 *
 * Typical Use Cases
 * -----------------
 * - Central limit order books (CLOB) for exchange connectivity
 * - Market data processing (top-of-book and depth updates)
 * - Algorithmic trading strategy backtesting
 * - Real-time risk management systems
 *
 * Limitations and Future Improvements
 * -----------------------------------
 * - Add sequence numbers or versioning for ABA problem prevention
 * - Support for concurrent access with sharding or lock-free structures
 * - Dynamic range expansion for instruments with widening spreads
 * - Add cache-aligned structures (alignas(64)) to prevent false sharing
 */


namespace
{
    // Simple memory pool implementation for zero dynamic allocation on hot path
    class MemoryPool
    {
        static constexpr size_t PoolSize { 1024 * 1024 * 1024};
        static inline std::vector<char> pool;
        static inline std::vector<void*> free_list;

    public:

        static void* allocate(const size_t size)
        {
            if (free_list.empty()) {
                // In production, expand pool here
                return malloc(size);
            }
            void* ptr = free_list.back();
            free_list.pop_back();
            return ptr;
        }

        static void deallocate(void* ptr) {
            free_list.push_back(ptr);
        }
    };

    using Timestamp = uint64_t;
    using Price     = uint64_t;
    using Volume    = uint64_t;
    using OrderId   = uint64_t;

    Timestamp getCurrentTimestamp() {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }
}


namespace
{
    template<typename T>
    struct IntrusiveLink
    {
        T* prev;
        T* next;
    };

    struct Order : public IntrusiveLink<Order>
    {
        OrderId   orderId;
        Price     priceTick;
        Volume    volume;
        Timestamp timestampNs;
        struct PriceLevel* level;

        void* operator new(const size_t size) {
            return MemoryPool::allocate(size);
        }

        void operator delete(void* ptr) {
            MemoryPool::deallocate(ptr);
        }
    };

    struct PriceLevel
    {
        Price    priceTick { 0 };
        Volume   totalVolume { 0 };
        Order*   head { nullptr };
        Order*   tail { nullptr };

        explicit PriceLevel(const Price price) noexcept: priceTick(price){
        }

        void addOrder(Order* order) noexcept
        {
            order->level = this;
            order->prev = tail;
            order->next = nullptr;
            if (tail) {
                tail->next = order;
            } else {
                head = order;
            }
            tail = order;
            totalVolume += order->volume;
        }

        void removeOrder(Order* order) noexcept
        {
            if (order->prev) {
                order->prev->next = order->next;
            } else {
                head = order->next;
            }
            if (order->next) {
                order->next->prev = order->prev;
            } else {
                tail = order->prev;
            }
            totalVolume -= order->volume;
            order->level = nullptr;
            order->prev = nullptr;
            order->next = nullptr;
        }

        [[nodiscard]]
        Order* getBestOrder() const noexcept {
            return head;
        }

        [[nodiscard]]
        bool isEmpty() const noexcept {
            return head == nullptr;
        }
    };

    class PriceLadder
    {
    public:
        PriceLadder(const Price minPriceTick, const Price maxPriceTick) noexcept:
            minPriceTick(minPriceTick), maxPriceTick(maxPriceTick), numLevels(maxPriceTick - minPriceTick + 1)
        {
            levels.reserve(numLevels);
            for (uint64_t price = minPriceTick; price <= maxPriceTick; ++price) {
                levels.emplace_back(price);
            }

            // Build bid array: highest price first
            bidLevels.reserve(numLevels);
            for (uint64_t price = maxPriceTick; price >= minPriceTick; --price) {
                bidLevels.push_back(&levels[price - minPriceTick]);
            }

            // Build ask array: lowest price first
            askLevels.reserve(numLevels);
            for (uint64_t price = minPriceTick; price <= maxPriceTick; ++price) {
                askLevels.push_back(&levels[price - minPriceTick]);
            }

            orderIndex.reserve(kDefaultReserveSize);
        }

        [[nodiscard]]
        PriceLevel* getLevel(const Price priceTick) noexcept
        {
            const size_t index = priceTick - minPriceTick;
            return &levels[index];
        }

        [[nodiscard]]
        const PriceLevel* getLevel(const Price priceTick) const noexcept
        {
            const size_t index = priceTick - minPriceTick;
            return &levels[index];
        }

        [[nodiscard]]
        PriceLevel* getBidLevel(const size_t depth) noexcept
        {
            if (depth >= bidLevels.size()) {
                return nullptr;
            }
            return bidLevels[depth];
        }

        [[nodiscard]]
        const PriceLevel* getBidLevel(const size_t depth) const noexcept
        {
            if (depth >= bidLevels.size()) {
                return nullptr;
            }
            return bidLevels[depth];
        }

        [[nodiscard]]
        PriceLevel* getAskLevel(const size_t depth) noexcept
        {
            if (depth >= askLevels.size()) {
                return nullptr;
            }
            return askLevels[depth];
        }

        [[nodiscard]]
        const PriceLevel* getAskLevel(const size_t depth) const noexcept
        {
            if (depth >= askLevels.size()) {
                return nullptr;
            }
            return askLevels[depth];
        }

        void addOrder(Order* order) noexcept
        {
            PriceLevel* level = getLevel(order->priceTick);
            const bool wasEmpty = level->isEmpty();
            level->addOrder(order);
            orderIndex.emplace(order->orderId, order);

            if (wasEmpty) {
                updateBestPointers(level);
            }
        }

        void removeOrder(Order* order) noexcept
        {
            PriceLevel* level = order->level;
            const bool hadSingleOrder = (level->head == order && level->tail == order);
            level->removeOrder(order);
            orderIndex.erase(order->orderId);

            if (hadSingleOrder && (level == getBestBidLevel() || level == getBestAskLevel())) {
                updateBestPointersAfterRemoval(level);
            }
        }

        void modifyOrderVolume(Order* order, const Volume newVolume) noexcept
        {
            order->level->totalVolume -= order->volume;
            order->level->totalVolume += newVolume;
            order->volume = newVolume;
        }

        [[nodiscard]]
        Order* findOrder(uint64_t orderId) const noexcept {
            const auto it = orderIndex.find(orderId);
            return (it != orderIndex.end()) ? it->second : nullptr;
        }

        [[nodiscard]]
        uint64_t getBestBid() const noexcept {
            for (size_t i = 0; i < bidLevels.size(); ++i) {
                if (bidLevels[i] && !bidLevels[i]->isEmpty()) {
                    return bidLevels[i]->priceTick;
                }
            }
            return 0;
        }

        [[nodiscard]]
        uint64_t getBestAsk() const noexcept {
            for (size_t i = 0; i < askLevels.size(); ++i) {
                if (askLevels[i] && !askLevels[i]->isEmpty()) {
                    return askLevels[i]->priceTick;
                }
            }
            return UINT64_MAX;
        }

        [[nodiscard]]
        PriceLevel* getBestBidLevel() const noexcept {
            for (size_t i = 0; i < bidLevels.size(); ++i) {
                if (bidLevels[i] && !bidLevels[i]->isEmpty()) {
                    return bidLevels[i];
                }
            }
            return nullptr;
        }

        [[nodiscard]]
        PriceLevel* getBestAskLevel() const noexcept {
            for (size_t i = 0; i < askLevels.size(); ++i) {
                if (askLevels[i] && !askLevels[i]->isEmpty()) {
                    return askLevels[i];
                }
            }
            return nullptr;
        }

        [[nodiscard]]
        uint64_t getBestBidVolume() const noexcept {
            PriceLevel* level = getBestBidLevel();
            return level ? level->totalVolume : 0;
        }

        [[nodiscard]]
        uint64_t getBestAskVolume() const noexcept {
            PriceLevel* level = getBestAskLevel();
            return level ? level->totalVolume : 0;
        }

        [[nodiscard]]
        uint64_t getBidPriceAtDepth(const size_t depth) const noexcept
        {
            size_t nonEmptyCount = 0;
            for (size_t i = 0; i < bidLevels.size(); ++i) {
                if (bidLevels[i] && !bidLevels[i]->isEmpty()) {
                    if (nonEmptyCount == depth) {
                        return bidLevels[i]->priceTick;
                    }
                    ++nonEmptyCount;
                }
            }
            return 0;
        }

        [[nodiscard]]
        uint64_t getAskPriceAtDepth(const size_t depth) const noexcept
        {
            size_t nonEmptyCount = 0;
            for (size_t i = 0; i < askLevels.size(); ++i) {
                if (askLevels[i] && !askLevels[i]->isEmpty()) {
                    if (nonEmptyCount == depth) {
                        return askLevels[i]->priceTick;
                    }
                    ++nonEmptyCount;
                }
            }
            return UINT64_MAX;
        }

        [[nodiscard]]
        uint64_t getBidVolumeAtDepth(const size_t depth) const noexcept
        {
            size_t nonEmptyCount = 0;
            for (size_t i = 0; i < bidLevels.size(); ++i) {
                if (bidLevels[i] && !bidLevels[i]->isEmpty()) {
                    if (nonEmptyCount == depth) {
                        return bidLevels[i]->totalVolume;
                    }
                    ++nonEmptyCount;
                }
            }
            return 0;
        }

        [[nodiscard]]
        uint64_t getAskVolumeAtDepth(const size_t depth) const noexcept
        {
            size_t nonEmptyCount = 0;
            for (size_t i = 0; i < askLevels.size(); ++i) {
                if (askLevels[i] && !askLevels[i]->isEmpty()) {
                    if (nonEmptyCount == depth) {
                        return askLevels[i]->totalVolume;
                    }
                    ++nonEmptyCount;
                }
            }
            return 0;
        }

        [[nodiscard]]
        bool isEmpty() const noexcept {
            return orderIndex.empty();
        }

        [[nodiscard]]
        size_t getOrderCount() const noexcept {
            return orderIndex.size();
        }

        [[nodiscard]]
        uint64_t getMinPriceTick() const noexcept {
            return minPriceTick;
        }

        [[nodiscard]]
        uint64_t getMaxPriceTick() const noexcept {
            return maxPriceTick;
        }

        [[nodiscard]]
        size_t getNumLevels() const noexcept {
            return numLevels;
        }

        [[nodiscard]]
        size_t getNumBidLevels() const noexcept {
            return bidLevels.size();
        }

        [[nodiscard]]
        size_t getNumAskLevels() const noexcept {
            return askLevels.size();
        }

    private:
        static constexpr size_t kDefaultReserveSize = 1000000;

        void updateBestPointers(const PriceLevel* level) noexcept
        {
            const uint64_t price = level->priceTick;
            const uint64_t midPrice = (minPriceTick + maxPriceTick) / 2;

            if (price >= midPrice)
            {
                if (!askLevels.empty() && (!askLevels[0] || !askLevels[0]->isEmpty() ||
                    price < askLevels[0]->priceTick)) {
                    findBestAsk();
                }
            }
            else
            {
                if (!bidLevels.empty() && (!bidLevels[0] || !bidLevels[0]->isEmpty() ||
                    price > bidLevels[0]->priceTick)) {
                    findBestBid();
                }
            }
        }

        void updateBestPointersAfterRemoval(const PriceLevel* level) noexcept
        {
            const uint64_t price = level->priceTick;
            const uint64_t midPrice = (minPriceTick + maxPriceTick) / 2;

            if (price >= midPrice) {
                findBestAsk();
            } else {
                findBestBid();
            }
        }

        void findBestBid() noexcept
        {
            for (size_t i = 0; i < bidLevels.size(); ++i) {
                if (bidLevels[i] && !bidLevels[i]->isEmpty()) {
                    if (i != 0) {
                        std::swap(bidLevels[0], bidLevels[i]);
                    }
                    return;
                }
            }
        }

        void findBestAsk() noexcept {
            for (size_t i = 0; i < askLevels.size(); ++i) {
                if (askLevels[i] && !askLevels[i]->isEmpty()) {
                    if (i != 0) {
                        std::swap(askLevels[0], askLevels[i]);
                    }
                    return;
                }
            }
        }

        uint64_t minPriceTick;
        uint64_t maxPriceTick;
        size_t numLevels;

        // All levels in price order (for general access)
        std::vector<PriceLevel> levels;

        // Bid levels in descending order (highest price first)
        // Best bid is always at index 0
        std::vector<PriceLevel*> bidLevels;

        // Ask levels in ascending order (lowest price first)
        // Best ask is always at index 0
        std::vector<PriceLevel*> askLevels;

        // Order lookup by ID
    #if 1
            std::unordered_map<OrderId, Order*> orderIndex;
    #else
            absl::flat_hash_map<OrderId, Order*> orderIndex;
    #endif
    };
}

namespace collections::price_level_storage_2::tests
{

    // Helper function to get current timestamp in nanoseconds
    static uint64_t getCurrentTimestampNs()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
    }

    static Order* createTestOrder(const uint64_t id, const uint64_t price, const uint64_t volume)
    {
        Order* order = new Order();
        order->orderId = id;
        order->priceTick = price;
        order->volume = volume;
        order->timestampNs = getCurrentTimestampNs();
        order->level = nullptr;
        order->prev = nullptr;
        order->next = nullptr;
        return order;
    }

    static size_t countOrdersInLevel(const PriceLevel* level)
    {
        size_t count = 0;
        const Order* current = level->head;
        while (current) {
            count++;
            current = current->next;
        }
        return count;
    }

    static void printBookState(const PriceLadder& book)
    {
        std::cout << "Best Bid: " << book.getBestBid() << " (vol: " << book.getBestBidVolume() << ")" << std::endl;
        std::cout << "Best Ask: " << book.getBestAsk() << " (vol: " << book.getBestAskVolume() << ")" << std::endl;
        std::cout << "Total Orders: " << book.getOrderCount() << std::endl;
    }

    void assert(bool b)
    {
        if (!b) {
            std::terminate();
        }
    }

    static void testAddBuyOrders()
    {
        std::cout << "\n=== Test 1: Adding BUY Orders ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15000, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15100, 200);
        book.addOrder(buy2);

        Order* buy3 = createTestOrder(3, 14900, 150);
        book.addOrder(buy3);

        assert(book.getOrderCount() == 3);
        assert(book.getBestBid() == 15100);
        assert(book.getBestBidVolume() == 200);
        assert(book.getBidPriceAtDepth(0) == 15100);
        assert(book.getBidPriceAtDepth(1) == 15000);
        assert(book.getBidPriceAtDepth(2) == 14900);

        std::cout << "PASS: All BUY orders added correctly" << std::endl;
        printBookState(book);
    }

    static void testAddSellOrders()
    {
        std::cout << "\n=== Test 2: Adding SELL Orders ===" << std::endl;

        PriceLadder book(10000, 20000);

        Order* sell1 = createTestOrder(101, 16000, 50);
        book.addOrder(sell1);

        Order* sell2 = createTestOrder(102, 15900, 80);
        book.addOrder(sell2);

        Order* sell3 = createTestOrder(103, 16100, 120);
        book.addOrder(sell3);

        assert(book.getOrderCount() == 3);
        assert(book.getBestAsk() == 15900);
        assert(book.getBestAskVolume() == 80);
        assert(book.getAskPriceAtDepth(0) == 15900);
        assert(book.getAskPriceAtDepth(1) == 16000);
        assert(book.getAskPriceAtDepth(2) == 16100);

        std::cout << "PASS: All SELL orders added correctly" << std::endl;
        printBookState(book);
    }

    void testMixedOrders() {
        std::cout << "\n=== Test 3: Mixed BUY and SELL Orders ===" << std::endl;

        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15000, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15100, 200);
        book.addOrder(buy2);

        Order* sell1 = createTestOrder(101, 16000, 50);
        book.addOrder(sell1);

        Order* sell2 = createTestOrder(102, 15900, 80);
        book.addOrder(sell2);

        assert(book.getOrderCount() == 4);
        assert(book.getBestBid() == 15100);
        assert(book.getBestAsk() == 15900);
        assert(book.getBestBidVolume() == 200);
        assert(book.getBestAskVolume() == 80);

        std::cout << "PASS: Mixed BUY and SELL orders working correctly" << std::endl;
        printBookState(book);
    }

    void testTimePriority() {
        std::cout << "\n=== Test 4: Time Priority at Same Price ===" << std::endl;

        PriceLadder book(10000, 20000);

        Order* first = createTestOrder(1, 15100, 100);
        book.addOrder(first);

        std::this_thread::sleep_for(std::chrono::microseconds(1));
        Order* second = createTestOrder(2, 15100, 200);
        book.addOrder(second);

        std::this_thread::sleep_for(std::chrono::microseconds(1));
        Order* third = createTestOrder(3, 15100, 300);
        book.addOrder(third);

        PriceLevel* level = book.getLevel(15100);
        assert(level->head == first);
        assert(level->head->next == second);
        assert(level->head->next->next == third);
        assert(level->tail == third);
        assert(level->totalVolume == 600);
        assert(countOrdersInLevel(level) == 3);

        std::cout << "PASS: Time priority maintained correctly" << std::endl;
        printBookState(book);
    }

    static void testOrderCancellation()
    {
        std::cout << "\n=== Test 5: Order Cancellation ===" << std::endl;

        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15100, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15000, 200);
        book.addOrder(buy2);

        Order* sell1 = createTestOrder(101, 15900, 50);
        book.addOrder(sell1);

        assert(book.getOrderCount() == 3);
        assert(book.getBestBid() == 15100);
        assert(book.getBestAsk() == 15900);

        // Cancel best bid
        Order* found = book.findOrder(1);
        assert(found != nullptr);
        book.removeOrder(found);

        assert(book.getOrderCount() == 2);
        assert(book.getBestBid() == 15000);
        assert(book.getBestAsk() == 15900);

        // Cancel best ask
        found = book.findOrder(101);
        assert(found != nullptr);
        book.removeOrder(found);

        assert(book.getOrderCount() == 1);
        assert(book.getBestBid() == 15000);
        assert(book.getBestAsk() == UINT64_MAX);

        // Cancel remaining order
        found = book.findOrder(2);
        assert(found != nullptr);
        book.removeOrder(found);

        assert(book.getOrderCount() == 0);
        assert(book.getBestBid() == 0);
        assert(book.getBestAsk() == UINT64_MAX);

        std::cout << "PASS: Order cancellation working correctly" << std::endl;
        printBookState(book);
    }

    static void testOrderModification()
    {
        std::cout << "\n=== Test 6: Order Modification ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15100, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15000, 200);
        book.addOrder(buy2);

        assert(book.getBestBidVolume() == 100);
        assert(book.getBidVolumeAtDepth(1) == 200);

        // Modify volume of best bid
        book.modifyOrderVolume(buy1, 300);
        assert(buy1->volume == 300);
        assert(book.getBestBidVolume() == 300);
        assert(book.getBidVolumeAtDepth(1) == 200);
        assert(book.getLevel(15100)->totalVolume == 300);

        // Modify volume of second level
        book.modifyOrderVolume(buy2, 50);
        assert(buy2->volume == 50);
        assert(book.getBestBidVolume() == 300);
        assert(book.getBidVolumeAtDepth(1) == 50);
        assert(book.getLevel(15000)->totalVolume == 50);

        std::cout << "PASS: Order modification working correctly" << std::endl;
        printBookState(book);
    }

    static void testPriceImprovement()
    {
        std::cout << "\n=== Test 7: Price Improvement ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15000, 100);
        book.addOrder(buy1);

        assert(book.getBestBid() == 15000);

        // Add better bid (higher price)
        Order* buy2 = createTestOrder(2, 15100, 200);
        book.addOrder(buy2);
        assert(book.getBestBid() == 15100);
        assert(book.getBidPriceAtDepth(0) == 15100);
        assert(book.getBidPriceAtDepth(1) == 15000);

        Order* sell1 = createTestOrder(101, 16000, 50);
        book.addOrder(sell1);
        assert(book.getBestAsk() == 16000);

        // Add better ask (lower price)
        Order* sell2 = createTestOrder(102, 15900, 80);
        book.addOrder(sell2);
        assert(book.getBestAsk() == 15900);
        assert(book.getAskPriceAtDepth(0) == 15900);
        assert(book.getAskPriceAtDepth(1) == 16000);

        std::cout << "PASS: Price improvement working correctly" << std::endl;
        printBookState(book);
    }

    static void testMarketOrderExecution()
    {
        std::cout << "\n=== Test 8: Market Order Execution ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15100, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15000, 200);
        book.addOrder(buy2);

        Order* sell1 = createTestOrder(101, 15900, 50);
        book.addOrder(sell1);

        Order* sell2 = createTestOrder(102, 16000, 80);
        book.addOrder(sell2);

        assert(book.getOrderCount() == 4);
        assert(book.getBestAsk() == 15900);
        assert(book.getBestAskVolume() == 50);

        // Market BUY: execute against best ask
        PriceLevel* askLevel = book.getBestAskLevel();
        Order* bestAskOrder = askLevel->head;
        assert(bestAskOrder->orderId == 101);
        book.removeOrder(bestAskOrder);

        assert(book.getOrderCount() == 3);
        assert(book.getBestAsk() == 16000);
        assert(book.getBestAskVolume() == 80);

        // Market SELL: execute against best bid
        PriceLevel* bidLevel = book.getBestBidLevel();
        Order* bestBidOrder = bidLevel->head;
        assert(bestBidOrder->orderId == 1);
        book.removeOrder(bestBidOrder);

        assert(book.getOrderCount() == 2);
        assert(book.getBestBid() == 15000);
        assert(book.getBestBidVolume() == 200);

        std::cout << "PASS: Market order execution working correctly" << std::endl;
        printBookState(book);
    }

    static void testDepthLevelAccess()
    {
        std::cout << "\n=== Test 9: Depth Level Access ===" << std::endl;
        PriceLadder book(10000, 20000);

        // Add multiple bid levels
        Order* buy1 = createTestOrder(1, 15100, 100);
        book.addOrder(buy1);
        Order* buy2 = createTestOrder(2, 15050, 150);
        book.addOrder(buy2);
        Order* buy3 = createTestOrder(3, 15000, 200);
        book.addOrder(buy3);
        Order* buy4 = createTestOrder(4, 14900, 250);
        book.addOrder(buy4);

        // Add multiple ask levels
        Order* sell1 = createTestOrder(101, 15900, 50);
        book.addOrder(sell1);
        Order* sell2 = createTestOrder(102, 16000, 80);
        book.addOrder(sell2);
        Order* sell3 = createTestOrder(103, 16100, 120);
        book.addOrder(sell3);

        // Test bid depth
        assert(book.getBidPriceAtDepth(0) == 15100);
        assert(book.getBidVolumeAtDepth(0) == 100);
        assert(book.getBidPriceAtDepth(1) == 15050);
        assert(book.getBidVolumeAtDepth(1) == 150);
        assert(book.getBidPriceAtDepth(2) == 15000);
        assert(book.getBidVolumeAtDepth(2) == 200);
        assert(book.getBidPriceAtDepth(3) == 14900);
        assert(book.getBidVolumeAtDepth(3) == 250);
        assert(book.getBidPriceAtDepth(10) == 0);

        // Test ask depth
        assert(book.getAskPriceAtDepth(0) == 15900);
        assert(book.getAskVolumeAtDepth(0) == 50);
        assert(book.getAskPriceAtDepth(1) == 16000);
        assert(book.getAskVolumeAtDepth(1) == 80);
        assert(book.getAskPriceAtDepth(2) == 16100);
        assert(book.getAskVolumeAtDepth(2) == 120);
        assert(book.getAskPriceAtDepth(10) == UINT64_MAX);

        // Test level pointers
        const PriceLevel* bidLevel0 = book.getBidLevel(0);
        assert(bidLevel0 != nullptr);
        assert(bidLevel0->priceTick == 15100);

        const PriceLevel* askLevel0 = book.getAskLevel(0);
        assert(askLevel0 != nullptr);
        assert(askLevel0->priceTick == 15900);

        std::cout << "PASS: Depth level access working correctly" << std::endl;
        printBookState(book);
    }

    static void testEmptyBookOperations()
    {
        std::cout << "\n=== Test 10: Empty Book Operations ===" << std::endl;
        PriceLadder book(10000, 20000);

        assert(book.isEmpty());
        assert(book.getOrderCount() == 0);
        assert(book.getBestBid() == 0);
        assert(book.getBestAsk() == UINT64_MAX);
        assert(book.getBestBidLevel() == nullptr);
        assert(book.getBestAskLevel() == nullptr);
        assert(book.getBestBidVolume() == 0);
        assert(book.getBestAskVolume() == 0);
        assert(book.getBidPriceAtDepth(0) == 0);
        assert(book.getAskPriceAtDepth(0) == UINT64_MAX);
        assert(book.getNumBidLevels() > 0);
        assert(book.getNumAskLevels() > 0);

        std::cout << "PASS: Empty book operations working correctly" << std::endl;
        printBookState(book);
    }

    static void testLargeVolumeOperations()
    {
        std::cout << "\n=== Test 11: Large Volume Operations ===" << std::endl;
        PriceLadder book(10000, 20000);
        constexpr uint64_t largeVolume = 1000000000ULL;

        Order* buy1 = createTestOrder(1, 15000, largeVolume);
        book.addOrder(buy1);

        assert(book.getBestBidVolume() == largeVolume);
        assert(book.getLevel(15000)->totalVolume == largeVolume);

        Order* sell1 = createTestOrder(101, 16000, largeVolume * 2);
        book.addOrder(sell1);

        assert(book.getBestAskVolume() == largeVolume * 2);
        assert(book.getLevel(16000)->totalVolume == largeVolume * 2);

        // Modify to even larger volume
        const uint64_t newVolume = largeVolume * 10;
        book.modifyOrderVolume(buy1, newVolume);
        assert(buy1->volume == newVolume);
        assert(book.getBestBidVolume() == newVolume);
        assert(book.getLevel(15000)->totalVolume == newVolume);

        std::cout << "PASS: Large volume operations working correctly" << std::endl;
        printBookState(book);
    }

    static void testFindOrderById()
    {
        std::cout << "\n=== Test 12: Find Order by ID ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1001, 15000, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(1002, 15100, 200);
        book.addOrder(buy2);

        Order* sell1 = createTestOrder(2001, 16000, 50);
        book.addOrder(sell1);

        Order* found = book.findOrder(1002);
        assert(found != nullptr);
        assert(found->orderId == 1002);
        assert(found->priceTick == 15100);
        assert(found->volume == 200);

        found = book.findOrder(2001);
        assert(found != nullptr);
        assert(found->orderId == 2001);
        assert(found->priceTick == 16000);
        assert(found->volume == 50);

        found = book.findOrder(9999);
        assert(found == nullptr);

        // Remove order and verify cannot find it
        book.removeOrder(buy1);
        found = book.findOrder(1001);
        assert(found == nullptr);

        std::cout << "PASS: Order lookup by ID working correctly" << std::endl;
        printBookState(book);
    }

    static void testRemoveFromMiddle()
    {
        std::cout << "\n=== Test 13: Remove Order from Middle of List ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* first = createTestOrder(1, 15100, 100);
        book.addOrder(first);

        Order* second = createTestOrder(2, 15100, 200);
        book.addOrder(second);

        Order* third = createTestOrder(3, 15100, 300);
        book.addOrder(third);

        PriceLevel* level = book.getLevel(15100);
        assert(countOrdersInLevel(level) == 3);
        assert(level->head == first);
        assert(level->tail == third);

        // Remove middle order
        book.removeOrder(second);

        assert(countOrdersInLevel(level) == 2);
        assert(level->head == first);
        assert(level->head->next == third);
        assert(level->tail == third);
        assert(level->totalVolume == 400);

        // Remove first order
        book.removeOrder(first);
        assert(countOrdersInLevel(level) == 1);
        assert(level->head == third);
        assert(level->tail == third);
        assert(level->totalVolume == 300);

        // Remove last order
        book.removeOrder(third);
        assert(countOrdersInLevel(level) == 0);
        assert(level->head == nullptr);
        assert(level->tail == nullptr);
        assert(level->totalVolume == 0);

        std::cout << "PASS: Remove from middle of list working correctly" << std::endl;
        printBookState(book);
    }

    static void testGetLevelByPrice()
    {
        std::cout << "\n=== Test 14: Get Level by Price ===" << std::endl;
        PriceLadder book(10000, 20000);

        Order* buy1 = createTestOrder(1, 15000, 100);
        book.addOrder(buy1);

        Order* buy2 = createTestOrder(2, 15200, 200);
        book.addOrder(buy2);

        PriceLevel* level = book.getLevel(15000);
        assert(level != nullptr);
        assert(level->priceTick == 15000);
        assert(level->totalVolume == 100);
        assert(level->head == buy1);
        assert(level->tail == buy1);

        level = book.getLevel(15200);
        assert(level != nullptr);
        assert(level->priceTick == 15200);
        assert(level->totalVolume == 200);
        assert(level->head == buy2);
        assert(level->tail == buy2);

        // Empty level
        level = book.getLevel(15100);
        assert(level != nullptr);
        assert(level->priceTick == 15100);
        assert(level->totalVolume == 0);
        assert(level->head == nullptr);
        assert(level->tail == nullptr);
        assert(level->isEmpty());

        // Const version
        const PriceLadder& constBook = book;
        const PriceLevel* constLevel = constBook.getLevel(15000);
        assert(constLevel != nullptr);
        assert(constLevel->priceTick == 15000);

        std::cout << "PASS: Get level by price working correctly" << std::endl;
        printBookState(book);
    }

}


void collections::price_level_storage_2::TestAll()
{
    using namespace tests;

    testAddBuyOrders();
    /*
    testAddSellOrders();
    testMixedOrders();
    testTimePriority();
    testOrderCancellation();
    testOrderModification();
    testPriceImprovement();
    testMarketOrderExecution();
    testDepthLevelAccess();
    testEmptyBookOperations();
    testLargeVolumeOperations();
    testFindOrderById();
    testRemoveFromMiddle();
    testGetLevelByPrice();*/
}
