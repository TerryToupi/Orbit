#include "jobDispathcer.h"    // include our interface 
#include "src/core/assert.h"

#include <algorithm>    // std::max
#include <atomic>    // to use std::atomic<uint64_t>
#include <thread>    // to use std::thread
#include <condition_variable>    // to use std::condition_variable
#include <sstream>
#include <assert.h>

#ifdef OP_WINDOWS 
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef OP_MACOS 
#include <pthread.h>           // For pthread_t, pthread_setname_np, pthread_mach_thread_np, etc.
#include <mach/mach.h>         // For thread_policy_set, thread_port_t, and kern_return_t types.
#include <mach/thread_policy.h> // For thread_affinity_policy_data_t and THREAD_AFFINITY_POLICY.
#endif

namespace Engine
{
	void JobManager::Init()
	{
		// Initialize the worker execution state to 0:
		m_finishedLabel.store(0);

		// Retrieve the number of hardware threads in this system:
		auto numCores = std::thread::hardware_concurrency();

		// Calculate the actual number of worker threads we want:
		m_numThreads = std::max(1u, numCores);

		// Create all our worker threads while immediately starting them:
		for (uint32_t threadID = 0; threadID < m_numThreads; ++threadID)
		{
			std::thread worker([this] {

				std::function<void()> job; // the current job for the thread, it's empty at start.

				// This is the infinite loop that a worker thread will do 
				while (true)
				{ 
					if (m_jobPool.pop_front(job)) // try to grab a job from the jobPool queue
					{
						// It found a job, execute it:
						job(); // execute job
						m_finishedLabel.fetch_add(1); // update worker label state
					}
					else
					{
						// no job, put thread to sleep
						std::unique_lock<std::mutex> lock(m_wakeMutex);
						m_wakeCondition.wait(lock);
					}
				}

			});

			#ifdef _WIN32 
			{
				// Do Windows-specific thread setup: 
				HANDLE handle = (HANDLE)worker.native_handle();

				// Put each thread on to dedicated core
				DWORD_PTR affinityMask = 1ull << threadID;
				DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
				ENGINE_ASSERT((affinity_result > 0));

				//// Increase thread priority:
				//BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
				//ENGINE_ASSERT(priority_result != 0);

				// Name the thread:
				std::wstringstream wss;
				wss << "EngineJob_" << threadID;
				HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
				ENGINE_ASSERT(SUCCEEDED(hr));
			}
			#endif // _WIN32 

			#ifdef __APPLE__
			{
				// Thread setup for macOS:
				pthread_t handle = worker.native_handle();

				// Attempt to set thread affinity:
				// Note: macOS does not officially support CPU affinity for threads.
				// You might need to use `thread_policy_set` with THREAD_AFFINITY_POLICY on macOS.
				// However, it's rarely reliable and may not work across all macOS versions.
                thread_affinity_policy_data_t policy = { static_cast<integer_t>(threadID % std::thread::hardware_concurrency()) };
				thread_port_t mach_thread = pthread_mach_thread_np(handle);
				kern_return_t kr = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
				ENGINE_ASSERT(kr == KERN_SUCCESS);

				// Name the thread:
				std::stringstream ss;
				ss << "EngineJob_" << threadID;
				int result = pthread_setname_np(ss.str().c_str());
				ENGINE_ASSERT(result == 0);
			}
			#endif

			worker.detach(); // forget about this thread, let it do it's job in the infinite loop that we created above
		}
	}

	void JobManager::ShutDown()
	{
	}

	// This little helper function will not let the system to be deadlocked while the main thread is waiting for something
	void JobManager::poll()
	{
		m_wakeCondition.notify_one(); // wake one worker thread
		std::this_thread::yield(); // allow this thread to be rescheduled 
	}

	void JobManager::Execute(const std::function<void()>& job)
	{
		// The main thread label state is updated:
		m_currentLabel += 1;

		// Try to push a new job until it is pushed successfully:
		while (!m_jobPool.push_back(job)) { poll(); }

		m_wakeCondition.notify_one(); // wake one thread
	}

	bool JobManager::IsBusy()
	{
		// Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
		return m_finishedLabel.load() < m_currentLabel;
	}

	void JobManager::Wait()
	{
		while (IsBusy()) { poll(); }
	}

	void JobManager::Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job)
	{
		if (jobCount == 0 || groupSize == 0)
		{
			return;
		}

		// Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
		const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

		// The main thread label state is updated:
		m_currentLabel += groupCount;

		for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			// For each group, generate one real job:
			const auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {

				// Calculate the current group's offset into the jobs:
				const uint32_t groupJobOffset = groupIndex * groupSize;
				const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

				JobDispatchArgs args;
				args.groupIndex = groupIndex;

				// Inside the group, loop through all job indices and execute job for each index:
				for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
				{
					args.jobIndex = i;
					job(args);
				}
				};

			// Try to push a new job until it is pushed successfully:
			while (!m_jobPool.push_back(jobGroup)) { poll(); }

			m_wakeCondition.notify_one(); // wake one thread
		}
	}
}
