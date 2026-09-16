#include <content.h>
#include <SDL3/SDL_stdinc.h>
#include <bit>

static void sha256_block(uint32_t* state, const uint8_t* data)
{
    static constexpr uint32_t constants[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    uint32_t words[64];
    for (uint32_t i = 0; i < 16; ++i)
        words[i] = (uint32_t(data[4 * i]) << 24) | (uint32_t(data[4 * i + 1]) << 16) | (uint32_t(data[4 * i + 2]) << 8) | data[4 * i + 3];
    for (uint32_t i = 16; i < 64; ++i) {
        uint32_t a = words[i - 15];
        uint32_t b = words[i - 2];
        words[i] = words[i - 16] + (std::rotr(a, 7) ^ std::rotr(a, 18) ^ (a >> 3)) + words[i - 7]
                 + (std::rotr(b, 17) ^ std::rotr(b, 19) ^ (b >> 10));
    }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t t1 = h + (std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25)) + ((e & f) ^ (~e & g)) + constants[i] + words[i];
        uint32_t t2 = (std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

ContentHash content_hash(Span<uint8_t> bytes)
{
    assert(bytes.data || bytes.size == 0);
    uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    size_t pos = 0;
    for (; bytes.size - pos >= 64; pos += 64)
        sha256_block(state, bytes.data + pos);
    uint8_t tail[128] = {};
    if (bytes.size != pos)
        SDL_memcpy(tail, bytes.data + pos, bytes.size - pos);
    tail[bytes.size - pos] = 0x80;
    uint32_t tail_size = bytes.size - pos < 56 ? 64 : 128;
    uint64_t bits = uint64_t(bytes.size) * 8;
    for (uint32_t i = 0; i < 8; ++i)
        tail[tail_size - 1 - i] = uint8_t(bits >> (i * 8));
    sha256_block(state, tail);
    if (tail_size == 128)
        sha256_block(state, tail + 64);
    ContentHash key;
    for (uint32_t i = 0; i < 32; ++i)
        key.bytes[i] = uint8_t(state[i / 4] >> (24 - 8 * (i % 4)));
    return key;
}

static uint32_t content_position(const ContentStripe& stripe, const ContentHash& key)
{
    uint32_t first = 0, last = stripe.count;
    while (first < last) {
        uint32_t mid = first + (last - first) / 2;
        if (SDL_memcmp(stripe.entries[mid].key.bytes, key.bytes, sizeof(key.bytes)) < 0)
            first = mid + 1;
        else
            last = mid;
    }
    return first;
}

bool create_content_cache(ContentCache& cache)
{
    for (ContentStripe& stripe : cache.stripes) {
        stripe.lock = SDL_CreateRWLock();
        if (!stripe.lock) {
            destroy_content_cache(cache);
            return false;
        }
        stripe.capacity = 256;
        stripe.entries = static_cast<ContentEntry*>(SDL_malloc(stripe.capacity * sizeof(ContentEntry)));
    }
    return true;
}

void destroy_content_cache(ContentCache& cache)
{
    for (ContentStripe& stripe : cache.stripes) {
        for (uint32_t i = 0; i < stripe.count; ++i)
            destroy_arena(stripe.entries[i].storage);
        SDL_free(stripe.entries);
        SDL_DestroyRWLock(stripe.lock);
        stripe = {};
    }
}

static void content_commit(ContentCache& cache, Arena& storage, Span<uint8_t> bytes, const ContentHash& key)
{
    ContentStripe& stripe = cache.stripes[key.bytes[0] % 16];
    SDL_LockRWLockForWriting(stripe.lock);
    uint32_t pos = content_position(stripe, key);
    bool duplicate = pos < stripe.count && stripe.entries[pos].key == key;
    if (!duplicate) {
        if (stripe.count == stripe.capacity) {
            stripe.capacity += 256;
            stripe.entries = static_cast<ContentEntry*>(SDL_realloc(stripe.entries, stripe.capacity * sizeof(ContentEntry)));
        }
        SDL_memmove(stripe.entries + pos + 1, stripe.entries + pos, (stripe.count - pos) * sizeof(ContentEntry));
        stripe.entries[pos] = {.key = key, .view = {.data = bytes.data, .size = bytes.size}, .storage = storage};
        ++stripe.count;
    }
    SDL_UnlockRWLock(stripe.lock);
    if (duplicate)
        destroy_arena(storage);
    storage = {};
}

ContentHash content_publish(ContentCache& cache, Arena& storage, Span<uint8_t> bytes)
{
    assert(storage.block && bytes.data);
    ContentHash key = content_hash(bytes);
    content_commit(cache, storage, bytes, key);
    return key;
}

ContentHash content_insert(ContentCache& cache, Span<uint8_t> bytes)
{
    ContentHash key = content_hash(bytes);
    if (content_get(cache, key).data)
        return key;
    Arena storage = {.block_capacity = bytes.size};
    uint8_t* data = arena_allocate<uint8_t>(storage, bytes.size);
    if (bytes.size)
        SDL_memcpy(data, bytes.data, bytes.size);
    content_commit(cache, storage, {data, bytes.size}, key);
    return key;
}

ContentView content_get(ContentCache& cache, const ContentHash& key)
{
    ContentStripe& stripe = cache.stripes[key.bytes[0] % 16];
    SDL_LockRWLockForReading(stripe.lock);
    uint32_t pos = content_position(stripe, key);
    ContentView view = {};
    if (pos < stripe.count && stripe.entries[pos].key == key)
        view = stripe.entries[pos].view;
    SDL_UnlockRWLock(stripe.lock);
    return view;
}
