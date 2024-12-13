#pragma once

#include "detail/ThreadCachedReaders.hpp"
#include "folly/Function.h"
#include "folly/MPMCQueue.h"
#include "folly/synchronization/detail/ThreadCachedReaders.h"
#include <array>
#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
#include <deque>
#include <source_location>
#include <numa.h>
#include <numaif.h>
#include <iostream>
namespace wbrcu
{

inline constexpr uint64_t hardware_concurrency = WBRCU_HARDWARE_CONCURRENCY;
static int get_current_numa_node() {
    int cpu = sched_getcpu();
    assert(cpu >= 0);
    int node = numa_node_of_cpu(cpu);
    return node < 0 ? 0 : node;
}

static int max_numa_nodes = [](){
    if (numa_available() == -1) {
        return 1; 
    }
    return numa_max_node() + 1; 
}();
// Generate compile-time random number with seeding from source location
consteval uint64_t rand(std::source_location const& loc = std::source_location::current()) {
    // Combine line, column and file_name hash
    uint64_t seed = loc.line();
    seed ^= loc.column() << 16;
    
    // Simple string hash for file name
    const char* str = loc.file_name();
    uint64_t hash = 0;
    for (const char* p = str; *p; ++p) {
        hash = hash * 31 + static_cast<unsigned char>(*p);
    }

    return seed ^ hash;
}

template <typename T, uint64_t TagId>
struct ThreadLocalTag
{
};

template <typename T>
struct NumaAllocator {
    int node;
    using value_type = T;
    NumaAllocator() : node(0) {}
    NumaAllocator(int numa_node) : node(numa_node) {}

    T* allocate(std::size_t n) {
        return static_cast<T*>(numa_alloc_onnode(n * sizeof(T), node));
    }

    void deallocate(T* p, std::size_t n) {
        numa_free(p, n * sizeof(T));
    }
    template <typename U>
    bool operator==(const NumaAllocator<U>& other) const noexcept {
        return node == other.node;
    }
    template <typename U>
    bool operator!=(const NumaAllocator<U>& other) const noexcept {
        return !(*this == other);
    }
};


template <typename T, uint64_t TagId = 0, uint64_t flushingThreshold = 20>
class rcu_protected
{
    using Tag = ThreadLocalTag<T, TagId>;
public:
    
    // explicit rcu_protected(T* ptr) : m_ptr{ptr} {}

    // ~rcu_protected()
    // {
    //     delete m_ptr.load();
    //     for (auto p : m_retireLists[0]) { delete p; }
    //     for (auto p : m_retireLists[1]) { delete p; }
    //     for (auto p : m_finished) { delete p; }
    // }
    explicit rcu_protected(T* ptr) : m_ptr{ptr} {
        is_numa_available = numa_available() >= 0;
        max_node = is_numa_available ? numa_max_node() : 0; 
        m_node_finished.resize(max_node + 1);
        m_node_retireLists.resize(max_node + 1);
        std::vector<NumaAllocator<T*>> allocators;

        for (int i = 0; i <= max_node; ++i) {
            allocators.emplace_back(i);
        }
        for (int i = 0; i <= max_node; ++i) {
            if (is_numa_available) {
                auto& allocator = allocators[i];
                T* numa_allocated_ptr = static_cast<T*>(numa_alloc_onnode(sizeof(T), i));  
                *numa_allocated_ptr = *ptr;
                m_node_ptrs.emplace_back(numa_allocated_ptr); 
                m_node_finished[i] = std::vector<T*, NumaAllocator<T*>>(allocator);
                m_node_retireLists[i] = {
                    std::vector<T*, NumaAllocator<T*>>(allocator),
                    std::vector<T*, NumaAllocator<T*>>(allocator)};
            }
        }
        //delete ptr;
    }

    ~rcu_protected() {
        if (is_numa_available) {
            for (int i = 0; i <= max_node; ++i) {
                if (T* ptr = m_node_ptrs[i].load()) {
                    numa_free(ptr, sizeof(T));
                }
                for (auto& retire_list : m_node_retireLists[i]) {
                    for (auto p : retire_list) {
                        numa_free(p, sizeof(T));
                    }
                }
                for (auto p : m_node_finished[i]) {
                    numa_free(p, sizeof(T));
                }
            }
        }
        //global
        delete m_ptr.load();
        for (auto p : m_retireLists[0]) { delete p; }
        for (auto p : m_retireLists[1]) { delete p; }
        for (auto p : m_finished) { delete p; }
    }

