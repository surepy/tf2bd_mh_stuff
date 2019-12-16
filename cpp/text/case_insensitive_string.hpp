#pragma once

#include <string>
#include <string_view>

namespace mh
{
	template<typename BaseTraits = std::char_traits<char>>
	class case_insensitive_char_traits : public BaseTraits
	{
	public:
		using char_type = typename BaseTraits::char_type;
		using int_type = typename BaseTraits::int_type;
		using off_type = typename BaseTraits::off_type;
		using pos_type = typename BaseTraits::pos_type;
		using state_type = typename BaseTraits::state_type;

		case_insensitive_char_traits() = default;
		case_insensitive_char_traits(const BaseTraits& base) : BaseTraits(base) {}
		case_insensitive_char_traits(BaseTraits&& base) : BaseTraits(base) {}

		static int compare(const char_type* s1, const char_type* s2, size_t count)
		{
			while (count--)
			{
				if (*s1 == *s2)
					continue;

				const auto c1 = std::toupper(*s1);
				const auto c2 = std::toupper(*s2);

				if (c1 < c2)
					return -1;
				if (c1 > c2)
					return 1;
			}

			return 0;
		}

		template<typename Traits1, typename Traits2>
		static int compare(const std::basic_string_view<char_type, Traits1>& s1, const std::basic_string_view<char_type, Traits2>& s2)
		{
			if (auto result = compare(s1.data(), s2.data(), s1.size() < s2.size() ? s1.size() : s2.size()); result != 0)
				return result;

			if (s1.size() < s2.size())
				return -1;
			if (s1.size() > s2.size())
				return 1;

			return 0;
		}

		static bool eq(char_type c1, char_type c2) { return std::toupper(c1) == std::toupper(c2); }
		static bool lt(char_type c1, char_type c2) { return std::toupper(c1) < std::toupper(c2); }

		static const char_type* find(const char_type* p, size_t count, const char_type& ch)
		{
			while (count-- && *p)
			{
				if (*p == ch)
					return p;

				p++;
			}
		}
	};

	template<typename CharT, typename Traits = std::char_traits<CharT>>
	auto case_insensitive_view(const std::basic_string_view<CharT, Traits>& sv)
	{
		return std::basic_string_view<CharT, case_insensitive_char_traits<Traits>>(sv.data(), sv.size());
	}
	template<typename CharT, typename Traits = std::char_traits<CharT>>
	auto case_insensitive_view(const CharT* str)
	{
		return case_insensitive_view(std::basic_string_view<CharT, Traits>(str));
	}
	template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	auto case_insensitive_view(const std::basic_string<CharT, Traits, Alloc>& str)
	{
		return case_insensitive_view(std::basic_string_view<CharT, Traits>(str));
	}

	template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	auto case_insensitive_string(const std::basic_string<CharT, Traits, Alloc>& str)
	{
		return std::basic_string<CharT, case_insensitive_char_traits<Traits>, Alloc>(str.data(), str.size(), str.get_allocator());
	}
	template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	auto case_insensitive_string(const std::basic_string_view<CharT, Traits>& str)
	{
		return std::basic_string<CharT, case_insensitive_char_traits<Traits>, Alloc>(str.data(), str.size());
	}
	template<typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	auto case_insensitive_string(const CharT* str)
	{
		return case_insensitive_string<CharT, Traits, Alloc>(std::basic_string_view<CharT, Traits>(str));
	}
}
