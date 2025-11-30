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

    // TODO: Concepts on <K>
    //  - Comparable
    template<typename Ty, SortOrder ordering = SortOrder::Ascending>
    class  SortedArray
    {
        using size_type      = uint32_t;
        using value_type     = Ty;
        using pointer        = value_type*;
        using const_pointer  = const pointer;
        using array_type     = value_type[];

        size_type _size { 0 };
        size_type _capacity { 0 };
        std::unique_ptr<array_type> _data { nullptr };

    public:

        explicit SortedArray(const size_type capacity) :
                _size { 0 }, _capacity { capacity }, _data { std::make_unique<array_type>(capacity) }  {
        }

        SortedArray(const SortedArray & other):
                _size { other._size },
                _capacity { other._capacity },
                _data { std::make_unique_for_overwrite<array_type>(_capacity) }
        {
            std::copy_n(other._data.get(), _size, _data.get());
        }

        SortedArray & operator=(const SortedArray & other)
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = std::make_unique_for_overwrite<array_type>(_capacity);
            std::copy_n(other._data.get(), _size, _data.get());

            return *this;
        }

        SortedArray(SortedArray && other) noexcept:
                _size { std::exchange(other._size, 0) },
                _capacity { std::exchange(other._capacity, 0) },
                _data { std::move(other._data) }
        {
        }

        SortedArray & operator=(SortedArray && other) noexcept
        {
            _size = std::exchange(other._size, 0);
            _capacity = std::exchange(other._capacity, 0);
            _data = std::move(other._data);

            return *this;
        }

        bool push(const value_type item)
        {
            if (_size > 0 && item > _data[_size - 1])
            {
                if (_size == _capacity)
                    return false;
                _data[_size++] = item;
                return true;
            }

            const size_type idxInsert = findInsertIndex(item);
            if (_capacity == idxInsert || item == _data[idxInsert]) {
                return false;
            }

            _size = (_capacity == _size) ? _size : _size + 1;
            for (size_type i = _size - 1; i > idxInsert; --i) /** TODO: Prefetch **/
                _data[i] = _data[i - 1];
            _data[idxInsert] = item;
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

        [[nodiscard]]
        size_type findInsertIndex(const value_type item) const noexcept
        {
            size_type left = 0, right = _size;
            while (left < right)
            {
                const size_type mid = (left + right) >> 1;
                if (compare(item, _data[mid]))
                    right = mid;
                else
                    left = mid + 1;
            }
            return left;
        }

        static constexpr bool compare(const value_type& a, const value_type& b) noexcept __attribute__((always_inline))
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
        // std::cout << array.Size() << std::endl;
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

        if (map.size() != array.size()) {
            std::cerr << "ERROR: Size mismatch" << std::endl;
            return;
        }

        auto iter = map.begin();
        for (uint32_t idx = 0; idx < collectionSize; ++idx)
        {
            const auto valMap = iter->first;
            const auto valArr = array.data()[idx];

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
    // static_sorted_array::testing::validation();
    static_sorted_array::testing::performance::benchmark();

}