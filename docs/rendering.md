# Rendering compositor

`AssetID` resolves once through `request_render_mesh` to a generational handle. `RenderAssetCache` uses its factory slot for direct
access to persistent `GpuMesh` buffers and primitive ranges. Resolve one mesh pointer per draw before rendering; DepthPrepass and
GBufferPass reuse that pointer through `GeometryDraw`. `MeshAsset`, importers and `AssetFactory` remain independent of SDL GPU.

The application owns the render asset cache and `GPUTransferQueue`; the compositor owns the SDL device, window claim, targets,
shaders and concrete passes. The frame sequence is change dispatch → upload preparation → draw resolution → compositor render →
asset publication. Cache mutation/preparation precedes draw resolution; draw pointers are borrowed only through frame submission.

The cache processes coalesced factory change notifications and queues only requested, changed mesh content. States are Missing,
PendingUpload and Ready. A pending replacement retains the previous ready mesh; an initial pending upload resolves to null and is
skipped. Upload preparation creates candidate buffers and packs positions/normals directly into staging once per changed asset.
CPU streams and GPU vertex layout remain separate. Indices retain their original width; draws never convert or upload geometry.

`GPUTransferQueue` owns one reusable SDL upload transfer buffer and a persistent array of copy requests. Preparation reserves the
whole batch, maps once with cycling enabled, and packs four-byte-aligned ranges for all meshes. Capacity grows in 64 KiB increments
only when the batch exceeds it, and never shrinks. Encoding unmaps once and records one copy pass containing all buffer uploads.
An empty batch does no mapping or allocation. The queue is logical pending work, with no backend queue or semaphore abstraction.

`RenderingCompositor::render(transfers, draws, view_projection)` independently acquires transfer and rendering command buffers when
uploads are pending. The transfer buffer records copy work; the rendering buffer records DepthPrepass, GBufferPass and the existing
debug color blit. Both are encoded before one transfer submission followed by one render submission, with no CPU fence wait between
them. Empty upload batches acquire/submit only rendering. While shaders or the swapchain are unavailable, uploads still submit
and rendering submits empty.
Swapchain frame pacing remains SDL-owned. Command buffers are never retained after submission or cancellation.

After submissions, `finish_render_assets` publishes successful candidates for the next frame and releases old buffers through SDL's
deferred release semantics. Creation, packing, staging or cancelled/failed transfer submission preserves the previous valid mesh.
The last attempted source hash suppresses repeated failed uploads until content changes. `asset_unload` invalidates handles and its
change notification removes the GPU representation, including pending replacements. Immutable CPU artifact versions remain cache-owned.
Shutdown waits for GPU idle once, destroys the asset cache and staging queue, then destroys the compositor/device and CPU caches.

Targets persist until the acquired pixel size changes; resize creates replacements before releasing old targets and leaves pipelines
untouched. Shader, mesh and target replacement introduce no normal-frame GPU idle waits. Readback tests alone use fences.

## Shader reload

Shaders use the existing File Stream → Artifact Cache → Content Cache path. The application dispatches `ArtifactEvent` completions
to `rendering_process_event` alongside asset events and calls `rendering_shaders_tick` outside rendering. Initial loading is asynchronous;
rendering starts after the first valid pipeline pair. Initial load failure is reported by the application.

This repository's file infrastructure supports explicit refresh and generation notifications, **not an autonomous watcher**.
The application requests shader and preview-mesh refresh every 500 ms through that existing mechanism. File metadata checks and
reads run on its server lanes; there is no renderer file watcher or direct shader IO. Unchanged requests reuse cached bytes.
Command-stream backpressure defers submission to a later tick.

After all three file results arrive, changed shader bytes create replacement SDL shaders. A geometry vertex change rebuilds both
pipelines together; each fragment change affects only its pass. All replacement resources must succeed before live handles change.
Missing files, failed shader compilation, and failed pipeline creation retain the previous rendering state. Failed GPU candidates
are released, and identical failed content is not compiled repeatedly. Correcting a file enables another attempt.

Build requires DXC and, on Apple, SPIRV-Cross. After editing HLSL, rebuild `orbit_render_shaders` while the application runs. Outputs
in `bin/shaders` are published by rename only after successful compilation; runtime refresh then detects them. The runtime loads
SPIR-V, DXIL or MSL through the same content path, with no extra byte copy. Reload preserves the fixed shader ABI: `TEXCOORD0/1`
vertex attributes, camera/world uniform buffers at `b0/b1, space1`, two vertex uniforms, and no fragment resources. ABI changes
require matching C++ changes and a rebuild. No runtime compiler, reflection framework or second filesystem is introduced.

## Math and coordinates

Spatial math uses the narrow Box3D math header: `b3Vec3`, `b3Quat`, `b3Transform`, `b3Matrix3`, and `b3AABB` mesh bounds. ORbit owns
only `Mat4` plus transform, view, perspective and multiplication functions. Packed imported vertex components and source node
matrices remain import data, not a second spatial math API.

World coordinates are right-handed, in meters, with +Y up and angles in radians. glTF already uses these units and axes. Local scale
precedes rotation and translation; camera forward is local -Z. Callers compute world/view matrices when their sources change.
The preview changes projection only on pixel-size changes. There is no matrix reconstruction or conversion in draw loops.

Projection maps view-space -near/-far to SDL GPU depth 0/1, retaining NDC +Y up. SDL supplies top-left viewport/texture origins and
backend conversion; no Vulkan Y flip is applied. The prepass clears depth to 1 and uses LESS; GBuffer loads that depth and uses EQUAL
without writing it. Both passes share the same vertex shader. Counterclockwise fronts and two-sided rasterization are intentional.
Normals use inverse-transpose direction, including nonuniform and mirrored transforms; missing normals use fragment derivatives.
The final blit uses SDL's texture orientation directly.

## Verification and limits

`ORBIT_BUILD_TESTS=ON` enables CPU tests, including transform composition, camera inversion and near/far projection checks.
`ORBIT_BUILD_GPU_TESTS=ON` adds real GPU readback, depth reuse, normals, both index widths, optional/interleaved attributes, resize,
shader reload/recovery, batched mesh uploads, staging growth/cycling, unload, replacement cancellation and shutdown checks.
Shader/compiler and pipeline-link failure injection runs on Metal. Tests and generated test shaders remain outside `runtime/`.

Ordinary ORbit rendering performs no heap allocation, asset searches, shader IO, pipeline creation or matrix copies into temporary
draw arrays. Camera data is pushed once per frame. SDL/backend allocation and lazy resource cycling remain implementation-owned;
allocator measurements include those calls. The asset upload/cache path adds no allocations on unchanged frames. The existing
`assets_tick` still scans factory slots, and the shared content/artifact caches retain old file versions until shutdown.
Metal has been exercised locally; Vulkan and D3D12 still require device testing. No rendering features were added.
