#pragma once  

#include <utilities/pools.h>

namespace Engine
{
	class Shader;
	class BindGroup;
	class Texture;
	class Buffer; 

	class ResourceManager
	{
	public: 
		static inline ResourceManager* instance;

	  	virtual ~ResourceManager() = 0;  

		virtual void Init(std::size_t memory_space) = 0;
		virtual void ShutDown() = 0;

		virtual Handle<Shader> createShader() = 0; 
		virtual Handle<BindGroup> createBindgroup() = 0; 
		virtual Handle<Texture> createTexture() = 0; 
		virtual Handle<Buffer> createBuffer() = 0; 

		virtual void destroyShader(Handle<Shader> handle) = 0; 
		virtual void destroyBindgroup(Handle<BindGroup> handle) = 0; 
		virtual void destroyTexture(Handle<Texture> handle) = 0; 
		virtual void destroyBuffer(Handle<Buffer> handle) = 0;  

	private: 
		ArenaAllocator arena;
	};
}
