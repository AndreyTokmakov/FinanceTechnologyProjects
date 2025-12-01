/**============================================================================
Name        : FlatMap.hpp
Created on  : 30.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : FlatMap.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_FLATMAP_HPP
#define FINANCETECHNOLOGYPROJECTS_FLATMAP_HPP

#include <utility>
#include <memory>

namespace flat_map
{
    enum class SortOrder
    {
        Ascending,
        Descending
    };

    // TODO: Concepts on <K>
    //  - Comparable
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

        node_pointer push(const key_type& key, const value_type& value)
        {
            if (_size > 0 && !compare(key, _data[_size - 1].key))
            {
                if (_size == _capacity)
                    return nullptr;
                _data[_size++] = Node {key, value};
                return &(_data[_size]);
            }

            const size_type idxInsert = findInsertIndex(key);
            if (_capacity == idxInsert || key == _data[idxInsert].key) {
                return &(_data[idxInsert]);
            }

            _size = (_capacity == _size) ? _size : _size + 1;
            // __builtin_prefetch(elements.get() + size - 32, 0, 2);
            for (size_type i = _size - 1; i > idxInsert; --i) { /** TODO: Prefetch **/
                _data[i] = _data[i - 1];
            }
            _data[idxInsert] = Node {key, value};
            return &(_data[idxInsert]);
        }

        bool erase(const key_type& key)
        {
            if (_size > 0 && (!compare(_data[0].key, key) || !compare(key, _data[_size - 1].key)))
            {
                return false;
            }

            const size_type idx = findInsertIndex(key);
            if (key != _data[idx].key) {
                return false;
            }

            for (size_type n = idx + 1; n < _size; ++n) {
                _data[n - 1] = _data[n];
            }
            --_size;
            return true;
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return _size;
        }

        [[nodiscard]]
        size_type empty() const noexcept {
            return 0 == _size;
        }

        [[nodiscard]]
        node_pointer data() const {
            return _data.get();
        }

        [[nodiscard]]
        node_pointer front() const noexcept {
            return &(_data[0]);
        }

        [[nodiscard]]
        node_pointer back() const noexcept {
            return &(_data[_size - 1]);
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

#endif //FINANCETECHNOLOGYPROJECTS_FLATMAP_HPP