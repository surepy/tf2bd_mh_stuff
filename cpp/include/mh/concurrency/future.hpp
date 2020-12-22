#pragma once

#include <cassert>
#include <condition_variable>
#include <coroutine>
#include <exception>
#include <future>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

namespace mh
{
	template<typename T> class future;

	namespace detail::future_hpp
	{
		template<typename T>
		struct future_state
		{
			using storage_type = std::conditional_t<
				std::is_reference_v<T>,
				std::reference_wrapper<std::remove_reference_t<T>>,
				std::conditional_t<std::is_void_v<T>, std::monostate, T>>;

		private:
			static constexpr auto get_ref_type_helper()
			{
				if constexpr (std::is_void_v<T>)
				{
					struct type_struct
					{
						using type = void;
					};
					return type_struct{};
				}
				else
				{
					struct type_struct
					{
						using type = const T&;
					};
					return type_struct{};
				}
			}

		public:
			using get_ref_type = typename std::decay_t<decltype(get_ref_type_helper())>::type;

			static constexpr size_t IDX_RUNNING = 0;
			static constexpr size_t IDX_INVALID = 1;
			static constexpr size_t IDX_COMPLETE = 2;
			static constexpr size_t IDX_FAILED = 3;

			bool is_ready() const
			{
				auto index = m_State.index();
				return index == IDX_COMPLETE || index == IDX_FAILED;
			}

			bool valid() const
			{
				return m_State.index() != IDX_INVALID;
			}

			void wait() const
			{
				if (!is_ready())
				{
					if (!valid())
						throw std::future_error(std::future_errc::no_state);

					std::unique_lock lock(m_Mutex);
					m_CV.wait(lock, [&] { return is_ready(); });
					assert(m_State.index() != IDX_RUNNING);
				}
			}

			template<typename Rep, typename Period>
			std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const
			{
				std::unique_lock lock(m_Mutex);
				if (is_ready())
					return std::future_status::ready;

				if (!valid())
					throw std::future_error(std::future_errc::no_state);

				if (!m_CV.wait_for(lock, timeout_duration, [&] { return is_ready(); }))
					return std::future_status::timeout;

				assert(m_State.index() != IDX_RUNNING);
				return std::future_status::ready;
			}
			template<typename Clock, typename Period>
			std::future_status wait_until(const std::chrono::time_point<Clock, Period>& timeout_time) const
			{
				std::unique_lock lock(m_Mutex);
				if (is_ready())
					return std::future_status::ready;

				if (!valid())
					throw std::future_error(std::future_errc::no_state);

				if (!m_CV.wait_until(lock, timeout_time, [&] { return is_ready(); }))
					return std::future_status::timeout;

				assert(m_State.index() != IDX_RUNNING);
				return std::future_status::ready;
			}

			void rethrow_if_exception() const
			{
				if (m_State.index() == IDX_FAILED)
					std::rethrow_exception(std::get<IDX_FAILED>(m_State));
			}

			T take_value()
			{
				wait();
				rethrow_if_exception();

				if constexpr (!std::is_void_v<T>)
				{
					auto value = std::move(std::get<IDX_COMPLETE>(m_State));
					m_State.emplace<IDX_INVALID>();
					return std::move(value);
				}
			}

			get_ref_type get_ref() const
			{
				wait();

				rethrow_if_exception();

				if constexpr (!std::is_void_v<T>)
					return std::get<IDX_COMPLETE>(m_State);
			}

			bool await_ready() const { return is_ready(); }
			bool await_suspend(std::coroutine_handle<> parent)
			{
				std::lock_guard lock(m_Mutex);
				if (m_State.index() != IDX_RUNNING)
					return false; // don't suspend, either we're done or something is wrong

				std::get<IDX_RUNNING>(m_State).push_back(parent);
				return true; // suspend
			}

			mutable std::mutex m_Mutex;
			mutable std::condition_variable m_CV;
			std::variant<std::vector<std::coroutine_handle<>>, std::monostate, storage_type, std::exception_ptr> m_State;
		};

		template<typename T>
		class future_base
		{
			using state_ptr = std::shared_ptr<future_state<T>>;

		public:
			future_base() = default;
			future_base(state_ptr state) : m_State(std::move(state)) {}

			future_base(const future_base&) = default;
			future_base& operator=(const future_base&) = default;

			future_base(future_base&&) noexcept = default;
			future_base& operator=(future_base&&) noexcept = default;

			bool is_ready() const { return m_State && m_State->is_ready(); }
			bool valid() const { return m_State && m_State->valid(); }

			template<typename TFunc>
			auto then(TFunc&& func)
			{
				//using continuation_t = std::invoke_result_t<TFunc,
				throw std::runtime_error("Not implemented");
			}

			void wait() const { return get_state().wait(); }

