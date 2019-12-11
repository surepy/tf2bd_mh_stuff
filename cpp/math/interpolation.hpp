#pragma once

#include <algorithm>
#include <type_traits>

namespace mh
{
	template<typename T1, typename T2, typename T3>
	inline constexpr auto lerp(const T1& in, const T2& min, const T3& max)
	{
		return min + (max - min) * in;
	}
	template<typename T1, typename T2, typename T3>
	inline constexpr auto lerp_clamped(const T1& in, const T2& min, const T3& max)
	{
		return std::clamp<std::common_type_t<T1, T2, T3>>(mh::lerp(in, min, max), min, max);
	}

	template<typename T1, typename T2, typename T3>
	inline constexpr auto lerp_slow(const T1& in, const T2& min, const T3& max)
	{
		return min * (1 - in) + max * in;
	}
	template<typename T1, typename T2, typename T3>
	inline constexpr auto lerp_slow_clamped(const T1& in, const T2& min, const T3& max)
	{
		return std::clamp<std::common_type_t<T1, T2, T3>>(mh::lerp(in, min, max), min, max);
	}

	template<typename T1, typename T2, typename T3>
	inline constexpr auto remap_to_01(const T1& in, const T2& in_min, const T3& in_max)
	{
		return (in - in_min) / (in_max - in_min);
	}
	template<typename T1, typename T2, typename T3>
	inline constexpr auto remap_to_01_clamped(const T1& in, const T2& in_min, const T3& in_max)
	{
		return std::clamp<std::common_type_t<T1, T2, T3>>(
			mh::remap_to_01(in, in_min, in_max), 0, 1);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	inline constexpr auto remap(const T1& in, const T2& in_min, const T3& in_max, const T4& out_min, const T5& out_max)
	{
		return mh::lerp(mh::remap_to_01(in, in_min, in_max), out_min, out_max);
	}
	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	inline constexpr auto remap_clamped(const T1& in, const T2& in_min, const T3& in_max, const T4& out_min, const T5& out_max)
	{
		return std::clamp<std::common_type_t<T1, T2, T3, T4, T5>>(
			mh::remap(in, in_min, in_max, out_min, out_max), out_min, out_max);
	}
}
