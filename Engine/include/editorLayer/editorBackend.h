#pragma once 

#include <core/layer.h> 

namespace Engine
{
	class EditorBackend: public Layer
	{ 
	public: 
		EditorBackend();
		virtual ~EditorBackend() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override; 
		virtual void OnUpdate() override;
		virtual void OnEvent(Event& e) override;
		
		void start(); 
		void finish();
	};
}
