#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <variant>

#if defined(_MSC_VER) && defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#include <experimental/coroutine>
namespace mh::detail::task_hpp
{
	namespace coro = std::experimental;
}
#else
#include <coroutine>
namespace mh::detail::task_hpp
{
	namespace coro = std;
}
#endif

namespace mh
{
	template<typename T> class task;

	enum class task_state
	{
		empty, // no state, never initialized, or was moved from
		running,
		value,
		exception,
	};

	namespace detail::task_hpp
	{
		template<typename T>
		struct promise
		{
			static constexpr size_t IDX_WAITERS = 0;
			static constexpr size_t IDX_VALUE = 1;
			static constexpr size_t IDX_EXCEPTION = 2;

			constexpr detail::task_hpp::coro::suspend_never initial_suspend() const noexcept { return {}; }
			constexpr detail::task_hpp::coro::suspend_always final_suspend() const noexcept { return {}; }

			constexpr task<T> get_return_object();

			void return_value(T value)
			{
				set_state<IDX_VALUE>(std::move(value));
			}

			void unhandled_exception()
			{
				set_state<IDX_EXCEPTION>(std::current_exception());
			}

			task_state get_task_state() const
			{
				switch (m_State.index())
				{
				case IDX_WAITERS:    return task_state::running;
				case IDX_VALUE:      return task_state::value;
				case IDX_EXCEPTION:  return task_state::exception;
				}
			}

			bool is_ready() const
			{
				auto index = m_State.index();
				return index == IDX_VALUE || index == IDX_EXCEPTION;
			}

			const T& value() const
			{
				std::unique_lock lock(m_Mutex);
				m_ValueReadyCV.wait(lock, [&] { return is_ready(); });

				if (auto ex = std::get_if<IDX_EXCEPTION>(&m_State))
					std::rethrow_exception(*ex);

				return std::get<IDX_VALUE>(m_State);
			}

			const T* try_get_value() const
			{
				std::unique_lock lock(m_Mutex);
				m_ValueReadyCV.wait(lock, [&] { return is_ready(); });
				return std::get_if<IDX_VALUE>(&m_State);
			}

			[[nodiscard]] bool add_waiter(coro::coroutine_handle<> handle)
			{
				if (is_ready())
				{
					return false;  // don't suspend
				}
				else
				{
					std::lock_guard lock(m_Mutex);
					if (is_ready())
					{
						return false; // don't suspend
					}
					else
					{
						std::get<IDX_WAITERS>(m_State).push_back(handle);
						return true;   // suspend
					}
				}
			}

		private:
			template<size_t IDX, typename TValue>
			void set_state(TValue&& value)
			{
				std::lock_guard lock(m_Mutex);
				auto waiters = std::move(std::get<IDX_WAITERS>(m_State));

				static_assert(IDX == IDX_VALUE || IDX == IDX_EXCEPTION);
				m_State.emplace<IDX>(std::move(value));

				m_ValueReadyCV.notify_all();
				for (const auto& waiter : waiters)
					waiter.resume();
			}

			mutable std::recursive_mutex m_Mutex;
			mutable std::condition_variable_any m_ValueReadyCV;
			std::variant<std::vector<coro::coroutine_handle<>>, T, std::exception_ptr> m_State;
		};

		template<typename T>
		struct handle_wrapper
		{
			using promise_type = promise<T>;
			using coroutine_type = coro::coroutine_handle<promise_type>;

			handle_wrapper() noexcept = default;
			handle_wrapper(std::nullptr_t) noexcept : m_Handle(nullptr) {}
			handle_wrapper(coroutine_type handle) noexcept : m_Handle(std::move(handle)) {}

			handle_wrapper(handle_wrapper&& other) noexcept : m_Handle(std::exchange(other.m_Handle, nullptr)) {}
			handle_wrapper& operator=(handle_wrapper&& other) noexcept
			{
				assert(std::addressof(other) != this);
				if (m_Handle)
					m_Handle.destroy();

				m_Handle = std::exchange(other.m_Handle, nullptr);
				return *this;
			}

			~handle_wrapper()
			{
				if (m_Handle)
					m_Handle.destroy();
			}

