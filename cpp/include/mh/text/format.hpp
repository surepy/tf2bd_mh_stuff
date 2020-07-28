#pragma once

#define MH_FORMATTER_NONE 0
#define MH_FORMATTER_FMTLIB 1
#define MH_FORMATTER_STL 2

#if __has_include(<format>)

#include <format>
#define MH_FORMATTER MH_FORMATTER_STL
namespace mh
{
	using std::format;
	using std::format_to_n;
}

#elif __has_include(<fmt/format.h>)

#include <fmt/format.h>
#include <fmt/ostream.h>
#define MH_FORMATTER MH_FORMATTER_FMTLIB
namespace mh
{
	using fmt::format;
	using fmt::format_to_n;
}

#else
#define MH_FORMATTER MH_FORMATTER_NONE
#endif

#if MH_FORMATTER != MH_FORMATTER_NONE
#include <string_view>

namespace mh
{
	template<typename CharT, size_t size, typename Traits = std::char_traits<CharT>, typename... TArgs>
	inline constexpr std::basic_string_view<CharT, Traits> format_to(CharT(&dest)[size], TArgs&&... args)
	{
		if (size == 0)
			return {};

		auto result = format_to_n(dest, size, std::forward<TArgs>(args)...);

		if (result.size >= size)
			result.size = size - 1;

		dest[result.size] = 0;

		return std::basic_string_view<CharT, Traits>(dest, result.size);
	}
	template<typename OutputIt, typename... TArgs>
	inline constexpr OutputIt format_to(OutputIt out, TArgs&&... args)
	{
		using decayed_t = std::remove_const_t<std::decay_t<OutputIt>>;
		static_assert(!std::is_same_v<decayed_t, char*>);
		static_assert(!std::is_same_v<decayed_t, const char*>);
		static_assert(!std::is_same_v<decayed_t, wchar_t*>);
		static_assert(!std::is_same_v<decayed_t, const wchar_t*>);

#if MH_FORMATTER == MH_FORMATTER_STL
		return std::format_to(std::forward<OutputIt>(out), std::forward<TArgs>(args)...);
#elif MH_FORMATTER == MH_FORMATTER_FMTLIB
		return fmt::format_to(std::forward<OutputIt>(out), std::forward<TArgs>(args)...);
#else
		static_assert(false, "Action not supported with this formatter");
#endif
	}
}
#endif
