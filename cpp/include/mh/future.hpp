#pragma once

#include <chrono>
#include <future>

namespace mh
{
	template<typename T>
	inline auto is_future_ready(const T& future) ->
		decltype(future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	template<typename T, typename... TArgs>
	auto emplace_ready_future(TArgs&&... args) ->
		decltype(T(std::forward<TArgs>(args)...), std::future<T>{})
	{
		std::promise<T> promise;
		promise.set_value(T(std::forward<TArgs>(args)...));
		return promise.get_future();
	}

	template<typename T>
	std::future<T> make_ready_future(T&& value)
	{
		return emplace_ready_future<T>(std::move(value));
	}
	template<typename T>
	std::future<T> make_ready_future(const T& value)
	{
		return emplace_ready_future<T>(T(value));
	}
	template<typename T>
	std::future<T> make_ready_future()
	{
		return emplace_ready_future<T>(T{});
	}

	template<typename T>
	std::future<T> make_failed_future(const std::exception_ptr& exception)
	{
		std::promise<T> promise;
		promise.set_exception(exception);
		return promise.get_future();
	}
	template<typename T, typename E>
	std::future<T> make_failed_future(const E& exception)
	{
		return make_failed_future<T>(std::make_exception_ptr(exception));
	}
}
