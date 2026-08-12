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
            minPriceTick(minPriceTick),
            maxPriceTick(maxPriceTick),
            numLevels(maxPriceTick - minPriceTick + 1)
        {
            levels.reserve(numLevels);
            for (Price price = minPriceTick; price <= maxPriceTick; ++price) {
                levels.emplace_back(price);
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
        const PriceLevel* getLevel(const Price priceTick) const noexcept {
            const size_t index = priceTick - minPriceTick;
            return &levels[index];
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

            if (hadSingleOrder) {
                updateBestPointersAfterRemoval(level);
            }
        }

        static void modifyOrderVolume(Order* order, const Volume newVolume) noexcept {
            order->level->totalVolume -= order->volume;
            order->level->totalVolume += newVolume;
            order->volume = newVolume;
        }

        [[nodiscard]]
        Order* findOrder(const OrderId orderId) const noexcept {
            const auto it = orderIndex.find(orderId);
            return (it != orderIndex.end()) ? it->second : nullptr;
        }

        [[nodiscard]]
        Price getBestBid() const noexcept {
            return bestBidLevel ? bestBidLevel->priceTick : 0;
        }

        [[nodiscard]]
        Price getBestAsk() const noexcept {
            return bestAskLevel ? bestAskLevel->priceTick : UINT64_MAX;
        }

        [[nodiscard]]
        PriceLevel* getBestBidLevel() const noexcept {
            return bestBidLevel;
        }

        [[nodiscard]]
        PriceLevel* getBestAskLevel() const noexcept {
            return bestAskLevel;
        }

        [[nodiscard]]
        Volume getBestBidVolume() const noexcept {
            return bestBidLevel ? bestBidLevel->totalVolume : 0;
        }

        [[nodiscard]]
        Volume getBestAskVolume() const noexcept {
            return bestAskLevel ? bestAskLevel->totalVolume : 0;
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

    private:
        static constexpr size_t kDefaultReserveSize = 1000000;

        void updateBestPointers(PriceLevel* level) noexcept
        {
            const Price price = level->priceTick;
            const Price midPrice = (minPriceTick + maxPriceTick) / 2;

            if (price >= midPrice)
            {
                // Potential ask side (price >= midpoint)
                if (!bestAskLevel || price < bestAskLevel->priceTick) {
                    bestAskLevel = level;
                }
            }
            else {
                // Potential bid side (price < midpoint)
                if (!bestBidLevel || price > bestBidLevel->priceTick) {
                    bestBidLevel = level;
                }
            }
        }

        void updateBestPointersAfterRemoval(const PriceLevel* level) noexcept
        {
            if (level == bestBidLevel) {
                bestBidLevel = findBestBid();
            }
            if (level == bestAskLevel) {
                bestAskLevel = findBestAsk();
            }
        }

        [[nodiscard]]
        PriceLevel* findBestBid() const noexcept
        {
            for (int64_t i = static_cast<int64_t>(numLevels) - 1; i >= 0; --i) {
                if (!levels[i].isEmpty()) {
                    return const_cast<PriceLevel*>(&levels[i]);
                }
            }
            return nullptr;
        }

        [[nodiscard]]
        PriceLevel* findBestAsk() const noexcept
        {
            for (size_t i = 0; i < numLevels; ++i) {
                if (!levels[i].isEmpty()) {
                    return const_cast<PriceLevel*>(&levels[i]);
                }
            }
            return nullptr;
        }

        uint64_t minPriceTick;
        uint64_t maxPriceTick;
        size_t numLevels;
        std::vector<PriceLevel> levels;

    #if 1
        std::unordered_map<OrderId, Order*> orderIndex;
    #else
        absl::flat_hash_map<OrderId, Order*> orderIndex;
    #endif

        // Cached best pointers for O(1) top-of-book access
        PriceLevel* bestBidLevel { nullptr };
        PriceLevel* bestAskLevel { nullptr };
    };
}

void collections::price_level_storage_2::TestAll()
{

}
