#ifndef ORBIT_CONTENT_H
#define ORBIT_CONTENT_H

#include <arena.h>
#include <span.h>
#include <SDL3/SDL_mutex.h>

struct ContentHash
{
    uint8_t bytes[32] = {};
    bool operator==(const ContentHash&) const = default;
};

struct ContentView
{
    const uint8_t* data = nullptr;
    size_t size = 0;
};

struct ContentEntry
{
    ContentHash key = {};
    ContentView view = {};
    Arena storage = {};
};

struct ContentStripe
{
    SDL_RWLock* lock = nullptr;
    ContentEntry* entries = nullptr;
    uint32_t count = 0;
    uint32_t capacity = 0;
};

// Owns immutable blobs. Cache maintenance requires quiescent readers; V1 only destroys, with no eviction.
struct ContentCache
{
    ContentStripe stripes[16] = {};
};

bool create_content_cache(ContentCache& cache);
void destroy_content_cache(ContentCache& cache);
ContentHash content_hash(Span<uint8_t> bytes);
// Transfers the entire arena and clears storage, including on deduplication. Bytes must belong to that arena; relinquish all write aliases.
ContentHash content_publish(ContentCache& cache, Arena& storage, Span<uint8_t> bytes);
ContentHash content_insert(ContentCache& cache, Span<uint8_t> bytes);
// Borrow for the current cache-use phase; retain hashes, not views, across maintenance boundaries.
// Concurrent lookup. A null data pointer means missing; a published empty blob has a non-null pointer.
ContentView content_get(ContentCache& cache, const ContentHash& key);

#endif