			coroutine_type m_Handle;
		};
	}

	template<typename T> class shared_task;

	template<typename T>
	class [[nodiscard]] task
	{
		using wrapper_type = detail::task_hpp::handle_wrapper<T>;
	public:
		using promise_type = typename wrapper_type::promise_type;
		using coroutine_type = typename wrapper_type::coroutine_type;

		task() = default;
		task(coroutine_type handle) : m_Wrapper(std::move(handle)) {}

		constexpr bool await_ready() const { return is_ready(); }
		const T& await_resume() const { return get(); }
		bool await_suspend(detail::task_hpp::coro::coroutine_handle<> parent)
		{
			return get_handle().promise().add_waiter(parent);
		}

		task_state state() const { return m_Wrapper ? m_Wrapper->get_task_state() : task_state::empty; }
		bool valid() const { return !!m_Wrapper.m_Handle; }
		operator bool() const { return valid(); }

		bool is_ready() const { return valid() && m_Wrapper.m_Handle.promise().is_ready(); }
		const T& get() const { return get_handle().promise().value(); }
		const T* try_get() const { return get_handle().promise().try_get_value(); }

	private:
		const coroutine_type& get_handle() const
		{
			if (!valid())
				throw std::runtime_error("empty task");

			return m_Wrapper.m_Handle;
		}
		coroutine_type& get_handle() { return const_cast<coroutine_type&>(const_cast<const task<T>*>(this)->get_handle()); }

		wrapper_type m_Wrapper;
		template<typename T> friend class shared_task;
	};

	template<typename T>
	class [[nodiscard]] shared_task
	{
		using wrapper_type = detail::task_hpp::handle_wrapper<T>;
	public:
		using promise_type = typename wrapper_type::promise_type;
		using coroutine_type = typename wrapper_type::coroutine_type;

		shared_task() = default;
		shared_task(coroutine_type handle) : m_Wrapper(std::make_shared<wrapper_type>(handle)) {}
		shared_task(task<T>&& task) : m_Wrapper(std::make_shared<wrapper_type>(std::exchange(task.m_Wrapper, nullptr))) {}

		shared_task(const shared_task& other) noexcept = default;
		shared_task& operator=(const shared_task& other) noexcept = default;

		shared_task(shared_task&& other) noexcept = default;
		shared_task& operator=(shared_task&& other) noexcept = default;

		constexpr bool await_ready() const { return is_ready(); }
		const T& await_resume() const { return get(); }
		bool await_suspend(detail::task_hpp::coro::coroutine_handle<> parent)
		{
			return get_handle().promise().add_waiter(parent);
		}

		task_state state() const { return m_Wrapper ? m_Wrapper->get_task_state() : task_state::empty; }
		bool valid() const { return m_Wrapper && m_Wrapper->m_Handle; }
		operator bool() const { return valid(); }

		bool is_ready() const { return valid() && m_Wrapper->m_Handle.promise().is_ready(); }
		const T& get() const { return get_handle().promise().value(); }
		const T* try_get() const { return get_handle().promise().try_get_value(); }

	private:
		const coroutine_type& get_handle() const
		{
			if (!valid())
				throw std::runtime_error("empty task");

			return m_Wrapper->m_Handle;
		}
		coroutine_type& get_handle() { return const_cast<coroutine_type&>(const_cast<const task<T>*>(this)->get_handle()); }

		std::shared_ptr<wrapper_type> m_Wrapper;
	};

	template<typename T>
	inline constexpr task<T> detail::task_hpp::promise<T>::get_return_object()
	{
		return { task<T>::coroutine_type::from_promise(*this) };
	}
}

#if __has_include(<mh/reflection/enum.hpp>)
#include <mh/reflection/enum.hpp>

MH_ENUM_REFLECT_BEGIN(mh::task_state)
	MH_ENUM_REFLECT_VALUE(empty)
	MH_ENUM_REFLECT_VALUE(running)
	MH_ENUM_REFLECT_VALUE(value)
	MH_ENUM_REFLECT_VALUE(exception)
MH_ENUM_REFLECT_END()
#endif
