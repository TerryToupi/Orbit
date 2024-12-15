#pragma once  

#include "src/utilities/pools.h"

#include "src/renderer/resources/bindGroups.h"
#include "src/renderer/resources/bindGroupLayout.h"
#include "src/renderer/resources/buffers.h"
#include "src/renderer/resources/textures.h" 
#include "src/renderer/resources/sampler.h"
#include "src/renderer/resources/shaders.h"
#include "src/renderer/resources/meshes.h" 
#include "src/renderer/resources/renderPassLayouts.h" 
#include "src/renderer/resources/renderPasses.h" 
#include "src/renderer/resources/frameBuffers.h"

namespace Engine
{
	class ResourceManager
	{
	public: 
		static inline ResourceManager* instance;

	  	virtual ~ResourceManager() = default;  

		virtual void Init() = 0;
		virtual void ShutDown() = 0;

		virtual SHADER createShader(const ShaderDesc&& desc) = 0;
		virtual BINDGROUP createBindgroup(const BindGroupDesc&& desc) = 0;
		virtual BINDGROUPLAYOUT createBindgroupLayout(const BindGroupLayoutDesc&& desc) = 0;
		virtual TEXTURE createTexture(const TextureDesc&& desc) = 0;
		virtual SAMPLER createSampler(const SamplerDesc&& desc) = 0;
		virtual BUFFER createBuffer(const BufferDesc&& desc) = 0;
		virtual MESH createMesh(const MeshDesc&& desc) = 0;
		virtual RENDERPASSLAYOUT createRenderPassLayout(const RenderPassLayoutDesc&& desc) = 0;
		virtual RENDERPASS createRenderPass(const RenderPassDesc&& desc) = 0;
		virtual FRAMEBUFFER createFrameBuffer(const FrameBufferDesc&& desc) = 0;

		virtual void destroyShader(SHADER handle) = 0;
		virtual void destroyBindgroup(BINDGROUP handle) = 0;
		virtual void destroyBindgroupLayout(BINDGROUPLAYOUT handle) = 0;
		virtual void destroyTexture(TEXTURE handle) = 0;
		virtual void destroySampler(SAMPLER handle) = 0;
		virtual void destroyBuffer(BUFFER handle) = 0;
		virtual void destroyMesh(MESH handle) = 0;
		virtual void destroyRenderPassLayout(RENDERPASSLAYOUT handle) = 0;
		virtual void destroyRenderPass(RENDERPASS handle) = 0;
		virtual void destroyFrameBuffer(FRAMEBUFFER handle) = 0;
	};
}
