#pragma once

namespace Engine
{ 
	class Renderer
	{
	public: 
		static inline Renderer* instance;  

		virtual ~Renderer() = default;

		void Init() = 0; 
		void ShutDown() = 0;

		void BeginFrame() = 0;
		void EndFrame() = 0; 
		void Present() = 0; 

	private: 

	};
}