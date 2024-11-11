#pragma once 

#include "core/core.h" 
#include "core/layer.h"

namespace Engine
{
	class LayerManager
	{
	public:
		LayerManager() = default;
		~LayerManager();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator				begin();
		std::vector<Layer*>::iterator				end();
		std::vector<Layer*>::reverse_iterator		rbegin();
		std::vector<Layer*>::reverse_iterator		rend();

		std::vector<Layer*>::const_iterator			begin()		const;
		std::vector<Layer*>::const_iterator			end()		const; 
		std::vector<Layer*>::const_reverse_iterator rbegin()	const;
		std::vector<Layer*>::const_reverse_iterator rend()		const; 

	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}
