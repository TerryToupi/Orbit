#include <EditorLayer.h>

namespace Editor {

	EditorLayer::EditorLayer() 
		: Engine::Layer("Editor Layer") 
	{
	}

	//EditorLayer &EditorLayer::Get()
	//{
	//	return *s_Instance;
	//}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::AddComponent(Component *comp)
	{
		this->m_Panels.insert(std::make_pair(comp->GetName(), comp));
	}

	void EditorLayer::Init()
	{
		Component* hierarchy = new Component("Hierarchy", ComponentType::Hierarchy);
		Component* inspector = new Component("Hierarchy", ComponentType::Inspector);
		Component* scene = new Component("Hierarchy", ComponentType::SceneViewer);
		Component* assets = new Component("Hierarchy", ComponentType::AssetManager);
		Component* menu = new Component("Hierarchy", ComponentType::MenuBar);
		Component* console = new Component("Hierarchy", ComponentType::Console);
		//Component* menu = new Component("Hierarchy", ComponentType::MenuBar);
	}


	// Event Callbacks

}
