#ifdef MH_COMPILE_LIBRARY
#include "dispatcher.hpp"
#else
#define MH_COMPILE_LIBRARY_INLINE inline
#endif

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace mh
{
	namespace detail::dispatcher_hpp
	{
		struct task_data
		{
			mutable std::mutex m_TaskCompleteCVMutex;
			mutable std::condition_variable m_TaskCompleteCV;
			std::atomic<bool> m_IsTaskComplete = false;

			std::coroutine_handle<> m_Handle;
		};

		struct thread_data
		{
			mutable std::mutex m_TasksMutex;
			std::queue<std::shared_ptr<task_data>> m_Tasks;

			const std::thread::id m_OwnerThread = std::this_thread::get_id();
		};

		MH_COMPILE_LIBRARY_INLINE co_dispatch_task::co_dispatch_task(std::shared_ptr<thread_data> threadData) noexcept :
			m_ThreadData(std::move(threadData))
		{
		}

		MH_COMPILE_LIBRARY_INLINE bool co_dispatch_task::await_ready() const
		{
			return std::this_thread::get_id() == m_ThreadData->m_OwnerThread;
		}

		MH_COMPILE_LIBRARY_INLINE void co_dispatch_task::await_resume() const
		{
			assert(m_TaskData);
			std::unique_lock lock(m_TaskData->m_TaskCompleteCVMutex);
			m_TaskData->m_TaskCompleteCV.wait(lock, [&] { return !m_TaskData->m_IsTaskComplete; });
		}

		MH_COMPILE_LIBRARY_INLINE bool co_dispatch_task::await_suspend(std::coroutine_handle<> handle)
		{
			// Should never hit this, await_ready() should prevent suspension of coroutines
			// that are already on the correct thread
			assert(std::this_thread::get_id() != m_ThreadData->m_OwnerThread);

			throw "Not implemented";
		}
	}

	MH_COMPILE_LIBRARY_INLINE dispatcher::dispatcher() :
		m_ThreadData(std::make_shared<thread_data>())
	{
	}

	MH_COMPILE_LIBRARY_INLINE size_t dispatcher::run()
	{
		size_t count = 0;

		while (run_one())
			count++;

		return count;
	}

	MH_COMPILE_LIBRARY_INLINE bool dispatcher::run_one()
	{
		using detail::dispatcher_hpp::task_data;

		assert(m_ThreadData->m_OwnerThread == std::this_thread::get_id());

		if (!m_ThreadData->m_Tasks.empty())
		{
			std::shared_ptr<task_data> task;
			{
				std::lock_guard lock(m_ThreadData->m_TasksMutex);
				task = std::move(m_ThreadData->m_Tasks.front());
				m_ThreadData->m_Tasks.pop();
			}

			// This could throw
			task->m_Handle.resume();
			return true;
		}

		return false;
	}

	MH_COMPILE_LIBRARY_INLINE detail::dispatcher_hpp::co_dispatch_task dispatcher::co_dispatch()
	{
		assert(m_ThreadData);
		return { m_ThreadData };
	}
}
