#pragma once

#include "src/utilities/handles.h"
#include "src/renderer/resources/resourceManager.h" 
#include "src/events/appEvents.h" 
#include "src/renderer/commandBuffer.h"

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

		virtual void BlitToSwapChain(TEXTURE texture) = 0;

		virtual CommandBuffer* BeginCommandRecording(const RenderPassStage stage, const CommandBufferType type) = 0;

		virtual void OnResize(WindowResizeEvent& e) = 0; 

		void SetUp();
		void CleanUp();
		void Render() {}


	protected: 
		// in actions render passes / layouts
		RENDERPASS m_mainPass;
		RENDERPASSLAYOUT m_mainPassLayout;  

		RENDERPASS m_uiPass; 
		RENDERPASSLAYOUT m_uiPassLayout;

		// in action textures
		TEXTURE m_mainColor; 
		TEXTURE m_mainDepth; 
		FRAMEBUFFER m_mainFrameBuffer;
	};
}