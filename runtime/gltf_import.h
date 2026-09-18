#ifndef ORBIT_GLTF_IMPORT_H
#define ORBIT_GLTF_IMPORT_H

#include <mesh.h>

// Self-contained GLB, static triangles. Owns no files; parser memory is scratch, output belongs exclusively to storage.
// Imports authored node transforms; animation tracks are ignored.
bool import_gltf(Arena& storage, Span<uint8_t> bytes, ImportedScene& scene, char (&error)[160]);

#endif
