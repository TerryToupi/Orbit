#ifndef ORBIT_SCENE_H
#define ORBIT_SCENE_H

#include <mesh.h>

using SceneMatrix = float[16];

struct SceneMesh
{
    const MeshAsset* mesh = nullptr;
    SceneMatrix world = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

// Main-thread borrow for one render call. Meshes are immutable and remain alive until the compositor is destroyed.
// Matrices are column-major, acting on column vectors. Projection uses SDL GPU's 0..1 clip depth.
struct Scene
{
    virtual Span<SceneMesh> meshes() const = 0;
    virtual const SceneMatrix& view_projection() const = 0;

protected:
    ~Scene() = default;
};

#endif
