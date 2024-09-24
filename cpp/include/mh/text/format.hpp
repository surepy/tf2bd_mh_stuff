#pragma once
// NOTICE: do not use this file! it's largely broken and I recommend you use fmt::format directly instead!

#define MH_FORMATTER_NONE 0
#define MH_FORMATTER_FMTLIB 1
#define MH_FORMATTER_STL 2

#if __has_include(<fmt/format.h>)

#include <fmt/format.h>

#if __has_include(<fmt/xchar.h>)
	#include <fmt/xchar.h>
#endif
#if __has_include(<fmt/ostream.h>)
	#include <fmt/ostream.h>
#endif
#if __has_include(<fmt/chrono.h>)
	#include <fmt/chrono.h>
#endif
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
#include <string>
#include <string_view>
#include <iomanip>

namespace mh
{
	using detail::format_hpp::fmtns::format_error;
	using detail::format_hpp::fmtns::formatter;
	using detail::format_hpp::fmtns::basic_format_parse_context;
	using detail::format_hpp::fmtns::basic_format_context;
	using detail::format_hpp::fmtns::format_parse_context;
	using detail::format_hpp::fmtns::wformat_parse_context;
	using detail::format_hpp::fmtns::format_context;
	using detail::format_hpp::fmtns::wformat_context;

	namespace detail::format_hpp
	{
		template<typename T>
		struct make_dependent
		{
			using type = T;
		};

		template<typename T, typename TChar, typename = typename make_dependent<mh::formatter<T, TChar>>::type>
		inline constexpr bool formatter_check(T*, TChar*) { return true; }

		inline constexpr bool formatter_check(void*, void*) { return false; }

		// Intellisense dies if you SFINAE on an explicit operator call
#ifndef __INTELLISENSE__
		template<typename T, typename TTo, typename = std::enable_if_t<std::is_convertible_v<T, TTo>>>
		inline constexpr auto implicit_conversion_check(T* t, TTo*) -> decltype(t->operator TTo())
		{
			return true;
		}
#endif

		inline constexpr bool implicit_conversion_check(void*, void*) { return false; }

		template<typename T>
		inline constexpr bool check_type_single()
		{
			using type = std::decay_t<T>;

			constexpr bool HAS_FORMATTER = formatter_check((type*)nullptr, (char*)nullptr);
			constexpr bool HAS_IMPLICIT_CONVERSION = implicit_conversion_check((type*)nullptr, (bool*)nullptr);
			constexpr bool IS_ENUM_CLASS = std::is_enum_v<type> && !std::is_convertible_v<type, int>;

			static_assert(HAS_FORMATTER, "Formatter for type missing");
			static_assert(!HAS_IMPLICIT_CONVERSION, "Formatting this type will result in it being formatted as one of its implicit converted types");
			static_assert(!IS_ENUM_CLASS, "enum class formatting requires mh::enum_fmt");

			return HAS_FORMATTER && !IS_ENUM_CLASS;
		}

		template<typename... T>
		inline constexpr bool check_type()
		{
			return (check_type_single<T>() && ...);
		}
	}

#if MH_FORMATTER == MH_FORMATTER_FMTLIB
	template<typename TChar, typename T>
	inline auto fmtarg(const TChar* argName, const T& argValue)
	{
		return detail::format_hpp::fmtns::arg(argName, argValue);
	}
#endif

	using format_args = detail::format_hpp::fmtns::format_args;
	using wformat_args = detail::format_hpp::fmtns::wformat_args;

	template<typename... TArgs, typename = std::enable_if_t<detail::format_hpp::check_type<TArgs...>()>>
	inline auto make_format_args(const TArgs&... args) ->
		decltype(detail::format_hpp::fmtns::make_format_args(args...))
	{
		return detail::format_hpp::fmtns::make_format_args(args...);
	}

	// Migrate OK!
	template<typename... TFmtStr, typename = std::enable_if_t<detail::format_hpp::check_type<TFmtStr...>()>>
	constexpr inline auto format(detail::format_hpp::fmtns::format_string<TFmtStr...> fmtStr, TFmtStr&&... args) ->
		decltype(detail::format_hpp::fmtns::format(fmtStr, std::forward<TFmtStr>(args)...))
	{
		return detail::format_hpp::fmtns::format(fmtStr, std::forward<TFmtStr>(args)...);
	}

