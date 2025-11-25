/**============================================================================
Name        : StaticSortedArray.cpp
Created on  : 23.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : StaticSortedArray.cpp
============================================================================**/

#include "Collections.hpp"
#include "PerfUtilities.hpp"

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


namespace static_sorted_array
{
     enum class SortOrder
    {
        Ascending,
        Descending
    };

    template<typename Ty, SortOrder ordering = SortOrder::Ascending>
    struct SortedArray
    {
        using size_type      = uint32_t;
        using value_type     = Ty;
        using pointer        = value_type*;
        using const_pointer  = const pointer;
        using array_type     = value_type[];

        size_type size { 0 };
        size_type capacity { 0 };
        std::unique_ptr<array_type> elements { nullptr };

        explicit SortedArray(const size_type capacity) :
                size { 0 }, capacity { capacity }, elements { std::make_unique<array_type>(capacity) }  {
        }

        SortedArray(const SortedArray & other):
                size { other.size },
                capacity { other.capacity },
                elements { std::make_unique_for_overwrite<array_type>(capacity) }
        {
            std::copy_n(other.elements.get(), size, elements.get());
        }

        SortedArray & operator=(const SortedArray & other)
        {
            size = other.size;
            capacity = other.capacity;
            elements = std::make_unique_for_overwrite<array_type>(capacity);
            std::copy_n(other.elements.get(), size, elements.get());

            return *this;
        }

        SortedArray(SortedArray && other) noexcept:
                size { std::exchange(other.size, 0) },
                capacity { std::exchange(other.capacity, 0) },
                elements { std::move(other.elements) }
        {
        }

        SortedArray & operator=(SortedArray && other) noexcept
        {
            size = std::exchange(other.size, 0);
            capacity = std::exchange(other.capacity, 0);
            elements = std::move(other.elements);

            return *this;
        }

        [[nodiscard]]
        size_type findInsertIndex(const value_type item) const noexcept
        {
            size_type left = 0, right = size;
            while (left < right)
            {
                const size_type mid = (left + right) >> 1;
                if (better(item, elements[mid]))
                    right = mid;
                else
                    left = mid + 1;
            }
            return left;
        }

        bool push(const value_type item)
        {
            if (size > 0 && item > elements[size - 1])
            {
                if (size == capacity)
                    return false;
                elements[size++] = item;
                return true;
            }

            const size_type idxInsert = findInsertIndex(item);
            if (capacity == idxInsert || item == elements[idxInsert]) {
                return false;
            }

            size = (capacity == size) ? size : size + 1;
            for (size_type i = size - 1; i > idxInsert; --i) /** TODO: Prefetch **/
                elements[i] = elements[i - 1];
            elements[idxInsert] = item;
            return true;
        }

        [[nodiscard]]
        pointer data() {
            return elements.get();
        }

        [[nodiscard]]
        const_pointer data() const {
            return elements.get();
        }

        [[nodiscard]]
        size_type Size() const {
            return size;
        }

        // TODO: compiler flags 'always inline'
        // TODO: Rename
        static constexpr bool better(const value_type a, const value_type b) noexcept
        {
            if constexpr (SortOrder::Descending == ordering)
                return a >= b;
            else
                return a <= b;
        }
    };
}

namespace static_sorted_array::testing
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

namespace static_sorted_array::testing::performance
{
    void benchmark()
    {
        constexpr uint32_t collectionSize { 1'000 }, testDataSize = 100'000'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        PerfUtilities::ScopedTimer timer { "SortedArray"};
        SortedArray<int> array (collectionSize);
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            array.push(key);
        }
        std::cout << array.Size() << std::endl;
    }
}

namespace static_sorted_array::testing
{
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

    void validation()
    {
        constexpr uint32_t collectionSize { 1'0000 }, testDataSize = 100'000'000;
        const std::vector<int32_t> data = getTestData(testDataSize);

        std::map<int32_t, int32_t> map;
        SortedArray<int> array (collectionSize);

        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            array.push(key);
            push<int32_t, int32_t, collectionSize>(map, key, key);
        }

        if (map.size() != array.Size()) {
            std::cerr << "ERROR: Size mismatch" << std::endl;
            return;
        }

        auto iter = map.begin();
        for (uint32_t idx = 0; idx < collectionSize; ++idx)
        {
            const auto valMap = iter->first;
            const auto valArr = array.elements[idx];

            std::cout << "[" << idx << "] = { " << valMap<< " | " << valArr << " } ";
            if (valMap != valArr) {
                std::cout << "                ERROR";
            }
            std::cout << std::endl;
            ++iter;
        }
    }
}

void collections::StaticSortedArray()
{
    static_sorted_array::testing::validation();
    static_sorted_array::testing::performance::benchmark();

}