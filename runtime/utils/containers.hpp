#ifndef __ORBIT_CONTAINERS__
#define __ORBIT_CONTAINERS__

#include <utils/types.hpp>

#include <bit>
#include <new>
#include <cassert>
#include <array>

namespace Orbit
{

/* containers */
template<typename T>
class Handle
{
public:
    Handle() : pIndex(0), pGeneration(0) {}
    Handle(u32 index, u32 generation) : pIndex(index), pGeneration(generation) {}

    b32 IsValid() const { return pGeneration != 0; }
    
    b32 operator==(const Handle<T>& other) const { return other.pIndex == pIndex && other.pGeneration == pGeneration; }
    b32 operator!=(const Handle<T>& other) const { return other.pIndex != pIndex && other.pGeneration != pGeneration; }
    
    u64 hashKey() const { return ((u64)pIndex << 32) + (u64)pGeneration; }
    
    u32 idx() const { return pIndex; }
    u32 gen() const { return pGeneration; }
    
private:
    u32 pIndex      = 0;
    u32 pGeneration = 0;
};

template<typename U>
class Pool
{
public:
    struct PoolMeta
    {
        u32 idx;
        u32 gen;
    };
    
    Pool() = default;
    ~Pool();
    
    Pool(const Pool&)            = delete;
    Pool& operator=(const Pool&) = delete;
    
    void     reset();
    PoolMeta emplace(U&& val);
    void     erase(PoolMeta h);
    U*       at(PoolMeta h);
    u32      count() const;
    u32      capacity() const;
    
private:
    static constexpr u32 kSmallSegmentsToSkip = 6;
    static constexpr u32 kNotInFreelist       = UINT32_MAX;
    static constexpr u32 kEndOfList           = kNotInFreelist - 1;
    
    static constexpr u64 int_log_2(u64 x)
    {
        return 63 - static_cast<u64>(std::countl_zero(x));
    }

    static constexpr u32 slots_in_segment(u32 segment_index)
    {
        return (1 << kSmallSegmentsToSkip) << segment_index;
    }
    
    static constexpr u32 capacity_for_segment_count(u32 segment_count)
    {
        return ((1 << kSmallSegmentsToSkip) << segment_count) - (1 << kSmallSegmentsToSkip);
    }
    
    struct Entry
    {
        u32 next;
        u32 gen;

        alignas(std::max_align_t) b8 data[sizeof(U)];
    };
    
    void   add_segment();
    Entry *get(u32 idx);

    u32 pUsedSegments    = 0;
    u32 pHead            = kEndOfList;
    u32 pCount           = 0;
    Entry *pSegments[26] = { nullptr };
    
    U pStub = {};
};

template <typename U>
Pool<U>::~Pool()
{
    reset();
}

template <typename U>
void Pool<U>::reset()
{
    for (u32 segment_idx = 0; segment_idx < pUsedSegments; ++segment_idx)
    {
        u32    segment_size = slots_in_segment(segment_idx);
        Entry* segment      = pSegments[segment_idx];

        for (u32 idx = 0; idx < segment_size; ++idx) 
        {
            if (segment[idx].next == kNotInFreelist) 
            {
                U* data = std::launder(reinterpret_cast<U*>(&segment[idx].data));
                std::destroy_at(data);
            }
        }

        std::free(segment);
        pSegments[segment_idx] = nullptr;
    }
    pUsedSegments = 0;
    pHead  = kEndOfList;
    pCount = 0;
}

template <typename U>
Pool<U>::PoolMeta Pool<U>::emplace(U&& val)
{
    if (pHead == kEndOfList) { add_segment(); }
    
    u32 idx = pHead;
    assert(idx != kNotInFreelist &&
           idx != kEndOfList);
    
    Entry *entry = get(idx);
    assert(entry->next != kNotInFreelist);
    pHead = entry->next;
    entry->next = kNotInFreelist;

    U* data = std::launder(reinterpret_cast<U*>(entry->data));
    std::construct_at(data, std::forward<U>(val));
    
    ++pCount;
    
    return {idx, entry->gen};
}

template <typename U>
void Pool<U>::erase(Pool<U>::PoolMeta h)
{
    Entry *entry = get(h.idx);
    assert(entry->gen == h.gen);

    U* data = std::launder(reinterpret_cast<U*>(entry->data));
    std::destroy_at(data);

    ++entry->gen;
    entry->next = pHead;
    pHead = h.idx;
    
    --pCount;
}

template <typename U>
U* Pool<U>::at(PoolMeta h)
{
    Entry *e = get(h.idx);
    if (!e) return nullptr;
    
    return e->next == kNotInFreelist ? &e->data : nullptr;
}

template <typename U>
void Pool<U>::add_segment()
{
    u64 segment_size = slots_in_segment(pUsedSegments);
    void *blk = std::malloc(sizeof(Entry) * segment_size);
    auto segment = reinterpret_cast<Entry*>(blk);
    pSegments[pUsedSegments]= segment;
    
    u32 segment_offset = capacity_for_segment_count(pUsedSegments);
    for (u64 i = segment_size; i > 0; --i)
    {
        segment[i - 1].gen = 0;
        segment[i - 1].next = pHead;
        pHead = (u32)(i + segment_offset) - 1;
    }
    
    ++pUsedSegments;
}

template <typename U>
Pool<U>::Entry *Pool<U>::get(u32 idx)
{
    u64 segment = int_log_2((idx >> kSmallSegmentsToSkip) + 1);
    u32 slot    = idx - capacity_for_segment_count((u32)segment);
    return &pSegments[segment][slot];
}

template <typename U>
u32 Pool<U>::count() const
{
    return pCount;
}

template <typename U>
u32 Pool<U>::capacity() const
{
    return capacity_for_segment_count(pUsedSegments);
}



template<typename U, u64 capacity>
class StaticPool
{
public:
    struct PoolMeta
    {
        u32 idx;
        u32 gen;
    };

