#ifndef ORBIT_ARTIFACT_CACHE_H
#define ORBIT_ARTIFACT_CACHE_H

#include <command_stream.h>
#include <content.h>
#include <file_stream.h>

struct ImportedScene;
struct MeshAsset;

constexpr uint32_t gltf_import_version = 3;

enum class ArtifactKind : uint32_t { ByteHistogram = 1, File = 2, ImportGLTF = 3 };
enum class ArtifactState : uint32_t { Missing, Building, Ready, Failed };
enum class ArtifactError : uint32_t { None, MissingContent, Unsupported, InvalidParameters, IOError, Changed, TooLarge, NotFile, InvalidData };

struct ArtifactKey
{
    ContentHash source = {};
    ContentHash parameters = {};
    ArtifactKind kind = ArtifactKind::ByteHistogram;
    uint32_t version = 1;
};

struct ArtifactView
{
    ContentHash content = {};
    const ImportedScene* scene = nullptr;
    Span<MeshAsset> meshes = {};
    uint64_t generation = 0;
    uint64_t ready_generation = 0;
    ArtifactState state = ArtifactState::Missing;
    ArtifactError error = ArtifactError::None;
};

struct ArtifactCommand
{
    uint64_t request = 0;
    ArtifactKey key = {};
    ContentHash hash = {};
    const char* path = nullptr;
};

struct ArtifactEvent
{
    uint64_t request = 0;
    ContentHash artifact = {};
    ArtifactView result = {};
    ArtifactKind kind = ArtifactKind::ByteHistogram;
    uint32_t lane = 0;
    bool cached = false;
    char error[160] = {};
};

struct ArtifactEntry
{
    ContentHash hash = {};
    ArtifactView result = {};
    SDL_PathInfo file_info = {};
    Arena storage = {};
};

struct ArtifactLane
{
    SDL_RWLock* lock = nullptr;
    ArtifactEntry* entries = nullptr;
    uint32_t count = 0;
    uint32_t capacity = 0;
};

// One writer per lane; concurrent lookup takes a read lock. Typed results live in this cache; byte results live in ContentCache.
struct ArtifactCache
{
    ContentCache* content = nullptr;
    uint64_t file_size_limit = 1024ull * 1024 * 1024;
    ArtifactLane* lanes = nullptr;
    uint32_t lane_count = 0;
    CommandStream<ArtifactCommand, ArtifactEvent> commands = {};
};

bool create_artifact_cache(ArtifactCache& cache, ContentCache& content, ServerSignal& signal, uint32_t lanes, uint32_t buffers);
void destroy_artifact_cache(ArtifactCache& cache);
// Hashes source, parameter-content key, kind and version using canonical little-endian scalar encoding.
ContentHash artifact_hash(const ArtifactKey& key);
// Main thread: commands_begin(cache.commands), append requests, commands_submit(*buffer). Request 0 means buffer capacity reached.
uint64_t artifact_request(ArtifactCommandBuffer& buffer, const ArtifactKey& key);
// Changed inputs have a new key. Keep the previous ready key while requesting/querying its replacement.
// File identities additionally retain ready_generation/content in the same view while Building or Failed.
ArtifactView artifact_get(ArtifactCache& cache, const ContentHash& hash);
// All lanes call this. Each key has one owner; later duplicate requests reuse the completed value without blocking other lanes.
bool artifact_cache_tick(ArtifactCache& cache);

// ByteHistogram v1: result is 256 little-endian uint64_t counts. Optional parameter content is two little-endian uint64_t
// values (offset, size); a zero parameter key selects the whole source. Failed requests may be retried by submitting again.

#endif
