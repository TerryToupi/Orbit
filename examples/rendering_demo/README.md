# Asset-to-GPU rendering demo

```sh
cmake -S . -B build -DORBIT_BUILD_EXAMPLES=ON
cmake --build build --target rendering_demo
./build/bin/rendering_demo
```

DXC is required; Apple builds also require SPIRV-Cross, as for the runtime. Press Escape or close the window to quit.
The executable finds its bundled asset and shaders beside itself, independent of the working directory.

`assets/ChronographWatch.glb` loads asynchronously through AssetFactory using named mesh selections. RenderAssetCache
uploads the watch's 13 meshes through reusable SDL transfer staging. Imported node transforms assemble the parts;
scene bounds center and scale the watch, which rotates once every 20 seconds. The hand animation is not played.

DepthPrepass and GBufferPass share the resident meshes. The display uses `abs(world_normal) * 0.6 + 0.2` against a dark
background; source materials, textures and glass transparency are not rendered.

Edit `runtime/render/shaders/gbuffer.frag.hlsl`, then run `cmake --build build --target orbit_render_shaders` while the demo runs.
The existing artifact/file refresh path checks every 500 ms and replaces affected shaders/pipelines after successful compilation.
Replace `build/bin/rendering_demo_assets/ChronographWatch.glb` with an updated GLB preserving the mesh names to exercise mesh reload.
The same refresh path reimports changed bytes, uploads a replacement, and preserves the previous GPU mesh on failure.
Optional arguments select an asset root containing `ChronographWatch.glb` and a compiled shader directory:
`rendering_demo [asset-root] [shader-directory]`.

Upload frames submit one transfer command buffer before one render command buffer, with no CPU wait between them.
Steady frames submit only rendering. The demo adds no per-frame heap allocation; SDL/backend allocations and periodic
asynchronous refresh work remain. Window events update projection; the compositor owns target resizing.
Shutdown drains the GPU once before releasing mesh/staging resources, compositor resources, device and window.

With `ORBIT_BUILD_TESTS=ON` and `ORBIT_BUILD_GPU_TESTS=ON`, build `rendering_demo_tests` and run
`ctest --test-dir build -R '^rendering_demo$' --output-on-failure`. It drives the actual demo callbacks, verifies GPU output,
resource persistence, shader/mesh reload and repeated resizing, and writes `build/tests/rendering_demo.bmp`.
