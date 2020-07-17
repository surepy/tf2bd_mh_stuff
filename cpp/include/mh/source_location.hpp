#pragma once

#include <cstdint>

#if __has_include(<source_location>)
#include <source_location>
#endif

namespace mh
{
#if __cpp_lib_source_location >= 201907

	using source_location = std::source_location;
#define MH_SOURCE_LOCATION_CURRENT() ::mh::source_location::current()

#else

	class source_location final
	{
	public:
		constexpr source_location() noexcept = default;
		constexpr source_location(std::uint_least32_t line, const char* fileName, const char* functionName) :
			m_Line(line), m_FileName(fileName), m_FunctionName(functionName)
		{
		}

		constexpr std::uint_least32_t line() const noexcept { return m_Line; }
		constexpr std::uint_least32_t column() const noexcept { return 0; } // unknown
		constexpr const char* file_name() const noexcept { return m_FileName; }
		constexpr const char* function_name() const noexcept { return m_FunctionName; }

	private:
		std::uint_least32_t m_Line{};
		const char* m_FileName = nullptr;
		const char* m_FunctionName = nullptr;
	};

#define MH_SOURCE_LOCATION_CURRENT() ::mh::source_location(__LINE__, __FILE__, __func__)

#endif
}
