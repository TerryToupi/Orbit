#pragma once 

#include <core/core.h>  
#include <core/logs.h>

namespace Engine
{
	inline std::size_t calculate_padding(const std::size_t baseAddress, const std::size_t alignment) {
		const std::size_t aligned_addr = ((baseAddress / alignment) + 1) * alignment;
		const std::size_t padding = aligned_addr - baseAddress;
		return padding;
	}

	class ArenaAllocator
	{
	public:
		ArenaAllocator(const std::size_t totalSize)
			: m_total_size(totalSize)
		{
			m_start_addr = (std::size_t*)malloc(m_total_size);
			m_offset = 0;
		}

		virtual ~ArenaAllocator()
		{
			free(m_start_addr);
			m_start_addr = nullptr;
		}

		template<typename T>
		T* allocate(const std::size_t size, const std::size_t alignment = 0)
		{
			std::size_t padding = 0;

			const std::size_t current_addr = (std::size_t)(m_start_addr + m_offset);
			if (alignment != 0 && m_offset % alignment != 0)
				padding = calculate_padding(current_addr, alignment);

			if (m_offset + padding + size > m_total_size)
			{
				ENGINE_CORE_ERROR("Arena out of memory: m_total:{0}", m_total_size);
				return nullptr;
			}

			const std::size_t next_addr = m_offset + padding;

			m_offset += padding;
			m_offset += size;

			return (T*)(m_start_addr + next_addr);
		}

		void Reset()
		{
			m_offset = 0;
		}

	private:
		std::size_t* m_start_addr = nullptr; 

		std::size_t m_offset;
		std::size_t m_total_size;
	};
}
