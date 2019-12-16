#pragma once

#if __cpp_lib_bit_cast >= 201806
#include <bit>
#endif

#include <climits>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace mh
{
	template<unsigned MantissaBits_, unsigned ExponentBits_, bool SignBit_>
	class bit_float
	{
		using this_t = bit_float<MantissaBits_, ExponentBits_, SignBit_>;

		static constexpr unsigned DBL_MNT_BITS = 52;
		static constexpr unsigned DBL_EXP_BITS = 11;
		static constexpr unsigned FLT_MNT_BITS = 23;
		static constexpr unsigned FLT_EXP_BITS = 8;

	public:
		static constexpr auto MantissaBits = MantissaBits_;
		static constexpr auto ExponentBits = ExponentBits_;
		static constexpr auto SignBit = SignBit_;
		static constexpr unsigned MANTISSA_OFFSET = 0;
		static constexpr unsigned EXPONENT_OFFSET = MantissaBits;
		static constexpr unsigned SIGN_OFFSET = MantissaBits + ExponentBits;
		static constexpr unsigned TOTAL_BITS = MantissaBits + ExponentBits + (SignBit ? 1 : 0);

		static_assert(MantissaBits > 0, "Mantissa bits cannot be 0");
		static_assert(ExponentBits > 0, "Exponent bits cannot be 0");
		static_assert(MantissaBits <= DBL_MNT_BITS, "Mantissa bits cannot be greater than those of a double");
		static_assert(ExponentBits <= DBL_EXP_BITS, "Exponent bits cannot be greater than those of a double");

	private:
		template<unsigned bits>
		static constexpr auto uint_for_bits()
		{
			static_assert(bits <= 64);
			if constexpr (bits > 32)
				return uint64_t{};
			else if constexpr (bits > 16)
				return uint32_t{};
			else if constexpr (bits > 8)
				return uint16_t{};
			else
				return uint8_t{};
		}
		template<unsigned bits> using uint_for_bits_t = decltype(uint_for_bits<bits>());

		template<typename T>
		static constexpr T bits_to_mask(T bits) { return (T(1) << bits) - 1; }

		template<typename To, typename From>
		static constexpr To bit_cast(const From& from)
		{
#if __cpp_lib_bit_cast >= 201806
			return std::bit_cast<To>(from);
#else
			static_assert(sizeof(To) == sizeof(From));
			return *reinterpret_cast<const To*>(&from);
#endif
		}

	public:
		using bits_t = uint_for_bits_t<TOTAL_BITS>;
		using native_t = std::conditional_t<MantissaBits <= FLT_MNT_BITS && ExponentBits <= FLT_EXP_BITS, float, double>;
		using native_bitfloat_t = bit_float<
			MantissaBits <= FLT_MNT_BITS ? FLT_MNT_BITS : DBL_MNT_BITS,
			ExponentBits <= FLT_EXP_BITS ? FLT_EXP_BITS : DBL_EXP_BITS,
			true>;

		bit_float() = delete;

		static constexpr bits_t bits_to_mantissa(bits_t bits)
		{
			return (bits >> MANTISSA_OFFSET) & bits_to_mask<bits_t>(MantissaBits);
		}
		static constexpr bits_t bits_to_exponent(bits_t bits)
		{
			return (bits >> EXPONENT_OFFSET) & bits_to_mask<bits_t>(ExponentBits);
		}
		static constexpr bool bits_to_sign(bits_t bits)
		{
			if constexpr (SignBit)
				return bits & (bits_t(1) << SIGN_OFFSET);
			else
				return false;
		}

		static constexpr bool is_nan(bits_t bits)
		{
			return get_exponent(bits) == bits_to_mask(ExponentBits);
		}

		template<unsigned newBits> static constexpr auto mantissa_to_mantissa(bits_t oldMantissa)
		{
			using ret_t = uint_for_bits_t<newBits>;

			if constexpr (newBits == MantissaBits)
				return ret_t(oldMantissa);
			else if constexpr (newBits > MantissaBits)
				return ret_t(ret_t(oldMantissa) << (newBits - MantissaBits));
			else
				return ret_t(oldMantissa >> (MantissaBits - newBits));
		}

		template<unsigned newBits> static constexpr auto exponent_to_exponent(bits_t oldExponent)
		{
			using ret_t = uint_for_bits_t<newBits>;
			if (oldExponent == 0)
				return ret_t(0);

			constexpr int bits_exp_offset = ((1 << ExponentBits) / 2) - 1;
			constexpr int native_exp_offset = ((1 << newBits) / 2) - 1;

			int exponent_actual = oldExponent - bits_exp_offset;
			int exponent_native = exponent_actual + native_exp_offset;
			return ret_t(exponent_native & bits_to_mask<ret_t>(newBits));
		}

		template<typename new_bit_float>
		static constexpr auto bits_to_bits(bits_t bits)
		{
			using new_bits_t = typename new_bit_float::bits_t;
			new_bits_t newBits{};

			if constexpr (SignBit) // Do we have a sign bit?
			{
				const auto sign = bits_to_sign(bits);
				if constexpr (new_bit_float::SignBit)
				{
					// Set the sign bit in the new bits
					newBits |= (sign ? 1 : 0) << new_bit_float::SIGN_OFFSET;
				}
				else if (sign)
				{
					// We are negative, and the new bits don't have a sign bit.
					// Clamp to 0.
					return new_bits_t{};
				}
			}

			const auto mantissa = bits_to_mantissa(bits);
			const auto exponent = bits_to_exponent(bits);
			newBits |= exponent_to_exponent<new_bit_float::ExponentBits>(exponent) << new_bit_float::EXPONENT_OFFSET;
			newBits |= mantissa_to_mantissa<new_bit_float::MantissaBits>(mantissa);

			return newBits;
		}

		template<unsigned newMantissaBits, unsigned newExponentBits, bool newSignBit>
		static constexpr auto bits_to_bits(bits_t bits)
		{
			return bits_to_bits<bit_float<newMantissaBits, newExponentBits, newSignBit>>(bits);
		}

		static constexpr native_t bits_to_native(bits_t bits)
		{
			static_assert(std::numeric_limits<native_t>::is_iec559);
			return bit_cast<native_t>(bits_to_bits<native_bitfloat_t>(bits));
		}

		static constexpr bits_t native_to_bits(native_t native)
		{
			static_assert(std::numeric_limits<native_t>::is_iec559);
			return native_bitfloat_t::template bits_to_bits<this_t>(bit_cast<typename native_bitfloat_t::bits_t>(native));
		}
	};

	using half_float = mh::bit_float<10, 5, true>;
	using native_float = mh::bit_float<23, 8, true>;
	using native_double = mh::bit_float<52, 11, true>;
}
