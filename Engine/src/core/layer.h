#pragma once 

#include "src/core/core.h"
#include "src/events/event.h"

namespace Engine
{
	class Layer
	{
	public:
		Layer(const std::string& name);
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {} 
		virtual void OnStart()	{}
		virtual void OnUpdate() {}
		virtual void OnImguiRender() {}
		virtual void OnEvent(Event& e) {}

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
} 

