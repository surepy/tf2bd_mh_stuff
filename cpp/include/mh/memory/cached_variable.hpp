#pragma once

#include <chrono>
#include <mutex>

namespace mh
{
	namespace detail::cached_variable_hpp
	{
		using default_clock = std::chrono::steady_clock;

		template<typename T, typename TUpdateFunc, typename TClock>
		class cached_variable_base
		{
		public:
			using value_type = T;
			using clock_type = TClock;
			using duration_type = typename clock_type::duration;
			using time_point_type = typename clock_type::time_point;

			cached_variable_base(duration_type updateInterval, TUpdateFunc updateFunc) :
				m_NextUpdate(clock_type::now() + updateInterval),
				m_UpdateInterval(updateInterval),
				m_UpdateFunc(std::move(updateFunc)),
				m_Value(std::invoke(m_UpdateFunc))
			{
			}

			duration_type time_since_update() const
			{
				return clock_type::now() - (m_NextUpdate - m_UpdateInterval);
			}
			duration_type time_until_update() const
			{
				return m_NextUpdate - clock_type::now();
			}

		private:
			time_point_type m_NextUpdate{};
			duration_type m_UpdateInterval{};
			[[no_unique_address]] TUpdateFunc m_UpdateFunc{};

		protected:
			bool try_update()
			{
				auto now = clock_type::now();
				if (now >= m_NextUpdate)
				{
					m_Value = std::invoke(m_UpdateFunc);
					m_NextUpdate = now + m_UpdateInterval;
					return true;
				}

				return false;
			}

			T m_Value{};
		};
	}

	template<bool threadSafe = true, typename T = void, typename TUpdateFunc = void,
		typename TClock = detail::cached_variable_hpp::default_clock>
	class cached_variable final : public detail::cached_variable_hpp::cached_variable_base<T, TUpdateFunc, TClock>
	{
		// Non-thread-safe version
	public:
		using cached_variable_base::cached_variable_base;

		T& get_no_update() { return m_Value; }
		const T& get_no_update() const { return m_Value; }

		T& get()
		{
			try_update();
			return m_Value;
		}
	};

	template<typename T, typename TUpdateFunc, typename TClock>
	class cached_variable<true, T, TUpdateFunc, TClock> final :
		public detail::cached_variable_hpp::cached_variable_base<T, TUpdateFunc, TClock>
	{
		// Thread-safe version
	public:
		using cached_variable_base::cached_variable_base;

		T get_no_update() const
		{
			std::lock_guard lock(m_Mutex);
			return m_Value;
		}

		T get()
		{
			std::lock_guard lock(m_Mutex);
			try_update();
			return m_Value;
		}

	private:
		mutable std::mutex m_Mutex;
	};

	template<bool threadSafe = true, typename TUpdateFunc = void, typename TClock = detail::cached_variable_hpp::default_clock>
	cached_variable(typename TClock::duration d, TUpdateFunc f) -> cached_variable<threadSafe, std::invoke_result_t<TUpdateFunc>, TUpdateFunc, TClock>;
}
