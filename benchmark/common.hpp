#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <concepts>
#include <memory>
#include "folly/synchronization/Rcu.h"

template <typename T>
class lock_protected
{
public:
    lock_protected(T* ptr) : ptr_{ptr} {}

    auto get_ptr() noexcept {
        mut_.lock();
        auto deleter = [&](T const*) { mut_.unlock(); };
        return std::unique_ptr<T const, decltype(deleter)>(
            ptr_.get(), deleter
        );
    }

    template <std::invocable<T*> UpdateFunc>
    void update(UpdateFunc&& updateCallback) {
        std::scoped_lock lg{mut_};
        std::invoke(std::forward<UpdateFunc>(updateCallback), ptr_.get());
    }
private:
    std::mutex mut_;
    std::unique_ptr<T> ptr_;
};

template <typename T>
class rwlock_protected
{
public:
    rwlock_protected(T* ptr) : ptr_{ptr} {}

    auto get_ptr() noexcept {
        mut_.lock_shared();
        auto deleter = [&](T const*) { mut_.unlock_shared(); };
        return std::unique_ptr<T const, decltype(deleter)>(
            ptr_.get(), deleter
        );
    }

    template <std::invocable<T*> UpdateFunc>
    void update(UpdateFunc&& updateCallback) {
        std::scoped_lock lg{mut_};
        std::invoke(std::forward<UpdateFunc>(updateCallback), ptr_.get());
    }
private:
    std::shared_mutex mut_;
    std::unique_ptr<T> ptr_;
};

template <typename T>
class follyrcu_protected
{
public:
    follyrcu_protected(T* ptr) : ptr_{ptr} {}

    ~follyrcu_protected() {
        folly::rcu_retire(ptr_.load());
    }

    auto get_ptr() noexcept {
        folly::rcu_default_domain().lock();
        auto deleter = [&](T const*) { folly::rcu_default_domain().unlock(); };
        return std::unique_ptr<T const, decltype(deleter)>(
            ptr_.load(std::memory_order_acquire), deleter
        );
    }

    template <std::invocable<T*> UpdateFunc>
    void update(UpdateFunc&& updateCallback) {
        mut_.lock();
        auto newPtr = new T{*ptr_.load(std::memory_order_relaxed)};
        std::invoke(std::forward<UpdateFunc>(updateCallback), newPtr);
        auto oldPtr = ptr_.exchange(newPtr, std::memory_order_release);
        mut_.unlock();
        folly::rcu_retire(oldPtr);
    }
private:
    std::mutex mut_;
    std::atomic<T*> ptr_;
};