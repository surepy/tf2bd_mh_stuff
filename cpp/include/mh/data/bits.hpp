#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <iostream>

namespace mh
{
	enum class int_for_bits_mode
	{
		exact,
		least,
		fast,
	};

	namespace detail { namespace bits_hpp
	{
		static constexpr unsigned BITS_PER_BYTE = std::numeric_limits<std::underlying_type_t<std::byte>>::digits;

		template<unsigned bits, int_for_bits_mode mode>
		constexpr auto uint_for_bits_helper()
		{
			if constexpr (bits > 64)
			{
				static_assert(bits >= 64 && sizeof(uintmax_t) > 8, "Requested more bits than the platform supports");
				return uintmax_t{};
			}
			else if constexpr (bits > 32)
			{
				if constexpr (mode == int_for_bits_mode::exact)
					return uint64_t{};
				else if constexpr (mode == int_for_bits_mode::least)
					return uint_least64_t{};
				else if constexpr (mode == int_for_bits_mode::fast)
					return uint_fast64_t{};
			}
			else if constexpr (bits > 16)
			{
				if constexpr (mode == int_for_bits_mode::exact)
					return uint32_t{};
				else if constexpr (mode == int_for_bits_mode::least)
					return uint_least32_t{};
				else if constexpr (mode == int_for_bits_mode::fast)
					return uint_fast32_t{};
			}
			else if constexpr (bits > 8)
			{
				if constexpr (mode == int_for_bits_mode::exact)
					return uint16_t{};
				else if constexpr (mode == int_for_bits_mode::least)
					return uint_least16_t{};
				else if constexpr (mode == int_for_bits_mode::fast)
					return uint_fast16_t{};
			}
			else
			{
				if constexpr (mode == int_for_bits_mode::exact)
					return uint8_t{};
				else if constexpr (mode == int_for_bits_mode::least)
					return uint_least8_t{};
				else if constexpr (mode == int_for_bits_mode::fast)
					return uint_fast8_t{};
			}
		}

		template<typename T> constexpr T min(T a, T b) { return a < b ? a : b; }
		template<typename T> constexpr T min(T a, T b, T c) { return min(min(a, b), c); }
		template<typename T> constexpr T min(T a, T b, T c, T d) { return min(min(a, b), min(c, d)); }

		template<typename T> constexpr T max(T a, T b) { return a > b ? a : b; }
		template<typename T> constexpr T max(T a, T b, T c) { return max(max(a, b), c); }

		template<typename T> constexpr auto intprint(T value) { return +value; }
		constexpr auto intprint(std::byte value) { return unsigned(value); }

		template<typename TFunc> constexpr void debug(const TFunc& f)
		{
#if (__cpp_lib_is_constant_evaluated >= 201811)
			if (!std::is_constant_evaluated())
				f();
#endif
		}
	} }

	template<typename T> constexpr std::array<T, std::numeric_limits<std::make_unsigned_t<T>>::digits + 1> BIT_MASKS = []
	{
		using unsigned_t = std::make_unsigned_t<T>;
		constexpr unsigned_t digits = std::numeric_limits<unsigned_t>::digits;
		std::array<T, digits + 1> retVal{};

		for (unsigned_t i = 1; i <= digits; i++)
		{
			constexpr auto maxBits = sizeof(uintmax_t) * std::numeric_limits<unsigned char>::digits;
			constexpr auto mask = std::numeric_limits<uintmax_t>::max();
			retVal[i] = mask >> (maxBits - i);
		}

		return retVal;
	}();

	template<unsigned bits, int_for_bits_mode mode = int_for_bits_mode::exact> using uint_for_bits_t =
		decltype(detail::bits_hpp::uint_for_bits_helper<bits, mode>());
	template<unsigned bits, int_for_bits_mode mode = int_for_bits_mode::exact> using sint_for_bits_t =
		std::make_signed_t<uint_for_bits_t<bits, mode>>;
	template<bool signed_, unsigned bits, int_for_bits_mode mode = int_for_bits_mode::exact> using int_for_bits_t =
		std::conditional_t<signed_, sint_for_bits_t<bits, mode>, uint_for_bits_t<bits, mode>>;

	enum class bit_clear_mode
	{
		// Doesn't clear anything. Only OR's stuff into dst. Fastest.
		none,

		// Clears only the bits being written, leaving others untouched.
		clear_bits,

		// Clears all touched TDst objects. Likely slower, requires additional branching
		clear_objects,
	};

	template<size_t bits_to_copy,
		size_t src_offset = 0, size_t dst_offset = 0,
		bit_clear_mode clear_mode = bit_clear_mode::none,
		typename TSrc = void, typename TDst = void>
	constexpr void bit_copy(TDst* dst, const TSrc* src)
	{
		using namespace detail::bits_hpp;

		static_assert(std::is_same_v<TSrc, std::byte> || std::is_unsigned_v<TSrc>);
		static_assert(std::is_same_v<TDst, std::byte> || std::is_unsigned_v<TDst>);

		using TSrcR = std::conditional_t<std::is_same_v<TSrc, std::byte>, std::underlying_type_t<std::byte>, TSrc>;
		using TDstR = std::conditional_t<std::is_same_v<TDst, std::byte>, std::underlying_type_t<std::byte>, TDst>;

		constexpr size_t bits_per_src = sizeof(TSrcR) * BITS_PER_BYTE;
		constexpr size_t bits_per_dst = sizeof(TDstR) * BITS_PER_BYTE;

		for (size_t b = 0; b < bits_to_copy; )
		{
			const size_t dst_byte = (b + dst_offset) / bits_per_dst;
			const size_t dst_bit = (b + dst_offset) % bits_per_dst;
			const size_t max_dst_writebits = bits_per_dst - dst_bit;

			const size_t src_byte = (b + src_offset) / bits_per_src;
			const size_t src_bit = (b + src_offset) % bits_per_src;
			const size_t max_src_readbits = bits_per_src - src_bit;

			const size_t loop_bits = min(max_dst_writebits, max_src_readbits, bits_to_copy - b);
			//debug([&]{ std::cerr << "loop_bits: " << loop_bits << '\n'; });

			TDstR dst_value = TDstR(dst[dst_byte]);
			TSrcR src_value = (TSrcR(src[src_byte]) >> src_bit) & BIT_MASKS<TSrcR>[loop_bits];

			if constexpr (clear_mode == bit_clear_mode::clear_objects)
			{
				if (dst_bit == 0)
					dst_value = {};
			}
			else if constexpr (clear_mode == bit_clear_mode::clear_bits)
			{
				dst_value &= ~(BIT_MASKS<TDstR>[loop_bits] << dst_bit);
			}

			dst_value |= TDstR(src_value) << dst_bit;

			dst[dst_byte] = TDst(dst_value);

			b += loop_bits;
		}
	}

	template<typename TOut,
		size_t bits_to_read = sizeof(TOut) * detail::bits_hpp::BITS_PER_BYTE,
		unsigned src_offset = 0,
		typename TIn = void>
	constexpr TOut bit_read(const TIn* src)
	{
		static_assert(bits_to_read <= (sizeof(TOut) * detail::bits_hpp::BITS_PER_BYTE));
		TOut retVal{};
		bit_copy<bits_to_read, src_offset, 0, bit_clear_mode::none>(&retVal, src);
		return retVal;
	}
}
