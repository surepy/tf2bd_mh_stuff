#pragma once

#include <ostream>
#include <streambuf>
#include <string>

namespace mh
{
	template<typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	class basic_strwrapperstream final : std::basic_streambuf<CharT, Traits>, public std::basic_ostream<CharT, Traits>
	{
		using ostream_type = std::basic_ostream<CharT, Traits>;
		using string_type = std::basic_string<CharT, Traits, Alloc>;
		using streambuf_type = std::basic_streambuf<CharT, Traits>;
		using int_type = typename streambuf_type::int_type;

	public:
		basic_strwrapperstream(std::basic_string<CharT, Traits, Alloc>& string) :
			ostream_type(this),
			m_String(string)
		{
		}

	protected:
		std::streamsize xsputn(const CharT* s, std::streamsize count) override
		{
			m_String.append(s, s + count);
			return count;
		}

		int_type overflow(int_type ch = Traits::eof()) override
		{
			if (ch != Traits::eof())
				m_String.push_back(static_cast<CharT>(ch));

			return ch;
		}

	private:
		std::basic_string<CharT, Traits, Alloc>& m_String;
	};

	using strwrapperstream = basic_strwrapperstream<>;
}

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
inline std::string& operator<<(std::basic_string<CharT, Traits, Alloc>& str, const T& value)
{
	mh::basic_strwrapperstream<CharT, Traits, Alloc> stream(str);
	stream << value;
	return str;
}

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT>>
inline std::string operator<<(std::basic_string<CharT, Traits, Alloc>&& str, const T& value)
{
	mh::basic_strwrapperstream<CharT, Traits, Alloc> stream(str);
	stream << value;
	return std::move(str);
}
