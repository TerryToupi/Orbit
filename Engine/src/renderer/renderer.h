#pragma once

#include "src/utilities/handles.h"
#include "src/renderer/resources/resourceManager.h"

namespace Engine
{ 
	class Renderer
	{
	public: 
		static inline Renderer* instance;  

		virtual ~Renderer() = default;

		virtual void Init() = 0; 
		virtual void ShutDown() = 0; 

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;

		void SetUp();
		void Render() {}

	private:
		// in actions render passes / layouts
		Handle<RenderPass> m_mainPass;
		Handle<RenderPassLayout> m_mainPassLayout;

		// Active buckbuffers / swapchain images
		std::vector<Handle<FrameBuffer>> m_backBuffers;
	};
}