			template<typename Rep, typename Period>
			std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const
			{
				return get_state().wait_for(timeout_duration);
			}
			template<typename Clock, typename Period>
			std::future_status wait_until(const std::chrono::time_point<Clock, Period>& timeout_time) const
			{
				return get_state().wait_until(timeout_time);
			}

			bool await_ready() const { return m_State && m_State->await_ready(); }
			bool await_suspend(std::coroutine_handle<> handle) { return get_state().await_suspend(handle); }

		protected:
			const future_state<T>& get_state() const
			{
				if (!m_State)
					throw std::future_error(std::future_errc::no_state);

				return *m_State;
			}
			future_state<T>& get_state()
			{
				return const_cast<future_state<T>&>(const_cast<const future_base<T>*>(this)->get_state());
			}

			state_ptr m_State;
		};

		template<typename T>
		class promise_base
		{
			using state_t = detail::future_hpp::future_state<T>;
			using state_ptr = std::shared_ptr<state_t>;
		public:
			promise_base() = default;
			promise_base(const promise_base&) = delete;
			promise_base(promise_base&&) noexcept = default;
			promise_base& operator=(const promise_base&) = delete;
			promise_base& operator=(promise_base&&) noexcept = default;

			future<T> get_future()
			{
				if (!m_State)
					throw std::future_error(std::future_errc::no_state);
				if (m_FutureRetrieved)
					throw std::future_error(std::future_errc::future_already_retrieved);

				m_FutureRetrieved = true;
				return future<T>(m_State);
			}

			void set_exception(std::exception_ptr p)
			{
				return try_set_state<true>(p);
			}

		protected:
			const state_t& get_state() const
			{
				if (!m_State)
					throw std::future_error(std::future_errc::no_state);

				return *m_State;
			}
			state_t& get_state()
			{
				return const_cast<state_t&>(const_cast<const promise_base*>(this)->get_state());
			}

			template<typename TValue>
			void try_set_value(TValue&& value)
			{
				return try_set_state<false>(std::move(value));
			}

		private:
			template<bool exception, typename TValue>
			void try_set_state(TValue&& value)
			{
				state_t& s = get_state();
				std::lock_guard lock(s.m_Mutex);

				if (s.is_ready())
					throw std::future_error(std::future_errc::promise_already_satisfied);

				std::vector<std::coroutine_handle<>> waiters = std::move(std::get<state_t::IDX_RUNNING>(s.m_State));

				constexpr size_t IDX = exception ? state_t::IDX_FAILED : state_t::IDX_COMPLETE;
				s.m_State.emplace<IDX>(std::move(value));
				assert(s.is_ready());
				s.m_CV.notify_all();

				for (const std::coroutine_handle<>& handle : waiters)
					handle.resume();
			}

			state_ptr m_State = std::make_shared<state_t>();
			bool m_FutureRetrieved = false;
		};
	}

	template<typename T> class shared_future;

	template<typename T>
	class future : public detail::future_hpp::future_base<T>
	{
		using super = detail::future_hpp::future_base<T>;
	public:
		using super::super;

		future(future&&) noexcept = default;
		future& operator=(future&&) noexcept = default;

		// No copying
		future(const future&) = delete;
		future& operator=(const future&) = delete;

		shared_future<T> share();

		T get()
		{
			auto value = super::get_state().take_value();
			super::m_State.reset();
			return std::move(value);
		}
		T await_resume() { return get(); }
	};

	template<typename T>
	class promise : public detail::future_hpp::promise_base<T>
	{
		using super = detail::future_hpp::promise_base<T>;
	public:
		using super::super;
		void set_value(const T& value)
		{
			super::try_set_value(value);
		}
		void set_value(T&& value)
		{
			super::try_set_value(std::move(value));
		}
	};
	template<typename T>
	class promise<T&> : public detail::future_hpp::promise_base<T&>
	{
		using super = detail::future_hpp::promise_base<T&>;
	public:
		using super::super;
		void set_value(T& value)
		{
			super::try_set_value(&value);
		}
	};
	template<>
	class promise<void> : public detail::future_hpp::promise_base<void>
	{
		using super = detail::future_hpp::promise_base<void>;
	public:
		using super::super;
		void set_value()
		{
			super::try_set_value(std::monostate{});
		}
	};

	template<typename T>
	class shared_future : public detail::future_hpp::future_base<T>
	{
		using super = detail::future_hpp::future_base<T>;
		using get_ref_type = typename detail::future_hpp::future_state<T>::get_ref_type;
	public:
		using super::super;
		shared_future(future<T>&& f) : shared_future(f.share()) {}

		shared_future(shared_future&&) noexcept = default;
		shared_future& operator=(shared_future&&) noexcept = default;

		shared_future(const shared_future&) = default;
		shared_future& operator=(const shared_future&) = default;

		get_ref_type get() const { return super::get_state().get_ref(); }
		get_ref_type await_resume() const { return get(); }
	};

	template<typename T>
	shared_future<T> future<T>::share()
	{
		return shared_future<T>(std::move(super::m_State));
	}
}
