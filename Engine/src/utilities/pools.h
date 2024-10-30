#pragma once 

#include <core/core.h>   
#include <utilities/handles.h> 
#include <utilities/arena.h> 
#include <stack>

namespace Engine
{
	template<typename U, typename V> class Pool
	{
	public: 
		Pool(ArenaAllocator& arena)
		{ 
			m_data = arena.allocate<U>(sizeof(U) * m_size, 8);

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
		std::vector<uint32_t> m_generation;
		std::vector<uint32_t> m_freeList;
		uint32_t m_size = 32;
	};
}
