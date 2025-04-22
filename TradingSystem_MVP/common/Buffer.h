/**============================================================================
Name        : Buffer.h
Created on  : 12.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Buffer.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BUFFER_H
#define FINANCETECHNOLOGYPROJECTS_BUFFER_H

#include <vector>
#include <cstdint>

namespace Common
{
    class Buffer
    {
        using size_type = uint32_t;
        using data_type = uint8_t;
        using pointer = data_type*;

        std::vector<data_type> buffer {};
        size_type length { 0 };

    public:

        explicit Buffer(const size_type size = 1024): buffer(size) {
        }

        void validateCapacity(const size_type size)
        {
            if (buffer.size() >= length + size)
                return;
            buffer.resize(length + size);
        }

        template<typename T = uint8_t>
        T* data() noexcept {
            return reinterpret_cast<T*>(buffer.data());
        }

        template<typename T = uint8_t>
        T* head() noexcept {
            return reinterpret_cast<T*>(buffer.data()) + length;
        }

        size_type size() const noexcept {
            return length;
        }

        void reset() noexcept {
            length = 0;
        }

        void incrementLength(const size_type size) noexcept {
            length += size;
        }

        void swap(Buffer &buff) noexcept
        {
            std::swap(buff.buffer, buffer);
            std::swap(buff.length, length);
        }

        static void swap(Buffer &a, Buffer &b) noexcept
        {
            a.swap(b);
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BUFFER_H
