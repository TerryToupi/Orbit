#include "src/renderer/renderer.h"

namespace Engine
{
	void Renderer::SetUp()
    {
		// Main pass
		m_mainPassLayout = ResourceManager::instance->createRenderPassLayout({
			.debugName = "main-renderpass-layout",
			.depthTargetFormat = TextureFormat::D32_FLOAT,
			.subPasses = {
				{ .depthTarget = true, .colorTargets = 1, },
			},
		});
		m_mainPass = ResourceManager::instance->createRenderPass({
			.debugName = "main-renderpass",
			.layout = m_mainPassLayout,
			.depthTarget = {
				.loadOp = LoadOperation::CLEAR,
				.storeOp = StoreOperation::STORE,
				.stencilLoadOp = LoadOperation::DONT_CARE,
				.stencilStoreOp = StoreOperation::DONT_CARE,
				.prevUsage = TextureLayout::UNDEFINED,
				.nextUsage = TextureLayout::DEPTH_STENCIL,
			},
			.colorTargets = {
				{
					.loadOp = LoadOperation::CLEAR,
					.storeOp = StoreOperation::STORE,
					.prevUsage = TextureLayout::UNDEFINED,
					.nextUsage = TextureLayout::RENDER_ATTACHMENT,
				},
			},
		}); 

		// UI pass
		m_uiPassLayout = ResourceManager::instance->createRenderPassLayout({
			.debugName = "imgui-renderpass-layout",
			.depthTargetFormat = TextureFormat::D32_FLOAT,
			.subPasses = {
				{.depthTarget = true, .colorTargets = 1, },
			},
		});
		m_uiPass = ResourceManager::instance->createRenderPass({
			.debugName = "imgui-renderpass",
			.layout = m_uiPassLayout,
			.depthTarget = {
				.loadOp = LoadOperation::LOAD,
				.storeOp = StoreOperation::STORE,
				.stencilLoadOp = LoadOperation::DONT_CARE,
				.stencilStoreOp = StoreOperation::DONT_CARE,
				.prevUsage = TextureLayout::DEPTH_STENCIL,
				.nextUsage = TextureLayout::DEPTH_STENCIL,
			},
			.colorTargets = {
				{
					.loadOp = LoadOperation::LOAD,
					.storeOp = StoreOperation::STORE,
					.prevUsage = TextureLayout::RENDER_ATTACHMENT,
					.nextUsage = TextureLayout::PRESENT,
				},
			},
		});

		m_mainColor = ResourceManager::instance->createTexture({
			.debugName = "main-color",
			.dimensions = { 1920, 1080, 1 },
			.format = TextureFormat::BGRA8_UNORM,
			.internalFormat = TextureFormat::BGRA8_UNORM,
			.usage = TextureUsage::RENDER_ATTACHMENT,
			.aspect = TextureAspect::COLOR,
		});
		
		m_mainDepth = ResourceManager::instance->createTexture({
			.debugName = "main-depth",
			.dimensions = { m_backBufferWidth, m_backBufferHeight, 1 },
			.format = TextureFormat::D32_FLOAT,
			.internalFormat = TextureFormat::D32_FLOAT,
			.usage = TextureUsage::DEPTH_STENCIL,
			.aspect = TextureAspect::DEPTH
		});

		SetUpFrameBuffers();
    }

	void Renderer::CleanUp()
    {
		ResourceManager::instance->destroyRenderPass(m_mainPass);
		ResourceManager::instance->destroyRenderPassLayout(m_mainPassLayout); 
		ResourceManager::instance->destroyRenderPass(m_uiPass);
		ResourceManager::instance->destroyRenderPassLayout(m_uiPassLayout); 
		ResourceManager::instance->destroyTexture(m_mainColor); 
		ResourceManager::instance->destroyTexture(m_mainDepth);
	}
}