    StaticPool();
    ~StaticPool();
    
    StaticPool(const StaticPool&)            = delete;
    StaticPool& operator=(const StaticPool&) = delete;
    
    void     reset();
    PoolMeta emplace(U&& val);
    void     erase(PoolMeta h);
    U*       at(PoolMeta h);
    u64      count();

private:
    static constexpr u32 kNotInFreelist = UINT32_MAX;
    static constexpr u32 kEndOfList     = kNotInFreelist - 1;

    struct Entry
    {
        u32 next;
        u32 gen;

        alignas(std::max_align_t) u8 data[sizeof(U)];
    };

    u32 pHead  = kEndOfList;
    u32 pCount = 0;

    std::array<Entry, capacity> pData;
};

template<typename U, u64 capacity>
StaticPool<U, capacity>::StaticPool()
{
    for (u64 i = capacity; i > 0; --i)
    {
        pData[i - 1].gen = 0;
        pData[i - 1].next = pHead;
        pHead = (u32)(i - 1);
    }
}

template<typename U, u64 capacity>
StaticPool<U, capacity>::~StaticPool()
{
    for (u64 i = 0; i < pCount; ++i)
    {
        Entry *entry = &pData.at(i);
        if (entry->next == kNotInFreelist)
        {
            U* data = std::launder(reinterpret_cast<U*>(entry->data));
            std::destroy_at(data);
        }
    }
}

template<typename U, u64 capacity>
StaticPool<U, capacity>::PoolMeta StaticPool<U, capacity>::emplace(U&& val)
{
    if (pHead == kEndOfList) { assert(false); }
    
    u32 idx = pHead;
    assert(idx != kNotInFreelist &&
           idx != kEndOfList);
    
    Entry *entry = &pData.at(idx);
    assert(entry->next != kNotInFreelist);
    pHead = entry->next;
    entry->next = kNotInFreelist;

    U* data = std::launder(reinterpret_cast<U*>(entry->data));
    std::construct_at(data, std::forward<U>(val));
    
    ++pCount;
    
    return {idx, entry->gen};
}

template<typename U, u64 capacity>
void StaticPool<U, capacity>::erase(StaticPool<U, capacity>::PoolMeta h)
{
    Entry *entry = &pData.at(h.idx);
    assert(entry->gen == h.gen);

    U* data = std::launder(reinterpret_cast<U*>(entry->data));
    std::destroy_at(data);

    ++entry->gen;
    entry->next = pHead;
    pHead = h.idx;

    --pCount;
}

template<typename U, u64 capacity>
U* StaticPool<U, capacity>::at(StaticPool<U, capacity>::PoolMeta h)
{
   Entry *entry = &pData.at(h.idx);
   if (entry->gen != h.gen) return nullptr;
   U* data = std::launder(reinterpret_cast<U*>(entry->data));
   return data;
}

template<typename U, u64 capacity>
u64 StaticPool<U, capacity>::count()
{
    return pCount;
}

}

#endif
