#pragma once

#include <cassert>
#include <cuchar>
#include <cwchar>
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

		// char underlying type
		template<typename T> struct cut { using type = T; };
		template<> struct cut<char8_t> { using type = char; };
		template<> struct cut<char16_t> { using type = wchar_t; };
		template<> struct cut<char32_t> { using type = uint32_t; };

		template<typename T>
		auto as_cut(T* value)
		{
			using type = typename cut<T>::type;
			return reinterpret_cast<type*>(value);
		}

		template<typename T> constexpr bool is_utf_v =
			std::is_same_v<T, char8_t> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

		template<typename T, typename = std::enable_if_t<std::is_same_v<T, char8_t>>>
		inline auto convert_to_mb(char* buf, T from, std::mbstate_t& state) ->
			decltype(std::c8rtomb(buf, from, &state))
		{
			return std::c8rtomb(buf, from, &state);
		}
		inline std::size_t convert_to_mb(char* buf, char16_t from, std::mbstate_t& state)
		{
			return std::c16rtomb(buf, from, &state);
		}
		inline std::size_t convert_to_mb(char* buf, char32_t from, std::mbstate_t& state)
		{
			return std::c32rtomb(buf, from, &state);
		}

		template<typename T, typename = std::enable_if_t<std::is_same_v<T, char8_t>>>
		inline auto convert_to_utf(T* buf, const char* mb, std::size_t mbmax, std::mbstate_t& state) ->
			decltype(std::mbrtoc8(buf, mb, mbmax, &state))
		{
			return std::mbrtoc8(buf, mb, mbmax, &state);
		}
		inline std::size_t convert_to_utf(char16_t* buf, const char* mb, std::size_t mbmax, std::mbstate_t& state)
		{
			return std::mbrtoc16(buf, mb, mbmax, &state);
		}
		inline std::size_t convert_to_utf(char32_t* buf, const char* mb, std::size_t mbmax, std::mbstate_t& state)
		{
			return std::mbrtoc32(buf, mb, mbmax, &state);
		}

		[[nodiscard]] inline constexpr char32_t convert_to_u32(const char8_t*& it, const char8_t* end)
		{
			unsigned continuationBytes = 0;

			uint32_t retVal{};

			// https://en.wikipedia.org/wiki/UTF-8#Examples
			const uint8_t firstByte = uint8_t(*it++);
			if ((firstByte & 0b1111'0000) == 0b1111'0000)
			{
				retVal = uint32_t(0b0000'0111 & firstByte) << 18;
				continuationBytes = 3;
			}
			else if ((firstByte & 0b1110'0000) == 0b1110'0000)
			{
				retVal = uint32_t(0b0000'1111 & firstByte) << 12;
				continuationBytes = 2;
			}
			else if ((firstByte & 0b1100'0000) == 0b1100'0000)
			{
				retVal = uint32_t(0b0001'1111 & firstByte) << 6;
				continuationBytes = 1;
			}
			else //if ((firstByte & 0b1000'0000) == 0b0000'0000)
			{
				return char32_t(0b0111'1111 & firstByte);
				//retVal = 0b0111'1111 & firstByte;
				//continuationBytes = 0;
			}

			// Stop at continuationBytes == 0 or reaching the end iterator, whichever is sooner
			for (; it != end && continuationBytes--; ++it)
			{
				retVal |= uint32_t((*it) & 0b0011'1111) << (6 * continuationBytes);
			}

			return char32_t(retVal);
		}

		[[nodiscard]] inline constexpr char32_t convert_to_u32(const char16_t*& it, const char16_t* end)
		{
			// https://en.wikipedia.org/wiki/UTF-16#Description
			constexpr uint16_t TOP6_MASK = 0xFC00;

			const uint16_t firstByte = uint16_t(*it++);
			if ((firstByte & TOP6_MASK) == 0xD800)
			{
				uint32_t retVal = uint32_t(firstByte & (~TOP6_MASK)) << 10;

				if (it != end)
					retVal |= uint32_t(uint32_t(*it++) & (~TOP6_MASK));

				return 0x10000 + retVal;
			}

			return firstByte;
		}

		[[nodiscard]] inline constexpr size_t convert_to_u16(char32_t in, char16_t out[2])
		{
			constexpr uint16_t TOP6_MASK = 0xFC00;
			constexpr uint16_t BYTE0_MARKER = 0xD800;
			constexpr uint16_t BYTE1_MARKER = 0xDC00;

			uint32_t in_raw = in;
			if (in_raw > 0xFFFF || ((in_raw & TOP6_MASK) == BYTE0_MARKER))
			{
				in_raw -= 0x10000;

				// Two bytes
				out[0] = BYTE0_MARKER | ((in_raw >> 10) & (~TOP6_MASK));
				out[1] = BYTE1_MARKER | (in_raw & (~TOP6_MASK));
				return 2;
			}
			else
			{
				// One byte
				out[0] = in_raw & 0xFFFF;
				return 1;
			}
		}

		template<typename T>
		[[nodiscard]] inline constexpr size_t convert_to_u8(char32_t in, T out[4])
		{
			const uint32_t in_raw = in;

			if (in_raw <= 0x7F)
			{
				out[0] = in_raw & 0b0111'1111;
				return 1;
			}
			else if (in_raw <= 0x7FF)
			{
				out[0] = 0b1100'0000 | ((in_raw >> 6) & 0b0001'1111);
				out[1] = 0b1000'0000 | (in_raw & 0b0011'1111);
				return 2;
			}
			else if (in_raw <= 0xFFFF)
			{
				out[0] = 0b1110'0000 | ((in_raw >> 12) & 0b0000'1111);
				out[1] = 0b1000'0000 | ((in_raw >> 6) & 0b0011'1111);
				out[2] = 0b1000'0000 | ((in_raw >> 0) & 0b0011'1111);
				return 3;
			}
			else if (in_raw <= 0x10FFFF)
			{
				out[0] = 0b1111'0000 | ((in_raw >> 18) & 0b0000'0111);
				out[1] = 0b1000'0000 | ((in_raw >> 12) & 0b0011'1111);
				out[2] = 0b1000'0000 | ((in_raw >> 6) & 0b0011'1111);
				out[3] = 0b1000'0000 | ((in_raw >> 0) & 0b0011'1111);
				return 4;
			}

			return -1;
		}

		template<typename T>
		inline void utf_to_mb(std::string& out, T u32, std::mbstate_t& state)
		{
			static_assert(is_utf_v<T>);
#ifndef MB_LEN_MAX
			constexpr int MB_LEN_MAX = 64;
#endif
			char tempBuf[MB_LEN_MAX];

			const auto bytesWritten = convert_to_mb(tempBuf, u32, state);
			if (bytesWritten == size_t(-1))
				throw std::invalid_argument("Failed to convert all characters");
			else if (bytesWritten > sizeof(tempBuf))
				throw std::runtime_error("Stack corruption");

			out.append(tempBuf, bytesWritten);
		}

		template<typename From, typename To, typename TEnable = void> struct change_encoding_impl;

		template<typename T>
		struct change_encoding_impl<T, T, std::enable_if_t<std::is_same_v<T, T>>>
		{
			std::basic_string<T> operator()(const T* begin, const T* end) const
			{
				return std::basic_string<T>(begin, end - begin);
			};
		};

		template<typename From>
		struct change_encoding_impl<From, char, std::enable_if_t<is_utf_v<From>>>
		{
			std::string operator()(const From* begin, const From* end) const
			{
				std::string retVal;
				std::mbstate_t state{};

				for (auto it = begin; it != end; ++it)
					utf_to_mb(retVal, *it, state);

				return retVal;
			}
		};

		template<typename To>
		struct change_encoding_impl<char, To, std::enable_if_t<!std::is_same_v<char, To>>>
		{
			std::basic_string<To> operator()(const char* begin, const char* end) const
			{
				std::basic_string<To> retVal;

				std::mbstate_t state{};
				for (auto it = begin; it != end; )
				{
					char32_t u32;
					const auto result = convert_to_utf(&u32, it, end - it, state);
					if (result == 0)
					{
						// Stored the null character, we're an std::string so we can continue
						retVal += u32;
						state = {};
						++it;
					}
					else if (result == -3)
					{
						// Another charN_t needs to be written to the output stream
						retVal += u32;
					}
					else if (result == -2)
					{
						throw std::invalid_argument("Segment forms invalid UTF character sequence");
					}
					else if (result == -1)
					{
						throw std::runtime_error("Encoding error");
					}
					else
					{
						it += result;
						retVal += u32;
					}
				}

				return retVal;
			}
		};

		template<>
		struct change_encoding_impl<char, wchar_t>
		{
			std::basic_string<wchar_t> operator()(const char* begin, const char* end) const
			{
				std::basic_string<wchar_t> retVal;

				std::mbstate_t state{};
				for (auto it = begin; it != end; )
				{
					wchar_t wc;
					const auto result = std::mbrtowc(&wc, it, end - it, &state);

					if (result == 0)
					{
						// Stored the null character, we're an std::string so we can continue
						retVal += wc;
						state = {};
						++it;
					}
					else if (result == -2)
					{
						throw std::invalid_argument("Segment forms invalid multibyte character sequence");
					}
					else if (result == -1)
					{
						throw std::runtime_error("Encoding error");
					}
					else
					{
						it += result;
						retVal += wc;
					}
				}

				return retVal;
			}
		};

		template<>
		struct change_encoding_impl<wchar_t, char>
		{
			std::basic_string<char> operator()(const wchar_t* begin, const wchar_t* end) const
			{
				std::basic_string<char> retVal;

				std::mbstate_t state{};
				for (auto it = begin; it != end; )
				{
#ifndef MB_LEN_MAX
					constexpr int MB_LEN_MAX = 64;
#endif
					char buf[MB_LEN_MAX];
					const auto result = std::wcrtomb(buf, *it, &state);

					if (result == -1)
					{
						throw std::invalid_argument("Invalid wide character");
					}
					else
					{
						assert(result != 0);
						it += result;
						retVal.append(buf, result);
					}
				}

				return retVal;
			}
		};

		template<typename From>
		struct change_encoding_impl<From, wchar_t>
		{
			std::basic_string<wchar_t> operator()(const From* begin, const From* end) const
			{
				auto converted = change_encoding_impl<From, char>(begin, end);
				return change_encoding_impl<char, wchar_t>(converted.data(), converted.data() + converted.size());
			}
		};

		template<typename To>
		struct change_encoding_impl<wchar_t, To, std::enable_if_t<!std::is_same_v<wchar_t, To>>>
		{
			std::basic_string<To> operator()(const wchar_t* begin, const wchar_t* end) const
			{
				auto converted = change_encoding_impl<wchar_t, char>{}(begin, end);
				return change_encoding_impl<char, To>{}(converted.data(), converted.data() + converted.size());
			}
		};

		template<typename From, typename To>
		struct change_encoding_impl<From, To, std::enable_if_t<!std::is_same_v<From, To> && is_utf_v<From> && is_utf_v<To>>>
		{
			std::basic_string<To> operator()(const From* begin, const From* end) const
			{
				std::basic_string<To> retVal;

				for (auto it = begin; it != end; )
				{
					char32_t u32;

					if constexpr (std::is_same_v<From, char32_t>)
						u32 = *it++;
					else
						u32 = convert_to_u32(it, end);

					if constexpr (std::is_same_v<To, char8_t>)
					{
#ifdef _DEBUG
						uint8_t buf2[4];
						const auto dummy = convert_to_u8(u32, buf2);
#endif

						char8_t buf[4];
						const size_t chars = convert_to_u8(u32, buf);
						retVal.append(buf, chars);
					}
					else if constexpr (std::is_same_v<To, char16_t>)
					{
						char16_t buf[2];
						const auto chars = convert_to_u16(u32, buf);
						retVal.append(buf, chars);
					}
					else //if constexpr (std::is_same_v<To, char32_t>)
					{
						retVal += u32;
					}
				}

				return retVal;
			}
		};

		template<>
		struct change_encoding_impl<char8_t, char>
		{
			std::basic_string<char> operator()(const char8_t* begin, const char8_t* end) const
			{
				std::basic_string<char> retVal;

				std::mbstate_t state{};
				for (auto it = begin; it != end; )
				{
					const char32_t u32 = convert_to_u32(it, end);
					utf_to_mb(retVal, u32, state);
				}

				return retVal;
			}
		};
	}

	template<typename To, typename From, typename FromTraits = std::char_traits<From>>
	auto change_encoding(const std::basic_string_view<From, FromTraits>& input) ->
		decltype(detail::codecvt_hpp::change_encoding_impl<From, To>{}(input.data(), input.data() + input.size()))
	{
		const detail::codecvt_hpp::change_encoding_impl<From, To> impl;
		return impl(input.data(), input.data() + input.size());
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
