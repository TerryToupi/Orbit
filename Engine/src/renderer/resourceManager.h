#pragma once  

#include "utilities/pools.h"

#include "renderer/resources/bindGroups.h"
#include "renderer/resources/buffers.h"
#include "renderer/resources/textures.h"
#include "renderer/resources/shaders.h" 
#include "renderer/resources/Meshes.h" 
#include "renderer/resources/RenderPassLayouts.h" 
#include "renderer/resources/RenderPasses.h" 
#include "renderer/resources/FrameBuffers.h"

namespace Engine
{
	class ResourceManager
	{
	public: 
		static inline ResourceManager* instance;

	  	virtual ~ResourceManager() = 0;  

		virtual void Init(std::size_t memory_space) = 0;
		virtual void ShutDown() = 0;

		virtual Handle<Shader> createShader(ShaderDesc desc) = 0; 
		virtual Handle<BindGroup> createBindgroup(BindGroupDesc desc) = 0; 
		virtual Handle<Texture> createTexture(TextureDesc desc) = 0; 
		virtual Handle<Buffer> createBuffer(BufferDesc desc) = 0;  
		virtual Handle<Mesh> createMesh(MeshDesc desc) = 0; 
		virtual Handle<RenderPassLayout> createRenderPassLayout(RenderPassLayoutDesc desc) = 0; 
		virtual Handle<RenderPass> createRenderPass(RenderPassDesc desc) = 0;
		virtual Handle<FrameBuffer> createFrameBuffer(FrameBufferDesc desc) = 0;

		virtual void destroyShader(Handle<Shader> handle) = 0; 
		virtual void destroyBindgroup(Handle<BindGroup> handle) = 0; 
		virtual void destroyTexture(Handle<Texture> handle) = 0; 
		virtual void destroyBuffer(Handle<Buffer> handle) = 0;  
		virtual void destroyMesh(Handle<Mesh> handle) = 0; 
		virtual void destroyRenderPassLayout(Handle<RenderPassLayout> handle) = 0;
		virtual void destroyRenderPass(Handle<RenderPass> handle) = 0;  
		virtual void destroyFrameBuffer(Handle<FrameBuffer> handle) = 0;

	private: 
		ArenaAllocator m_arena;
	};
}
