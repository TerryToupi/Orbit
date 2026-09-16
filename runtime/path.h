#ifndef ORBIT_PATH_H
#define ORBIT_PATH_H

#include <arena.h>

// Lexical, case-sensitive normalization: '/', '.', '..'. No IO, symlink resolution, case folding or Unicode folding.
// Relative names cannot escape their root. Null reports invalid external input through SDL_GetError.
char* path_normalize(Arena& arena, const char* path);
// Captures an absolute lexical root using SDL's current directory. Use during initialization, not per load.
char* path_absolute(Arena& arena, const char* path);
char* path_join(Arena& arena, const char* root, const char* relative);

#endif
