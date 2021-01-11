#pragma once

#include <mh/coroutine/task.hpp>

#ifdef MH_COROUTINES_SUPPORTED

#include "dispatcher.hpp"

#include <memory>
#include <optional>

namespace mh
{
	namespace detail::thread_pool_hpp
	{
		struct thread_data;

		struct dispatcher_task_wrapper
		{
			explicit dispatcher_task_wrapper(mh::dispatcher::dispatch_task_t dispatchTask);
			explicit dispatcher_task_wrapper(std::nullptr_t) noexcept;

			bool await_ready() const;
			void await_resume() const;
			bool await_suspend(coro::coroutine_handle<> parent);

		private:
			std::optional<mh::dispatcher::dispatch_task_t> m_DispatchTask;
		};
	}

	class thread_pool final
	{
		using thread_data = detail::thread_pool_hpp::thread_data;

	public:
		using clock_t = mh::dispatcher::clock_t;

		thread_pool();
		thread_pool(size_t threadCount);
		~thread_pool();

		detail::thread_pool_hpp::dispatcher_task_wrapper co_add_task();

		mh::dispatcher::delay_task_t co_delay_until(clock_t::time_point timePoint);
		mh::dispatcher::delay_task_t co_delay_for(clock_t::duration duration);

		template<typename TFunc, typename... TArgs>
		mh::task<std::invoke_result_t<TFunc, TArgs...>> add_task(TFunc func, TArgs... args)
		{
			co_await co_add_task();

			co_return func(std::move(args)...);
		}

		size_t thread_count() const;
		size_t task_count() const;

	private:
		std::shared_ptr<thread_data> m_ThreadData;

		static void ThreadFunc(std::shared_ptr<thread_data> data);
	};
}

#ifndef MH_COMPILE_LIBRARY
#include "thread_pool.inl"
#endif

#endif
