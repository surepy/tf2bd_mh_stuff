#include <catch2/catch.hpp>

#include <mh/text/codecvt.hpp>

template<typename T>
static void RequireEqual(const std::basic_string_view<T>& a, const std::basic_string_view<T>& b)
{
	std::vector<int64_t> araw(a.begin(), a.end()), braw(b.begin(), b.end());
	CAPTURE(araw, braw);

	REQUIRE(araw == braw);
}

template<typename T1, typename T2>
static void CompareExpected(const std::basic_string_view<T1>& v1, const std::basic_string_view<T2>& v2)
{
	RequireEqual<T2>(mh::change_encoding<T2>(v1), v2);
	RequireEqual<T1>(v1, mh::change_encoding<T1>(v2));
}

#if MH_HAS_CHAR8
#define U8_SV(str) u8 ## str
#define U8_SV_REF const std::u8string_view&
#else
#define U8_SV(str) {}
#define U8_SV_REF const std::monostate&
#endif

#if MH_HAS_UNICODE
#define U16_SV(str) u ## str
#define U32_SV(str) U ## str
#define U16_SV_REF const std::u16string_view&
#define U32_SV_REF const std::u32string_view&
#else
#define U16_SV(str) {}
#define U32_SV(str)
#define U16_SV_REF const std::monostate&
#define U32_SV_REF const std::monostate&
#endif

static void CompareExpected3(U8_SV_REF v1, U16_SV_REF v2, U32_SV_REF v3)
{
#if MH_HAS_UNICODE
#if MH_HAS_CHAR8
	CompareExpected(v1, v2);

	CompareExpected(v1, v3);
#endif

	CompareExpected(v2, v3);
#endif
}

TEST_CASE("change_encoding fundamental", "[CharConverter]")
{
#define COMPARE_EXPECTED_3(str) CompareExpected3(U8_SV(str), U16_SV(str), U32_SV(str))

	COMPARE_EXPECTED_3("\U00010348");
	COMPARE_EXPECTED_3("\u0024");
	COMPARE_EXPECTED_3("\u00a2");
	COMPARE_EXPECTED_3("\u0939");
	COMPARE_EXPECTED_3("\u20ac");
	COMPARE_EXPECTED_3("\ud55c");
	COMPARE_EXPECTED_3("😐");
}

template<typename TConvertTo, typename TInput>
static void CompareRoundtrip(const std::basic_string_view<TInput>& val)
{
	const auto converted = mh::change_encoding<TConvertTo>(val);
	const auto convertedBack = mh::change_encoding<TInput>(converted);

	REQUIRE(convertedBack.size() == val.size());
	for (size_t i = 0; i < val.size(); i++)
	{
		CAPTURE(i);
		REQUIRE(((int64_t)convertedBack.at(i)) == ((int64_t)val.at(i)));
	}
}

template<typename T>
static void CompareStringsAll(const std::basic_string_view<T>& val)
{
#if MH_HAS_CHAR8
	CompareRoundtrip<char8_t>(val);
#endif

#if MH_HAS_UNICODE
	CompareRoundtrip<char16_t>(val);
	CompareRoundtrip<char32_t>(val);
#endif
}

TEST_CASE("change_encoding roundtrip", "[CharConverter]")
{
#if MH_HAS_CHAR8
	constexpr const std::u8string_view value_u8 = u8"😐";
	CompareStringsAll(value_u8);
#endif

#if MH_HAS_UNICODE
	constexpr const std::u16string_view value_u16 = u"😐";
	constexpr const std::u32string_view value_u32 = U"😐";
	CompareStringsAll(value_u16);
	CompareStringsAll(value_u32);
#endif
}
