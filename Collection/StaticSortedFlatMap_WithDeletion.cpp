/**============================================================================
Name        : StaticSortedFlatMap_WithDeletion.cpp
Created on  : 28.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : StaticSortedFlatMap_WithDeletion.cpp
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


namespace static_sorted_flat_map_with_deletion
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

        size_type size { 0 };
        size_type capacity { 0 };
        std::unique_ptr<array_type> elements { nullptr };

        explicit FlatMap(const size_type capacity) :
                size { 0 }, capacity { capacity }, elements { std::make_unique<array_type>(capacity) }  {
        }

        FlatMap(const FlatMap & other):
                size { other.size },
                capacity { other.capacity },
                elements { std::make_unique_for_overwrite<array_type>(capacity) }
        {
            std::copy_n(other.elements.get(), size, elements.get());
        }

        FlatMap & operator=(const FlatMap & other)
        {
            size = other.size;
            capacity = other.capacity;
            elements = std::make_unique_for_overwrite<array_type>(capacity);
            std::copy_n(other.elements.get(), size, elements.get());

            return *this;
        }

        FlatMap(FlatMap && other) noexcept:
                size { std::exchange(other.size, 0) },
                capacity { std::exchange(other.capacity, 0) },
                elements { std::move(other.elements) }
        {
        }

        FlatMap & operator=(FlatMap && other) noexcept
        {
            size = std::exchange(other.size, 0);
            capacity = std::exchange(other.capacity, 0);
            elements = std::move(other.elements);

            return *this;
        }

        [[nodiscard]]
        size_type findInsertIndex(const key_type& key) const noexcept
        {
            size_type left = 0, right = size;
            while (left < right)
            {
                const size_type mid = (left + right) >> 1;
                if (compare(key, elements[mid].key))
                    right = mid;
                else
                    left = mid + 1;
            }
            return left;
        }

        bool push(const key_type& key, const value_type& value)
        {
            if (size > 0 && !compare(key, elements[size - 1].key))
            {
                if (size == capacity)
                    return false;
                elements[size++] = Node {key, value};
                return true;
            }

            const size_type idxInsert = findInsertIndex(key);
            if (capacity == idxInsert || key == elements[idxInsert].key) { // TODO: Fixme
                return false;
            }

            size = (capacity == size) ? size : size + 1;
            // __builtin_prefetch(elements.get() + size - 32, 0, 2);
            for (size_type i = size - 1; i > idxInsert; --i) { /** TODO: Prefetch **/
                elements[i] = elements[i - 1];
            }
            elements[idxInsert] = Node {key, value};
            return true;
        }

        bool erase(const key_type& key)
        {
            if (size > 0 && (!compare(elements[0].key, key) || !compare(key, elements[size - 1].key)))
            {
                return true;
            }

            const size_type idx = findInsertIndex(key);
            if (key != elements[idx].key) {
                return false;
            }

            elements[idx].key = -1;
            // std::cout << "Erasing: " << key << ". at [" << idx << "] = " << elements[idx].key << std::endl;

            return true;
        }

        [[nodiscard]]
        node_pointer data() {
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

        static constexpr bool compare(const key_type& a, const key_type& b) noexcept __attribute__((always_inline))
        {
            if constexpr (SortOrder::Descending == ordering)
                return a >= b;
            else
                return a <= b;
        }
    };
}

namespace static_sorted_flat_map_with_deletion::testing
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

namespace static_sorted_flat_map_with_deletion::testing
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
        constexpr uint32_t collectionSize { 10 }, testDataSize = 100;
        const std::vector<int32_t> data = getTestData(testDataSize);

        FlatMap<int, int> flatMap (collectionSize);
        for (uint32_t idx = 0; idx < testDataSize; ++idx)
        {
            const auto key = data[idx];
            flatMap.push(key,key * key);
        }

        for (uint32_t idx = 0; idx < collectionSize; ++idx)
        {
            const auto node = flatMap.elements[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }
    }

    void erase_ascending()
    {
        constexpr uint32_t collectionSize { 10 };

        FlatMap<int, int> flatMap (collectionSize);
        for (int i = 40; i >= 0; i -= 2) {
            flatMap.push(i,i * i);
        }

        for (uint32_t idx = 0; idx < collectionSize; ++idx) {
            const auto node = flatMap.elements[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }

        std::cout << std::string(120, '-') << std::endl;
        flatMap.erase(-1);
        flatMap.erase(0);
        flatMap.erase(1);
        flatMap.erase(8);
        flatMap.erase(9);
        flatMap.erase(18);
        flatMap.erase(21);
        std::cout << std::string(120, '-') << std::endl;

        for (uint32_t idx = 0; idx < collectionSize; ++idx) {
            const auto node = flatMap.elements[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }
    }

    void erase_descending()
    {
        constexpr uint32_t collectionSize { 10 };

        FlatMap<int, int, SortOrder::Descending> flatMap (collectionSize);
        for (int i = 0; i <= 20; ++i) {
            flatMap.push(i,i * i);
        }

        for (uint32_t idx = 0; idx < collectionSize; ++idx) {
            const auto node = flatMap.elements[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }

        std::cout << std::string(120, '-') << std::endl;
        flatMap.erase(10);
        flatMap.erase(11);
        flatMap.erase(12);
        flatMap.erase(19);
        flatMap.erase(20);
        flatMap.erase(21);
        std::cout << std::string(120, '-') << std::endl;

        for (uint32_t idx = 0; idx < collectionSize; ++idx) {
            const auto node = flatMap.elements[idx];
            std::cout << "[" << idx << "] = { " << node.key<< " | " << node.value << " } " << std::endl;
        }
    }
}

// TODO:
//  - front()
//  - back()

void collections::StaticSortedFlatMap_WithDeletion()
{
    // static_sorted_flat_map_with_deletion::testing::validation();
    static_sorted_flat_map_with_deletion::testing::erase_ascending();
    // static_sorted_flat_map_with_deletion::testing::erase_descending();


}