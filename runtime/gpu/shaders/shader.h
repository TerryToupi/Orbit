#ifndef __ORBIT_GPU_SHADERS__
#define __ORBIT_GPU_SHADERS__

#include <base/base.hpp>

/* Compiled shader blobs, embedded as byte arrays.
 *
 * The engine never compiles a shader. Sources live outside the runtime and are
 * cross-compiled at build time into whatever the target backend consumes --
 * SPIR-V for Vulkan, DXIL for D3D12, MSL or metallib for Metal -- and land here
 * as arrays that gpu.cpp hands straight to SDL_CreateGPUShader. No shader
 * compiler is linked into the binary.
 *
 * Shape once the tooling exists, one pair per backend format:
 *
 *     static const u8 kTriangleVert[] = { ... };
 *     static const u8 kTriangleFrag[] = { ... };
 *
 * TODO(terry, 2026-09-07): fill these in with the build-time cross-compile
 * step. Empty until then -- placeholder bytecode would only ever fail at
 * SDL_CreateGPUShader, and later than it should.
 */

#endif
