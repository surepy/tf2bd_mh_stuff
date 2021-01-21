#pragma once

#include <exception>
#include <string>
#include <typeinfo>

namespace mh
{
	struct exception_details;

	class exception_details_handler
	{
	public:
		virtual ~exception_details_handler() = default;

		virtual bool try_handle(const std::exception_ptr& e, exception_details& details) const noexcept = 0;
	};

	struct exception_details
	{
		exception_details() noexcept = default;
		MH_STUFF_API exception_details(const std::exception_ptr& e);

		const std::type_info* m_Type = nullptr;
		std::string m_Message;

		MH_STUFF_API const char* type_name() const noexcept;

		// When this goes out of scope, the associated handler is removed.
		struct MH_STUFF_API [[nodiscard]] handler final
		{
			handler() = default;
			~handler();

			handler(const handler&) = delete;
			handler& operator=(const handler&) = delete;

			handler(handler&& other) noexcept;
			handler& operator=(handler&& other) noexcept;

		private:
			friend struct exception_details;
			explicit handler(const std::type_info* type) noexcept;
			void release();

			const std::type_info* m_Type = nullptr;
		};

		// Adds a new handler. Returns false if there is already a handler for this type.
		MH_STUFF_API static handler add_handler(const std::type_info& type, const exception_details_handler& handler);
	};
}

#ifndef MH_COMPILE_LIBRARY
#include "exception_details.inl"
#endif
