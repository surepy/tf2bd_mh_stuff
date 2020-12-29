#pragma once

#include <stdexcept>
#include <thread>

namespace mh
{
	class thread_sentinel_exception : public std::runtime_error
	{
	public:
		thread_sentinel_exception(const std::thread::id& expectedID);
	};

	class thread_sentinel
	{
	public:
		thread_sentinel() noexcept;

		void check() const;

		void reset_id() noexcept;
		void reset_id(const std::thread::id& id) noexcept;

	private:
		std::thread::id m_ExpectedID;
	};
}

#ifndef MH_COMPILE_LIBRARY
#include "thread_sentinel.inl"
#endif
