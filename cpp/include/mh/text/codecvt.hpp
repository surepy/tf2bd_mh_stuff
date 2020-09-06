#pragma once

#include <locale>
#include <string>
#include <string_view>
#include <type_traits>

namespace mh
{
	namespace detail::codecvt_hpp
	{
		template<typename T1, typename T2>
		struct in_out_types
		{
			using intern_type = T1;
			using extern_type = T2;
		};

		template<> struct in_out_types<char, char16_t> : in_out_types<char16_t, char> {};
		template<> struct in_out_types<char8_t, char16_t> : in_out_types<char16_t, char8_t> {};
		template<> struct in_out_types<char, char32_t> : in_out_types<char32_t, char> {};
		template<> struct in_out_types<char8_t, char32_t> : in_out_types<char32_t, char8_t> {};
		template<> struct in_out_types<char, wchar_t> : in_out_types<wchar_t, char> {};

		template<typename T1, typename T2> using intern_type_t = typename in_out_types<T1, T2>::intern_type;
		template<typename T1, typename T2> using extern_type_t = typename in_out_types<T1, T2>::extern_type;
	}

	template<typename To, typename From, typename FromTraits = std::char_traits<From>>
	std::basic_string<To> change_encoding(const std::basic_string_view<From, FromTraits>& input)
	{
		using intern_type = detail::codecvt_hpp::intern_type_t<To, From>;
		using extern_type = detail::codecvt_hpp::extern_type_t<To, From>;

		struct design_by_committee : std::codecvt<intern_type, extern_type, std::mbstate_t> {} cvt;

		std::mbstate_t state{};

		std::basic_string<To> retVal;
		retVal.resize(cvt.length(state, input.data(), input.data() + input.size(),
			std::numeric_limits<size_t>::max()));

		state = {};

		if constexpr (std::is_same_v<To, intern_type>)
		{
			const extern_type* fromNext;
			intern_type* toNext;

			cvt.in(state,
				input.data(), input.data() + input.size(), fromNext,
				retVal.data(), retVal.data() + retVal.size(), toNext);
		}
		else
		{
			const intern_type* fromNext;
			extern_type* toNext;

			cvt.out(state,
				input.data(), input.data() + input.size(), fromNext,
				retVal.data(), retVal.data() + retVal.size(), toNext);
		}

		return retVal;
	}

	template<typename To, typename From>
	inline auto change_encoding(const From* input)
	{
		return change_encoding<To>(std::basic_string_view<From>(input));
	}
	template<typename To, typename From,
		typename FromTraits = std::char_traits<From>,
		typename FromAlloc = std::allocator<From>>
	inline auto change_encoding(const std::basic_string<From, FromTraits, FromAlloc>& input)
	{
		return change_encoding<To>(std::basic_string_view<From, FromTraits>(input));
	}
}
