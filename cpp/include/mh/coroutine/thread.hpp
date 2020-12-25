#pragma once

#include <coroutine>

namespace mh
{
	namespace detail::coroutine::thread_hpp
	{
		struct task;

		struct [[nodiscard]] promise
		{
			promise();
			promise(const promise&) = delete;
			promise(promise&&) = delete;
			promise& operator=(const promise&) = delete;
			promise& operator=(promise&&) = delete;
			~promise();

			task get_return_object();

			constexpr std::suspend_never initial_suspend() const noexcept { return {}; }
			constexpr std::suspend_always final_suspend() const noexcept { return {}; }
		};

		struct [[nodiscard]] task
		{
			using promise_type = promise;

			constexpr bool await_ready() const { return false; }
			constexpr void await_resume() const {}
			bool await_suspend(std::coroutine_handle<> parent);
		};
	}

	detail::coroutine::thread_hpp::task co_create_thread();
}

#ifndef $MH_COMPILE_LIBRARY
#include <mh/coroutine/thread.inl>
#endif
