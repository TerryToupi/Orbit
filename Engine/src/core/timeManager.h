#pragma once 

#include <chrono>

namespace Engine
{
	class SystemClock final
	{
	public:
		inline static SystemClock *instance;
		
		void Init() {}
		void ShutDown() {}

		uint64_t milli_sec(void) const;
		uint64_t micro_sec(void) const;
		uint64_t nano_sec(void) const;
		uint64_t sec(void) const;

		uint64_t GetTime(void) const;

	private:
		std::chrono::high_resolution_clock m_Clock;
	};
}