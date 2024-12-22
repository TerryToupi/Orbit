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
	class VulkanResourceManager final : public ResourceManager
	{ 
	public: 
		virtual ~VulkanResourceManager() = default;

		virtual void Init() override;
		virtual void ShutDown() override;

		virtual SHADER createShader(const ShaderDesc&& desc) override;
		virtual BINDGROUP createBindgroup(const BindGroupDesc&& desc) override;
		virtual BINDGROUPLAYOUT createBindgroupLayout(const BindGroupLayoutDesc&& desc) override;
		virtual TEXTURE createTexture(const TextureDesc&& desc) override;
		virtual SAMPLER createSampler(const SamplerDesc&& desc) override;
		virtual BUFFER createBuffer(const BufferDesc&& desc) override;
		virtual MESH createMesh(const MeshDesc&& desc) override;
		virtual RENDERPASSLAYOUT createRenderPassLayout(const RenderPassLayoutDesc&& desc) override;
		virtual RENDERPASS createRenderPass(const RenderPassDesc&& desc) override;
		virtual FRAMEBUFFER createFrameBuffer(const FrameBufferDesc&& desc) override;

		virtual void destroyShader(SHADER handle) override;
		virtual void destroyBindgroup(BINDGROUP handle) override;
		virtual void destroyBindgroupLayout(BINDGROUPLAYOUT handle) override;
		virtual void destroyTexture(TEXTURE handle) override;
		virtual void destroySampler(SAMPLER handle) override;
		virtual void destroyBuffer(BUFFER handle) override;
		virtual void destroyMesh(MESH handle) override;
		virtual void destroyRenderPassLayout(RENDERPASSLAYOUT handle) override;
		virtual void destroyRenderPass(RENDERPASS handle) override;
		virtual void destroyFrameBuffer(FRAMEBUFFER handle) override;

		GfxVkShader* getShader(SHADER handle);
		GfxVkBindGroup* getBindgroup(BINDGROUP handle);
		GfxVkBindGroupLayout* getBindgroupLayout(BINDGROUPLAYOUT handle);
		GfxVkTexture* getTexture(TEXTURE handle);
		GfxVkSampler* getSampler(SAMPLER handle);
		GfxVkBuffer* getBuffer(BUFFER handle);
		GfxVkMesh* getMesh(MESH handle);
		GfxVkRenderPassLayout* getRenderPassLayout(RENDERPASSLAYOUT handle);
		GfxVkRenderPass* getRenderPass(RENDERPASS handle);
		GfxVkFrameBuffer* getFrameBuffer(FRAMEBUFFER handle);

		SHADER appendShader(const GfxVkShader& shader);
		BINDGROUP appendBindgroup(const GfxVkBindGroup& bindGroup);
		BINDGROUPLAYOUT appendBindgroupLayout(const GfxVkBindGroupLayout& bindGroupLayout);
		TEXTURE appendTexture(const GfxVkTexture& texture);
		SAMPLER appendSampler(const GfxVkSampler& sampler);
		BUFFER appendBuffer(const GfxVkBuffer& buffer);
		MESH appendMesh(const GfxVkMesh& mesh);
		RENDERPASSLAYOUT appendRenderPassLayout(const GfxVkRenderPassLayout& renderPassLayout);
		RENDERPASS appendRenderPass(const GfxVkRenderPass& renderPass);
		FRAMEBUFFER appendFrameBuffer(const GfxVkFrameBuffer& frameBuffer);

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