    // Returns a protected pointer to T that will automatically unlock when
    // destroyed. Nested read lock is not handled, user is responsible to
    // release previous ptr before calling get_ptr again.
    auto
    get_ptr() noexcept
    {
        int node = get_current_numa_node();
        rcu_read_lock();
        auto deleter = [&](T const*) { rcu_read_unlock(); };
        if(is_numa_available){
            return std::unique_ptr<T const, decltype(deleter)>(
                m_node_ptrs[node].load(std::memory_order_acquire), deleter
            );
        }
        else return std::unique_ptr<T const, decltype(deleter)>(
            m_ptr.load(std::memory_order_acquire), deleter
        );
    }

    template <std::invocable<T*> UpdateFunc>
    void
    update(UpdateFunc&& updateCallback)
    {
        if (T* copied = try_register(); copied)
        {
            // Registered as the updater.
            // First perform the update which current thread intends.
            std::invoke(std::forward<UpdateFunc>(updateCallback), copied);

            // Then perform the updates in m_updateQueue.
            do_updates(copied);
        }
        else
        {
            // Other updater is working, enqueue the update to perform.
            m_updateQueue.blockingWrite(std::forward<UpdateFunc>(updateCallback));
        }
    }

private:
    bool is_numa_available;
    int max_node; 
    // Pointer to current object that we returns to readers.
    std::atomic<T*> m_ptr;
    // Lists of objects that are not accessible by new readers and waiting to be
    // reclaimed.
    // m_retireLists[m_epoch] is the list of objects that are protected for
    // current epoch, any readers locking in current epoch will prevent this
    // list of objects to be reclaimed. Similarly, m_retireLists[!m_epoch] is
    // the list of objects that are protected for previous epoch.
    std::array<std::vector<T*>, 2> m_retireLists;
    // List of retired objects ready to be reclaimed. We don't reclaim all of
    // them immediately, instead, we use it as the object pool of T to reuse the
    // allocated memory.
    std::vector<T*> m_finished;

    std::deque<std::atomic<T*>> m_node_ptrs; 
    std::vector<std::vector<T*, NumaAllocator<T*>>> m_node_finished; 
    std::vector<std::array<std::vector<T*, NumaAllocator<T*>>, 2>> m_node_retireLists; 

    // Current epoch. Previous epoch is !m_epoch.
    folly::relaxed_atomic<bool> m_epoch{0};
    // Counters for readers, each thread has a thread_local counter, it avoids
    // reader contention that std::shared_mutex has.
    detail::ThreadCachedReaders<Tag> m_counters;

    // Count of updates to do for updater, every call to update will increment
    // it. If it is greater than 0, then there is an updater in work, the call
    // to update will enqueue the update-to-do. This atomic variable effectively
    // prevents data race on m_retireLists and m_finished.
    std::atomic<uint64_t> m_updateCnt{0};
    // Queue of updates to perform.
    folly::MPMCQueue<folly::Function<void(T*)>> m_updateQueue{
        500 * hardware_concurrency
    };

    void
    rcu_read_lock() noexcept
    {
        m_counters.increment(m_epoch);
    }

    void
    rcu_read_unlock() noexcept
    {
        m_counters.decrement();
    }

    T*
    get_copy()
    {
        T* copied;
        if(is_numa_available){
            int node = get_current_numa_node();
            T& curr = *m_node_ptrs[node].load(std::memory_order_relaxed);
            if (m_node_finished[node].empty()) { 
                copied = static_cast<T*>(numa_alloc_onnode(sizeof(T), node)); 
            }
            else
            {
                // Reuse memory from the object pool and perform copy
                // assignment.
                copied = m_node_finished[node].back();
                m_node_finished[node].pop_back();
                *copied = curr;
            }
        }else{
            T& curr = *m_ptr.load(std::memory_order_relaxed);
            if (m_finished.empty()) { copied = new T(curr); }
            else
            {
                // Reuse memory from the object pool and perform copy
                // assignment.
                copied = m_finished.back();
                m_finished.pop_back();
                *copied = curr;
            }
        }
        return copied;
    }

