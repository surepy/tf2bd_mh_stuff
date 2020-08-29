#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace mh
{
	namespace detail::reflection::enum_hpp
	{
		template<typename T, typename... TArgs> using all_defined = T;

		template<typename TSelf, typename type>
		class enum_type_base
		{
		public:
			static constexpr std::string_view type_name()
			{
				auto typeName = TSelf::type_name_full();
				auto last = typeName.rfind("::");
				if (last == typeName.npos)
					throw std::invalid_argument("Invalid type name");

				return typeName.substr(last + 2);
			}

			static constexpr std::string_view find_value_name(type value)
			{
				for (size_t i = 0; i < std::size(TSelf::VALUES); i++)
				{
					if (TSelf::VALUES[i].value() == value)
						return TSelf::VALUES[i].value_name();
				}

				return std::string_view{};
			}
		};
	}

	template<typename T> class enum_type;

	template<typename T>
	class enum_value
	{
	public:
		using value_type = T;
		using underlying_value_type = std::underlying_type_t<value_type>;

		constexpr enum_value(value_type value, const std::string_view& value_name) :
			m_Value(value),
			m_ValueName(value_name)
		{
		}

		static constexpr std::string_view type_name() { return enum_type<T>::type_name(); }
		constexpr std::string_view value_name() const { return m_ValueName; }

		constexpr value_type value() const { return m_Value; }
		constexpr underlying_value_type underlying_value() const { return static_cast<underlying_value_type>(value()); }

	private:
		value_type m_Value;
		std::string_view m_ValueName;
	};

#define MH_ENUM_REFLECT_BEGIN(enumType) \
	template<> \
	class ::mh::enum_type<enumType> final : \
		public ::mh::detail::reflection::enum_hpp::enum_type_base<::mh::enum_type<enumType>, enumType> \
	{ \
	public: \
		using type = enumType; \
		using underlying_type = std::underlying_type_t<type>; \
		\
		static constexpr std::string_view type_name_full() { return #enumType; } \
		\
		static constexpr ::mh::enum_value<type> VALUES[] = {

#define MH_ENUM_REFLECT_VALUE(value) ::mh::enum_value{ type::value, #value },

#define MH_ENUM_REFLECT_END() \
		}; \
	};
}

#if __has_include(<mh/text/format.hpp>)
#include <mh/text/format.hpp>

template<typename T>
struct mh::formatter<::mh::detail::reflection::enum_hpp::all_defined<T, typename ::mh::enum_type<T>::type>>
{
	static constexpr char PRES_TYPE_LONG = 'T';
	static constexpr char PRES_TYPE_SHORT = 't';
	static constexpr char PRES_VALUE = 'v';

	enum class TypePresentation
	{
		None,
		Short,
		Long,
	};

	TypePresentation m_Type = TypePresentation::Short;
	bool m_Value = true;

	constexpr auto parse(format_parse_context& ctx)
	{
		auto it = ctx.begin();
		const auto end = ctx.end();
		if (it != ctx.end())
		{
			if (*it != '}')
			{
				m_Type = TypePresentation::None;
				m_Value = false;
			}

			for (; it != end; ++it)
			{
				if (*it == PRES_TYPE_SHORT || *it == PRES_TYPE_LONG)
				{
					if (m_Type != TypePresentation::None)
						throw format_error("Type presentation was already set: 't' and 'T' are mututally exclusive");

					if (*it == PRES_TYPE_SHORT)
						m_Type = TypePresentation::Short;
					else
						m_Type = TypePresentation::Long;
				}
				else if (*it == PRES_VALUE)
					m_Value = true;
				else if (*it == '}')
					return it;
				else
					throw format_error("Unexpected character in formatting string");
			}
		}

		throw format_error("Unexpected end of format string when looking for '}'");
	}

	template<typename FormatContext>
	auto format(const T& rc, FormatContext& ctx)
	{
		const auto valueName = ::mh::enum_type<T>::find_value_name(rc);

		const auto get_format_string = [&]
		{
			using namespace std::string_view_literals;
			const bool withName = m_Value && !valueName.empty();

			if (m_Type == TypePresentation::None)
				return withName ? "{2}"sv : throw format_error("Invalid flags combination: result cannot be blank");
			else if (m_Type == TypePresentation::Short)
				return withName ? "{0}::{2}"sv : "{0}({3})"sv;
			else if (m_Type == TypePresentation::Long)
				return withName ? "{1}::{2}"sv : "{1}({3})"sv;
			else
				throw format_error("Invalid TypePresentation value");
		};

		return format_to(ctx.out(), get_format_string(),
			::mh::enum_type<T>::type_name(), ::mh::enum_type<T>::type_name_full(),
			valueName, +std::underlying_type_t<T>(rc));
	}
};

#endif
