#ifndef __ORBIT_CORE__
#define __ORBIT_CORE__

#include <bit>
#include <new>
#include <thread>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <utility>
#include <type_traits>

namespace Orbit
{

typedef std::uint8_t 	u8;
typedef std::uint16_t 	u16;
typedef std::uint32_t 	u32;
typedef std::uint64_t 	u64;
typedef std::int8_t 	s8;
typedef std::int16_t 	s16;
typedef std::int32_t 	s32;
typedef std::int64_t 	s64;
typedef std::uint8_t 	b8;
typedef std::uint16_t 	b16;
typedef std::uint32_t 	b32;
typedef std::uint64_t 	b64;
typedef float			f32;
typedef double 			f64;

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
    static_assert(std::is_trivially_copyable_v<U>,
                  "Pool payload must be memcpy-safe");
    static_assert(std::is_trivially_default_constructible_v<U>,
                  "Pool never constructs slots; U must tolerate raw storage");
    static_assert(std::is_trivially_destructible_v<U>,
                  "Pool never destructs slots; U must tolerate raw storage");
    
    struct PoolMeta
    {
        u32 idx;
        u32 gen;
    };
    
    Pool() = default;
    ~Pool();
    
    Pool(const Pool&)            = delete;
    Pool& operator=(const Pool&) = delete;
    
    void reset();
    
    PoolMeta emplace(U&& val);
    void     erase(PoolMeta h);
    U&       at(PoolMeta h);
    U*       at(u32 idx);
    u32      count() const;
    
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
        U data;
        u32 next;
        u32 gen;
    };
    
    void add_segment();
    Entry *get(u32 idx);
    Entry *get(u32 idx, u32 gen);

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
        Entry *segment = pSegments[segment_idx];
        std::free(segment);
        pSegments[segment_idx] = nullptr;
    }
    pUsedSegments = 0;
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
    ::new (&entry->data) U(std::forward<U&&>(val));
    
    ++pCount;
    
    return {idx, ++entry->gen};
}

template <typename U>
void Pool<U>::erase(Pool<U>::PoolMeta h)
{
    Entry *entry = get(h.idx);
    assert(entry->gen == h.gen);
    entry->next = pHead;
    pHead = h.idx;
    
    --pCount;
}

template <typename U>
U& Pool<U>::at(Pool<U>::PoolMeta h)
{
    Entry *e = get(h.idx, h.gen);
    return e ? e->data : pStub;
}

template <typename U>
U* Pool<U>::at(u32 idx)
{
    Entry *e = get(idx);
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
Pool<U>::Entry *Pool<U>::get(u32 idx, u32 gen)
{
    u64 segment = int_log_2((idx >> kSmallSegmentsToSkip) + 1);
    u32 slot    = idx - capacity_for_segment_count((u32)segment);
    auto entry  = &pSegments[segment][slot];
    if (entry->gen != gen) return nullptr;
    return entry;
}

template <typename U>
u32 Pool<U>::count() const
{
    return pCount;
}

}

#endif
