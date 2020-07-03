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
}
