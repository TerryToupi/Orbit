#pragma once

#include "src/utilities/handles.h"
#include "src/renderer/resources/resourceManager.h" 
#include "src/events/appEvents.h"

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

		virtual void OnResize(WindowResizeEvent& e) = 0; 

		virtual void SetUpFrameBuffers() = 0;

		void SetUp();
		void CleanUp();
		void Render() {}


	protected: 
		// Active buckbuffers / swapchain images
		std::vector<Handle<FrameBuffer>> m_backBuffers;
		uint32_t m_backBufferWidth; 
		uint32_t m_backBufferHeight;

		// in actions render passes / layouts
		Handle<RenderPass> m_mainPass;
		Handle<RenderPassLayout> m_mainPassLayout; 

		// in action textures
		Handle<Texture> m_mainColor; 
		Handle<Texture> m_mainDepth;
	};
}