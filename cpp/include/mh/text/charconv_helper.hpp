#pragma once

#include <charconv>
#include <optional>
#include <string_view>

namespace mh
{
	struct from_chars_helper_result : public std::from_chars_result
	{
		from_chars_helper_result(const std::from_chars_result& result) : std::from_chars_result(result) {}
		operator bool() const { return ec == std::errc{}; }
	};
	struct to_chars_helper_result : public std::to_chars_result
	{
		to_chars_helper_result(const std::to_chars_result& result) : std::to_chars_result(result) {}
		operator bool() const { return ec == std::errc{}; }
	};

	template<typename T>
	struct from_chars_value : public from_chars_helper_result
	{
		from_chars_value(const std::from_chars_result& result, T v) :
			from_chars_helper_result(result),
			value(v)
		{
		}

		T value;
	};

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline from_chars_helper_result from_chars(const std::string_view& str, T& value, std::chars_format fmt = std::chars_format::general)
	{
		return std::from_chars(str.data(), str.data() + str.size(), value, fmt);
	}
	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline from_chars_helper_result from_chars(const std::string_view& str, T& value, int base = 10)
	{
		return std::from_chars(str.data(), str.data() + str.size(), value, base);
	}

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline std::optional<T> from_chars(const std::string_view& str, size_t* charsRead = nullptr, std::chars_format fmt = std::chars_format::general)
	{
		T value;
		const auto result = from_chars(str, value, fmt);

		if (charsRead)
			*charsRead = result.ptr - (str.data() + str.size());

		return result ? value : std::nullopt;
	}
	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline std::optional<T> from_chars(const std::string_view& str, size_t* charsRead = nullptr, int base = 10)
	{
		T value;
		const auto result = from_chars(str, value, base);

		if (charsRead)
			*charsRead = result.ptr - (str.data() + str.size());

		return result ? std::optional<T>(value) : std::nullopt;
	}

	template<typename T, size_t size, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline to_chars_helper_result to_chars(char (&array)[size], T value, std::chars_format fmt)
	{
		return std::to_chars(array, array + size, value, fmt);
	}
	template<typename T, size_t size, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	inline to_chars_helper_result to_chars(char (&array)[size], T value, std::chars_format fmt, int precision)
	{
		return std::to_chars(array, array + size, value, fmt, precision);
	}
	template<typename T, size_t size, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline to_chars_helper_result to_chars(char (&array)[size], T value, int base = 10)
	{
		return std::to_chars(array, array + size, value, base);
	}
}
