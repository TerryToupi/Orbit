#pragma once

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

	private: 

	};
}