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

		template<typename T> constexpr auto intprint(T value) { return +value; }
		constexpr auto intprint(std::byte value) { return unsigned(value); }

		template<typename TFunc> constexpr void debug(const TFunc& f)
		{
#if (__cpp_lib_is_constant_evaluated > 201811)
			//if (!std::is_constant_evaluated())
			//	f();
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

		// Clears the whole byte when writing into it. Faster in some circumstances vs clear_bits.
		clear_bytes,

		// Clears only the bits being written, leaving others untouched.
		clear_bits,
	};

	template<size_t bits_to_copy,
		unsigned src_offset = 0, unsigned dst_offset = 0,
		bit_clear_mode clear_mode = bit_clear_mode::none,
		typename TSrc = void, typename TDst = void>
	constexpr void bit_copy(const TSrc* src, TDst* dst)
	{
		using namespace detail::bits_hpp;

		static_assert(src_offset < BITS_PER_BYTE);
		static_assert(dst_offset < BITS_PER_BYTE);
		static_assert(std::is_same_v<TSrc, std::byte> || std::is_unsigned_v<TSrc>);
		static_assert(std::is_same_v<TDst, std::byte> || std::is_unsigned_v<TDst>);

		using TSrcR = std::conditional_t<std::is_same_v<TSrc, std::byte>, std::underlying_type_t<std::byte>, TSrc>;
		using TDstR = std::conditional_t<std::is_same_v<TDst, std::byte>, std::underlying_type_t<std::byte>, TDst>;

		constexpr size_t bits_per_src = sizeof(TSrcR) * BITS_PER_BYTE;
		constexpr size_t bits_per_dst = sizeof(TDstR) * BITS_PER_BYTE;
		static_assert(bits_to_copy <= bits_per_dst);

		size_t bits_remaining = bits_to_copy;

		TDst* out_buffer = dst;
		size_t out_buffer_bits = bits_per_dst - dst_offset;
		if constexpr (clear_mode == bit_clear_mode::clear_bytes)
			*out_buffer = {};

		const TSrc* in_buffer = src;
		size_t in_buffer_bits = bits_per_src - src_offset;

		const auto copy_to_output = [&]
		{
			// gcc bug with constexpr and lambda capture by reference
			constexpr size_t bits_per_src2 = sizeof(TSrcR) * BITS_PER_BYTE;
			constexpr size_t bits_per_dst2 = sizeof(TDstR) * BITS_PER_BYTE;

			size_t bits_this_loop{};
			if constexpr (bits_per_src2 >= bits_per_dst2)
				bits_this_loop = min(in_buffer_bits, out_buffer_bits, bits_per_dst, bits_remaining);
			else // bits_per_src < bits_per_dst
				bits_this_loop = min(in_buffer_bits, out_buffer_bits, bits_per_src, bits_remaining);

			const TDstR mask = BIT_MASKS<TDstR>[bits_this_loop];

			const auto src_value = TSrcR(*in_buffer) >> (bits_per_src - in_buffer_bits);
			const auto cur_dst_offset = bits_per_dst - out_buffer_bits;
			const TDstR dst_value = TDstR(src_value & mask) << cur_dst_offset;

			if constexpr (clear_mode == bit_clear_mode::clear_bits)
			{
				const TDstR clear_mask = TDstR(~(mask << cur_dst_offset));
				debug([&]{ std::cerr << "clearing via & with " << +clear_mask << '\n'; });
				(*out_buffer) &= clear_mask;
			}

			debug([&]{ std::cerr << "writing " << +dst_value << '\n'; });
			(*out_buffer) = TDst(TDstR(*out_buffer) | TDstR(dst_value));

			in_buffer_bits -= bits_this_loop;
			out_buffer_bits -= bits_this_loop;
			bits_remaining -= bits_this_loop;
		};

		copy_to_output();

		debug([&] { std::cerr << "initial in_buffer: " << std::hex << intprint(*in_buffer) << '\n'; });

		constexpr bool multi_src = (bits_to_copy + src_offset) > bits_per_src;
		constexpr bool multi_dst = (bits_to_copy + dst_offset) > bits_per_dst;

		if constexpr (multi_src || multi_dst)
		{
			while (bits_remaining)
			{
				if constexpr (multi_src)
				{
					if (in_buffer_bits <= 0)
					{
						in_buffer++;
						in_buffer_bits = bits_per_src;
						debug([&] { std::cerr << "in_buffer empty, refilling: " << std::hex << intprint(*in_buffer)
							<< " (" << in_buffer_bits << " bits)\n"; });
					}
				}
				if constexpr (multi_dst)
				{
					if (out_buffer_bits <= 0)
					{
						out_buffer++;
						out_buffer_bits = bits_per_dst;

						if constexpr (clear_mode == bit_clear_mode::clear_bytes)
							(*out_buffer) = {};
					}
				}

				copy_to_output();
			}
		}
	}

	template<typename TOut, size_t bits_to_read = sizeof(TOut) * detail::bits_hpp::BITS_PER_BYTE,
		unsigned src_offset = 0, typename TIn = void>
	constexpr TOut bit_read(const TIn* src)
	{
		TOut retVal{};
		bit_copy<bits_to_read, src_offset, 0, bit_clear_mode::none>(src, &retVal);
		return retVal;
	}
}
