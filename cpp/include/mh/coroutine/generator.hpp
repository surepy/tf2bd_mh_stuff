#pragma once

//#define MH_COROUTINE_GENERATOR_FORCE_HANDROLLED_IMPL 1

#if !defined(MH_COROUTINE_GENERATOR_FORCE_HANDROLLED_IMPL) && __has_include(<generator>)
#include <generator>
namespace mh
{
	template<typename T> using generator = std::generator<T>;
}
#elif !defined(MH_COROUTINE_GENERATOR_FORCE_HANDROLLED_IMPL) && __has_include(<experimental/generator>)
#include <experimental/generator>
namespace mh
{
	template<typename T> using generator = std::experimental::generator<T>;
}
#else

#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
#error No coroutine header supported
#endif

namespace mh
{
	// TODO
	template<typename T>
	class generator
	{
	public:
		struct promise_type
		{

		};
	};
}
#endif
