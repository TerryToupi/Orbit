#ifndef ORBIT_MEMORY_SETUP_H
#define ORBIT_MEMORY_SETUP_H

// Installs the engine's existing rpmalloc-backed SDL hooks before SDL initialization.
bool initialize_memory();
void dump_memory();

#endif
