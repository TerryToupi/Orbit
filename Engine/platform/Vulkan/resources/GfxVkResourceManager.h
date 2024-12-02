#pragma once

#include "src/renderer/resources/resourceManager.h"  

#include "platform/Vulkan/GfxVulkanCore.h"
#include "platform/Vulkan/resources/GfxVkBindGroups.h"
#include "platform/Vulkan/resources/GfxVkBindGroupLayout.h"
#include "platform/Vulkan/resources/GfxVkBuffers.h"
#include "platform/Vulkan/resources/GfxVkTextures.h" 
#include "platform/Vulkan/resources/GfxVkSamplers.h" 
#include "platform/Vulkan/resources/GfxVkShaders.h"
#include "platform/Vulkan/resources/GfxVkMeshes.h"
#include "platform/Vulkan/resources/GfxVkRenderPassLayout.h"
#include "platform/Vulkan/resources/GfxVkRenderPasses.h"
#include "platform/Vulkan/resources/GfxVkFrameBuffers.h" 

namespace Engine
{
	class VkResourceManager final : public ResourceManager
	{ 
	public: 
		virtual ~VkResourceManager() = default;

		virtual void Init() override;
		virtual void ShutDown() override;

		virtual Handle<Shader> createShader(ShaderDesc desc) override { return Handle<Shader>(); }
		virtual Handle<BindGroup> createBindgroup(BindGroupDesc desc) override{return Handle<BindGroup>();}
		virtual Handle<BindGroupLayout> createBindgroupLayout(BindGroupLayoutDesc desc) override{return Handle<BindGroupLayout>();}
		virtual Handle<Texture> createTexture(TextureDesc desc) override{return Handle<Texture>();}
		virtual Handle<Sampler> createSampler(SamplerDesc) override{return Handle<Sampler>();}
		virtual Handle<Buffer> createBuffer(BufferDesc desc) override{return Handle<Buffer>();}
		virtual Handle<Mesh> createMesh(MeshDesc desc) override {return Handle<Mesh>();}
		virtual Handle<RenderPassLayout> createRenderPassLayout(RenderPassLayoutDesc desc) override{return Handle<RenderPassLayout>();}
		virtual Handle<RenderPass> createRenderPass(RenderPassDesc desc) override{return Handle<RenderPass>();}
		virtual Handle<FrameBuffer> createFrameBuffer(FrameBufferDesc desc) override{return Handle<FrameBuffer>();}

		virtual void destroyShader(Handle<Shader> handle) override{}
		virtual void destroyBindgroup(Handle<BindGroup> handle)override{}
		virtual void destroyBindgroupLayout(Handle<BindGroupLayout> handle) override{}
		virtual void destroyTexture(Handle<Texture> handle) override{}
		virtual void destroySampler(Handle<Sampler> handle) override{}
		virtual void destroyBuffer(Handle<Buffer> handle) override{}
		virtual void destroyMesh(Handle<Mesh> handle) override{}
		virtual void destroyRenderPassLayout(Handle<RenderPassLayout> handle) override{}
		virtual void destroyRenderPass(Handle<RenderPass> handle) override{}
		virtual void destroyFrameBuffer(Handle<FrameBuffer> handle) override{} 

		GfxVkShader* getShader(Handle<Shader> handle) { return nullptr; }
		GfxVkBindGroup* getBindgroup(Handle<BindGroup> handle) { return nullptr; }
		GfxVkBindGroupLayout* getBindgroupLayout(Handle<BindGroupLayout> handle) { return nullptr; }
		GfxVkTexture* getTexture(Handle<Texture> handle) { return nullptr; }
		GfxVkSampler* getSampler(Handle<Sampler> handle) { return nullptr; }
		GfxVkBuffer* getBuffer(Handle<Buffer> handle) { return nullptr; }
		GfxVkMesh* getMesh(Handle<Mesh> handle) { return nullptr; }
		GfxVkRenderPassLayout* getRenderPassLayout(Handle<RenderPassLayout> handle) { return nullptr; }
		GfxVkRenderPass* getRenderPass(Handle<RenderPass> handle) { return nullptr; }
		GfxVkFrameBuffer* getFrameBuffer(Handle<FrameBuffer> handle) { return nullptr; }

		VmaAllocator& GetVmaAllocator() { return m_allocator; }

	private:
		void CreateMemoryAllocator();

	private: 
		Pool<GfxVkShader, Shader> m_shaders{ 32, "GfxvkShaders" };
		Pool<GfxVkBindGroup, BindGroup> m_bindGroups{ 32, "GfxVkBindGroups" };
		Pool<GfxVkBindGroupLayout, BindGroupLayout> m_bindGroupLayout{ 32, "GfxVkBindGroupLayouts" };
		Pool<GfxVkTexture, Texture> m_textures{ 32, "GfxVkTextures" }; 
		Pool<GfxVkSampler, Sampler> m_samplers{ 32, "GfxVkSampler" };
		Pool<GfxVkBuffer, Buffer> m_buffers{ 32, "GfxVkBuffers" };
		Pool<GfxVkRenderPassLayout, RenderPassLayout> m_renderPassLayouts{ 32, "GfxVkRenderPassLayout" };
		Pool<GfxVkRenderPass, RenderPass> m_renderPasses{ 32, "GfxVkRenderPasses" };
		Pool<GfxVkFrameBuffer, FrameBuffer> m_frameBuffers{ 32, "GfxVkFrameBuffers" }; 

	private: 
		VmaAllocator m_allocator;
	}; 

}