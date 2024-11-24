#pragma once 

#include "src/core/assert.h"
#include "src/core/core.h"
#include "src/utilities/handles.h"
#include "src/utilities/arena.h" 

#include <stack>  
#include <string>
#include <stdlib.h>

namespace Engine
{
	template<typename U, typename V> class Pool
	{
	public: 
		Pool(uint32_t reserveSize, std::string debugName)
			: m_size(reserveSize), m_debugName(debugName)
		{  
			m_data = (U*)malloc(sizeof(U) * m_size);

			memset(m_data, 0, sizeof(U) * m_size);

			m_freeList.reserve(m_size);
			m_generation.reserve(m_size); 
	
			for (uint32_t i = 0; i < m_size; i++)  
			{ 
				m_generation.emplace_back(1);
			}  

			for (uint32_t i = 0; i < m_size; i++)
			{   
				uint32_t offset = (m_size - (i + 1));
				m_freeList.emplace_back(offset); 
			} 
		} 

		Handle<V> Insert(const U& data)
		{ 
			// TODO: assert error
			//if (m_freeList.empty())
			//	Reallocate();  

			//if (m_freeList.empty())
			//	ENGINE_ASSERT(false, "Pool {} out of memory space!", m_debugName);

			uint32_t index = m_freeList.back();
			m_freeList.pop_back(); 
			
			memmove(&m_data[index], &data, sizeof(U));

			return { index, m_generation[index] };
		} 

		U* Get(Handle<V> handle) const
		{
			if (!handle.IsValid())
			{
				return nullptr;
			}

			if (handle.m_generation != m_generation[handle.m_index])
			{
				return nullptr;
			}

			return &m_data[handle.m_index];
		} 

		void Remove(Handle<V> handle)
		{
			m_generation[handle.m_index]++; 
			m_freeList.emplace_back(handle.m_index);
		}

	private:
		U* m_data;   
		std::string m_debugName;
		std::vector<uint32_t> m_generation;
		std::vector<uint32_t> m_freeList;
		uint32_t m_size;
	};
}
