#include <mesh.h>
#include <SDL3/SDL_stdinc.h>
#include <cfloat>
#include <cmath>

bool build_mesh_asset(Arena& storage, const ImportedMesh& source, MeshAsset& mesh, char (&error)[160])
{
    size_t stream_count = 0, vertex_bytes = 0, index_count = 0;
    uint32_t max_index = 0;
    if (!source.primitives.size) {
        SDL_strlcpy(error, "Mesh has no primitives", sizeof(error));
        return false;
    }
    for (size_t i = 0; i < source.primitives.size; ++i) {
        const ImportedPrimitive& p = source.primitives.data[i];
        size_t count = p.positions.size / 3;
        if (!count || p.positions.size % 3 || !p.indices.size || p.indices.size % 3 ||
            (p.normals.size && p.normals.size != count * 3) || (p.tangents.size && p.tangents.size != count * 4) ||
            (p.texcoords.size && p.texcoords.size != count * 2)) {
            SDL_strlcpy(error, "Invalid mesh attribute or triangle counts", sizeof(error));
            return false;
        }
        Span<float> attributes[] = {p.positions, p.normals, p.tangents, p.texcoords};
        for (Span<float> values : attributes) {
            stream_count += values.size != 0;
            vertex_bytes += values.size * sizeof(float);
            for (size_t v = 0; v < values.size; ++v) {
                if (!std::isfinite(values.data[v])) {
                    SDL_strlcpy(error, "Nonfinite mesh attribute", sizeof(error));
                    return false;
                }
            }
        }
        index_count += p.indices.size;
        for (size_t j = 0; j < p.indices.size; ++j) {
            uint32_t index = p.indices.data[j];
            if (index >= count) {
                SDL_strlcpy(error, "Mesh index exceeds vertex count", sizeof(error));
                return false;
            }
            if (index > max_index)
                max_index = index;
        }
    }
    MeshIndexType index_type = max_index <= UINT16_MAX ? MeshIndexType::UInt16 : MeshIndexType::UInt32;
    size_t index_bytes = index_count * (index_type == MeshIndexType::UInt16 ? 2 : 4);
    VertexStream* streams = arena_allocate<VertexStream>(storage, stream_count);
    VertexAttribute* attributes = arena_allocate<VertexAttribute>(storage, stream_count);
    MeshPrimitive* primitives = arena_allocate<MeshPrimitive>(storage, source.primitives.size);
    uint8_t* vertices = static_cast<uint8_t*>(arena_allocate(storage, vertex_bytes, alignof(float)));
    uint8_t* indices = static_cast<uint8_t*>(arena_allocate(storage, index_bytes, alignof(uint32_t)));
    mesh = {.streams = {streams, stream_count}, .indices = {indices, index_bytes}, .primitives = {primitives, source.primitives.size},
            .index_type = index_type, .bounds = {.lowerBound = {.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX},
                       .upperBound = {.x = -FLT_MAX, .y = -FLT_MAX, .z = -FLT_MAX}}};
    uint32_t stream = 0, first_index = 0;
    for (size_t i = 0; i < source.primitives.size; ++i) {
        const ImportedPrimitive& p = source.primitives.data[i];
        MeshPrimitive& primitive = primitives[i];
        primitive = {.first_index = first_index, .index_count = uint32_t(p.indices.size), .first_stream = stream,
                     .vertex_count = uint32_t(p.positions.size / 3),
                     .bounds = {.lowerBound = {.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX},
                       .upperBound = {.x = -FLT_MAX, .y = -FLT_MAX, .z = -FLT_MAX}}};
        Span<float> values[] = {p.positions, p.normals, p.tangents, p.texcoords};
        const VertexSemantic semantics[] = {VertexSemantic::Position, VertexSemantic::Normal, VertexSemantic::Tangent, VertexSemantic::TexCoord0};
        const VertexFormat formats[] = {VertexFormat::Float3, VertexFormat::Float3, VertexFormat::Float4, VertexFormat::Float2};
        const uint32_t strides[] = {12, 12, 16, 8};
        for (uint32_t a = 0; a < 4; ++a) {
            if (!values[a].size)
                continue;
            attributes[stream] = {.semantic = semantics[a], .format = formats[a]};
            streams[stream] = {.data = {vertices, values[a].size * sizeof(float)}, .attributes = {attributes + stream, 1}, .stride = strides[a]};
            SDL_memcpy(vertices, values[a].data, values[a].size * sizeof(float));
            vertices += values[a].size * sizeof(float);
            ++stream;
        }
        primitive.stream_count = stream - primitive.first_stream;
        for (size_t v = 0; v < p.positions.size; v += 3) {
            b3Vec3 position = {.x = p.positions.data[v], .y = p.positions.data[v + 1], .z = p.positions.data[v + 2]};
            primitive.bounds.lowerBound = b3Min(primitive.bounds.lowerBound, position);
            primitive.bounds.upperBound = b3Max(primitive.bounds.upperBound, position);
        }
        mesh.bounds = b3AABB_Union(mesh.bounds, primitive.bounds);
        for (size_t j = 0; j < p.indices.size; ++j) {
            uint32_t index = p.indices.data[j];
            if (index_type == MeshIndexType::UInt16)
                reinterpret_cast<uint16_t*>(indices)[first_index++] = uint16_t(index);
            else
                reinterpret_cast<uint32_t*>(indices)[first_index++] = index;
        }
    }
    return true;
}
