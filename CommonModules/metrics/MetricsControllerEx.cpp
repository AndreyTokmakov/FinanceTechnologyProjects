/**============================================================================
Name        : MetricsController.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : C++ Experiments
============================================================================**/

#include <iostream>
#include <print>
#include <format>
#include <string_view>
#include <array>
#include <vector>
#include <thread>

#include <numeric>
#include <cstdint>
#include <functional>
#include <memory>

#include "metrics.hpp"


namespace
{
    enum class MetricType : uint8_t
    {
        None,
        Counter,
        Gauge,
        Histogram
    };

    constexpr size_t MaxMetrics = 256;
    constexpr size_t CacheLineSize = std::hardware_constructive_interference_size;
    constexpr uint64_t InitialMinValue = std::numeric_limits<uint64_t>::max();
    constexpr uint32_t SlotMagic = 0xDEADBEEF;

    struct alignas(CacheLineSize) MetricSlot
    {
        std::atomic<uint64_t> value{0};
        std::atomic<uint64_t> sum{0};
        std::atomic<uint64_t> count{0};
        std::atomic<uint64_t> min{InitialMinValue};
        std::atomic<uint64_t> max{0};
        std::atomic<uint32_t> magic{SlotMagic};
    };

    struct MetricHandle
    {
        uint16_t index { 0 };
        MetricType type { MetricType::None };
        MetricSlot* slot { nullptr };

        constexpr MetricHandle() noexcept: type(MetricType::Counter) {
        }

        constexpr MetricHandle(const uint16_t idx, const MetricType t, MetricSlot* s) noexcept:
            index(idx), type(t) , slot(s) {
        }

        [[nodiscard]]
        constexpr bool isValid() const noexcept{
            return slot != nullptr && type != MetricType::None;
        }

        void increment(const uint64_t delta = 1) const noexcept
        {
            if (!isValid() || type != MetricType::Counter) {
                return;
            }
            slot->value.fetch_add(delta, std::memory_order_relaxed);
        }

        void add(const uint64_t delta) const noexcept {
            increment(delta);
        }

        void set(const uint64_t value) const noexcept
        {
            if (!isValid() || type != MetricType::Gauge) {
                return;
            }
            slot->value.store(value, std::memory_order_release);
        }

        void update(const uint64_t value) const noexcept {
            set(value);
        }


        void record(const uint64_t value) const noexcept
        {
            if (!isValid() || type != MetricType::Histogram) {
                return;
            }

            slot->count.fetch_add(1, std::memory_order_relaxed);
            slot->sum.fetch_add(value, std::memory_order_relaxed);

            for (uint64_t oldMin = slot->min.load(std::memory_order_relaxed); value < oldMin;) {
                if (slot->min.compare_exchange_weak(oldMin, value, std::memory_order_relaxed)) {
                    break;
                }
            }
            for ( uint64_t oldMax = slot->max.load(std::memory_order_relaxed); value > oldMax;) {
                if (slot->max.compare_exchange_weak(oldMax, value,std::memory_order_relaxed)) {
                    break;
                }
            }
        }

        void observe(const uint64_t value) const noexcept {
            record(value);
        }

