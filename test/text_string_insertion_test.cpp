#include "../cpp/text/string_insertion.hpp"
#include "catch2/repo/single_include/catch2/catch.hpp"

TEST_CASE("string insertion op", "[text][string_insertion]")
{
	std::string test;
	test << "Hello" << " world" << " !";
	REQUIRE(test == "Hello world !");
}
