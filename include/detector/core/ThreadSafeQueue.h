#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>

namespace detector {

// A bounded, thread-safe blocking queue used to connect the pipeline stages.
//
// Producers block on push() when the queue is full; consumers block on
// waitPop() when it is empty. Blocking is implemented with two condition
// variables (instead of sleep-polling), so neither side burns CPU while idle.
//
// The queue supports a cooperative shutdown protocol: close() wakes every
// waiter, after which push() refuses new items and waitPop() drains whatever is
// left and then returns false. This lets each stage propagate end-of-stream to
// the next one and lets the workers exit their loops cleanly.
template <typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue(std::size_t capacity = 16)
        : capacity_(capacity == 0 ? 1 : capacity) {}

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Block until there is room, then enqueue `item`.
    // Returns false if the queue was closed (the item is not enqueued).
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] { return closed_ || queue_.size() < capacity_; });
        if (closed_) {
            return false;
        }
        queue_.push_back(std::move(item));
        lock.unlock();
        notEmpty_.notify_one();
        return true;
    }

    // Enqueue `item` without ever blocking. If the queue is full the oldest
    // item is discarded to make room. This favours freshness over completeness
    // and is the right policy for a live video stream, where stale frames are
    // worthless and unbounded buffering would only add latency.
    // Returns false if the queue was closed.
    bool pushDropOldest(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (closed_) {
            return false;
        }
        if (queue_.size() >= capacity_) {
            queue_.pop_front();
        }
        queue_.push_back(std::move(item));
        lock.unlock();
        notEmpty_.notify_one();
        return true;
    }

    // Block until an item is available, then move it into `out`.
    // Returns false when the queue is closed AND drained (nothing left to pop).
    bool waitPop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this] { return closed_ || !queue_.empty(); });
        if (queue_.empty()) {
            return false; // closed and drained
        }
        out = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        notFull_.notify_one();
        return true;
    }

    // Mark the queue as closed and wake every waiter. Idempotent.
    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    bool closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::deque<T> queue_;
    const std::size_t capacity_;
    bool closed_ = false;
};

} // namespace detector
