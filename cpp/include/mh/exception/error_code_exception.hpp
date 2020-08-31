#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace mh
{
	namespace detail::error_code_exception
	{
		template<typename TBase, typename TError>
		class basic_error_code_exception : public TBase
		{
		public:
			using base_type = TBase;
			using code_type = TError;

		private:
			static std::string format_message(const code_type& code, const std::string_view& message)
			{
				std::string retVal = "(" + std::to_string(+code.value()) + ") " + code.message();

				if (!message.empty())
					retVal += ": " + message;

				return retVal;
			}

		public:
			basic_error_code_exception(code_type code, const std::string_view& message = {}) :
				base_type(format_message(code, message)),
				m_Code(std::move(code))
			{
			}

			const code_type& code() const { return m_Code; }

		private:
			code_type m_Code;
		};
	}

	template<typename TBase, typename TError = std::error_code> using basic_error_code_exception =
		detail::error_code_exception::basic_error_code_exception<TBase, std::error_code>;

	template<typename TBase> using basic_error_condition_exception =
		basic_error_code_exception<TBase, std::error_condition>;

	using error_code_exception = basic_error_code_exception<std::runtime_error>;
	using error_condition_exception = basic_error_condition_exception<std::runtime_error>;
}
