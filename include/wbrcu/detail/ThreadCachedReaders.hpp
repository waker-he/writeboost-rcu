#pragma once

#include "folly/ThreadLocal.h"
#include "folly/synchronization/RelaxedAtomic.h"

namespace wbrcu::detail
{

using EpochReading = folly::relaxed_atomic<uint8_t>;

// A data structure that keeps a per-thread cache of a bitfield that contains
// the current active epoch for reader in the thread, and whether the reader
// is in the read-side critical section.
//
//                _______________________________________
//                |  Irrelevant |   Epoch   |  Reading  |
// EpochReading:  | 7 6 5 4 3 2 |     1     |     0     |
//                o-------------|-----------|-----------o
//
// Note that this does not handle nested read-side critical section.
template <class ThreadLocalTag>
class ThreadCachedReaders
{
public:
    void
    increment(uint8_t epoch)
    {
        epochReading->store((epoch << 1) + 1);
    }

    void
    decrement()
    {
        epochReading->store(0);
    }

    bool
    epochIsClear(uint8_t epoch)
    {
        auto access = epochReading.accessAllThreads();
        return !std::any_of(
            access.begin(),
            access.end(),
            [reading = static_cast<uint8_t>((epoch << 1) + 1)](EpochReading& i)
            { return i == reading; }
        );
    }

private:
    folly::ThreadLocal<EpochReading, ThreadLocalTag> epochReading;
};

} // namespace wbrcu::detail