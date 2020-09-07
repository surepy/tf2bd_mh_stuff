#pragma once

#define MH_FORMATTER_NONE 0
#define MH_FORMATTER_FMTLIB 1
#define MH_FORMATTER_STL 2

#if __has_include(<format>)

#include <format>
#define MH_FORMATTER MH_FORMATTER_STL
namespace mh::detail::format_hpp
{
	namespace fmtns = ::std;
}

#elif __has_include(<fmt/format.h>)

#include <fmt/format.h>
#include <fmt/ostream.h>
#define MH_FORMATTER MH_FORMATTER_FMTLIB
namespace mh::detail::format_hpp
{
	namespace fmtns = ::fmt;
}

#else
#define MH_FORMATTER MH_FORMATTER_NONE
#endif

#if MH_FORMATTER != MH_FORMATTER_NONE
#include <string_view>

namespace mh
{
	namespace detail::format_hpp
	{
		template<typename T>
		inline constexpr bool enum_class_check_single()
		{
			constexpr bool is_enum_class = std::is_enum_v<T> && !std::is_convertible_v<T, int>;
			static_assert(!is_enum_class, "enum class formatting requires mh::enum_fmt");
			return !is_enum_class;
		}

		template<typename... T>
		inline constexpr bool enum_class_check()
		{
			return (enum_class_check_single<T>() && ...);
		}
	}

	template<typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::enum_class_check<TArgs...>()>>
	inline auto format(const TFmtStr& fmtStr, const TArgs&... args)
	{
		return detail::format_hpp::fmtns::format(fmtStr, args...);
	}

	template<typename TOutputIt, typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::enum_class_check<TArgs...>()>>
	inline auto format_to(TOutputIt&& outputIt, const TFmtStr& fmtStr, const TArgs&... args)
	{
		return detail::format_hpp::fmtns::format(std::forward<TOutputIt>(outputIt), fmtStr, args...);
	}

	template<typename TOutputIt, typename TFmtStr, typename... TArgs,
		typename = std::enable_if_t<detail::format_hpp::enum_class_check<TArgs...>()>>
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
		return format("FORMATTING ERROR @ {}: Unable to construct string with fmtstr {}: {}",
			__FUNCSIG__, std::quoted(fmtStr), e.what());
	}
}
#endif
