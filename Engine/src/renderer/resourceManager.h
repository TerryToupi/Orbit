#pragma once  

#include "src/utilities/pools.h"

#include "src/renderer/resources/bindGroups.h"
#include "src/renderer/resources/bindGroupLayout.h"
#include "src/renderer/resources/buffers.h"
#include "src/renderer/resources/textures.h"
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

		virtual Handle<Shader> createShader(ShaderDesc desc) = 0; 
		virtual Handle<BindGroup> createBindgroup(BindGroupDesc desc) = 0; 
		virtual Handle<BindGroupLayout> createBindgroupLayout(BindGroupLayoutDesc desc) = 0;
		virtual Handle<Texture> createTexture(TextureDesc desc) = 0; 
		virtual Handle<Buffer> createBuffer(BufferDesc desc) = 0;  
		virtual Handle<Mesh> createMesh(MeshDesc desc) = 0; 
		virtual Handle<RenderPassLayout> createRenderPassLayout(RenderPassLayoutDesc desc) = 0; 
		virtual Handle<RenderPass> createRenderPass(RenderPassDesc desc) = 0;
		virtual Handle<FrameBuffer> createFrameBuffer(FrameBufferDesc desc) = 0;

		virtual void destroyShader(Handle<Shader> handle) = 0; 
		virtual void destroyBindgroup(Handle<BindGroup> handle) = 0; 
		virtual void destroyBindgroupLaout(Handle<BindGroupLayout> handle) = 0; 
		virtual void destroyTexture(Handle<Texture> handle) = 0; 
		virtual void destroyBuffer(Handle<Buffer> handle) = 0;  
		virtual void destroyMesh(Handle<Mesh> handle) = 0; 
		virtual void destroyRenderPassLayout(Handle<RenderPassLayout> handle) = 0;
		virtual void destroyRenderPass(Handle<RenderPass> handle) = 0;  
		virtual void destroyFrameBuffer(Handle<FrameBuffer> handle) = 0;
	};
}
