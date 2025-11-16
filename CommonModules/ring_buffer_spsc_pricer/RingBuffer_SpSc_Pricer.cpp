/**============================================================================
Name        : RingBuffer_SpSc_Pricer.cpp
Created on  : 15.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : RingBuffer_SpSc_Pricer.cpp
============================================================================**/

#include "RingBuffer_SpSc_Pricer.hpp"
#include "StringUtilities.hpp"
#include "DateTimeUtilities.hpp"

#include <cstdint>
#include <memory>
#include <cstring>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>
#include <atomic>
#include <thread>
#include <syncstream>
#include <random>
#include <variant>

namespace
{
    std::random_device rd{};
    auto generator = std::mt19937{ rd() };

    int getRandomInRange(const int start, const int end) noexcept
    {
        auto distribution = std::uniform_int_distribution<>{ start, end };
        return distribution(generator);
    }

    constexpr uint32_t fast_modulo(const uint32_t n, const uint32_t d) noexcept {
        return n & (d - 1);
    }

    constexpr bool is_pow_of_2(const int value) noexcept {
        return (value && !(value & (value - 1)));
    }
}

namespace
{
    template<typename T, uint32_t Capacity>
    struct RingBuffer
    {
        using size_type  = uint32_t;
        using value_type = T;
        using collection_type = std::vector<value_type>;

        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");
        static_assert(is_pow_of_2(Capacity), "ERROR: Capacity must be a power of 2");

        RingBuffer(): buffer(Capacity) {
        }

        [[nodiscard]]
        bool put(value_type& item)
        {
            const size_type headCached = head.load(std::memory_order::relaxed);
            const size_type headNext = fast_modulo(headCached + 1, Capacity);
            if (headNext == tail.load(std::memory_order::acquire)) {
                return false;
            }

            buffer[headCached] = item;
            head.store(headNext, std::memory_order::release);

            return true;
        }

        [[nodiscard]]
        bool pop(value_type& item)
        {
            const size_type tailLocal = tail.load(std::memory_order::relaxed);
            if (tailLocal == head.load(std::memory_order::acquire)) {
                return false;
            }

            item = std::move(buffer[tailLocal]);
            tail.store(fast_modulo(tailLocal + 1, Capacity), std::memory_order::release);

            return true;
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return head.load(std::memory_order::relaxed) - tail.load(std::memory_order::relaxed);
        }

        [[nodiscard]]
        size_type empty() const noexcept
        {
            // TODO: can 'acquire' be replaced with 'relaxed' ?
            return head.load(std::memory_order::acquire) == tail.load(std::memory_order::acquire);
        }

        [[nodiscard]]
        size_type full() const noexcept
        {
            return size() == Capacity;
        }

    private:

        std::vector<value_type> buffer;

        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> tail {0};
        alignas(std::hardware_destructive_interference_size) std::atomic<size_type> head {0};
    };
}

namespace tests
{
    struct PriceLevel
    {
        PriceLevel() {
            std::cout << "PriceLevel::PriceLevel()\n";
        }

        ~PriceLevel() {
            std::cout << "PriceLevel::~PriceLevel()\n";
        }

        PriceLevel(const PriceLevel &) {
            std::cout << "PriceLevel(const PriceLevel &)\n";
        }

        PriceLevel(PriceLevel &&) noexcept {
            std::cout << "PriceLevel(PriceLevel &&) noexcept\n";
        }

        PriceLevel & operator=(const PriceLevel&) {
            std::cout << " PriceLevel & operator=(const PriceLevel&)\n";
            return *this;
        }

        PriceLevel & operator=(PriceLevel &&) noexcept {
            std::cout << "PriceLevel & operator=(PriceLevel &&) noexcept\n";
            return *this;
        }
    };

    struct Trade {};
    struct Ticker {};
    struct BookUpdate
    {
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
    };

    using MarketEvent = std::variant<Trade, BookUpdate, Ticker>;

    struct EventHandler
    {
        void operator()(const Trade&) const {
            std::cout << "===> Trade" << std::endl;
        }
        void operator()(const Ticker&) const {
            std::cout << "===> Ticker" << std::endl;
        }
        void operator()(const BookUpdate& update) const {
            std::cout << "===> BookUpdate: size = {" << update.asks.size() << ", " << update.asks.size() << "}\n";
        }
    };

    BookUpdate makeBookUpdate()
    {
        BookUpdate update;

        update.bids.reserve(2);
        update.bids.emplace_back();
        update.bids.emplace_back();


        update.asks.reserve(2);
        update.asks.emplace_back();
        update.asks.emplace_back();

        return update;
    }

    Trade makeTrade()
    {
        Trade trade;
        return trade;
    }

    Ticker makeTicker()
    {
        Ticker ticker;
        return ticker;
    }

    uint32_t counter = 0;

    MarketEvent getEvent()
    {
        std::cout << "getEvent: " << counter << std::endl;
        ++counter;
        counter = counter % 3;


        if (0 == counter)
            return makeBookUpdate();
        if (1 == counter)
            return makeTrade();

        return makeTicker();
    }
}

namespace tests
{
    void processStringTest()
    {
        RingBuffer<std::string, 1024> queue {};

        auto produce = [&queue]
        {
            while (true)
            {
                const int32_t size = getRandomInRange(0, 1024);
                const std::string text = StringUtilities::randomString(size);
                std::osyncstream { std::cout } << DateTimeUtilities::getCurrentTime() << "Producer: " << size
                    << " bytes written\n" << text << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds (100U));
            }
        };

        auto consume = [&queue]
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds (100U));
            }
        };

        std::jthread producer { produce },
                     consumer { consume };
    }
}

namespace tests
{
    void processEventsTests()
    {
        RingBuffer<MarketEvent, 16> queue {};

        auto produce = [&queue]
        {
            while (true)
            {
                MarketEvent event = getEvent();
                queue.put(event);
                std::osyncstream { std::cout } << DateTimeUtilities::getCurrentTime() << "added" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds (250U));
            }
        };

        auto consume = [&queue]
        {
            EventHandler eventHandler;
            MarketEvent event;
            while (true)
            {
                if (queue.pop(event)) {
                    std::visit(eventHandler, event);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds (1u));
            }
        };

        std::jthread producer { produce },
                     consumer { consume };
    }
}


void RingBuffer_SpSc_Pricer::TestAll()
{
    // tests::processStringTest();
    // tests::processEventsTests();

    tests::BookUpdate update1 = tests::makeBookUpdate();

    std::cout << "===> BookUpdate1: size = {" << update1.asks.size() << ", " << update1.asks.size() << "}\n";

    tests::BookUpdate update2 = std::move(update1);

    std::cout << "===> BookUpdate2: size = {" << update2.asks.size() << ", " << update2.asks.size() << "}\n";
    std::cout << "===> BookUpdate1: size = {" << update1.asks.size() << ", " << update1.asks.size() << "}\n";
}