#pragma once

#include <type_traits>

namespace mh
{
	namespace detail::scope_exit_hpp
	{
		template<typename EF, typename Fn, typename TSelf>
		concept ConstructibleFunc =
			!std::is_same_v<std::remove_cvref_t<Fn>, TSelf> &&
			std::is_constructible_v<EF, Fn>;

		template<typename EF, typename Fn, typename TSelf>
		concept ConstructibleForwardFunc =
			ConstructibleFunc<EF, Fn, TSelf> &&
			!std::is_lvalue_reference_v<Fn> &&
			std::is_nothrow_constructible_v<EF, Fn>;

		template<typename EF, typename Fn, typename TSelf>
		concept ConstructibleCopyFunc =
			ConstructibleFunc<EF, Fn, TSelf> &&
			!ConstructibleForwardFunc<EF, Fn, TSelf>;

		template<typename EF>
		concept MoveFunc =
			std::is_nothrow_move_constructible_v<EF> ||
			std::is_copy_constructible_v<EF>;

		template<typename EF>
		concept MoveForwardFunc = MoveFunc<EF> && std::is_nothrow_move_constructible_v<EF>;

		template<typename EF>
		concept MoveCopyFunc = MoveFunc<EF> && !MoveForwardFunc<EF>;
	}

	// https://en.cppreference.com/w/cpp/experimental/scope_exit
	template<typename EF>
	class scope_exit
	{
		using self_type = scope_exit<EF>;

	public:
		template<typename Fn>
		explicit scope_exit(Fn&& fn)
			noexcept(std::is_nothrow_constructible_v<EF, Fn> || std::is_nothrow_constructible_v<EF, Fn&>)
			requires detail::scope_exit_hpp::ConstructibleForwardFunc<EF, Fn, scope_exit> :
			m_Func(std::forward<Fn>(fn))
		{
		}
		template<typename Fn>
		explicit scope_exit(Fn&& fn)
			noexcept(std::is_nothrow_move_constructible_v<EF> || std::is_nothrow_copy_constructible_v<EF>)
			requires detail::scope_exit_hpp::ConstructibleCopyFunc<EF, Fn, scope_exit> :
			m_Func(fn)
		{
		}

		scope_exit(scope_exit&& other)
			noexcept(std::is_nothrow_move_constructible_v<EF> || std::is_nothrow_copy_constructible_v<EF>)
			requires detail::scope_exit_hpp::MoveForwardFunc<EF> :
			m_Active(other.m_Active),
			m_Func(std::forward<EF>(other.m_Func))
		{
			other.release();
		}
		scope_exit(scope_exit&& other)
			noexcept(std::is_nothrow_move_constructible_v<EF> || std::is_nothrow_copy_constructible_v<EF>)
			requires detail::scope_exit_hpp::MoveCopyFunc<EF> :
			m_Active(other.m_Active),
			m_Func(std::forward<EF>(other.m_Func))
		{
			other.release();
		}

		scope_exit(const scope_exit&) = delete;

		~scope_exit() noexcept
		{
			if (m_Active)
				m_Func();
		}

		scope_exit& operator=(const scope_exit&) = delete;
		scope_exit& operator=(scope_exit&&) = delete;

		void release() noexcept
		{
			m_Active = false;
		}

	private:
		bool m_Active = true;
		[[no_unique_address]] EF m_Func;
	};

	template<class EF> scope_exit(EF) -> scope_exit<EF>;
}
