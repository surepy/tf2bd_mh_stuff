#pragma once

#include <mh/coroutine/coroutine_include.hpp>

#ifdef MH_COROUTINES_SUPPORTED

#include <mh/concurrency/dispatcher.hpp>
#include <mh/coroutine/task.hpp>

#include <chrono>
#include <exception>
#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

namespace mh
{
	namespace detail::thread_pool_hpp
	{
		struct thread_data
		{
			bool m_IsShuttingDown = false;

			mh::dispatcher m_Dispatcher{ false };
			std::vector<std::thread> m_Threads;
		};
	}

	class thread_pool final
	{
		using thread_data = detail::thread_pool_hpp::thread_data;

	public:
		using clock_t = mh::dispatcher::clock_t;

		thread_pool(size_t threadCount = std::thread::hardware_concurrency());
		~thread_pool();

		auto co_add_task() { return m_ThreadData->m_Dispatcher.co_dispatch(); }

		auto co_delay_until(clock_t::time_point timePoint)
		{
			return m_ThreadData->m_Dispatcher.co_delay_until(timePoint);
		}
		template<typename TRep, typename TPeriod>
		auto co_delay_for(std::chrono::duration<TRep, TPeriod> duration)
		{
			return m_ThreadData->m_Dispatcher.co_delay_for(duration);
		}

		template<typename TFunc, typename... TArgs>
		mh::task<std::invoke_result_t<TFunc, TArgs...>> add_task(TFunc func, TArgs... args)
		{
			co_await co_add_task();

			co_return func(std::move(args)...);
		}

		size_t thread_count() const;
		size_t task_count() const;

	private:
		std::shared_ptr<thread_data> m_ThreadData = std::make_shared<thread_data>();

		static void ThreadFunc(std::shared_ptr<thread_data> data);
	};
}

#ifndef MH_COMPILE_LIBRARY
#include "thread_pool.inl"
#endif

#endif
