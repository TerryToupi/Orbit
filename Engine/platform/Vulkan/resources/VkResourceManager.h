#pragma once

#include "src/renderer/resourceManager.h"  

#include "platform/Vulkan/VulkanCore.h"
#include "platform/Vulkan/resources/VkBindGroups.h"
#include "platform/Vulkan/resources/VkBindGroupLayout.h"
#include "platform/Vulkan/resources/VkBuffers.h"
#include "platform/Vulkan/resources/VkTextures.h"
#include "platform/Vulkan/resources/VkShaders.h"
#include "platform/Vulkan/resources/VkMeshes.h"
#include "platform/Vulkan/resources/VkRenderPassLayout.h"
#include "platform/Vulkan/resources/VkRenderPasses.h"
#include "platform/Vulkan/resources/VkFrameBuffers.h" 

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
		virtual Handle<Buffer> createBuffer(BufferDesc desc) override{return Handle<Buffer>();}
		virtual Handle<Mesh> createMesh(MeshDesc desc) override {return Handle<Mesh>();}
		virtual Handle<RenderPassLayout> createRenderPassLayout(RenderPassLayoutDesc desc) override{return Handle<RenderPassLayout>();}
		virtual Handle<RenderPass> createRenderPass(RenderPassDesc desc) override{return Handle<RenderPass>();}
		virtual Handle<FrameBuffer> createFrameBuffer(FrameBufferDesc desc) override{return Handle<FrameBuffer>();}

		virtual void destroyShader(Handle<Shader> handle) override{}
		virtual void destroyBindgroup(Handle<BindGroup> handle)override{}
		virtual void destroyBindgroupLaout(Handle<BindGroupLayout> handle) override{}
		virtual void destroyTexture(Handle<Texture> handle) override{}
		virtual void destroyBuffer(Handle<Buffer> handle) override{}
		virtual void destroyMesh(Handle<Mesh> handle) override{}
		virtual void destroyRenderPassLayout(Handle<RenderPassLayout> handle) override{}
		virtual void destroyRenderPass(Handle<RenderPass> handle) override{}
		virtual void destroyFrameBuffer(Handle<FrameBuffer> handle) override{}

	private:
		void CreateMemoryAllocator();

	private: 
		Pool<VkShader, Shader> m_shaders{32, "vkShaders"};
		Pool<VkBindGroup, BindGroup> m_bindGroups{32, "VkBindGroups"};
		Pool<VkBindGroupLayout, BindGroupLayout> m_bindGroupLayout{32, "VkBindGroupLayouts"};
		Pool<VkTexture, Texture> m_textures{32, "VkTextures"};
		Pool<VkBuffer, Buffer> m_buffers{32, "VkBuffers"};
		Pool<VkRenderPassLayout, RenderPassLayout> m_renderPassLayouts{32, "VkRenderPassLayout"};
		Pool<VkRenderPass, RenderPass> m_renderPasses{32, "VkRenderPasses"};
		Pool<VkFrameBuffer, FrameBuffer> m_frameBuffers{32, "VkFrameBuffers"};

	private: 
		VmaAllocator m_allocator;
	}; 

}