#pragma once

#include <iostream>
#include "Component.h"
#include "src/core/layer.h"

namespace Editor { 

	class EditorLayer : public Engine::Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override {}
		virtual void OnDetach() override {}
		virtual void OnUpdate(uint64_t ts) override {}
		virtual void OnUIUpdate(uint64_t ts) override {}
		virtual void OnEvent(Engine::Event& e) override {}

	public:
		//static EditorLayer& Get();
		void AddComponent(Component* comp);
		void Init();

	private:
		bool onMouseButtonPress();
		bool onMouseButtonRelease();
		bool onMouseScroll();
		bool onKeyPress();
		bool onKeyRelease(); 
		bool onKeyTap();

	private:
		//static EditorLayer *s_Instance;
		std::unordered_map<std::string, Component*> m_Panels;
	};
}