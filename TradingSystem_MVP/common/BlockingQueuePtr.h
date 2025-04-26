/**============================================================================
Name        : BlockingQueuePtr.h
Created on  : 24.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BlockingQueue.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BLOCKINGQUEUE_PTR_H
#define FINANCETECHNOLOGYPROJECTS_BLOCKINGQUEUE_PTR_H

#include <deque>
#include <mutex>
#include <condition_variable>

namespace common
{
    template<typename T>
    class BlockingQueuePtr
    {
        using value_type = T;
        using pointer = T*;


        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");

        static constexpr std::chrono::duration<int64_t, std::ratio<1, 1000>> waitTimeout { std::chrono::seconds(5U) };

        mutable std::mutex mutex;
        std::deque<pointer> queue;
        std::condition_variable updated;

    public:

        struct Wrapper {
            pointer ptr {nullptr};
        };

        void push(pointer new_value)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                queue.push_back(new_value);
            }
            updated.notify_one();
        }

        bool pop(Wrapper& data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            bool ok = updated.wait_for(lock, waitTimeout, [this] {
                return false == queue.empty();
            });
            if (!ok)
                return false;
            data.ptr = queue.front();
            queue.pop_front();
            return true;
        }

        [[nodiscard]]
        bool empty() const noexcept
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.empty();
        }

        [[nodiscard]]
        size_t size() const noexcept
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size();
        }
    };
};

#endif //FINANCETECHNOLOGYPROJECTS_BLOCKINGQUEUE_PTR_H
