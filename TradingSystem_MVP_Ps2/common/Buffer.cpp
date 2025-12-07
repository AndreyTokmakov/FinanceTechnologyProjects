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
#include <ostream>
#include <utility>

namespace buffer
{
    using pointer = Buffer::pointer;
    using size_type = Buffer::size_type;
    using reference = Buffer::reference;
    using const_reference = Buffer::const_reference;

    Buffer::Buffer(const size_type initialCapacity):
        capacity { initialCapacity },
        size { 0 },
        data { std::make_unique_for_overwrite<array_type>(initialCapacity) }  {
    }

    Buffer::Buffer(const Buffer& other):
        capacity { other.capacity },
        size { other.size },
        data { std::make_unique_for_overwrite<array_type>(other.capacity) }
    {
        std::memcpy(data.get(), other.data.get(), other.size);
    }

    Buffer& Buffer::operator=(const Buffer& other)
    {
        capacity = other.capacity;
        size = other.size;
        data = std::make_unique_for_overwrite<array_type>(capacity);
        std::memcpy(data.get(), other.data.get(), size);
        return *this;
    }

    Buffer::Buffer(Buffer&& other) noexcept:
        capacity { std::exchange(other.capacity, 0) },
        size { std::exchange(other.size, 0) },
        data { std::move(other.data) }
    {
        other.size = 0;
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        capacity = std::exchange(other.capacity, 0);
        size = std::exchange(other.size, 0);
        data = std::move(other.data);
        return *this;
    }

    void Buffer::push_back(const value_type c)
    {
        ensure_capacity(size + 1);
        data[size++] = c;
    }

    void Buffer::append(const_pointer str, const size_type len)
    {
        ensure_capacity(size + len);
        std::memcpy(data.get() + size, str, len);
        size += len;
    }

    [[nodiscard]]
    pointer Buffer::head() const noexcept {
        return data.get();
    }

    [[nodiscard]]
    pointer Buffer::tail() const noexcept {
        return data.get() + size;
    }

    [[nodiscard]]
    pointer Buffer::tail(const size_type n) noexcept
    {
        validateCapacity(n);
        return data.get() + size;
    }

    [[nodiscard]]
    size_type Buffer::length() const noexcept {
        return size;
    }

    [[nodiscard]]
    bool Buffer::empty() const noexcept {
        return 0 == size;
    }

    void Buffer::clear() noexcept {
        size = 0;
    }

    void Buffer::reset() noexcept {
        size = 0;
    }

    reference Buffer::operator[](const size_type idx) {
        return data[idx];
    }

    const_reference Buffer::operator[](const size_type idx) const {
        return data[idx];
    }

    void Buffer::incrementLength(const size_type incrSize) noexcept {
        size += incrSize;
    }

    void Buffer::validateCapacity(const size_type n)
    {
        if (capacity - size >= n)
            return;
        ensure_capacity(capacity + n);
    }

    constexpr uint32_t Buffer::round_up_to_pow2(const uint32_t value) {
        return 1u << (32 - std::countl_zero(value - 1));
    }

    void Buffer::ensure_capacity(size_type newCapacity)
    {
        newCapacity = round_up_to_pow2(newCapacity);
        auto new_data = std::make_unique_for_overwrite<array_type>(newCapacity);
        if (data)
            std::memcpy(new_data.get(), data.get(), size);
        data = std::move(new_data);
        capacity = newCapacity;
        // std::cout << "Buffer capacity increased to " << capacity << std::endl; /** DEBUG: **/
    }
}
