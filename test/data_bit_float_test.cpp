#include "../cpp/data/bit_float.hpp"
#include "catch2/repo/single_include/catch2/catch.hpp"

using half_float = mh::half_float;
using native_float = mh::native_float;
using gl_float14 = mh::bit_float<9, 5, false>;
using gl_float11 = mh::bit_float<6, 5, false>;
using gl_float10 = mh::bit_float<5, 5, false>;

TEST_CASE("bit_float - half")
{
	REQUIRE(half_float::mantissa_to_mantissa<23>(0) == 0);
	REQUIRE(half_float::exponent_to_exponent<8>(0) == 0);

	REQUIRE(half_float::bits_to_sign(half_float::native_to_bits(-306)) == true);
	REQUIRE(half_float::bits_to_sign(half_float::native_to_bits(306)) == false);

	{
		uint32_t bits = 0;
		const float native = half_float::bits_to_native(bits);

		uint32_t native_bits = *reinterpret_cast<const uint32_t*>(&native);
		REQUIRE(native_bits == 0);

		REQUIRE(half_float::bits_to_exponent(bits) == 0);
		REQUIRE(half_float::bits_to_mantissa(bits) == 0);
		REQUIRE(half_float::bits_to_sign(bits) == false);
		REQUIRE(native == 0);
	}

	const auto CheckValue = [](uintmax_t bits, double exp_value, uintmax_t exp_mantissa, uintmax_t exp_exponent)
	{
		REQUIRE(half_float::bits_to_exponent(bits) == exp_exponent);
		REQUIRE(half_float::bits_to_mantissa(bits) == exp_mantissa);
		REQUIRE(half_float::bits_to_native(bits) == Approx(exp_value));
	};

	const auto CheckRoundTrip = [](float value)
	{
		const auto native_bits = native_float::native_to_bits(value);
		const auto native_mantissa = native_float::bits_to_mantissa(native_bits);
		const auto native_exponent = native_float::bits_to_exponent(native_bits);

		REQUIRE((native_mantissa & ((1 << (23 - 11)) - 1)) == 0);
		REQUIRE((native_exponent & ((1 << (8 - 5)) - 1)) == 0);
	};

	{
		const float value = 982.0f;
		const auto bits = native_float::native_to_bits(value);
		REQUIRE(bits == *reinterpret_cast<const uint32_t*>(&value));
		REQUIRE(half_float::exponent_to_exponent<native_float::ExponentBits>(0b11000) == 0b10001000);
		REQUIRE(native_float::exponent_to_exponent<half_float::ExponentBits>(0b10001000) == 0b11000);
		REQUIRE(half_float::mantissa_to_mantissa<native_float::MantissaBits>(0b1110101100) == 0b11101011000000000000000);
		REQUIRE(native_float::mantissa_to_mantissa<half_float::MantissaBits>(0b11101011000000000000000) == 0b1110101100);

		REQUIRE(native_float::bits_to_exponent(bits) == 0b10001000);
		REQUIRE(native_float::bits_to_mantissa(bits) == 0b11101011000000000000000);
		REQUIRE(native_float::bits_to_sign(bits) == false);
		REQUIRE(native_float::bits_to_native(bits) == value);
	}

	REQUIRE(half_float::bits_to_native(0b0110100000000000) == Approx(2048));
	REQUIRE(half_float::native_to_bits(2048) == 0b0110100000000000);
	REQUIRE(half_float::bits_to_native(0b0110001110101100) == Approx(982));
	REQUIRE(half_float::native_to_bits(982) == 0b0110001110101100);
	REQUIRE(half_float::bits_to_native(0b1101101000001011) == Approx(-193.4).epsilon(0.05f));

	{
		const auto bits = 0b1001010100000000;
		REQUIRE(half_float::bits_to_exponent(bits) == 5);
		REQUIRE(half_float::bits_to_mantissa(bits) == 0b0100000000);
		REQUIRE(half_float::bits_to_native(bits) == Approx(-0.0012207031));
	}

	REQUIRE(half_float::bits_to_native(0b0101110010011001) == Approx(294.25));
	REQUIRE(half_float::native_to_bits(294.25) == 0b0101110010011001);
}

TEST_CASE("bit_float - gl11")
{
	REQUIRE(gl_float11::bits_to_native(0) == 0);
	REQUIRE(gl_float11::native_to_bits(0) == 0);
	REQUIRE(gl_float11::native_to_bits(-1) == 0);

	{
		const double value = 0.1;
		const auto bits = half_float::native_to_bits(value);
		const auto rt_value = half_float::bits_to_native(bits);
		REQUIRE(rt_value == Approx(value).epsilon(0.001));
	}
}
