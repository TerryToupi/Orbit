#pragma once 

#include <core/core.h> 

namespace Engine
{   
	template<typename T>
	class Handle
	{
	public:
		Handle() : m_index(0), m_generation(0) {} 

		bool IsValid() const { return m_generation != 0; }
		bool operator==(const Handle<T>& other) const { return other.m_index == m_index && other.m_generation == m_generation; }
		bool operator!=(const Handle<T>& other) const { return other.m_index != m_index || other.m_generation != m_generation; }

		static Handle sentinal() { return Handle(0, 0xffffffff); } 

		uint64_t hashKey() const { return (((uint64_t)m_index) << 32) + (uint64_t)m_generation; } 
		uint32_t index()  const { return m_index; }

	private:
		Handle(uint32_t index, uint32_t generation) : m_index(index), m_generation(generation) {}

	private:
		uint32_t m_index;
		uint32_t m_generation;  

		template<typename U, typename V> friend class Pool;
	};
}
