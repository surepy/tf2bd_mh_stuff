#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace mh
{
	namespace detail::stringops_hpp
	{
		template<typename CharT> inline static constexpr std::initializer_list<CharT> WHITESPACE_CHARS =
		{
			CharT(' '),
			CharT('\n'),
			CharT('\r'),
			CharT('\t'),
			CharT('\v'),
			CharT('\f'),
		};
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] std::basic_string<CharT, Traits, Alloc> trim_start(
		std::basic_string<CharT, Traits, Alloc>&& str, std::initializer_list<CharT> trimChars)
	{
		size_t charsToTrim = 0;
		for (CharT c : str)
		{
			bool found = false;
			for (CharT trimChar : trimChars)
			{
				if (c == trimChar)
				{
					found = true;
					break;
				}
			}

			if (!found)
				break;

			charsToTrim++;
		}

		str.erase(0, charsToTrim);
		return std::move(str);
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> trim_start(std::basic_string<CharT, Traits, Alloc>&& str)
	{
		return mh::trim_start(std::move(str), mh::detail::stringops_hpp::WHITESPACE_CHARS<CharT>);
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] std::basic_string<CharT, Traits, Alloc> trim_end(
		std::basic_string<CharT, Traits, Alloc>&& str, std::initializer_list<CharT> trimChars)
	{
		size_t charsToTrim = 0;
		for (size_t i = str.size(); i--; )
		{
			CharT c = str[i];

			bool found = false;
			for (CharT trimChar : trimChars)
			{
				if (c == trimChar)
				{
					found = true;
					break;
				}
			}

			if (!found)
				break;

			charsToTrim++;
		}

		str.erase(str.size() - charsToTrim);
		return std::move(str);
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> trim_end(std::basic_string<CharT, Traits, Alloc>&& str)
	{
		return mh::trim_end(std::move(str), mh::detail::stringops_hpp::WHITESPACE_CHARS<CharT>);
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> trim(std::basic_string<CharT, Traits, Alloc>&& str, std::initializer_list<CharT> trimChars)
	{
		return mh::trim_start(mh::trim_end(std::move(str), trimChars), trimChars);
	}

	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> trim(std::basic_string<CharT, Traits, Alloc>&& str)
	{
		return mh::trim(std::move(str), mh::detail::stringops_hpp::WHITESPACE_CHARS<CharT>);
	}

	template<typename CharT, typename Traits, typename Alloc>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> find_and_replace(
		std::basic_string<CharT, Traits, Alloc> str, const std::basic_string_view<CharT, Traits>& find,
		const std::basic_string_view<CharT, Traits>& replace)
	{
		size_t curIndex = 0;
		while (true)
		{
			curIndex = str.find(find, curIndex);
			if (curIndex == str.npos)
				break;

			str.replace(curIndex, find.size(), replace);
			curIndex += replace.size();
		}

		return std::move(str);
	}

	template<typename CharT, typename Traits, typename Alloc = std::allocator<CharT>>
	[[nodiscard]] inline std::basic_string<CharT, Traits, Alloc> find_and_replace(
		const std::basic_string_view<CharT, Traits>& str, const std::basic_string_view<CharT, Traits>& find,
		const std::basic_string_view<CharT, Traits>& replace)
	{
		return find_and_replace<CharT, Traits, Alloc>(std::basic_string<CharT, Traits, Alloc>(str), find, replace);
	}
}
