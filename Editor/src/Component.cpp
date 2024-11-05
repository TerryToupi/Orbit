#include "Component.h"

namespace Editor 
{
	Component::Component(std::string name, ComponentType type)
	{
		this->m_name = name;
		this->m_type = type;
		this->m_closable = false;
	}

	void Component::SetClosable(bool closable)
	{
		this->m_closable = closable;
	}

	std::string Component::GetName() const
	{
		return this->m_name;
	}

	ComponentType Component::GetType() const
	{
		return this->m_type;
	}

	bool Component::IsClosable() const
	{
		return this->m_closable;
	}
}