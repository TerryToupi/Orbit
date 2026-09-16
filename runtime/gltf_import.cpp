#include <gltf_import.h>
#include <thread_context.h>
#include <SDL3/SDL_stdinc.h>
#include <cmath>

#define CGLTF_MALLOC(size) SDL_malloc(size)
#define CGLTF_FREE(ptr) SDL_free(ptr)
#define CGLTF_ATOI(str) SDL_atoi(str)
#define CGLTF_ATOF(str) SDL_atof(str)
#define CGLTF_ATOLL(str) SDL_strtoll(str, nullptr, 10)
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

static void* import_allocate(void* user, size_t size)
{
    return arena_allocate(*static_cast<Arena*>(user), size);
}

static void import_free(void*, void*) {}

static bool validate_source(cgltf_data& data, Arena& scratch, char (&error)[160])
{
    if (data.file_type != cgltf_file_type_glb || data.buffers_count != 1 || data.buffers[0].uri) {
        SDL_strlcpy(error, "Only self-contained GLB buffers are supported; external resources require asynchronous dependency loading", sizeof(error));
        return false;
    }
    if (data.skins_count || data.animations_count || data.extensions_required_count) {
        SDL_strlcpy(error, "Skins, animations and required extensions are not supported", sizeof(error));
        return false;
    }
    if (!data.bin || data.buffers[0].size > data.bin_size) {
        SDL_strlcpy(error, "Invalid GLB binary buffer", sizeof(error));
        return false;
    }
    data.buffers[0].data = const_cast<void*>(data.bin);
    if (reinterpret_cast<uintptr_t>(data.bin) % 4) {
        data.buffers[0].data = arena_allocate(scratch, data.buffers[0].size, 4);
        SDL_memcpy(data.buffers[0].data, data.bin, data.buffers[0].size);
    }
    for (size_t i = 0; i < data.buffer_views_count; ++i) {
        const cgltf_buffer_view& view = data.buffer_views[i];
        if (view.has_meshopt_compression || !view.buffer || view.offset > view.buffer->size || view.size > view.buffer->size - view.offset) {
            SDL_strlcpy(error, "Invalid or compressed buffer view", sizeof(error));
            return false;
        }
    }
    // Validate ranges before cgltf_validate: its accessor arithmetic and index reads assume nonoverflowing, aligned ranges.
    for (size_t i = 0; i < data.accessors_count; ++i) {
        const cgltf_accessor& accessor = data.accessors[i];
        size_t element = cgltf_calc_size(accessor.type, accessor.component_type);
        size_t component = cgltf_component_size(accessor.component_type);
        if (accessor.is_sparse || !accessor.buffer_view || !accessor.count || !element || !component || accessor.stride < element) {
            SDL_strlcpy(error, "Invalid accessor or unsupported sparse accessor", sizeof(error));
            return false;
        }
        const cgltf_buffer_view& view = *accessor.buffer_view;
        if (accessor.offset > view.size || element > view.size - accessor.offset ||
            accessor.count - 1 > (view.size - accessor.offset - element) / accessor.stride ||
            (view.offset + accessor.offset) % component || accessor.stride % component) {
            SDL_strlcpy(error, "Accessor exceeds its buffer view or has invalid alignment", sizeof(error));
            return false;
        }
    }
    for (size_t i = 0; i < data.nodes_count; ++i) {
        const cgltf_node& node = data.nodes[i];
        if (node.skin || node.has_mesh_gpu_instancing || node.weights_count ||
            (node.has_matrix && (node.has_translation || node.has_rotation || node.has_scale))) {
            SDL_strlcpy(error, "Unsupported or invalid mesh node", sizeof(error));
            return false;
        }
        if (node.has_rotation) {
            float length = 0;
            for (float value : node.rotation)
                length += value * value;
            if (!std::isfinite(length) || SDL_fabsf(length - 1) > 0.0001f) {
                SDL_strlcpy(error, "Node rotation must be a unit quaternion", sizeof(error));
                return false;
            }
        }
    }
    if (cgltf_validate(&data) != cgltf_result_success) {
        SDL_strlcpy(error, "Invalid glTF geometry or node hierarchy", sizeof(error));
        return false;
    }
    return true;
}

