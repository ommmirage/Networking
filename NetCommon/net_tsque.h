#pragma once
#include "net_common.h"

// Thread-safe queue
template<typename T>
class tsqueue
{
public:
    tsqueue() = default;
	tsqueue(const tsqueue<T>&) = delete;
	virtual ~tsqueue() { clear(); }

public:
    T& front()
    {
        std::scoped_lock lock(muxQueue);
        return deq.front();
    }

    T& back()
    {
        std::scoped_lock lock(muxQueue);
        return deq.back();
    }

    void push_front(const T item)
    {
        std::scoped_lock lock(muxQueue);
        deq.push_front(item);
    }

    void push_back(const T item)
    {
        std::scoped_lock lock(muxQueue);
        deq.push_back(item);

        std::unique_lock<std::mutex> ul(mutexBlocking);
        cvBlocking.notify_one();
    }

    bool empty()
    {
        std::scoped_lock lock(muxQueue);
        return deq.empty();
    }

    size_t count()
    {
        std::scoped_lock lock(muxQueue);
        return deq.size();
    }

    void clear()
    {
        std::scoped_lock lock(muxQueue);
        deq.clear();
    }

    // Removes and returns item from front of Queue
    T pop_front()
    {
        std::scoped_lock lock(muxQueue);
        auto t = deq.front();
        deq.pop_front();
        return t;
    }

    T pop_back()
    {
        std::scoped_lock lock(muxQueue);
        auto t = deq.back();
        deq.pop_back();
        return t;
    }

    void wait()
    {
        while (empty())
        {
            std::unique_lock<std::mutex> ul(mutexBlocking);
            // Sends the thread to sleep until smth signals the condition_variable
            // to wake up
            cvBlocking.wait(ul);
        }
    }

    protected:
        std::mutex muxQueue;
        std::deque<T> deq;

        std::condition_variable cvBlocking;
        std::mutex mutexBlocking;
};