#pragma once

#include <vector> 
#include <functional> 

#define RESERVED_FUNCS 48

namespace Engine
{
	class CleanUpQueue
	{
	public:   
		using cleanUpFunc = std::function<void(void)>; 
		
		CleanUpQueue()
		{
			m_fqueue.reserve(RESERVED_FUNCS);
		}
		
		void appendFunction(const cleanUpFunc&& func)
		{
			m_fqueue.emplace_back(func);
		} 

		void Flush()
		{
			for (auto it = m_fqueue.rbegin(); it != m_fqueue.rend(); it++)
			{
				(*it)();
			} 

			m_fqueue.clear();
		}

	private: 
		std::vector<cleanUpFunc> m_fqueue;
	};
}