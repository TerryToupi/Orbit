#include "src/utilities/ringBuffer.h"
#include <functional>   

namespace Engine
{
	// A Dispatched job will receive this as function argument:
	struct JobDispatchArgs
	{
		uint32_t jobIndex;
		uint32_t groupIndex;
	};

	class JobManager final
	{ 
	public:
		JobManager() = default;

		static inline JobManager* instance = nullptr;

		// Create the internal resources such as worker threads, etc. Call it once when initializing the application.
		void Init(); 

		// Destroy Manager and thread
		void ShutDown();

		// Add a job to execute asynchronously. Any idle thread will execute this job.
		void Execute(const std::function<void()>& job);

		// Divide a job onto multiple jobs and execute in parallel.
		//	jobCount	: how many jobs to generate for this task.
		//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//	func		: receives a JobDispatchArgs as parameter
		void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job);

		// Check if any threads are working currently or not
		bool IsBusy();

		// Wait until all threads become idle
		void Wait();

	private: 
		JobManager(const JobManager&);
		JobManager(const JobManager&&);

		inline void poll(); 

	private: 
		// number of worker threads, it will be initialized in the Initialize() function
		uint32_t m_numThreads = 0;
		// a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning
		ThreadSafeRingBuffer<std::function<void()>, 256> m_jobPool;
		// used in conjunction with the wakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
		std::condition_variable m_wakeCondition;
		// used in conjunction with the wakeCondition above
		std::mutex m_wakeMutex;
		// tracks the state of execution of the main thread
		uint64_t m_currentLabel = 0;
		// track the state of execution across background worker threads
		std::atomic<uint64_t> m_finishedLabel;
	};
}