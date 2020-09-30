#pragma once

#define MH_FORMATTER_NONE 0
#define MH_FORMATTER_FMTLIB 1
#define MH_FORMATTER_STL 2

#if __has_include(<format>)

#include <format>
#define MH_FORMATTER MH_FORMATTER_STL
namespace mh::detail::format_hpp
{
#define MH_FMT_STRING(...) __VA_ARGS__
	namespace fmtns = ::std;
}

#elif __has_include(<fmt/format.h>)

#include <fmt/format.h>
#include <fmt/ostream.h>
#define MH_FORMATTER MH_FORMATTER_FMTLIB
namespace mh::detail::format_hpp
{
#define MH_FMT_STRING(...) FMT_STRING(__VA_ARGS__)
	namespace fmtns = ::fmt;
}

#else
#define MH_FORMATTER MH_FORMATTER_NONE
#endif

#if MH_FORMATTER != MH_FORMATTER_NONE
#include <string_view>
#include <iomanip>

namespace mh
{
	namespace detail::format_hpp
	{
		template<typename T>
		inline constexpr bool enum_class_check()
		{
			constexpr bool is_enum_class = std::is_enum_v<T> && !std::is_convertible_v<T, int>;
			static_assert(!is_enum_class, "enum class formatting requires mh::enum_fmt");
			return !is_enum_class;
		}

		// Intellisense dies if you SFINAE on an explicit operator call
#ifndef __INTELLISENSE__
		template<typename T, typename TTo, typename = std::enable_if_t<std::is_convertible_v<T, TTo>>>
		inline constexpr auto implicit_conversion_check(T* t, TTo*) -> decltype(t->operator TTo())
		{
			static_assert(false, "Formatting this type will result in it being formatted as one of its implicit converted types");
			return false;
		}
#endif

		inline constexpr bool implicit_conversion_check(void*, void*) { return true; }

		template<typename T>
		inline constexpr bool check_type_single()
		{
			return enum_class_check<T>() &&
				implicit_conversion_check((T*)nullptr, (bool*)nullptr);
		}

		template<typename... T>
		inline constexpr bool check_type()
		{
			return (check_type_single<T>() && ...);
		}
	}

	template<typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::check_type<TArgs...>()>>
	inline auto format(const TFmtStr& fmtStr, const TArgs&... args)
	{
		return detail::format_hpp::fmtns::format(fmtStr, args...);
	}

	template<typename TOutputIt, typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::check_type<TArgs...>()>>
	inline auto format_to(TOutputIt&& outputIt, const TFmtStr& fmtStr, const TArgs&... args) ->
		decltype(detail::format_hpp::fmtns::format_to(std::forward<TOutputIt>(outputIt), fmtStr, args...))
	{
		return detail::format_hpp::fmtns::format_to(std::forward<TOutputIt>(outputIt), fmtStr, args...);
	}

	template<typename TContainer, typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::check_type<TArgs...>()>>
	inline auto format_to_container(TContainer& container, const TFmtStr& fmtStr, const TArgs&... args)
	{
		return format_to(std::back_inserter(container), fmtStr, args...);
	}

	template<typename TOutputIt, typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::check_type<TArgs...>()>>
	inline auto format_to_n(TOutputIt&& outputIt, size_t n, const TFmtStr& fmtStr, const TArgs&... args)
	{
		return detail::format_hpp::fmtns::format_to_n(std::forward<TOutputIt>(outputIt), n, fmtStr, args...);
	}

	using detail::format_hpp::fmtns::format_error;
	using detail::format_hpp::fmtns::formatter;
	using detail::format_hpp::fmtns::basic_format_parse_context;
	using detail::format_hpp::fmtns::basic_format_context;
	using detail::format_hpp::fmtns::format_parse_context;
	using detail::format_hpp::fmtns::wformat_parse_context;
	using detail::format_hpp::fmtns::format_context;
	using detail::format_hpp::fmtns::wformat_context;

	template<typename TFmtStr, typename... TArgs>
	inline auto try_format(const TFmtStr& fmtStr, const TArgs&... args) -> decltype(format(fmtStr, args...)) try
	{
		return format(fmtStr, args...);
	}
	catch (const format_error& e)
	{
		return format(MH_FMT_STRING("FORMATTING ERROR @ {}: Unable to construct string with fmtstr {}: {}"),
			__FUNCSIG__, std::quoted(fmtStr), e.what());
	}
}
#endif
