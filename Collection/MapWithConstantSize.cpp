/**============================================================================
Name        : MapWithConstantSize.cpp
Created on  : 23.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : MapWithConstantSize.cpp
============================================================================**/

#include "Collections.hpp"
#include "PerfUtilities.hpp"

#include <iostream>
#include <print>
#include <ostream>
#include <map>
#include <vector>
#include <random>


namespace
{
    std::random_device rd{};
    std::mt19937 generator = std::mt19937 {rd()};

    int getRandomInRange(const int32_t start, const int32_t end) noexcept
    {
        auto distribution = std::uniform_int_distribution<>{ start, end };
        return distribution(generator);
    }

    template<typename K, typename V >
    std::ostream& operator<<(std::ostream& stream, const std::map<K, V>& map)
    {
        for (const auto & [k, v]: map)
            stream << "{" << k << " = " << v << "} ";
        return stream;
    }
}

namespace static_size_map
{
    constexpr uint32_t maxSize { 10 };

    template<typename K, typename V >
    bool push(std::map<K, V>& map, const K& key, const V& value)
    {
        const auto size = map.size();
        if (maxSize == size && key >= map.rbegin()->first)
            return false;

        const auto [iter, ok] = map.emplace(key, value);
        if (maxSize == size) {
            map.erase(std::prev(map.end()));
        }

        return true;;
    }

    void test_map_with_constant_size()
    {
        std::map<int, int> map;
        for (int i = 0; i < 20; i += 2)
        {
            push(map, i, i);
            std::cout << map << std::endl;
        }

        push(map, 3 ,3);
        std::cout << map << std::endl;
    }
}

namespace performance_test
{
    [[nodiscard]]
    std::vector<int32_t> getTestData(const size_t size = 10'000'000)
    {
        std::vector<int32_t> data(size);
        for (size_t i = 0; i < size; ++i) {
            data[i] = getRandomInRange(0, size);
        }
        return data;
    }

    template<typename K, typename V, size_t maxSize>
    bool push(std::map<K, V>& map, const K& key, const V& value)
    {
        const auto size = map.size();
        if (maxSize == size && key >= map.rbegin()->first)
            return false;

        const auto [iter, ok] = map.emplace(key, value);
        if (maxSize == size) {
            map.erase(std::prev(map.end()));
        }

        return true;;
    }

    void benchmark()
    {
        constexpr uint32_t collectionSize { 1'000 }, testDataSize = 100'000'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        {
            PerfUtilities::ScopedTimer timer { "std::map"};
            std::map<int32_t, int32_t> map;

            for (uint32_t idx = 0; idx < testDataSize; ++idx)
            {
                const auto key = data[idx];
                push<int32_t, int32_t, collectionSize>(map, key, key);
            }

            // std::cout << map << std::endl;
        }
    }
}

void collections::MapWithConstantSize()
{
    // static_size_map::test_map_with_constant_size();

    performance_test::benchmark();
}