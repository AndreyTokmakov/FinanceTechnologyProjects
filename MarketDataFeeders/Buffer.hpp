/**============================================================================
Name        : Buffer.hpp
Created on  : 26.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Buffer.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BUFFER_HPP
#define FINANCETECHNOLOGYPROJECTS_BUFFER_HPP

#include <cstdint>
#include <memory>

namespace buffer
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

        explicit Buffer(size_type initialCapacity);

        Buffer(const Buffer& other);
        Buffer& operator=(const Buffer& other);

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        void push_back(value_type c);

        void append(const_pointer str, size_type len);

        [[nodiscard]]
        pointer head() const noexcept;

        [[nodiscard]]
        pointer tail() const noexcept;

        [[nodiscard]]
        pointer tail(size_type n) noexcept;

        [[nodiscard]]
        size_type length() const noexcept;

        [[nodiscard]]
        bool empty() const noexcept;

        void clear() noexcept ;

        void reset() noexcept ;

        reference operator[](size_type idx) ;

        const_reference operator[](size_type idx) const;

        void incrementLength(size_type incrSize) noexcept;

        void validateCapacity(size_type n);

    private:

        static constexpr uint32_t round_up_to_pow2(uint32_t value);

        void ensure_capacity(size_type newCapacity);

        size_type capacity { 0 };
        size_type size { 0 };
        std::unique_ptr<array_type> data;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BUFFER_HPP