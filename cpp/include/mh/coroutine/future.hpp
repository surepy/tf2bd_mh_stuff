#pragma once

#include "task.hpp"

#ifdef MH_COROUTINES_SUPPORTED

namespace mh
{
	template<typename T> class future;
	template<typename T> class shared_future;
	template<typename T> class promise;

	namespace detail::future_hpp
	{
		template<typename T>
		class future_obj_base
		{
			using promise_type = mh::detail::promise<T>;

		public:
			future_obj_base() noexcept = default;

			explicit future_obj_base(detail::promise<T>* promise) noexcept :
				m_Promise(promise)
			{
				if (m_Promise)
					m_Promise->add_ref();
			}
			explicit future_obj_base(const future_obj_base& other) noexcept : future_obj_base(other.m_Promise) {}
			future_obj_base& operator=(const future_obj_base& other) noexcept
			{
				release_promise();
				m_Promise = other.m_Promise;
				if (m_Promise)
					m_Promise->add_ref();

				return *this;
			}

			explicit future_obj_base(future_obj_base&& other) noexcept :
				m_Promise(std::exchange(other.m_Promise, nullptr))
			{
				assert(std::addressof(other) != this);
			}
			future_obj_base& operator=(future_obj_base&& other) noexcept
			{
				assert(std::addressof(other) != this);
				release_promise();
				m_Promise = std::exchange(other.m_Promise, nullptr);
				return *this;
			}

			~future_obj_base()
			{
				release_promise();
			}

			task<T> get_task() const { return task<T>(m_Promise); }

		protected:
			promise_type& get_promise() { return const_cast<promise_type&>(std::as_const(*this).get_promise()); }
			promise_type& get_promise_unsafe() const { return const_cast<promise_type&>(std::as_const(*this).get_promise()); }
			const promise_type& get_promise() const
			{
				if (!m_Promise)
					throw std::future_error(std::future_errc::no_state);

				return *m_Promise;
			}

			void create_promise()
			{
				assert(!m_Promise);
				release_promise();
				m_Promise = new mh::detail::promise<T>;
				m_Promise->add_ref();
				assert(m_Promise->get_ref_count() == 1);
			}

			bool valid() const noexcept { return m_Promise && m_Promise->get_task_state() != task_state::empty; }

			void release_promise()
			{
				if (m_Promise)
				{
					m_Promise->release_promise_ref([&]
						{
							delete m_Promise;
						});

					m_Promise = nullptr;
				}
			}

			promise_type* try_get_promise() { return m_Promise; }
			const promise_type* try_get_promise() const { return m_Promise; }

		private:
			mh::detail::promise<T>* m_Promise = nullptr;
		};
	}

	template<typename T>
	class future final : detail::future_hpp::future_obj_base<T>
	{
	public:
		future() noexcept = default;

		future(future&&) noexcept = default;
		future& operator=(future&&) noexcept = default;

		// No copying
		future(const future&) = delete;
		future& operator=(const future&) = delete;

		using detail::future_hpp::future_obj_base<T>::valid;

		shared_future<T> share() noexcept;

		void wait() const { return this->promise().wait(); }
		template<typename Rep, typename Period>
		std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const
		{
			return this->get_promise().wait_for(timeout_duration);
		}
		template<typename Clock, typename Period>
		std::future_status wait_until(const std::chrono::time_point<Clock, Period>& timeout_time) const
		{
			return this->get_promise().wait_until(timeout_time);
		}

		T get()
		{
			auto value = std::move(this->get_promise().get_value());
			this->release_promise();
			return std::move(value);
		}
		T await_resume() { return get(); }

	private:
		template<typename T2> friend class promise;
		using detail::future_hpp::future_obj_base<T>::future_obj_base;
	};

	template<typename T>
	class shared_future final : detail::future_hpp::future_obj_base<T>
	{
	public:
		shared_future() noexcept = default;
		shared_future(future<T>&& f) : shared_future(f.share()) {}

		shared_future(shared_future&&) noexcept = default;
		shared_future& operator=(shared_future&&) noexcept = default;

		shared_future(const shared_future&) = default;
		shared_future& operator=(const shared_future&) = default;

		using detail::future_hpp::future_obj_base<T>::valid;

		void wait() const { return this->promise().wait(); }
		template<typename Rep, typename Period>
		std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const
		{
			return this->get_promise().wait_for(timeout_duration);
		}
		template<typename Clock, typename Period>
		std::future_status wait_until(const std::chrono::time_point<Clock, Period>& timeout_time) const
		{
			return this->get_promise().wait_until(timeout_time);
		}

		const T& get() const { return this->get_promise().get_value(); }
		const T& await_resume() const { return get(); }

	private:
		template<typename T2> friend class promise;
		using detail::future_hpp::future_obj_base<T>::future_obj_base;
	};

	template<typename T>
	class promise final : detail::future_hpp::future_obj_base<T>
	{
	public:
		promise()
		{
			this->create_promise();
		}

		using detail::future_hpp::future_obj_base<T>::valid;
		mh::future<T> get_future() const
		{
			return mh::future<T>(&this->get_promise_unsafe());
		}
		mh::task<T> get_task() const
		{
			return mh::task<T>(&this->get_promise_unsafe());
		}

		void set_value(T value)
		{
			this->get_promise().set_state<detail::promise<T>::IDX_VALUE>(std::move(value));
		}
		void set_exception(std::exception_ptr p)
		{
			this->get_promise().set_state<detail::promise<T>::IDX_EXCEPTION>(p);
		}
	};

	template<typename T>
	shared_future<T> future<T>::share() noexcept
	{
		return shared_future<T>(this->try_get_promise());
	}
}
#else
#include <future>
namespace mh
{
	template<typename T> using promise = std::promise<T>;
	template<typename T> using future = std::future<T>;
	template<typename T> using shared_future = std::shared_future<T>;
}
#endif
