#pragma once

#include <chrono>
#include <future>

namespace mh
{
	template<typename T>
	inline bool is_future_ready(const T& future)
	{
		return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	template<typename T>
	std::future<T> make_ready_future(T&& value)
	{
		std::promise<T> promise;
		promise.set_value(std::move(value));
		return promise.get_future();
	}

	template<typename T>
	std::future<T> make_ready_future(const T& value)
	{
		return make_ready_future<T>(T(value));
	}

	template<typename T>
	std::future<T> make_ready_future()
	{
		return make_ready_future<T>(T{});
	}
}
