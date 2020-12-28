#define _SILENCE_CXX20_CODECVT_FACETS_DEPRECATION_WARNING

#include <fstream>
#include <string>

int main(int argc, char** argv)
{
	std::basic_ifstream<char16_t> c16;
	std::basic_string<char16_t> s16;
	std::basic_ifstream<char32_t> c32;
	std::basic_string<char32_t> s32;
}
