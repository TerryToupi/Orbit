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

		virtual Handle<Shader> createShader(const ShaderDesc&& desc) override;
		virtual Handle<BindGroup> createBindgroup(const BindGroupDesc&& desc) override;
		virtual Handle<BindGroupLayout> createBindgroupLayout(const BindGroupLayoutDesc&& desc) override;
		virtual Handle<Texture> createTexture(const TextureDesc&& desc) override;
		virtual Handle<Sampler> createSampler(const SamplerDesc&& desc) override;
		virtual Handle<Buffer> createBuffer(const BufferDesc&& desc) override;
		virtual Handle<Mesh> createMesh(const MeshDesc&& desc) override;
		virtual Handle<RenderPassLayout> createRenderPassLayout(const RenderPassLayoutDesc&& desc) override;
		virtual Handle<RenderPass> createRenderPass(const RenderPassDesc&& desc) override;
		virtual Handle<FrameBuffer> createFrameBuffer(const FrameBufferDesc&& desc) override;

		virtual void destroyShader(Handle<Shader> handle) override;
		virtual void destroyBindgroup(Handle<BindGroup> handle) override;
		virtual void destroyBindgroupLayout(Handle<BindGroupLayout> handle) override;
		virtual void destroyTexture(Handle<Texture> handle) override;
		virtual void destroySampler(Handle<Sampler> handle) override;
		virtual void destroyBuffer(Handle<Buffer> handle) override;
		virtual void destroyMesh(Handle<Mesh> handle) override;
		virtual void destroyRenderPassLayout(Handle<RenderPassLayout> handle) override;
		virtual void destroyRenderPass(Handle<RenderPass> handle) override;
		virtual void destroyFrameBuffer(Handle<FrameBuffer> handle) override; 

		GfxVkShader* getShader(Handle<Shader> handle);
		GfxVkBindGroup* getBindgroup(Handle<BindGroup> handle);
		GfxVkBindGroupLayout* getBindgroupLayout(Handle<BindGroupLayout> handle);
		GfxVkTexture* getTexture(Handle<Texture> handle);
		GfxVkSampler* getSampler(Handle<Sampler> handle);
		GfxVkBuffer* getBuffer(Handle<Buffer> handle);
		GfxVkMesh* getMesh(Handle<Mesh> handle);
		GfxVkRenderPassLayout* getRenderPassLayout(Handle<RenderPassLayout> handle);
		GfxVkRenderPass* getRenderPass(Handle<RenderPass> handle);
		GfxVkFrameBuffer* getFrameBuffer(Handle<FrameBuffer> handle); 

		Handle<Shader> appendShader(const GfxVkShader& shader);
		Handle<BindGroup> appendBindgroup(const GfxVkBindGroup& bindGroup);
		Handle<BindGroupLayout> appendBindgroupLayout(const GfxVkBindGroupLayout& bindGroupLayout);
		Handle<Texture> appendTexture(const GfxVkTexture& texture);
		Handle<Sampler> appendSampler(const GfxVkSampler& sampler);
		Handle<Buffer> appendBuffer(const GfxVkBuffer& buffer);
		Handle<Mesh> appendMesh(const GfxVkMesh& mesh);
		Handle<RenderPassLayout> appendRenderPassLayout(const GfxVkRenderPassLayout& renderPassLayout);
		Handle<RenderPass> appendRenderPass(const GfxVkRenderPass& renderPass);
		Handle<FrameBuffer> appendFrameBuffer(const GfxVkFrameBuffer& frameBuffer);

		VmaAllocator& GetVmaAllocator() { return m_allocator; }

	private:
		void CreateMemoryAllocator();

	private: 
		Pool<GfxVkShader, Shader> m_shaders{ 32, "GfxvkShaders" };
		Pool<GfxVkBindGroup, BindGroup> m_bindGroups{ 32, "GfxVkBindGroups" };
		Pool<GfxVkBindGroupLayout, BindGroupLayout> m_bindGroupLayouts{ 32, "GfxVkBindGroupLayouts" };
		Pool<GfxVkTexture, Texture> m_textures{ 32, "GfxVkTextures" }; 
		Pool<GfxVkSampler, Sampler> m_samplers{ 32, "GfxVkSampler" };
		Pool<GfxVkBuffer, Buffer> m_buffers{ 32, "GfxVkBuffers" };
		Pool<GfxVkMesh, Mesh> m_meshes { 32, "GfxVkMeshes" };
		Pool<GfxVkRenderPassLayout, RenderPassLayout> m_renderPassLayouts{ 32, "GfxVkRenderPassLayout" };
		Pool<GfxVkRenderPass, RenderPass> m_renderPasses{ 32, "GfxVkRenderPasses" };
		Pool<GfxVkFrameBuffer, FrameBuffer> m_frameBuffers{ 32, "GfxVkFrameBuffers" }; 

	private: 
		VmaAllocator m_allocator;
	}; 

}