#pragma once

#include <iostream>
#include <core/layer.h> 

namespace Editor { 

	class EditorLayer : public Engine::Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override {}
		virtual void OnDetach() override {}
		virtual void OnUpdate() override {}
		virtual void OnEditorRender() override {}
		virtual void OnEvent(Engine::Event& e) override {}

	private:
		bool onMouseButtonPress();
		bool onMouseButtonRelease();
		bool onMouseScroll();
		bool onKeyPress();
		bool onKeyRelease(); 
		bool onKeyTap();

	private: 
		
	};
}