#include "mh/math/interpolation.hpp"
#include "catch2/repo/single_include/catch2/catch.hpp"

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
