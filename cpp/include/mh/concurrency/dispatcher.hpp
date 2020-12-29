#pragma once

#include <coroutine>
#include <memory>

namespace mh
{
	namespace detail::dispatcher_hpp
	{
		struct thread_data;
		struct task_data;

		struct [[nodiscard]] co_dispatch_task
		{
			co_dispatch_task(std::shared_ptr<thread_data> threadData) noexcept;

			bool await_ready() const;
			void await_resume() const;
			bool await_suspend(std::coroutine_handle<> parent);

		private:
			std::shared_ptr<thread_data> m_ThreadData;

			std::shared_ptr<task_data> m_TaskData;
		};
	}

	class dispatcher
	{
		using thread_data = detail::dispatcher_hpp::thread_data;

	public:
		dispatcher();

		size_t run();
		bool run_one();

		detail::dispatcher_hpp::co_dispatch_task co_dispatch();

	private:
		std::shared_ptr<thread_data> m_ThreadData;
	};
}

#ifndef MH_COMPILE_LIBRARY
#include "dispatcher.inl"
#endif
