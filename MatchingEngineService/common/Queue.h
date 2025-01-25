/**============================================================================
Name        : Queue.h
Created on  : 24.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Queue.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_QUEUE_H
#define FINANCETECHNOLOGYPROJECTS_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

namespace Common
{
    template<typename T>
    class Queue
    {
        using value_type = T;
        static_assert(!std::is_same_v<value_type, void>, "ERROR: Value type can not be void");

        static constexpr std::chrono::duration<int64_t, std::ratio<1, 1000>> waitTimeout { std::chrono::seconds(5U) };

        mutable std::mutex mutex;
        std::deque<value_type> data_queue;
        std::condition_variable updated;

    public:

        void push(value_type&& new_value)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                data_queue.push_back(std::move(new_value));
            }
            updated.notify_one();
        }

        bool pop(value_type& value)
        {
            std::unique_lock<std::mutex> lock(mutex);
            bool ok = updated.wait_for(lock, waitTimeout, [this] {
                return false == data_queue.empty();
            });
            if (!ok)
                return false;
            value = std::move(data_queue.front());
            data_queue.pop_front();
            return true;
        }

        [[nodiscard]]
        bool empty() const noexcept
        {
            std::lock_guard<std::mutex> lock(mutex);
            return data_queue.empty();
        }

        [[nodiscard]]
        size_t size() const noexcept
        {
            std::lock_guard<std::mutex> lock(mutex);
            return data_queue.size();
        }
    };
};

#endif //FINANCETECHNOLOGYPROJECTS_QUEUE_H