static bool primitive_attributes(const cgltf_primitive& primitive, const cgltf_accessor*& positions,
                                 const cgltf_accessor*& normals, const cgltf_accessor*& tangents, const cgltf_accessor*& uv, char (&error)[160])
{
    positions = normals = tangents = uv = nullptr;
    if (primitive.type != cgltf_primitive_type_triangles || primitive.targets_count || primitive.has_draco_mesh_compression) {
        SDL_strlcpy(error, "Only uncompressed static triangle primitives are supported", sizeof(error));
        return false;
    }
    for (size_t i = 0; i < primitive.attributes_count; ++i) {
        const cgltf_attribute& attribute = primitive.attributes[i];
        const cgltf_accessor** target = nullptr;
        if (attribute.type == cgltf_attribute_type_position)
            target = &positions;
        else if (attribute.type == cgltf_attribute_type_normal)
            target = &normals;
        else if (attribute.type == cgltf_attribute_type_tangent)
            target = &tangents;
        else if (attribute.type == cgltf_attribute_type_texcoord && attribute.index == 0)
            target = &uv;
        if (target) {
            if (*target) {
                SDL_strlcpy(error, "Duplicate vertex attribute", sizeof(error));
                return false;
            }
            *target = attribute.data;
        }
    }
    if (!positions || positions->type != cgltf_type_vec3 || positions->component_type != cgltf_component_type_r_32f || positions->normalized ||
        (normals && (normals->type != cgltf_type_vec3 || normals->component_type != cgltf_component_type_r_32f || normals->normalized)) ||
        (tangents && (tangents->type != cgltf_type_vec4 || tangents->component_type != cgltf_component_type_r_32f || tangents->normalized)) ||
        (uv && (uv->type != cgltf_type_vec2 || !((uv->component_type == cgltf_component_type_r_32f && !uv->normalized) ||
        ((uv->component_type == cgltf_component_type_r_8u || uv->component_type == cgltf_component_type_r_16u) && uv->normalized))))) {
        SDL_strlcpy(error, "Missing positions or unsupported vertex encoding", sizeof(error));
        return false;
    }
    if ((primitive.indices ? primitive.indices->count : positions->count) % 3 || (primitive.indices && primitive.indices->normalized)) {
        SDL_strlcpy(error, "Invalid triangle index count or encoding", sizeof(error));
        return false;
    }
    return true;
}

