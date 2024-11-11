#include "core/layerManager.h"

namespace Engine
{
	LayerManager::~LayerManager()
	{ 
		for (auto layer : m_Layers)
		{ 
			layer->OnDetach(); 
			delete layer;
		}
	} 

	void LayerManager::PushLayer(Layer* layer)
	{ 
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);   
		layer->OnAttach();
		m_LayerInsertIndex++;
	} 

	void LayerManager::PushOverlay(Layer* overlay)
	{ 
		m_Layers.emplace_back(overlay); 
		overlay->OnAttach();
	} 

	void LayerManager::PopLayer(Layer* layer)
	{ 
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	} 

	void LayerManager::PopOverlay(Layer* overlay)
	{ 
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	} 

	std::vector<Layer*>::iterator LayerManager::begin()
	{
		return m_Layers.begin();
	} 

	std::vector<Layer*>::iterator LayerManager::end()
	{
		return m_Layers.end();
	} 

	std::vector<Layer*>::reverse_iterator LayerManager::rbegin()
	{
		return m_Layers.rbegin();
	} 

	std::vector<Layer*>::reverse_iterator LayerManager::rend()
	{
		return m_Layers.rend();
	} 

	std::vector<Layer*>::const_iterator LayerManager::begin() const
	{
		return m_Layers.begin();
	} 

	std::vector<Layer*>::const_iterator LayerManager::end() const
	{
		return m_Layers.end();
	} 

	std::vector<Layer*>::const_reverse_iterator LayerManager::rbegin() const
	{
		return m_Layers.rbegin();
	} 

	std::vector<Layer*>::const_reverse_iterator LayerManager::rend() const
	{
		return m_Layers.rend();
	}
}