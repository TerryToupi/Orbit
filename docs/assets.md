# CPU static-mesh assets

```cpp
AssetHandle<MeshAsset> dragon = asset_load(assets, "models/dragon.glb");

// Later, after the application's normal event dispatch and assets_tick:
if (asset_status(assets, dragon).state == AssetState::Ready) {
    const MeshAsset* mesh = asset_get(assets, dragon);
    // Consume CPU mesh data; retain dragon, not mesh.
}
```

`runtime/program.cpp` creates the factory after servers, dispatches polled artifact events to `assets_process_event`, calls
`assets_tick`, and destroys the factory before server caches. Other result consumers can handle events the factory does not claim.
There is no private event drain, callback API or synchronous asset wait.

## Identity and lifetime

`AssetID` is FNV-1a 64 of a normalized UTF-8 root-relative logical name, with zero reserved. It is independent of the absolute
installation directory. Renaming the logical name changes its ID; an eventual manifest can maintain identities independently.
Colliding IDs with different names are rejected. `asset_find` resolves an ID already registered with the factory.

`model.glb#Body` selects a unique, case-sensitive source mesh name. The normalized path and selector form the logical ID;
only the path goes to File Stream. A bare path requires exactly one mesh, including meshes not instantiated by the default scene.
Missing or ambiguous selections fail. Names remain stable across source mesh reordering; renaming a mesh changes its logical identity.
`#` is reserved for selection. There is no sub-asset database or unnamed multi-mesh fallback.

`AssetHandle<T>` contains a 32-bit slot index and 32-bit slot generation. It survives slot-array growth and reload. V1 does not
recycle slots; `asset_valid` detects invalid indices/generation mismatches. Handles belong to their originating factory lifetime.
File generations, content hashes and artifact keys remain separate from both asset IDs and handle generations.

Factory slots and path storage are main-thread owned. Server commands never contain factory/slot pointers. Immutable source bytes
belong to Content; parsed scenes and CPU meshes belong to artifact-owned arenas until cache shutdown. Destroying a factory with
work pending is safe, provided the server caches still live. `asset_get` returns a borrow, valid through the current main-thread
use until subsequent factory event processing/reload/destruction. No owning pointers or references between assets are introduced.

`asset_reload` queues an explicit refresh. Ready data remains accessible during loading and after a failed replacement; status
reports the last error separately. Successful replacement changes the current mesh and file generation, not the handle.
Repeated reloads during a pending load coalesce to one subsequent refresh. Automatic file watching remains deferred.

## Pipeline and representation

File commands produce immutable Content. `ImportGLTF` keys include source ContentHash, kind and version (currently 2). The existing
artifact owner lane suppresses duplicate successful computation, including identical files at different paths. Its result owns
an `ImportedScene` and one separately built `MeshAsset` per imported mesh. Named selections share this computation and its immutable
results. These are typed in-memory results, not serialized structs or a disk format.

The importer emits meshes with optional tightly packed attribute arrays, primitive-local uint32 indices, mesh instances with world
matrices, and source material associations. The material-name table retains source ordering (null for unnamed materials); primitive
indices address that table. Material shading properties are not imported yet. `mesh.cpp` copies each mesh into independent runtime
geometry and computes mesh/primitive bounds. No instances are flattened and no materials enter `MeshAsset`.

Runtime primitives preserve source order and identify index and vertex-stream ranges. Indices are local to each primitive's streams;
the mesh selects UInt16 when all indices fit, otherwise UInt32. Position, normal, tangent and UV0 use separate float streams initially.
Absent attributes consume no storage. Attribute descriptors also permit future interleaved layouts without changing the asset API.
Descriptors are batched; all vertex payloads share one arena allocation and all indices another. No per-vertex/primitive heap allocation
or persistent importer pointer is retained. The cache owns the backing arena, not the factory slot.

Conversion runs before server publication; the factory selects a mesh and replaces its pointer on the main thread.
A future compiled loader can produce the same geometry representation without glTF, materials or changes to handles/access APIs.

Coordinates are right-handed, +Y up, +Z model front, in meters. Matrices are column-major acting on column vectors; hierarchy composes
parent × local and local TRS means translation × rotation × scale. This matches glTF, so no axis swizzle is needed. Geometry stays
mesh-local: normals, tangent handedness and counterclockwise triangle order are preserved. Instance transforms, including mirrored
or singular transforms, remain source scene information; they do not alter geometry. Nonfinite transforms fail import.
Future instance rendering must account for normal transforms and mirrored winding when applying those transforms.

## Source-format boundary

The pinned, MIT-licensed [cgltf](https://github.com/jkuhlmann/cgltf) C parser is private to `gltf_import.cpp`. Unlike larger C++ loaders,
it requires no exceptions, container allocations or image-library dependencies. Explicit callbacks allocate parser state in SDL-backed
scratch arenas; default allocation macros also route to SDL. Parser state is reclaimed in bulk. No parser pointers or source-buffer
pointers survive in ImportedScene/MeshAsset. Unaligned input binary data is copied to aligned scratch only when needed.
The importer never calls cgltf's filesystem loading APIs.

V1 accepts embedded-buffer GLB static triangle meshes, float positions/normals/tangents, float or normalized unsigned-byte/unsigned-short UV0,
unsigned indices or implicit sequential indices, multiple primitives, and the default scene (first scene if no default is selected).
With no scenes, root nodes are used. Required extensions, compressed/sparse geometry, skins, animation, morph targets and mesh GPU
instancing fail explicitly. Cameras, lights, extra UVs, vertex colors and material/image shading data are ignored.
External buffers and JSON `.gltf` are deferred until asynchronous dependency loading exists; no synchronous fallback is hidden here.

There was no existing shared path normalizer or engine coordinate convention. `path.*` now provides lexical normalization outside
the factory; File Stream's exact-path contract remains unchanged. Names are case-sensitive and normalize separators, `.` and `..`;
root escape is rejected. This is not a filesystem sandbox: symlinks, case aliases and Unicode aliases are not resolved. UNC paths are
not supported. No filesystem IO occurs while deriving a logical identity.

## Verification and limits

`asset_tests` uses hand-written JSON chunks under `tests/data` and explicitly constructs tiny GLBs in the test build directory.
It covers optional streams, both index widths, primitive bounds/order, separate named meshes, transforms, malformed inputs,
identity/deduplication, stale handles, reload failure/success, independent output ownership, scratch
lifetime, bounded-queue backpressure and pending-import shutdown. Run both suites with `ctest --test-dir <build-directory>`.

Slot metadata grows in coarse blocks; ID lookup is binary search. Pending-slot scanning and event matching are linear in registered
slots for this first slice. Idle scans allocate nothing; loading commands batch into existing bounded queues and retry on later ticks
when saturated. Imported and runtime geometry both remain resident for now; eviction and representation optimization await profiling.
No GPU resources, compiler, binary format, packages, generic dependency graph or importer plugin system are implemented.
