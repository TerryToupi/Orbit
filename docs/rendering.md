# Rendering compositor

`RenderingCompositor::render(const Scene&)` runs on the window/main thread. Scene supplies only a borrowed span of mesh instances
and a view-projection matrix. It owns no renderer resources. Null mesh pointers are skipped, allowing the application to expose
assets that are still loading. The only virtual interface is this explicitly requested Scene contract; no ECS, extraction system,
pass hierarchy, or graphics abstraction has been introduced.

`program.cpp` owns the SDL window and replaces SDL_Renderer with the compositor. Its temporary single-mesh preview borrows the
ready result of an optional command-line asset name (including named sub-assets). It uses an identity camera; it is not a scene loader
or an automatic model viewer. With no asset it renders the cleared targets.

## Ownership and frame order

The compositor owns the SDL GPU device/window claim, depth and two GBuffer targets, concrete passes, and immutable GPU mesh cache.
The application destroys it before the window, AssetFactory/server caches, and SDL. Passes own shaders/pipelines, not shared targets.

Each frame prepares missing uploads, acquires a command buffer and swapchain texture, recreates targets if acquired pixel dimensions
changed, runs depth then GBuffer, blits the debug base-color target, and submits. SDL submission presents the acquired swapchain;
there is no separate present call. A null swapchain texture skips recording. Once acquired, the swapchain command buffer is submitted,
never cancelled. Future final UI recording belongs after the final output and before this submission.

Depth is D32_FLOAT, cleared to 1 and stored by the prepass. GBuffer loads that exact depth, tests EQUAL, and does not write depth.
Both use the same vertex shader. The two temporary RGBA8_UNORM attachments hold linear debug base color and encoded world normal
plus constant roughness. Formats live in the pass headers, not scattered through creation code. These are not final material formats.

SDL performs transitions and safe deferred releases. Clear targets cycle at frame start; the GBuffer's depth load does not cycle.
Resize creates replacement targets before releasing old ones. Shutdown waits for GPU idle, releases resources and the window claim,
then destroys the device. There are no per-frame idle waits, manual barriers, frame-fence rings, or deletion queues.

## Geometry and coordinates

MeshAsset remains CPU-only geometry. First use uploads position/normal to a private 24-byte GPU vertex layout and preserves the
source's UInt16/UInt32 indices and primitive boundaries. Upload accepts separate or interleaved Float3 attributes; unused UV/tangent
data is not uploaded. Missing normals use fragment derivatives. This packing is renderer-local, not a change to the runtime mesh format.

The cache identifies immutable MeshAsset versions by their addresses. Repeated instances share uploads; a successful asset replacement
gets a new GPU entry. CPU mesh identities must remain alive and immutable until compositor destruction, matching the current resident
artifact-cache lifetime. The Scene/application resolves stable AssetHandles to those borrows; GPU pointers never enter AssetFactory.
Old GPU versions remain resident in V1. Cache lookup is linear; arrays grow in blocks of 128. Warm frames allocate no engine-owned
heap storage. Eviction and a larger-scale lookup policy are deferred.

World data stays right-handed with +Y up. Matrices are column-major and act on column vectors. The supplied projection must produce
SDL GPU clip coordinates (+Y up, depth 0..1); SDL handles backend coordinate differences. No importer axis conversion is added.
Normals use inverse-transpose direction, including non-uniform/mirrored transforms. Rasterization is temporarily two-sided.

## Build and verification

Build requires DXC; Apple builds additionally require SPIRV-Cross. Three HLSL shaders compile offline to SPIR-V, plus MSL on Apple
or DXIL on Windows. Generated files live in `bin/shaders` beside the runtime. Metal compiles the MSL through SDL at initialization.
No runtime shader compiler library, permutations, reflection system, or shader asset type is added.

`ORBIT_BUILD_TESTS=ON` enables existing CPU tests. Also set `ORBIT_BUILD_GPU_TESTS=ON` for `rendering_compositor`, which requires a
real GPU and window system. It checks pixel readback, both GBuffer attachments, prepass depth reuse, camera/world/normal transforms,
primitive boundaries, interleaved/optional attributes, both index widths, cache reuse, resize, partial initialization, and shutdown.

Metal GPU tests passed in a non-ASan debug build with Metal API Validation enabled. On this machine the existing process-wide rpmalloc override conflicts with ASan's
delete interceptor inside Apple's Metal/AppKit initialization, before compositor creation; allocator policy was not changed.
CPU foundation/asset sanitizer tests remain separate. Other GPU backends still need device testing.

GTAO, lighting, bloom, TAA, tone mapping, UI, materials, visibility, Scene/ECS, and a render graph remain unimplemented.