	template<typename S>
	inline auto runtime(const S& s) {
#if MH_FORMATTER == MH_FORMATTER_FMTLIB
		return detail::format_hpp::fmtns::runtime(s);
#else

		return s;
#endif
	}

	// only used in try_vformat
	template<typename TFmtStr, typename TFmtArgs>
	inline auto vformat(const TFmtStr& fmtStr, const TFmtArgs& args) ->
		decltype(detail::format_hpp::fmtns::vformat(fmtStr, args))
	{
		return detail::format_hpp::fmtns::vformat(fmtStr, args);
	}

	// Migrate OK!
	template<typename TOutputIt, typename... TFmtStr, typename = std::enable_if_t<detail::format_hpp::check_type<TFmtStr...>()>>
	constexpr inline auto format_to(TOutputIt&& outputIt, detail::format_hpp::fmtns::format_string<TFmtStr...> fmtStr, TFmtStr&&... args) ->
		decltype(detail::format_hpp::fmtns::format_to(outputIt, fmtStr, std::forward<TFmtStr>(args)...))
	{
		return detail::format_hpp::fmtns::format_to(outputIt, fmtStr, std::forward<TFmtStr>(args)...);
	}

	template<typename TContainer, typename... TFmtStr, typename = std::enable_if_t<detail::format_hpp::check_type<TFmtStr...>()>>
	inline auto format_to_container(TContainer& container, detail::format_hpp::fmtns::format_string<TFmtStr...> fmtStr, TFmtStr&&... args)
	{
		return ::mh::format_to(std::back_inserter(container), fmtStr, args...);
	}

	// Migrate OK!
	template<typename TOutputIt, typename... TFmtStr, typename = std::enable_if_t<detail::format_hpp::check_type<TFmtStr...>()>>
	constexpr inline auto format_to_n(TOutputIt&& outputIt, size_t n, detail::format_hpp::fmtns::format_string<TFmtStr...> fmtStr, TFmtStr&&... args) ->
		decltype(detail::format_hpp::fmtns::format_to_n(outputIt, n, fmtStr, std::forward<TFmtStr>(args)...))
	{
		return detail::format_hpp::fmtns::format_to_n(outputIt, n, fmtStr, std::forward<TFmtStr>(args)...);
	}

	// Migrate OK!
	template<typename... TFmtStr>
	constexpr inline auto try_format(fmt::format_string<TFmtStr...> fmtStr, TFmtStr&&... args)
		-> decltype(::mh::format(fmtStr, args...)) try
	{
		return ::mh::format(fmtStr, args...);
	}
	catch (const format_error& e)
	{
		// std::quoted can't be evaluated constexpr-ly, so we're just removing std::quoted
		return ::mh::format(MH_FMT_STRING("FORMATTING ERROR: Unable to construct string with fmtstr \"{}\": {}"), fmtStr, e.what());
	}

	template<typename TFmtStr, typename TFmtArgs>
	inline auto try_vformat(const TFmtStr& fmtStr, const TFmtArgs& args) try
	{
		return ::mh::vformat(fmtStr, args);
	}
	catch (const format_error& e)
	{
		using char_type_t = std::decay_t<decltype(fmtStr[0])>;
		if constexpr (std::is_same_v<char_type_t, char>)
		{
			return ::mh::format(MH_FMT_STRING("FORMATTING ERROR: Unable to construct string with fmtstr {}: {}"), std::quoted(fmtStr), e.what());
		}
		else if constexpr (std::is_same_v<char_type_t, wchar_t>)
		{
			// Can't print error message from exception because fmt does not handle conversion from char -> wchar_t on its own unfortunately
			return ::mh::format(MH_FMT_STRING(L"FORMATTING ERROR: Unable to construct string with fmtstr {}"), std::quoted(fmtStr));
		}
		else
		{
			// Other character types are a compile error for now
		}
	}

	template<typename TChar = char, typename TTraits = std::char_traits<TChar>, typename TAlloc = std::allocator<TChar>, typename... TArgs>
	inline std::basic_string<TChar, TTraits, TAlloc> build_string(const TArgs&... args)
	{
		std::basic_string<TChar, TTraits, TAlloc> str;

		auto inserter = std::back_inserter(str);
		(format_to(inserter, MH_FMT_STRING("{}"), args), ...);

		return str;
	}
}
#endif
