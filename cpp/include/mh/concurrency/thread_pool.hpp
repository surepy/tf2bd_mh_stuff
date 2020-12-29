#pragma once

#if __has_include(<mh/coroutine/future.hpp>)
#include <mh/coroutine/future.hpp>
#else
#include <future>
#endif

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace mh
{
	namespace detail::thread_pool_hpp
	{
		template<typename T>
		struct traits
		{
			using function_type = std::function<T()>;

#if __has_include(<mh/coroutine/future.hpp>)
			using promise_type = mh::promise<T>;
			using future_type = mh::shared_future<T>;
#else
			using promise_type = std::promise<T>;
			using future_type = std::shared_future<T>;
#endif
		};

		template<typename T>
		struct task : traits<T>
		{
			using super = traits<T>;

			typename super::function_type m_Function;
			typename super::promise_type m_Promise;
		};

		template<typename T>
		struct thread_data : traits<T>
		{
			bool m_IsShuttingDown = false;

			std::queue<task<T>> m_Tasks;
			mutable std::mutex m_TasksMutex;
			std::condition_variable m_TasksCV;
			std::vector<std::thread> m_Threads;
		};

		template<typename T>
		struct [[nodiscard]] co_task : traits<T>
		{
			co_task(std::shared_ptr<thread_data<T>> poolData) : m_PoolData(std::move(poolData)) {}

			struct [[nodiscard]] promise_type
			{
				constexpr std::suspend_never initial_suspend() const noexcept { return {}; }
				constexpr std::suspend_always final_suspend() const noexcept { return {}; }
			};

			constexpr bool await_ready() const { return false; }
			constexpr void await_resume() const {}
			bool await_suspend(std::coroutine_handle<> parent)
			{
#if 0
				std::lock_guard lock(m_PoolData->m_TasksMutex);

				auto func = std::bind([](std::coroutine_handle<> handle)
					{
						__debugbreak();
						handle.resume();
					}, parent);

				using function_type = typename traits<T>::function_type;

				m_PoolData->m_Tasks.push(function_type(std::move(func)));
#else
				throw "Not implemented, waiting for type-erased thread_pool rewrite";
#endif

				return true; // always suspend
			}

		private:
			std::shared_ptr<thread_data<T>> m_PoolData;
		};
	}

	template<typename T>
	class thread_pool final : public detail::thread_pool_hpp::traits<T>
	{
		using traits = detail::thread_pool_hpp::traits<T>;
		using thread_data = detail::thread_pool_hpp::thread_data<T>;
		using task_type = detail::thread_pool_hpp::task<T>;
	public:
		using typename traits::function_type;
		using typename traits::future_type;
		using typename traits::promise_type;

		thread_pool(size_t threadCount = std::thread::hardware_concurrency())
		{
			if (threadCount < 1)
				throw std::invalid_argument("threadCount must be >= 1");

			for (size_t i = 0; i < threadCount; i++)
				m_ThreadData->m_Threads.push_back(std::thread(&ThreadFunc, m_ThreadData));
		}
		~thread_pool()
		{
			m_ThreadData->m_IsShuttingDown = true;
			for (auto& thread : m_ThreadData->m_Threads)
				thread.detach();
		}

		future_type add_task(function_type func)
		{
			std::lock_guard lock(m_ThreadData->m_TasksMutex);

			task_type& task = m_ThreadData->m_Tasks.emplace();
			task.m_Function = std::move(func);

			m_ThreadData->m_TasksCV.notify_one();

			return task.m_Promise.get_future();
		}

		size_t thread_count() const { return m_ThreadData->m_Threads.size(); }
		size_t task_count() const { return m_ThreadData->m_Tasks.size(); }

		detail::thread_pool_hpp::co_task<T> co_add_task()
		{
			return { m_ThreadData };
		}

	private:
		std::shared_ptr<thread_data> m_ThreadData = std::make_shared<thread_data>();

		static void ThreadFunc(std::shared_ptr<thread_data> data)
		{
			using namespace std::chrono_literals;

			while (!data->m_IsShuttingDown)
			{
				{
					std::unique_lock lock(data->m_TasksMutex);
					data->m_TasksCV.wait_for(lock, 1s);
				}

				while (!data->m_Tasks.empty() && !data->m_IsShuttingDown)
				{
					task_type task;
					{
						std::lock_guard lock(data->m_TasksMutex);
						if (data->m_Tasks.empty())
							break;

						task = std::move(data->m_Tasks.front());
						data->m_Tasks.pop();
					}

					try
					{
						if constexpr (std::is_same_v<T, void>)
						{
							task.m_Function();
							task.m_Promise.set_value();
						}
						else
						{
							task.m_Promise.set_value(task.m_Function());
						}
					}
					catch (...)
					{
						task.m_Promise.set_exception(std::current_exception());
					}
				}
			}
		}
	};
}
