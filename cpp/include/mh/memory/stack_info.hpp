#pragma once

#include <cassert>

namespace mh
{
#ifdef _WIN32
	// Gets the upper and lower bounds of the current thread's stack
	void get_current_thread_stack_range(void*& lower, void*& upper);

	bool is_variable_on_current_stack(const void* var);

	// Checks if a variable is on the current thread's stack
	template<typename T>
	bool is_variable_on_current_stack(const T& var)
	{
		return is_variable_on_current_stack((const void*)&var);
	}
#endif
}

#ifndef MH_COMPILE_LIBRARY
#include "stack_info.inl"
#endif
