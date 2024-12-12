/**============================================================================
Name        : LockFreeQueue.h
Created on  : 12.12.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : LockFreeQueue.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_LOCKFREEQUEUE_H
#define FINANCETECHNOLOGYPROJECTS_LOCKFREEQUEUE_H

#include <iostream>
#include <vector>
#include <atomic>


namespace LockFreeQueue
{
    template<typename T>
    struct LFQueue final
    {
        using object_type = T;
        using size_type = size_t;

        explicit LFQueue(size_type num_elems) :
                storage(num_elems, object_type()) /* pre-allocation of vector storage. */ {
        }

        size_type getNextToWriteTo() noexcept
        {
            return &storage[idxWriteNext.load(std::memory_order_relaxed)];
        }

        void updateWriteIndex() noexcept
        {
            idxWriteNext = (idxWriteNext + 1) % storage.size();
            numElements.fetch_add(1, std::memory_order_relaxed);
        }

        const object_type* getNextToRead() const noexcept  {
            return (size() ? &storage[idxWriteRead] : nullptr);
        }

        void updateReadIndex() noexcept
        {
            idxWriteRead = (idxWriteRead + 1) % storage.size();
            numElements.fetch_sub(1, std::memory_order_relaxed);
        }

        [[nodiscard]]
        size_type size() const noexcept
        {
            return numElements.load(std::memory_order_relaxed);
        }

        LFQueue() = delete;

        LFQueue(const LFQueue &) = delete;
        LFQueue(const LFQueue &&) = delete;

        LFQueue &operator=(const LFQueue &) = delete;
        LFQueue &operator=(const LFQueue &&) = delete;

    private:

        std::vector<object_type> storage;
        std::atomic<size_type> idxWriteNext { 0 };
        std::atomic<size_type> idxWriteRead { 0 };
        std::atomic<size_type> numElements { 0 };
    };
};

#endif //FINANCETECHNOLOGYPROJECTS_LOCKFREEQUEUE_H
