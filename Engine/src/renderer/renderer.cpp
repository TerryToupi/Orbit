#include "src/renderer/renderer.h"

namespace Engine
{
	void Renderer::SetUp()
    {
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
    }
}
