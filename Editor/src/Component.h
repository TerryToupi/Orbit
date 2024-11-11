#pragma once

#include <iostream>

namespace Editor 
{

	enum class ComponentType 
	{
		Inspector,
		Hierarchy,
		AssetManager,
		Console,
		SceneViewer,
		GameViwer,
		MenuBar
	};

	class Component 
	{
	public:
		Component(std::string name, ComponentType type);
		~Component();

		//virtual void onStart();
		//virtual void onEnd();
		//virtual void onUpdate();
		//virtual void onRender();

	public:
		void SetClosable(bool closable);
		std::string GetName() const;
		ComponentType GetType() const;

	public:
		bool IsClosable() const;

	private:
		std::string m_name;
		bool m_closable;
		ComponentType m_type;
	};
}
