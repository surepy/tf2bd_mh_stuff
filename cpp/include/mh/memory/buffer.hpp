#pragma once

#ifdef __cpp_impl_three_way_comparison
#include <compare>
#endif
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace mh
{
	class buffer final
	{
	public:
		buffer() = default;
		buffer(buffer&& other) :
			m_Data(std::move(other.m_Data)),
			m_Size(other.m_Size)
		{
			other.m_Size = 0;
		}
		explicit buffer(const buffer& other) :
			buffer(other.data(), other.size())
		{
		}
		explicit buffer(size_t initialSize)
		{
			resize(initialSize);
		}
		buffer(const std::byte* ptr, size_t bytes) :
			buffer(bytes)
		{
			std::memcpy(data(), ptr, bytes);
		}

		void resize(size_t newSize)
		{
			const auto newPtr = std::realloc(m_Data.get(), newSize);
			if (!newPtr)
				throw std::runtime_error("Failed to realloc");

			m_Data.release();
			m_Data.reset(static_cast<std::byte*>(newPtr));
			m_Size = newSize;
		}

		bool reserve(size_t minSize)
		{
			if (size() < minSize)
			{
				resize(minSize + minSize / 2);
				return true;
			}

			return false;
		}

#if __cpp_impl_three_way_comparison >= 201907
		std::strong_ordering operator<=>(const buffer& other) const
		{
			if (auto result = m_Size <=> other.m_Size; std::is_neq(result))
				return result;

			const auto result = memcmp(data(), other.data(), size());
			if (result < 0)
				return std::strong_ordering::less;
			else if (result > 0)
				return std::strong_ordering::greater;
			else
				return std::strong_ordering::equal;
		}
#endif

		void clear()
		{
			m_Data.reset();
			m_Size = 0;
		}
		size_t size() const { return m_Size; }

		std::byte* data() { return m_Data.get(); }
		const std::byte* data() const { return m_Data.get(); }

	private:
		struct free_deleter final { void operator()(std::byte* p) { free(p); } };

		std::unique_ptr<std::byte, free_deleter> m_Data;
		size_t m_Size = 0;
	};
}
