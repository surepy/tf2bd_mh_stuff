#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <semaphore>
#include <stdexcept>
#include <variant>

#if defined(_MSC_VER) && defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#include <experimental/coroutine>
namespace mh::detail::thread_task_hpp
{
	namespace coro = std::experimental;
}
#else
#include <coroutine>
namespace mh::detail::thread_task_hpp
{
	namespace coro = std;
}
#endif

#ifndef $MH_NOINLINE
#ifdef _MSC_VER
#define $MH_NOINLINE __declspec(noinline)
#else
#define $MH_NOINLINE
#endif
#endif

namespace mh
{
	template<typename T> class thread_task;

	namespace detail::thread_task_hpp
	{
		template<typename T>
		struct promise
		{
			static constexpr size_t IDX_WAITERS = 0;
			static constexpr size_t IDX_VALUE = 1;
			static constexpr size_t IDX_EXCEPTION = 2;

			constexpr detail::thread_task_hpp::coro::suspend_never initial_suspend() const noexcept { return {}; }
			constexpr detail::thread_task_hpp::coro::suspend_always final_suspend() const noexcept { return {}; }

			constexpr thread_task<T> get_return_object();

			$MH_NOINLINE void return_value(T value)
			{
				std::lock_guard lock(m_Mutex);
				auto waiters = std::move(std::get<IDX_WAITERS>(m_State));

				m_State = std::move(value);

				for (const auto& waiter : waiters)
					waiter.resume();
			}

			$MH_NOINLINE void unhandled_exception()
			{
				std::lock_guard lock(m_Mutex);
				//__debugbreak();
				m_State = std::current_exception();
			}

			$MH_NOINLINE bool is_ready() const
			{
				auto index = m_State.index();
				return index == IDX_VALUE || index == IDX_EXCEPTION;
			}

			$MH_NOINLINE const T& value() const
			{
				std::unique_lock lock(m_Mutex);
				m_ValueReadyCV.wait(lock, [&] { return is_ready(); });

				if (auto ex = std::get_if<IDX_EXCEPTION>(&m_State))
					std::rethrow_exception(*ex);

				return std::get<IDX_VALUE>(m_State);
			}

			[[nodiscard]] $MH_NOINLINE bool add_waiter(coro::coroutine_handle<> handle)
			{
				if (is_ready())
				{
					// handle.resume();
					return false;  // don't suspend
				}
				else
				{
					std::lock_guard lock(m_Mutex);
					std::get<IDX_WAITERS>(m_State).push_back(handle);
					return true;   // suspend
				}
			}

		private:
			mutable std::recursive_mutex m_Mutex;
			mutable std::condition_variable_any m_ValueReadyCV;
			std::variant<std::vector<coro::coroutine_handle<>>, T, std::exception_ptr> m_State;
		};
	}

	template<typename T>
	class [[nodiscard]] thread_task
	{
	public:
		using promise_type = detail::thread_task_hpp::promise<T>;
		using coroutine_type = detail::thread_task_hpp::coro::coroutine_handle<promise_type>;

		thread_task() = default;
		thread_task(coroutine_type handle) : m_Handle(std::move(handle)) {}

		$MH_NOINLINE constexpr bool await_ready() const
		{
			return is_ready();
		}

		$MH_NOINLINE const T& await_resume() const
		{
			//__debugbreak();
			return get();
		}

		$MH_NOINLINE bool await_suspend(detail::thread_task_hpp::coro::coroutine_handle<> parent)
		{
			return m_Handle.promise().add_waiter(parent);
		}

		$MH_NOINLINE bool is_ready() const { return m_Handle.promise().is_ready(); }
		$MH_NOINLINE const T& get() const { return m_Handle.promise().value(); }

	private:
		coroutine_type m_Handle;
	};

	template<typename T>
	$MH_NOINLINE inline constexpr thread_task<T> detail::thread_task_hpp::promise<T>::get_return_object()
	{
		return { thread_task<T>::coroutine_type::from_promise(*this) };
	}
}
