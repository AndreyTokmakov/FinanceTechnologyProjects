/**============================================================================
Name        : Buffer.cpp
Created on  : 26.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Buffer.cpp
============================================================================**/

#include "Buffer.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

namespace
{
    struct Buffer
    {
        using value_type      = char;
        using size_type       = uint32_t;
        using array_type      = value_type[];
        using pointer         = value_type*;
        using const_pointer   = const pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

        Buffer() = default;

        explicit Buffer(const size_type initialSize)
            : capacity { initialSize }, size { 0 }, data { std::make_unique_for_overwrite<array_type>(initialSize) }  {
        }

        Buffer(const Buffer& other):
            capacity { other.capacity },
            size { other.size },
            data { std::make_unique_for_overwrite<array_type>(other.capacity) }
        {
            std::memcpy(data.get(), other.data.get(), other.size);
        }

        Buffer& operator=(const Buffer& other)
        {
            capacity = other.capacity;
            size = other.size;
            data = std::make_unique_for_overwrite<array_type>(capacity);
            std::memcpy(data.get(), other.data.get(), size);
            return *this;
        }

        Buffer(Buffer&& other) noexcept:
            capacity { std::exchange(other.capacity, 0) },
            size { std::exchange(other.size, 0) },
            data { std::move(other.data) }
        {
            other.size = 0;
        }

        Buffer& operator=(Buffer&& other) noexcept
        {
            capacity = std::exchange(other.capacity, 0);
            size = std::exchange(other.size, 0);
            data = std::move(other.data);
            return *this;
        }

        void push_back(const value_type c)
        {
            ensure_capacity(size + 1);
            data[size++] = c;
        }

        void append(const_pointer str, const size_type len)
        {
            ensure_capacity(size + len);
            std::memcpy(data.get() + size, str, len);
            size += len;
        }

        [[nodiscard]]
        const_pointer Data() const noexcept {
            return data.get();
        }

        [[nodiscard]]
        size_type Size() const noexcept {
            return size;
        }

        [[nodiscard]] size_type Capacity() const noexcept {
            return capacity;
        }

        void clear() noexcept {
            size = 0;
        }

    private:

        void ensure_capacity(size_type new_size)
        {
            if (new_size <= capacity)
                return;
            size_type new_capacity = capacity ? capacity * 2 : 8;
            while (new_capacity < new_size) new_capacity *= 2;


            auto new_data = std::make_unique_for_overwrite<array_type>(new_capacity);
            if (data)
                std::memcpy(new_data.get(), data.get(), size);
            data = std::move(new_data);
            capacity = new_capacity;
        }

        size_type capacity { 0 };
        size_type size { 0 };
        std::unique_ptr<array_type> data;
    };
}


void buffer::TestAll()
{

}