static bool convert_scene(Arena& storage, Arena& scratch, cgltf_data& data, ImportedScene& scene, char (&error)[160])
{
    ImportedMesh* meshes = arena_allocate<ImportedMesh>(storage, data.meshes_count);
    ImportedNode* nodes = arena_allocate<ImportedNode>(storage, data.nodes_count);
    const char** materials = arena_allocate<const char*>(storage, data.materials_count);
    for (size_t i = 0; i < data.materials_count; ++i) {
        materials[i] = nullptr;
        if (data.materials[i].name) {
            char* name = arena_allocate<char>(storage, SDL_strlen(data.materials[i].name) + 1);
            SDL_strlcpy(name, data.materials[i].name, SDL_strlen(data.materials[i].name) + 1);
            size_t name_size = cgltf_decode_string(name);
            if (name_size != SDL_strlen(name)) {
                SDL_strlcpy(error, "Material name contains a null character", sizeof(error));
                return false;
            }
            materials[i] = name;
        }
    }
    scene = {.meshes = {meshes, data.meshes_count}, .materials = {materials, data.materials_count}};
    for (size_t m = 0; m < data.meshes_count; ++m) {
        const cgltf_mesh& source = data.meshes[m];
        ImportedPrimitive* primitives = arena_allocate<ImportedPrimitive>(storage, source.primitives_count);
        meshes[m] = {.primitives = {primitives, source.primitives_count}};
        if (source.name) {
            char* name = arena_allocate<char>(storage, SDL_strlen(source.name) + 1);
            SDL_strlcpy(name, source.name, SDL_strlen(source.name) + 1);
            size_t name_size = cgltf_decode_string(name);
            if (name_size != SDL_strlen(name)) {
                SDL_strlcpy(error, "Mesh name contains a null character", sizeof(error));
                return false;
            }
            meshes[m].name = name;
        }
        if (!source.primitives_count) {
            SDL_strlcpy(error, "Empty mesh", sizeof(error));
            return false;
        }
        for (size_t p = 0; p < source.primitives_count; ++p) {
            const cgltf_primitive& primitive = source.primitives[p];
            const cgltf_accessor* positions;
            const cgltf_accessor* normals;
            const cgltf_accessor* tangents;
            const cgltf_accessor* uv;
            if (!primitive_attributes(primitive, positions, normals, tangents, uv, error))
                return false;
            primitives[p] = {.material = primitive.material ? uint32_t(primitive.material - data.materials) : UINT32_MAX};
            const cgltf_accessor* accessors[] = {positions, normals, tangents, uv};
            Span<float>* output[] = {&primitives[p].positions, &primitives[p].normals, &primitives[p].tangents, &primitives[p].texcoords};
            const uint32_t components[] = {3, 3, 4, 2};
            for (uint32_t a = 0; a < 4; ++a) {
                if (!accessors[a])
                    continue;
                if (accessors[a]->count != positions->count) {
                    SDL_strlcpy(error, "Mismatched vertex attribute counts", sizeof(error));
                    return false;
                }
                float* values = arena_allocate<float>(storage, positions->count * components[a]);
                *output[a] = {values, positions->count * components[a]};
                for (size_t v = 0; v < positions->count; ++v) {
                    if (!cgltf_accessor_read_float(accessors[a], v, values + v * components[a], components[a])) {
                        SDL_strlcpy(error, "Unreadable vertex attribute", sizeof(error));
                        return false;
                    }
                    for (uint32_t c = 0; c < components[a]; ++c) {
                        if (!std::isfinite(values[v * components[a] + c])) {
                            SDL_strlcpy(error, "Nonfinite vertex attribute", sizeof(error));
                            return false;
                        }
                    }
                    if (a == 2 && values[v * 4 + 3] != 1 && values[v * 4 + 3] != -1) {
                        SDL_strlcpy(error, "Tangent handedness must be +1 or -1", sizeof(error));
                        return false;
                    }
                }
            }
            size_t count = primitive.indices ? primitive.indices->count : positions->count;
            uint32_t* indices = arena_allocate<uint32_t>(storage, count);
            primitives[p].indices = {indices, count};
            for (size_t i = 0; i < count; ++i)
                indices[i] = uint32_t(primitive.indices ? cgltf_accessor_read_index(primitive.indices, i) : i);
        }
    }

    cgltf_node** stack = arena_allocate<cgltf_node*>(scratch, data.nodes_count);
    float* worlds = arena_allocate<float>(scratch, data.nodes_count * 16);
    uint8_t* visited = arena_allocate<uint8_t>(scratch, data.nodes_count);
    SDL_memset(visited, 0, data.nodes_count);
    size_t pending = 0, node_count = 0;
    const cgltf_scene* selected = data.scene ? data.scene : (data.scenes_count ? &data.scenes[0] : nullptr);
    for (size_t i = selected ? selected->nodes_count : data.nodes_count; i > 0; --i) {
        cgltf_node* node = selected ? selected->nodes[i - 1] : &data.nodes[i - 1];
        if (!selected && node->parent)
            continue;
        if (visited[node - data.nodes] || pending == data.nodes_count) {
            SDL_strlcpy(error, "Repeated scene root", sizeof(error));
            return false;
        }
        visited[node - data.nodes] = 1;
        stack[pending++] = node;
    }
    while (pending) {
        cgltf_node* node = stack[--pending];
        float local[16];
        cgltf_node_transform_local(node, local);
        float* world = worlds + (node - data.nodes) * 16;
        if (node->parent) {
            const float* parent = worlds + (node->parent - data.nodes) * 16;
            for (uint32_t column = 0; column < 4; ++column) {
                for (uint32_t row = 0; row < 4; ++row) {
                    world[column * 4 + row] = 0;
                    for (uint32_t k = 0; k < 4; ++k)
                        world[column * 4 + row] += parent[k * 4 + row] * local[column * 4 + k];
                }
            }
        } else {
            SDL_memcpy(world, local, sizeof(local));
        }
        for (uint32_t i = 0; i < 16; ++i) {
            if (!std::isfinite(world[i])) {
                SDL_strlcpy(error, "Nonfinite node transform", sizeof(error));
                return false;
            }
        }
        if (world[3] != 0 || world[7] != 0 || world[11] != 0 || world[15] != 1) {
            SDL_strlcpy(error, "Node matrix must be affine", sizeof(error));
            return false;
        }
        if (node->mesh) {
            nodes[node_count] = {.mesh = uint32_t(node->mesh - data.meshes)};
            SDL_memcpy(nodes[node_count++].world, world, sizeof(local));
        }
        for (size_t i = node->children_count; i > 0; --i) {
            cgltf_node* child = node->children[i - 1];
            if (visited[child - data.nodes] || pending == data.nodes_count) {
                SDL_strlcpy(error, "Repeated scene node", sizeof(error));
                return false;
            }
            visited[child - data.nodes] = 1;
            stack[pending++] = child;
        }
    }
    scene.nodes = {nodes, node_count};
    return true;
}

bool import_gltf(Arena& storage, Span<uint8_t> bytes, ImportedScene& scene, char (&error)[160])
{
    assert(bytes.data || !bytes.size);
    if (bytes.size >= 12 && SDL_memcmp(bytes.data, "glTF", 4) == 0) {
        uint32_t length = uint32_t(bytes.data[8]) | (uint32_t(bytes.data[9]) << 8) | (uint32_t(bytes.data[10]) << 16) | (uint32_t(bytes.data[11]) << 24);
        if (length != bytes.size) {
            SDL_strlcpy(error, "GLB length does not match its source bytes", sizeof(error));
            return false;
        }
    }
    ArenaTemp scratch = scratch_begin({&storage});
    cgltf_options options = {.memory = {.alloc_func = import_allocate, .free_func = import_free, .user_data = scratch.arena}};
    cgltf_data* data = nullptr;
    bool success = cgltf_parse(&options, bytes.data, bytes.size, &data) == cgltf_result_success;
    if (!success)
        SDL_strlcpy(error, "Malformed glTF/GLB document", sizeof(error));
    if (success && data->bin && (static_cast<const uint8_t*>(data->bin) - bytes.data) % 4) {
        SDL_strlcpy(error, "Unaligned GLB binary chunk offset", sizeof(error));
        success = false;
    }
    if (success)
        success = validate_source(*data, *scratch.arena, error) && convert_scene(storage, *scratch.arena, *data, scene, error);
    scratch_end(scratch);
    return success;
}
