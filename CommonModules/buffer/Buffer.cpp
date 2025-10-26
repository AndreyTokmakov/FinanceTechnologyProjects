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
#include <iomanip>
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

        explicit Buffer(const size_type initialCapacity):
            capacity { initialCapacity },
            size { 0 },
            data { std::make_unique_for_overwrite<array_type>(initialCapacity) }  {
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

        // TODO: Rename
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
        pointer head() const noexcept {
            return data.get();
        }

        [[nodiscard]]
        pointer tail() const noexcept {
            return data.get() + size;
        }

        [[nodiscard]]
        pointer tail(const size_type n) noexcept
        {
            validateCapacity(n);
            return data.get() + size;
        }

        [[nodiscard]]
        size_type length() const noexcept {
            return size;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return 0 == size;
        }

        void clear() noexcept {
            size = 0;
        }

        void reset() noexcept {
            size = 0;
        }

        reference operator[](const size_type idx) {
            return data[idx];
        }

        const_reference operator[](const size_type idx) const {
            return data[idx];
        }

        void incrementLength(const size_type incrSize) noexcept {
            size += incrSize;
        }

        void validateCapacity(const size_type n)
        {
            if ((capacity - size) >= n)
                return;
            ensure_capacity(capacity + n);
        }

    private:

        static constexpr uint32_t round_up_to_pow2(const uint32_t value) {
            return 1u << (32 - std::countl_zero(value - 1));
        }

        void ensure_capacity(size_type newCapacity)
        {
            newCapacity = round_up_to_pow2(newCapacity);
            auto new_data = std::make_unique_for_overwrite<array_type>(newCapacity);
            if (data)
                std::memcpy(new_data.get(), data.get(), size);
            data = std::move(new_data);
            capacity = newCapacity;
        }

        size_type capacity { 0 };
        size_type size { 0 };
        std::unique_ptr<array_type> data;
    };
}


namespace buffer_tests
{

    void test_validateCapacity()
    {
        Buffer buffer;

        std::cout << buffer.length() << std::endl;
        buffer.incrementLength(1024);
        buffer.validateCapacity(512);

        std::cout << buffer.length() << std::endl;
    }


    void test_data()
    {
        Buffer buffer;

        {
            std::string message{"qwerty"};
            std::copy_n(message.data(), message.size(), buffer.tail(message.size()));
            buffer.incrementLength(message.length());
            std::cout << std::quoted(std::string_view(buffer.head(), buffer.length())) << std::endl;
        }

        {
            std::string message{"1111111111111"};
            std::copy_n(message.data(), message.size(), buffer.tail(message.size()));
            buffer.incrementLength(message.length());
            std::cout << std::quoted(std::string_view(buffer.head(), buffer.length())) << std::endl;
        }
    }

    /*
    void server_like_test()
    {
        Buffer buffer;
        std::string dataExpected;

        for (int i = 'a'; i < 'z'; ++i)
        {
            std::string message(128, (char)i);
            dataExpected += message;

            buffer.validateCapacity(128);
            std::copy_n(message.data(), message.size(), buffer.head());
            buffer.incrementLength(message.length());
        }

        std::string result(buffer.data<char>(), buffer.size());
        std::cout << std::boolalpha << (result == dataExpected) << std::endl;
    }
    */
}


void buffer::TestAll()
{
    // buffer_tests::test_validateCapacity();
    buffer_tests::test_data();
}
