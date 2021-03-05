#pragma once

namespace mh
{
	template<typename T>
	MH_STUFF_API T get_random(T min_inclusive, T max_inclusive);
}

#ifndef MH_COMPILE_LIBRARY
#include "random.inl"
#endif
