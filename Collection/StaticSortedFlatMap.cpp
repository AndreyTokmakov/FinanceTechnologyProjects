/**============================================================================
Name        : StaticSortedFlatMap.cpp
Created on  : 25.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : StaticSortedFlatMap.cpp
============================================================================**/

#include "Collections.hpp"
#include "PerfUtilities.hpp"
#include "Testing.hpp"

#include <iostream>
#include <print>
#include <utility>
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


namespace static_sorted_flat_map
{
    enum class SortOrder {
        Ascending,
        Descending
    };

    template<typename K, typename V, SortOrder ordering = SortOrder::Ascending>
    struct FlatMap
    {
        static_assert(!std::is_same_v<K, void>, "ERROR: Key type can not be void");
        static_assert(!std::is_same_v<V, void>, "ERROR: Value type can not be void");

        struct Node
        {
            K key;
            V value;
        };

        using key_type       = K;
        using value_type     = V;
        using size_type      = uint32_t;
        using node_pointer   = Node*;
        using const_pointer  = const node_pointer;
        using array_type     = Node[];

        explicit FlatMap(const size_type capacity) :
                _size { 0 }, _capacity { capacity }, _data { std::make_unique<array_type>(capacity) }  {
        }

        FlatMap(const FlatMap & other):
                _size { other._size },
                _capacity { other._capacity },
                _data { std::make_unique_for_overwrite<array_type>(_capacity) }
        {
            std::copy_n(other._data.get(), _size, _data.get());
        }

        FlatMap & operator=(const FlatMap & other)
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = std::make_unique_for_overwrite<array_type>(_capacity);
            std::copy_n(other._data.get(), _size, _data.get());

            return *this;
        }

        FlatMap(FlatMap && other) noexcept:
                _size { std::exchange(other._size, 0) },
                _capacity { std::exchange(other._capacity, 0) },
                _data { std::move(other._data) }
        {
        }

        FlatMap & operator=(FlatMap && other) noexcept
        {
            _size = std::exchange(other._size, 0);
            _capacity = std::exchange(other._capacity, 0);
            _data = std::move(other._data);

            return *this;
        }

        bool push(const key_type& key, const value_type& value)
        {
            if (_size > 0 && !compare(key, _data[_size - 1].key))
            {
                if (_size == _capacity)
                    return false;
                _data[_size++] = Node {key, value};
                return true;
            }

            const size_type idxInsert = findInsertIndex(key);
            if (_capacity == idxInsert || key == _data[idxInsert].key) { // TODO: Fixme
                return false;
            }

            _size = (_capacity == _size) ? _size : _size + 1;
            // __builtin_prefetch(elements.get() + size - 32, 0, 2);
            for (size_type i = _size - 1; i > idxInsert; --i) { /** TODO: Prefetch **/
                _data[i] = _data[i - 1];
            }
            _data[idxInsert] = Node {key, value};
            return true;
        }

        [[nodiscard]]
        const_pointer data() const noexcept {
            return _data.get();
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return _size;
        }

    private:

        size_type _size { 0 };
        size_type _capacity { 0 };
        std::unique_ptr<array_type> _data { nullptr };

        [[nodiscard]]
        size_type findInsertIndex(const key_type& key) const noexcept
        {
            size_type left = 0, right = _size;
            while (left < right)
            {
                const size_type mid = (left + right) >> 1;
                if (compare(key, _data[mid].key))
                    right = mid;
                else
                    left = mid + 1;
            }
            return left;
        }

        static constexpr bool compare(const key_type& a, const key_type& b) noexcept __attribute__((always_inline))
        {
            if constexpr (SortOrder::Descending == ordering)
                return a >= b;
            else
                return a <= b;
        }
    };
}

namespace static_sorted_flat_map::testing
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
}

namespace static_sorted_flat_map::testing
{
    using ::testing::AssertEqual;
    using ::testing::AssertTrue;

    template<typename K, typename V, SortOrder ordering = SortOrder::Ascending>
    void print(const FlatMap<K, V, ordering>& flatMap)
    {
        for (typename FlatMap<K, V>::size_type idx = 0; idx < flatMap.Size(); ++idx) {
            std::cout << flatMap.data()[idx].key << " ";
        }
        std::cout << std::endl;
    }

    void validation()
    {
        constexpr uint32_t collectionSize { 10 }, testDataSize = 1000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        FlatMap<int, int> flatMap (collectionSize);
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key,key * key);
        }

        for (uint32_t idx = 0; idx < collectionSize; ++idx)
        {
            const auto node = flatMap.data()[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }
    }

    void checkIsSorted_Ascending()
    {
        constexpr uint32_t collectionSize { 100 }, testDataSize = 10'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        FlatMap<int, int> flatMap (collectionSize);
        using Node = FlatMap<int, int>::Node;
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key,key * 10);
            const bool isSorted = std::is_sorted(flatMap.data(), flatMap.data() + flatMap.size(), [](const Node& a, const Node& b) {
                return b.key >= a.key;
            });

            AssertTrue(isSorted);
        }
        AssertEqual(collectionSize, flatMap.size());
        std::cout << "OK" << std::endl;
    }

    void checkIsSorted_Descending()
    {
        constexpr uint32_t collectionSize { 100 }, testDataSize = 10'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        FlatMap<int, int, SortOrder::Descending> flatMap (collectionSize);
        using Node = decltype(flatMap)::Node;
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key,key * 10);
            const bool isSortedDesc = std::is_sorted(flatMap.data(), flatMap.data() + flatMap.size(), [](const Node& a, const Node& b) {
                return a.key >= b.key;
            });

            //print(flatMap);
            AssertTrue(isSortedDesc);
        }
        AssertEqual(collectionSize, flatMap.size());
        std::cout << "OK" << std::endl;
    }
}

namespace static_sorted_flat_map::testing::performance
{
    void benchmark_Ascending()
    {
        constexpr uint32_t collectionSize { 1'000 }, testDataSize = 100'000'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        PerfUtilities::ScopedTimer timer { "FlatMap"};
        FlatMap<int, int> flatMap (collectionSize);
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key, key);
        }
        AssertEqual(collectionSize, flatMap.size());
    }

    void benchmark_Descending()
    {
        constexpr uint32_t collectionSize { 1'000 }, testDataSize = 100'000'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        PerfUtilities::ScopedTimer timer { "FlatMap"};
        FlatMap<int, int, SortOrder::Descending> flatMap (collectionSize);
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key, key);
        }
        AssertEqual(collectionSize, flatMap.size());
    }
}


// TODO:
//  - front()
//  - back()

void collections::StaticSortedFlatMap()
{
    // static_sorted_flat_map::testing::validation();
    // static_sorted_flat_map::testing::checkIsSorted_Ascending();
    // static_sorted_flat_map::testing::checkIsSorted_Descending();

    static_sorted_flat_map::testing::performance::benchmark_Ascending();
    // static_sorted_flat_map::testing::performance::benchmark_Descending();
}