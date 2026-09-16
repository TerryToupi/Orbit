#include <path.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>

char* path_normalize(Arena& arena, const char* path)
{
    assert(path);
    size_t size = SDL_strlen(path);
    if (!size) {
        SDL_SetError("Empty path");
        return nullptr;
    }
    char* result = arena_allocate<char>(arena, size + 2);
    size_t read = 0, write = 0, root = 0;
    if (path[0] == '/' || path[0] == '\\') {
        result[write++] = '/';
        root = 1;
        read = 1;
        if (size > 1 && (path[1] == '/' || path[1] == '\\')) {
            SDL_SetError("UNC paths are not supported");
            return nullptr;
        }
    } else if (size >= 2 && path[1] == ':') {
        if (size < 3 || (path[2] != '/' && path[2] != '\\') || !SDL_isalpha(path[0])) {
            SDL_SetError("Expected an absolute drive path");
            return nullptr;
        }
        result[write++] = char(SDL_toupper(path[0]));
        result[write++] = ':';
        result[write++] = '/';
        root = write;
        read = 3;
    }
    while (read < size) {
        while (read < size && (path[read] == '/' || path[read] == '\\'))
            ++read;
        size_t first = read;
        while (read < size && path[read] != '/' && path[read] != '\\') {
            if (path[read] == ':') {
                SDL_SetError("Invalid path component");
                return nullptr;
            }
            ++read;
        }
        size_t count = read - first;
        if (!count || (count == 1 && path[first] == '.'))
            continue;
        if (count == 2 && path[first] == '.' && path[first + 1] == '.') {
            if (write == root) {
                SDL_SetError("Path escapes its root");
                return nullptr;
            }
            while (write > root && result[write - 1] != '/')
                --write;
            if (write > root)
                --write;
        } else {
            if (write > root)
                result[write++] = '/';
            SDL_memcpy(result + write, path + first, count);
            write += count;
        }
    }
    if (!write)
        result[write++] = '.';
    result[write] = 0;
    return result;
}

char* path_join(Arena& arena, const char* root, const char* relative)
{
    assert(root && relative);
    size_t size = SDL_strlen(root) + SDL_strlen(relative) + 2;
    char* joined = arena_allocate<char>(arena, size);
    SDL_snprintf(joined, size, "%s%s%s", root, root[0] && root[SDL_strlen(root) - 1] == '/' ? "" : "/", relative);
    return joined;
}

char* path_absolute(Arena& arena, const char* path)
{
    assert(path);
    if (path[0] == '/' || path[0] == '\\' || (path[0] && path[1] == ':'))
        return path_normalize(arena, path);
    char* current = SDL_GetCurrentDirectory();
    if (!current)
        return nullptr;
    char* result = path_normalize(arena, path_join(arena, current, path));
    SDL_free(current);
    return result;
}
