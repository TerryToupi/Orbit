#ifndef ORBIT_MESH_H
#define ORBIT_MESH_H

#include <arena.h>
#include <span.h>

// Right-handed, +Y up, +Z model front; column vectors and column-major matrices. Front faces are counterclockwise.
struct MeshBounds
{
    float min[3] = {};
    float max[3] = {};
};

// Tightly packed float components; absent attributes have empty spans. Indices are primitive-local.
struct ImportedPrimitive
{
    Span<float> positions = {};
    Span<float> normals = {};
    Span<float> tangents = {};
    Span<float> texcoords = {};
    Span<uint32_t> indices = {};
    uint32_t material = UINT32_MAX;
};

struct ImportedMesh
{
    const char* name = nullptr;
    Span<ImportedPrimitive> primitives = {};
};

struct ImportedNode
{
    uint32_t mesh = 0;
    float world[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

struct ImportedScene
{
    Span<ImportedMesh> meshes = {};
    Span<ImportedNode> nodes = {};
    // Source material names in source order; primitive material indices address this table. Null means unnamed.
    Span<const char*> materials = {};
};

enum class VertexSemantic : uint32_t { Position, Normal, Tangent, TexCoord0 };
enum class VertexFormat : uint32_t { Float2, Float3, Float4 };
enum class MeshIndexType : uint32_t { UInt16, UInt32 };

struct VertexAttribute
{
    VertexSemantic semantic = VertexSemantic::Position;
    VertexFormat format = VertexFormat::Float3;
    uint32_t offset = 0;
};

struct VertexStream
{
    Span<uint8_t> data = {};
    Span<VertexAttribute> attributes = {};
    uint32_t stride = 0;
};

struct MeshPrimitive
{
    uint32_t first_index = 0;
    uint32_t index_count = 0;
    uint32_t first_stream = 0;
    uint32_t stream_count = 0;
    uint32_t vertex_count = 0;
    MeshBounds bounds = {};
};

// Triangle geometry in mesh-local space. Each primitive binds its stream range; indices address those streams from zero.
// Storage belongs to the arena supplied to build_mesh_asset (the artifact cache for imported assets).
struct MeshAsset
{
    Span<VertexStream> streams = {};
    Span<uint8_t> indices = {};
    Span<MeshPrimitive> primitives = {};
    MeshIndexType index_type = MeshIndexType::UInt16;
    MeshBounds bounds = {};
};

// Copies into independent arena-owned storage. Node transforms and source material assignments are not baked into geometry.
bool build_mesh_asset(Arena& storage, const ImportedMesh& source, MeshAsset& mesh, char (&error)[160]);

#endif
