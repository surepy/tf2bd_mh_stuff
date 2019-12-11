#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#include "../cpp/math/interpolation.hpp"
#include "../cpp/memory/unique_object.hpp"
#ifdef __cpp_lib_to_chars
#include "../cpp/text/charconv_helper.hpp"
#endif
#include "../cpp/text/memstream.hpp"
#include "../cpp/text/string_insertion.hpp"

#include "catch2/repo/single_include/catch2/catch.hpp"

#include <iostream>
#include <string.h>

TEST_CASE("lerp", "[math][interpolation]")
{
	REQUIRE(mh::lerp(0, 0, 0) == 0);
	REQUIRE(mh::lerp(0, 0, 1) == 0);
	REQUIRE(mh::lerp(0, 1, 1) == 1);
	REQUIRE(mh::lerp(1, 1, 1) == 1);
	REQUIRE(mh::lerp(0.5f, 1, 1) == 1);
	REQUIRE(mh::lerp(0.5f, -49, 1) == Approx(-24));

	REQUIRE(mh::lerp(0.5f, -100000, -10000) == Approx(-55000));

	// Test upper bound
	REQUIRE(mh::lerp(1.1, 0, 10) == Approx(11));
	REQUIRE(mh::lerp_clamped(1.1, 0, 10) == Approx(10));

	// Test lower bound
	REQUIRE(mh::lerp(-1.1, 0, 10) == Approx(-11));
	REQUIRE(mh::lerp_clamped(-1.1, 0, 10) == Approx(0));
}

TEST_CASE("lerp_slow", "[math][interpolation]")
{
	REQUIRE(mh::lerp_slow(0.5f, std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::max()) == Approx(0));
	REQUIRE(mh::lerp_slow(0.5f, std::numeric_limits<double>::lowest(),
		std::numeric_limits<double>::max()) == Approx(0));
	REQUIRE(mh::lerp_slow(0.5f, std::numeric_limits<long double>::lowest(),
		std::numeric_limits<long double>::max()) == Approx(0));

	for (int i = 0; i < 1000; i++)
	{
		const auto t = i * ((i % 2) * 2 - 1) * 0.01f * (1.0f / 3);
		const auto min = -i;
		const auto max = i;
		CAPTURE(t, min, max);

		REQUIRE(mh::lerp(t, min, max) == Approx(mh::lerp_slow(t, min, max)));
		REQUIRE(mh::lerp_clamped(t, min, max) == Approx(mh::lerp_slow_clamped(t, min, max)));
	}
}

#ifdef __cpp_lib_to_chars
TEST_CASE("charconv helpers", "[text][charconv_helper]")
{
	std::string test;
	test << "Hello" << " world" << " !";
	REQUIRE(test == "Hello world !");
}
#endif

TEST_CASE("string insertion op", "[text][string_insertion]")
{
	std::string test;
	test << "Hello" << " world" << " !";
	REQUIRE(test == "Hello world !");
}

TEST_CASE("memstream put", "[text][memstream]")
{
	char buf[128];
	mh::memstream ms(buf);

	constexpr std::string_view TEST_STRING = "my test string";
	ms << TEST_STRING;

	REQUIRE_THAT(buf, Catch::Equals(TEST_STRING.data()));
	REQUIRE(ms.view() == TEST_STRING);
	CHECK(!ms.fail());
	CHECK(ms.good());
	REQUIRE(ms.tellp() == 14);
	REQUIRE(ms.tellg() == 0);
	REQUIRE(ms.good());

	ms.seekp(7);
	ms << " foo";
	REQUIRE(ms.view() == "my test foo");
	REQUIRE_THAT(buf, Catch::Equals("my test foo"));

	{
		REQUIRE(ms.seekg(2));
		std::string testWord;
		REQUIRE(ms >> testWord);
		REQUIRE(ms.good());
		REQUIRE(testWord == "test");

		REQUIRE(ms >> testWord);
		REQUIRE(ms.eof());
		REQUIRE(testWord == "foo");

		ms.clear(ms.rdstate() & ~std::ios_base::eofbit);
		REQUIRE(ms.good());

		REQUIRE(ms.seekp(0));
		REQUIRE(ms.good());

		REQUIRE(ms.write("foo", 3));
		REQUIRE(ms.good());
		REQUIRE(ms.view_full() == "foo");
		REQUIRE(ms.view() == "");
		REQUIRE(ms.seekg(1));
		REQUIRE(ms.view() == "oo");

		REQUIRE(ms.seekg(0));
		REQUIRE(ms.good());
		REQUIRE(ms.view() == "foo");

		REQUIRE(ms << "bar");
		REQUIRE(ms.view() == "foobar");
		REQUIRE(ms.good());
	}

	{
		constexpr int TEST_INT_VALUE = 487;

		ms.seekp(0);
		REQUIRE(ms.tellp() == 0);
		ms << TEST_INT_VALUE;
		REQUIRE(ms.view() == "487");

		int testInt;
		ms.seekg(0);
		ms >> testInt;
		REQUIRE(testInt == TEST_INT_VALUE);
	}
}