        [[nodiscard]]
        uint64_t value() const noexcept
        {
            if (!isValid()) {
                return 0;
            }
            return slot->value.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        uint64_t sum() const noexcept
        {
            if (!isValid() || type != MetricType::Histogram) {
                return 0;
            }
            return slot->sum.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        uint64_t count() const noexcept
        {
            if (!isValid() || type != MetricType::Histogram) {
                return 0;
            }
            return slot->count.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        uint64_t min() const noexcept
        {
            if (!isValid() || type != MetricType::Histogram) {
                return 0;
            }
            return slot->min.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        uint64_t max() const noexcept
        {
            if (!isValid() || type != MetricType::Histogram) {
                return 0;
            }
            return slot->max.load(std::memory_order_acquire);
        }
    };
}

namespace
{
    template<size_t MaxMetrics = MaxMetrics>
    class MetricRegistry
    {
    public:

        [[nodiscard]]
        constexpr MetricRegistry() noexcept:
            slots {}, names{} , types{} , registeredCount{0} , initialized{false}
        {
        }

        template<MetricType Type>
        [[nodiscard]]
        MetricHandle registerMetric(std::string_view name) noexcept
        {
            static_assert(Type != MetricType::None, "Cannot register metric with type None");

            if (initialized) {
                return MetricHandle{};
            }
            uint16_t idx = registeredCount.fetch_add(1, std::memory_order_relaxed);
            if (idx >= MaxMetrics) {
                __builtin_trap();
            }
            names[idx] = name.data();
            types[idx] = Type;
            return MetricHandle{idx, Type, &slots[idx]};
        }

        constexpr void finalize() noexcept{
            initialized = true;
        }

        struct MetricSnapshot
        {
            std::string_view name;
            MetricType type { MetricType::None };
            uint64_t value { 0 };
            uint64_t sum { 0 };
            uint64_t count { 0 };
            uint64_t min { 0 };
            uint64_t max { 0 };
        };

        [[nodiscard]]
        std::vector<MetricSnapshot> snapshot() const
        {
            std::vector<MetricSnapshot> result;
            size_t count = registeredCount.load(std::memory_order_acquire);
            result.reserve(count);
            for (uint16_t i = 0; i < count; ++i)
            {
                const MetricSlot& slot = slots[i];
                MetricSnapshot& snap = result.emplace_back();
                snap.name  = names[i];
                snap.type  = types[i];
                snap.value = slot.value.load(std::memory_order_acquire);
                snap.sum   = slot.sum.load(std::memory_order_acquire);
                snap.count = slot.count.load(std::memory_order_acquire);
                snap.min   = slot.min.load(std::memory_order_acquire);
                snap.max   = slot.max.load(std::memory_order_acquire);
            }
            return result;
        }

        constexpr void resetHistograms() noexcept
        {
            const size_t count = registeredCount.load(std::memory_order_acquire);
            for (uint16_t i = 0; i < count; ++i)
            {
                if (types[i] == MetricType::Histogram)
                {
                    MetricSlot& slot = slots[i];
                    slot.sum.store(0, std::memory_order_relaxed);
                    slot.count.store(0, std::memory_order_relaxed);
                    slot.min.store(InitialMinValue, std::memory_order_relaxed);
                    slot.max.store(0, std::memory_order_relaxed);
                }
            }
        }

        [[nodiscard]]
        constexpr size_t size() const noexcept{
            return registeredCount.load(std::memory_order_acquire);
        }


    private:
        std::array<MetricSlot, MaxMetrics> slots;
        std::array<const char*, MaxMetrics> names;
        std::array<MetricType, MaxMetrics> types;
        std::atomic<uint16_t> registeredCount;
        bool initialized { false };
    };

    using DefaultRegistry = MetricRegistry<>;

    [[nodiscard]]
    DefaultRegistry& getRegistry() noexcept
    {
        static DefaultRegistry registry;
        return registry;
    }
}

namespace metrics::metrics_controller_ex
{

    MetricHandle gOrdersProcessed;
    MetricHandle gProcessingLatencyUs;
    MetricHandle gQueueDepth;

    static constexpr void initMetrics() noexcept
    {
        auto& registry = getRegistry();
        gOrdersProcessed = registry.registerMetric<MetricType::Counter>("orders.processed");
        gProcessingLatencyUs = registry.registerMetric<MetricType::Histogram>("latency.processing_us");
        gQueueDepth = registry.registerMetric<MetricType::Gauge>("queue.depth");
        registry.finalize();
    }

    [[nodiscard]]
    static uint64_t getCurrentTimestampNs() noexcept
    {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    static void processOrder(uint64_t orderId) noexcept
    {
        const uint64_t start = getCurrentTimestampNs();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        const uint64_t end = getCurrentTimestampNs();
        const uint64_t latencyUs = (end - start) / 1000;

        // Новый API - методы вызываются на самом хендле
        gOrdersProcessed.increment();           // Counter
        gProcessingLatencyUs.record(latencyUs); // Histogram

        static uint64_t queueSize = 0;
        queueSize = (queueSize + 1) % 100;
        gQueueDepth.set(queueSize);              // Gauge
    }

    void exporterThread() noexcept
    {
        auto& registry = getRegistry();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto snapshot = registry.snapshot();
            std::println("\n=== Metrics ===");
            for (const auto& m : snapshot) {
                if (m.type == MetricType::Counter) {
                    std::println("Counter: {} = {}", m.name, m.value);
                } else if (m.type == MetricType::Gauge) {
                    std::println("Gauge: {} = {}", m.name, m.value);
                } else if (m.type == MetricType::Histogram) {
                    double avg = m.count > 0 ? static_cast<double>(m.sum) / m.count : 0.0;
                    std::println("Histogram: {} count={} avg={:.2f} min={} max={}",
                                 m.name, m.count, avg, m.min, m.max);
                }
            }
            registry.resetHistograms();
        }
    }

    void runTest()
    {
        std::println("Initializing metrics system...");
        initMetrics();
        std::println("Starting worker thread...");
        std::thread worker([]() {
            uint64_t orderId = 0;
            while (true) {
                processOrder(++orderId);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        std::println("Starting exporter thread...");
        std::thread exporter(exporterThread);
        std::println("System running. Press Ctrl+C to stop.");
        worker.join();
        exporter.join();
    }
}

void metrics::metrics_controller_ex::TestAll()
{
    runTest();
}