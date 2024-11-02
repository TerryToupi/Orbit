#pragma once  

#include <utilities/pools.h> 

#include <renderer/resources/bindGroups.h>
#include <renderer/resources/buffers.h>
#include <renderer/resources/textures.h>
#include <renderer/resources/shaders.h>

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

		virtual void destroyShader(Handle<Shader> handle) = 0; 
		virtual void destroyBindgroup(Handle<BindGroup> handle) = 0; 
		virtual void destroyTexture(Handle<Texture> handle) = 0; 
		virtual void destroyBuffer(Handle<Buffer> handle) = 0;  

	private: 
		ArenaAllocator arena;
	};
}
