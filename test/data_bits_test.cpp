#include "mh/data/bits.hpp"
#include "catch2/repo/single_include/catch2/catch.hpp"

template<unsigned bits_to_copy, unsigned src_offset, typename TSrc = void, typename TDst = void>
static void test_bit_functions(const TSrc* src, const TDst expected)
{
	CAPTURE(*src, expected, bits_to_copy, src_offset);

	const auto read = +mh::bit_read<TDst, bits_to_copy, src_offset>(src);

	constexpr TDst dst_max = std::numeric_limits<TDst>::max();
	constexpr TDst dst_mask = mh::BIT_MASKS<TDst>[bits_to_copy];
	CAPTURE(dst_mask);

	TDst copied = dst_max;
	mh::bit_copy<bits_to_copy, src_offset, 0, mh::bit_clear_mode::none>(&copied, src);
	REQUIRE(copied == dst_max);

	{
		constexpr auto clear_mode = mh::bit_clear_mode::clear_bits;
		CAPTURE(clear_mode);
		copied = dst_max;
		mh::bit_copy<bits_to_copy, src_offset, 0, clear_mode>(&copied, src);
		CAPTURE(copied);
		REQUIRE((copied & dst_mask) == expected);
		REQUIRE((copied & ~dst_mask) == ~dst_mask);
	}

	copied = dst_max;
	mh::bit_copy<bits_to_copy, src_offset, 0, mh::bit_clear_mode::clear_objects>(&copied, src);
	REQUIRE(copied == read);
	REQUIRE(read == expected);
	REQUIRE(copied == expected);

	copied = {};
	mh::bit_copy<bits_to_copy, src_offset, 0, mh::bit_clear_mode::none>(&copied, src);
	REQUIRE(copied == expected);
}

TEST_CASE("bit_read")
{
	SECTION("uint64_t source")
	{
		constexpr uint64_t src_value = 0xFEDCBA9876543210;

		test_bit_functions<16, 0>(&src_value, 0x3210U);
		test_bit_functions<16, 1>(&src_value, 0x1908U);
		test_bit_functions<16, 2>(&src_value, 0xC84U);
		test_bit_functions<16, 3>(&src_value, 0x8642U);
		test_bit_functions<16, 4>(&src_value, 0x4321U);
		test_bit_functions<16, 5>(&src_value, 0xA190U);
		test_bit_functions<16, 6>(&src_value, 0x50C8U);
		test_bit_functions<16, 7>(&src_value, 0xA864U);
		test_bit_functions<13, 7>(&src_value, 0x0864U);
	}

	SECTION("uint8_t source")
	{
		constexpr uint8_t src_value_raw[] = { 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe };
		const std::byte* src_value = reinterpret_cast<const std::byte*>(src_value_raw);

		test_bit_functions<16, 0>(src_value, 0x3210U);
		test_bit_functions<16, 1>(src_value, 0x1908U);
		test_bit_functions<16, 2>(src_value, 0xC84U);
		test_bit_functions<16, 3>(src_value, 0x8642U);
		test_bit_functions<16, 4>(src_value, 0x4321U);
		test_bit_functions<16, 5>(src_value, 0xA190U);
		test_bit_functions<16, 6>(src_value, 0x50C8U);
		test_bit_functions<16, 7>(src_value, 0xA864U);
		test_bit_functions<13, 7>(src_value, 0x0864U);
	}

	SECTION("uint8_t source/dest")
	{
		uint8_t src[] = { 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe };
		uint8_t dst[32];

		mh::bit_copy<17, 2, 1, mh::bit_clear_mode::clear_bits>(src, dst);
		//REQUIRE(dst[0] == )
	}
}
