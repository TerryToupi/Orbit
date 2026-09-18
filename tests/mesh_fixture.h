#ifndef ORBIT_TEST_MESH_FIXTURE_H
#define ORBIT_TEST_MESH_FIXTURE_H

#include <arena.h>
#include <span.h>
#include <SDL3/SDL.h>

static void put_u32(uint8_t* output, uint32_t value)
{
    for (uint32_t i = 0; i < 4; ++i)
        output[i] = uint8_t(value >> (i * 8));
}

static void put_float(uint8_t* output, float value)
{
    uint32_t bits;
    SDL_memcpy(&bits, &value, sizeof(bits));
    put_u32(output, bits);
}

static Span<uint8_t> fixture(Arena& arena, const char* name, uint32_t variant = 0, const char* from = nullptr, const char* to = nullptr)
{
    char path[1024];
    SDL_snprintf(path, sizeof(path), "%s/%s", ORBIT_TEST_DATA, name);
    size_t size = 0;
    char* json = static_cast<char*>(SDL_LoadFile(path, &size));
    if (!json)
        return {};
    const char* text = json;
    if (from) {
        const char* match = SDL_strstr(json, from);
        if (!match) {
            SDL_free(json);
            return {};
        }
        size_t before = size_t(match - json), after = size - before - SDL_strlen(from);
        size = before + SDL_strlen(to) + after;
        char* changed = arena_allocate<char>(arena, size);
        SDL_memcpy(changed, json, before);
        SDL_memcpy(changed + before, to, SDL_strlen(to));
        SDL_memcpy(changed + before + SDL_strlen(to), match + SDL_strlen(from), after);
        text = changed;
    }
    size_t padded = (size + 3) / 4 * 4 + variant * 4;
    uint32_t binary_size = SDL_strcmp(name, "tangent.json") == 0 ? 152 : (SDL_strcmp(name, "index32.json") == 0 ? 108 : 104);
    size_t total = 12 + 8 + padded + 8 + binary_size;
    uint8_t* bytes = arena_allocate<uint8_t>(arena, total);
    put_u32(bytes, 0x46546c67);
    put_u32(bytes + 4, 2);
    put_u32(bytes + 8, uint32_t(total));
    put_u32(bytes + 12, uint32_t(padded));
    put_u32(bytes + 16, 0x4e4f534a);
    SDL_memcpy(bytes + 20, text, size);
    SDL_memset(bytes + 20 + size, ' ', padded - size);
    SDL_free(json);
    put_u32(bytes + 20 + padded, binary_size);
    put_u32(bytes + 24 + padded, 0x004e4942);
    uint8_t* bin = bytes + 28 + padded;
    SDL_memset(bin, 0, binary_size);
    put_float(bin + 12, float(variant + 1));
    put_float(bin + 28, float(variant + 1));
    for (uint32_t i = 0; i < 3; ++i) {
        if (SDL_strcmp(name, "transform.json") == 0) {
            put_float(bin + 36 + i * 12, 0.70710678118f);
            put_float(bin + 40 + i * 12, 0.70710678118f);
        } else {
            put_float(bin + 44 + i * 12, 1);
        }
    }
    put_float(bin + 80, 1);
    put_float(bin + 92, 1);
    bin[98] = 1;
    bin[100] = 2;
    if (binary_size == 152) {
        for (uint32_t i = 0; i < 3; ++i) {
            put_float(bin + 104 + i * 16, 1);
            put_float(bin + 116 + i * 16, -1);
        }
    } else if (binary_size == 108) {
        put_u32(bin + 96, 0);
        put_u32(bin + 100, 1);
        put_u32(bin + 104, 2);
    }
    return {bytes, total};
}

#endif
