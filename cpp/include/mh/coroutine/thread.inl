#ifdef $MH_COMPILE_LIBRARY
#include <mh/coroutine/thread.hpp>
#endif

#ifndef $MH_COMPILE_LIBRARY_INLINE
#define $MH_COMPILE_LIBRARY_INLINE inline
#endif

#include <cassert>
#include <thread>

namespace mh::detail::coroutine::thread_hpp
{
	$MH_COMPILE_LIBRARY_INLINE bool task::await_suspend(std::coroutine_handle<> waiter)
	{
		std::thread t([waiter]() { waiter.resume(); });
		t.detach();

		// always suspend
		return true;
	}

	$MH_COMPILE_LIBRARY_INLINE task promise::get_return_object()
	{
		return {};
	}
}

$MH_COMPILE_LIBRARY_INLINE mh::detail::coroutine::thread_hpp::task mh::co_create_thread()
{
	return {};
}