    // Returns a pointer to the copied object if current thread successfully
    // register as the updater, otherwise returns nullptr.
    T*
    try_register()
    {
        if (!m_updateCnt.fetch_add(1, std::memory_order_relaxed))
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            return get_copy();
        }
        return nullptr;
    }

    // Perform updates in the update queue, publish updates and push the old
    // object to the retire list.
    void
    do_updates(T* copied)
    {
        folly::Function<void(T*)> updateToDo;
        uint64_t                  done = 1;
        auto updateCnt = m_updateCnt.load(std::memory_order_relaxed);
        while (true)
        {
            uint64_t unflushed = 0;
            do {
                while (done < updateCnt)
                {
                    m_updateQueue.blockingRead(updateToDo);
                    updateToDo(copied);
                    ++done;
                    if (++unflushed == flushingThreshold) {
                        break;
                    }
                }

                if (unflushed == flushingThreshold) {
                    break;
                }
                updateCnt = m_updateCnt.load(std::memory_order_relaxed);
            } while (done != updateCnt);

            // Publish updates to readers.
            // auto old_ptr = m_ptr.exchange(copied, std::memory_order_release);
            // retire(old_ptr);
            if(is_numa_available){
                for (int i = 0; i <= max_node; ++i) {
                    auto old_ptr = m_node_ptrs[i].exchange(copied, std::memory_order_release);
                    retire(old_ptr, i);
                }
            }else{
                auto old_ptr = m_ptr.exchange(copied, std::memory_order_release);
                retire(old_ptr,0);
            }

            // Check if there is new updates enqueued after we retire the old
            // pointer
            if (done == updateCnt && m_updateCnt.compare_exchange_strong(
                    updateCnt,
                    0,
                    std::memory_order_release,
                    std::memory_order_relaxed
                ))
            {
                return; // Finished updating
            }

            copied = get_copy();
        }
    }

    // void
    // retire(T* ptr)
    // {
    //     constexpr static uint64_t cleanupThreshold = hardware_concurrency;

    //     bool curr = m_epoch, prev = !curr;
    //     m_retireLists[curr].push_back(ptr);

    //     if (m_retireLists[curr].size() < cleanupThreshold
    //         || !m_counters.epochIsClear(prev))
    //     {
    //         return;
    //     }

    //     // All readers locking previous epoch have finished, it is now safe to
    //     // reclaim any object in m_retireLists[prev] and increment current
    //     // epoch.
    //     std::swap(m_finished, m_retireLists[prev]);
    //     for (auto p : m_retireLists[prev]) { delete p; }
    //     m_retireLists[prev].clear();
    //     m_epoch.store(prev);
    // }
    void retire(T* ptr, int node) {
        //constexpr static const static uint64_t cleanupThreshold = hardware_concurrency/(max_node+1);
        static const uint64_t cleanupThreshold = hardware_concurrency/(max_node+1);
        bool curr = m_epoch, prev = !curr;
        if(is_numa_available){
            //auto& retireLists = m_node_retireLists[node];
            m_node_retireLists[node][curr].push_back(ptr);
            if (m_node_retireLists[node][curr].size() < cleanupThreshold 
                || !m_counters.epochIsClear(prev)) {
                return;
            }
            std::swap(m_node_finished[node], m_node_retireLists[node][prev]);            
            for (auto p : m_node_retireLists[node][prev]) {
                numa_free(p, sizeof(T)); 
            }
            m_node_retireLists[node][prev].clear();
            m_epoch.store(prev);
        }else{
            m_retireLists[curr].push_back(ptr);
            if (m_retireLists[curr].size() < cleanupThreshold
                || !m_counters.epochIsClear(prev))
            {
                return;
            }
            // All readers locking previous epoch have finished, it is now safe to
            // reclaim any object in m_retireLists[prev] and increment current
            // epoch.
            std::swap(m_finished, m_retireLists[prev]);
            for (auto p : m_retireLists[prev]) { delete p; }
            m_retireLists[prev].clear();
            m_epoch.store(prev);
        }
    }
};

} // namespace wbrcu
